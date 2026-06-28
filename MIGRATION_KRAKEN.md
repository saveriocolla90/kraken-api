# Binance → Kraken REST migration plan (in-place rewrite)

Goal: repoint this library (niXman `binapi`) at the **Kraken REST API**, keeping the
core architecture (pimpl `api` class, Boost.Beast/Asio + OpenSSL transport, async
invoker, `flatjson` parsing, `result<T>` model) and reusing as much code as possible.

Scope of this document: **REST only.** WebSockets (`websocket.cpp/.hpp`) are a separate
protocol on Kraken and are treated as a follow-up, not part of this plan.

> Note: Kraken docs show `curl` examples, but `curl` is only their illustration of an
> HTTPS call. The existing Boost.Beast transport already does the same thing — **no
> cURL dependency is introduced.**

---

## 1. Architecture: what stays vs. what changes

| Layer | File(s) | Action |
|---|---|---|
| TCP/TLS transport, async chain, request queue | `src/api.cpp` `sync_post`/`async_post`/`on_*` | **Keep.** Only host + headers + signing branch change. |
| Async invoker / callbacks | `include/krapi/invoker.hpp` | **Keep as-is.** |
| `result<T>` + `post()` dispatch | `src/api.cpp` `impl::post` | **Keep structure**, rewrite signing + error-detection calls inside it. |
| JSON parser | `include/krapi/flatjson.hpp` | **Keep as-is.** |
| Helpers: `double_type`, `tools`, `dtf`, `fnv1a`, `iofmt` | various | **Keep as-is.** |
| Build | `CMakeLists.txt` | **Keep**; no new deps (OpenSSL already linked). |
| Public endpoint methods | `include/krapi/api.hpp`, `src/api.cpp` | **Rewrite** paths/params/verbs; reshape method set to Kraken. |
| Auth / signing | `src/api.cpp` `impl::post` + `sync_post`/`async_post` headers | **Rewrite** (Binance HMAC-SHA256 → Kraken HMAC-SHA512 scheme). |
| Error detection | `src/errors.cpp`, `include/krapi/errors.hpp` | **Rewrite** for Kraken `{"error":[...]}`. |
| Response types | `include/krapi/types.hpp`, `src/types.cpp` (2292 lines) | **Rewrite** the `construct()` bodies + struct fields. Bulk of the labor. |
| Enums | `enums.cpp/.hpp` | **Adapt** order side/type/timeinforce strings to Kraken. |
| Reports / pairslist | `reports.cpp`, `pairslist.cpp` | **Adapt** after types settle (depend on type field names). |
| Examples + `main.cpp` | `examples/`, `main.cpp` | **Update** host/port + call sites last. |

---

## 2. The signing change (the critical piece)

### Binance (current — `src/api.cpp:295-317`, `:427`, `:488`)
- Build query string of params.
- Append `&timestamp=<ms>&recvWindow=<n>`.
- `signature = hex( HMAC_SHA256(secret, querystring) )`, appended as `&signature=`.
- Header `X-MBX-APIKEY: <public key>`.
- GET/DELETE put params in the URL; others in the body.

### Kraken (target)
- **Public** endpoints: `GET /0/public/<Method>`, no auth. (Route these with `_signed=false`.)
- **Private** endpoints: **`POST /0/private/<Method>`**, body is form-urlencoded and
  **must contain a `nonce`** (always-increasing integer; use µs or ms epoch).
- Signature algorithm:
  ```
  postdata = urlencode(params including nonce)
  message  = path_bytes  ++  SHA256( nonce_ascii ++ postdata )
  API-Sign = base64( HMAC_SHA512( base64decode(api_secret), message ) )
  ```
- Headers: `API-Key: <public key>`, `API-Sign: <signature>`.

### Concrete edits
1. Add helpers next to the existing `hmac_sha256`/`b2a_hex` in `src/api.cpp`:
   - `sha256_raw(data) -> std::string` (32 raw bytes) — OpenSSL `SHA256`.
   - `hmac_sha512_raw(key, data) -> std::string` — `HMAC(EVP_sha512(), ...)`.
   - `base64_encode` / `base64_decode` — OpenSSL `EVP_EncodeBlock`/`EVP_DecodeBlock`
     (already used in `rsa_sha256`).
   - `get_nonce()` — monotonic counter seeded from epoch.
