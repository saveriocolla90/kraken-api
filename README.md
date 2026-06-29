# kraken-api

A C++ client for the [Kraken](https://www.kraken.com/) REST API, supporting both
synchronous and asynchronous calls.

This project is a port of [niXman/binapi](https://github.com/niXman/binapi) (a
Binance client) to Kraken: the core architecture — the `pimpl` `api` class, the
Boost.Beast/Asio + OpenSSL transport, the asynchronous invoker, the `flatjson`
parser and the `result<T>` model — is reused unchanged. The REST endpoints,
request signing and response types have been reworked for Kraken.

# Implementation details
Written in C++14 using [boost](https://www.boost.org/) (>= 1.70) and OpenSSL.
[boost.beast](https://www.boost.org/doc/libs/release/libs/beast/index.html) is
used for the HTTPS transport (no `curl` dependency), and OpenSSL provides the
HMAC/SHA/base64 primitives used for request signing.

## Authentication
Public endpoints are unauthenticated `GET`s. Private endpoints are signed `POST`s
following Kraken's scheme:

```
API-Sign = base64( HMAC_SHA512( base64decode(secret),
                                URI_path + SHA256(nonce + POST_body) ) )
```

The signature is sent in the `API-Sign` header alongside the `API-Key` header, with
a monotonically increasing `nonce` in the request body. The `sk` constructor
argument is your **base64-encoded private key** (as shown in the Kraken UI).

# REST API
## Public (market data)
- [Server time](https://docs.kraken.com/rest/#tag/Market-Data/operation/getServerTime) -> `api::server_time()`
- [System status](https://docs.kraken.com/rest/#tag/Market-Data/operation/getSystemStatus) -> `api::system_status()`
- [Ticker](https://docs.kraken.com/rest/#tag/Market-Data/operation/getTickerInformation) -> `api::ticker(pair)` / `api::tickers()`
- [Asset pairs](https://docs.kraken.com/rest/#tag/Market-Data/operation/getTradableAssetPairs) -> `api::asset_pairs()` / `api::asset_pairs(pair)`
- [Order book](https://docs.kraken.com/rest/#tag/Market-Data/operation/getOrderBook) -> `api::order_book(pair, count)`
- [OHLC / candles](https://docs.kraken.com/rest/#tag/Market-Data/operation/getOHLCData) -> `api::ohlc(pair, interval, since)`
- [Recent trades](https://docs.kraken.com/rest/#tag/Market-Data/operation/getRecentTrades) -> `api::recent_trades(pair, since)`

## Private (account & trading)
- [Account balance](https://docs.kraken.com/rest/#tag/Account-Data/operation/getAccountBalance) -> `api::balances()`
- [Open orders](https://docs.kraken.com/rest/#tag/Account-Data/operation/getOpenOrders) -> `api::open_orders()`
- [Closed orders](https://docs.kraken.com/rest/#tag/Account-Data/operation/getClosedOrders) -> `api::closed_orders()`
- [Query orders](https://docs.kraken.com/rest/#tag/Account-Data/operation/getOrdersInfo) -> `api::query_orders(txid)`
- [Trade history](https://docs.kraken.com/rest/#tag/Account-Data/operation/getTradeHistory) -> `api::trades_history()`
- [Query trades](https://docs.kraken.com/rest/#tag/Account-Data/operation/getTradesInfo) -> `api::query_trades(txid)`
- [Open positions](https://docs.kraken.com/rest/#tag/Account-Data/operation/getOpenPositions) -> `api::open_positions(docalcs)`
- [Add order](https://docs.kraken.com/rest/#tag/Trading/operation/addOrder) -> `api::add_order(pair, type, ordertype, volume, price, validate)`
- [Cancel order](https://docs.kraken.com/rest/#tag/Trading/operation/cancelOrder) -> `api::cancel_order(txid)`
- [Cancel all orders](https://docs.kraken.com/rest/#tag/Trading/operation/cancelAllOrders) -> `api::cancel_all()`
- [Cancel all orders after](https://docs.kraken.com/rest/#tag/Trading/operation/cancelAllOrdersAfter) -> `api::cancel_all_after(timeout)`

`add_order(...)` accepts `validate = true` to validate an order without placing it.
`query_orders(txid)` / `query_trades(txid)` accept a comma-separated list of ids.
`cancel_all_after(timeout)` is a dead-man's switch (seconds; `0` disables it).

# Building
```sh
cmake -S . -B build
cmake --build build -j
```
The shared library is the default target. To also build the bundled CLI demo
(`main.cpp`):
```sh
cmake -S . -B build -DKRAPI_BUILD_EXAMPLE=ON
cmake --build build -j
```
See `examples/kraken-balance/` for a standalone example.

# Testing
Tests live under `tests/` and are off by default. Enable them with
`-DKRAPI_BUILD_TESTS=ON`; [doctest](https://github.com/doctest/doctest) is
fetched automatically.
```sh
cmake -S . -B build -DKRAPI_BUILD_TESTS=ON
cmake --build build -j
```
There are two suites, registered with CTest:

- **`offline`** — parser tests that feed captured Kraken payloads
  (`tests/fixtures/*.json`) into each `T::construct()` and assert the parsed
  fields. No network or credentials. This is the default CI net.
  ```sh
  ctest --test-dir build -LE live --output-on-failure
  ```
- **`live`** — integration tests against `api.kraken.com`. Public endpoints run
  whenever the network is reachable (and are skipped, not failed, otherwise).
  Private endpoints run only when credentials are present and are **read-only or
  validate-only** — they never place or cancel a real order.
  ```sh
  ctest --test-dir build -L live --output-on-failure
  # or run the binary directly:
  ./build/tests/krapi_tests_live
  ```

To exercise the private live tests, copy `tests/.env.example` to `tests/.env`
(gitignored) and fill in your key/secret, or export `KRAKEN_API_KEY` /
`KRAKEN_API_SECRET` in your shell. A read-only API key is sufficient.

# Install and consume in your project
```sh
cmake --install build
```
Find the package in your CMakeLists.txt
```
find_package(krapi CONFIG REQUIRED)
target_link_libraries(Main PRIVATE krapi::krapi)
```

# Synchronous example
```cpp
#include <krapi/api.hpp>

#include <boost/asio/io_context.hpp>

#include <iostream>

int main() {
    const std::string pk = "...";  // API key
    const std::string sk = "...";  // base64 private key

    boost::asio::io_context ioctx;
    krapi::rest::api api(
         ioctx
        ,"api.kraken.com"
        ,"443"
        ,pk
        ,sk
        ,10000 // timeout
    );

    // public
    auto tick = api.ticker("XBTUSD");
    if ( !tick ) {
        std::cerr << "ticker error: " << tick.errmsg << std::endl;
        return EXIT_FAILURE;
    }
    for ( const auto &kv : tick.v.tickers ) {
        std::cout << kv.first << " last=" << kv.second.last << std::endl;
    }

    // private
    auto balances = api.balances();
    if ( !balances ) {
        std::cerr << "balances error: " << balances.errmsg << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "balances: " << balances.v << std::endl;

    return EXIT_SUCCESS;
}
```

# Asynchronous example
Passing a callback makes the call asynchronous; drive it with `ioctx.run()`.
```cpp
#include <krapi/api.hpp>

#include <boost/asio/io_context.hpp>

#include <iostream>

int main() {
    const std::string pk = "...";  // API key
    const std::string sk = "...";  // base64 private key

    boost::asio::io_context ioctx;
    krapi::rest::api api(ioctx, "api.kraken.com", "443", pk, sk, 10000);

    api.balances(
        [](const char *fl, int ec, std::string errmsg, krapi::rest::balances_t res) {
            if ( ec ) {
                std::cerr << "balances error: fl=" << fl
                          << ", ec=" << ec << ", emsg=" << errmsg << std::endl;
                return false;
            }

            std::cout << "balances: " << res << std::endl;

            return true;
        }
    );

    ioctx.run();

    return EXIT_SUCCESS;
}
```

# WebSocket API
The WebSocket layer (`krapi::ws`) is still the upstream **Binance** implementation
and has **not** been ported to Kraken yet. It builds, but it targets Binance stream
endpoints and message formats. Use the REST API above for Kraken.

# License
Apache License 2.0. Original work Copyright (c) niXman (binapi); see `LICENSE`.