2. Rewrite the `if (_signed)` block in `impl::post`:
   - inject `nonce` into `data`,
   - compute `API-Sign` per above (needs the request **path**, so pass `target`/method
     into the signing step — both are already in scope in `post()`),
   - **stop** appending `&signature=` / `timestamp` / `recvWindow`.
3. Force private calls to **POST with body** (drop the GET-puts-params-in-URL path for
   signed requests); public calls stay GET.
4. In `sync_post` **and** `async_post`, replace the `X-MBX-APIKEY` header with
   `API-Key` + `API-Sign`. The Beast `content_type` (`application/x-www-form-urlencoded`)
   already matches Kraken.

> The signing function is small but easy to get subtly wrong (byte concatenation, base64
> of the *raw* digest, secret is base64-*decoded* before use). Recommend a standalone
> unit test against Kraken's published example vector before wiring it in.

---

## 3. Error detection (`src/errors.cpp`)

Kraken responses are `{"error":[...], "result":{...}}`. Replace:
- `is_api_error`: currently checks `code`+`msg`; change to "`error` array is non-empty".
- `construct_error`: pull the first string from the `error` array (e.g.
  `"EAPI:Invalid key"`), map the `EClass:EType` prefix to your `e_error` enum (or keep a
  generic code + pass the string through). Update `enums.hpp` `e_error` accordingly.

This is called from `impl::post` at `src/api.cpp:346-352` — the call site stays, only the
two functions change.

## 4. Endpoint + type remap

For each method, change path/verb/params in `src/api.cpp` and reshape the matching
`*_t` struct + `construct()` in `types.cpp`. Suggested Kraken mapping:

| Current method | Kraken endpoint |
|---|---|
| `ping` / `server_time` | `public/Time` |
| `exchange_info` | `public/AssetPairs` (+ `public/Assets`) |
| `price` / `prices` / `_24hrs_ticker` | `public/Ticker` |
| `depths` | `public/Depth` |
| `trades` / `agg_trades` | `public/Trades` |
| `klines` | `public/OHLC` |
| `account_info` (balances) | `private/Balance` / `private/TradeBalance` |
| `open_orders` | `private/OpenOrders` |
| `all_orders` / `order_info` | `private/ClosedOrders` / `private/QueryOrders` |
| `new_order` | `private/AddOrder` |
| `cancel_order` | `private/CancelOrder` |
| `cancel_all_open_orders` | `private/CancelAll` |
| `my_trades` | `private/TradesHistory` |
| `*_user_data_stream` (listenKey) | **Remove** — Kraken uses `private/GetWebSocketsToken` instead. |

Param renames are pervasive: `symbol`→`pair`, order fields differ (`AddOrder` uses
`ordertype`, `type` (buy/sell), `volume`, `price`, `pair`). The `init_list_type`
param-map mechanism in `post()` is reused unchanged — only the keys/values differ.

## 5. Suggested execution order

1. **Signing + base64/sha helpers** in `api.cpp` (+ standalone test vector). ← do first
2. **Error detection** rewrite (`errors.cpp`).
3. **Transport headers** (`sync_post`/`async_post`): `API-Key`/`API-Sign`.
4. **Proof of concept**: wire `private/Balance` end-to-end, confirm a real authenticated
   round-trip works. This validates 1–3 before touching the rest.
5. Public endpoints (`Time`, `Ticker`, `AssetPairs`, `Depth`, `OHLC`) + their types.
6. Remaining private endpoints (orders, trades) + their types + enums.
7. `reports.cpp` / `pairslist.cpp`, then `main.cpp`/examples (host `api.kraken.com:443`).
8. README + naming pass (optional rename of `binapi`/`X-MBX` remnants).

## 6. Construction / call-site changes
`main.cpp:66-69` and examples instantiate with `"api.binance.com","443"` — change host to
`api.kraken.com` (port 443). The `pk`/`sk` constructor args carry over (now Kraken
API key + base64 private key). `recvWindow`/`timeout` semantics no longer apply to
signing but the ctor param can stay (unused or repurposed).
