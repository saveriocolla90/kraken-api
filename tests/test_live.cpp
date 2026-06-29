
// ----------------------------------------------------------------------------
// Live integration tests (opt-in).
//
// These hit the real Kraken REST API (api.kraken.com) and assert that the parsed
// responses are sane end-to-end (signing + transport + parse).
//
//  * Public endpoints always run. If a call fails with a transport/network error
//    the case is skipped (reported, not failed) so the suite is usable offline.
//  * Private endpoints run only when KRAKEN_API_KEY / KRAKEN_API_SECRET are set
//    (via the environment or tests/.env). They are read-only or validate-only —
//    nothing here places or cancels a real order.
//
// Run explicitly with `ctest -L live` or by invoking the binary directly.
// ----------------------------------------------------------------------------

#include "test_helpers.hpp"

#include <krapi/api.hpp>

#include <boost/asio/io_context.hpp>

#include <doctest/doctest.h>

#include <string>

using namespace krapi::rest;
using krapi_test::env_or_empty;
using krapi_test::have_credentials;

namespace
{
    // Builds an api client bound to a fresh io_context, reading credentials from
    // the environment (tests/.env is loaded first). Empty creds are fine for the
    // public endpoints.
    struct client
    {
        boost::asio::io_context ioctx;
        api rest;

        client()
            : rest{ioctx, "api.kraken.com", "443",
                   env_or_empty("KRAKEN_API_KEY"),
                   env_or_empty("KRAKEN_API_SECRET"), 10000}
        {
        }
    };

    bool init_env = (krapi_test::load_test_env(), true);
} // anon ns

// ---- public -----------------------------------------------------------------

TEST_CASE("live: server_time" * doctest::description("public"))
{
    client c;
    const auto r = c.rest.server_time();
    if (!r)
    {
        MESSAGE("skipped (transport error): " << r.errmsg);
        return;
    }
    CHECK(r.v.unixtime > 0u);
    CHECK_FALSE(r.v.rfc1123.empty());
}

TEST_CASE("live: system_status" * doctest::description("public"))
{
    client c;
    const auto r = c.rest.system_status();
    if (!r)
    {
        MESSAGE("skipped (transport error): " << r.errmsg);
        return;
    }
    CHECK_FALSE(r.v.status.empty());
    CHECK_FALSE(r.v.timestamp.empty());
}

TEST_CASE("live: ticker XBTUSD" * doctest::description("public"))
{
    client c;
    const auto r = c.rest.ticker("XBTUSD");
    if (!r)
    {
        MESSAGE("skipped (transport error): " << r.errmsg);
        return;
    }
    REQUIRE_FALSE(r.v.tickers.empty());
    const auto &t = r.v.tickers.begin()->second;
    CHECK(t.ask > krapi::double_type("0"));
    CHECK(t.bid > krapi::double_type("0"));
    CHECK(t.last > krapi::double_type("0"));
}

TEST_CASE("live: asset_pairs XBTUSD" * doctest::description("public"))
{
    client c;
    const auto r = c.rest.asset_pairs("XBTUSD");
    if (!r)
    {
        MESSAGE("skipped (transport error): " << r.errmsg);
        return;
    }
    REQUIRE_FALSE(r.v.pairs.empty());
    const auto &p = r.v.pairs.begin()->second;
    CHECK_FALSE(p.base.empty());
    CHECK_FALSE(p.quote.empty());
}

TEST_CASE("live: order_book XBTUSD" * doctest::description("public"))
{
    client c;
    const auto r = c.rest.order_book("XBTUSD", 5);
    if (!r)
    {
        MESSAGE("skipped (transport error): " << r.errmsg);
        return;
    }
    CHECK_FALSE(r.v.pair.empty());
    CHECK_FALSE(r.v.asks.empty());
    CHECK_FALSE(r.v.bids.empty());
}

TEST_CASE("live: ohlc XBTUSD" * doctest::description("public"))
{
    client c;
    const auto r = c.rest.ohlc("XBTUSD", 60);
    if (!r)
    {
        MESSAGE("skipped (transport error): " << r.errmsg);
        return;
    }
    CHECK_FALSE(r.v.pair.empty());
    CHECK_FALSE(r.v.candles.empty());
}

TEST_CASE("live: recent_trades XBTUSD" * doctest::description("public"))
{
    client c;
    const auto r = c.rest.recent_trades("XBTUSD");
    if (!r)
    {
        MESSAGE("skipped (transport error): " << r.errmsg);
        return;
    }
    CHECK_FALSE(r.v.pair.empty());
    CHECK_FALSE(r.v.trades.empty());
}

// ---- private (read-only / validate-only) ------------------------------------

TEST_CASE("live: balances" * doctest::description("private"))
{
    if (!have_credentials())
    {
        MESSAGE("skipped: no KRAKEN_API_KEY / KRAKEN_API_SECRET");
        return;
    }
    client c;
    const auto r = c.rest.balances();
    INFO("errmsg: " << r.errmsg);
    CHECK(static_cast<bool>(r));
}

TEST_CASE("live: open_orders" * doctest::description("private"))
{
    if (!have_credentials())
    {
        MESSAGE("skipped: no credentials");
        return;
    }
    client c;
    const auto r = c.rest.open_orders();
    INFO("errmsg: " << r.errmsg);
    CHECK(static_cast<bool>(r));
}

TEST_CASE("live: closed_orders" * doctest::description("private"))
{
    if (!have_credentials())
    {
        MESSAGE("skipped: no credentials");
        return;
    }
    client c;
    const auto r = c.rest.closed_orders();
    INFO("errmsg: " << r.errmsg);
    CHECK(static_cast<bool>(r));
}

TEST_CASE("live: trades_history" * doctest::description("private"))
{
    if (!have_credentials())
    {
        MESSAGE("skipped: no credentials");
        return;
    }
    client c;
    const auto r = c.rest.trades_history();
    INFO("errmsg: " << r.errmsg);
    CHECK(static_cast<bool>(r));
}

TEST_CASE("live: open_positions" * doctest::description("private"))
{
    if (!have_credentials())
    {
        MESSAGE("skipped: no credentials");
        return;
    }
    client c;
    const auto r = c.rest.open_positions(/*docalcs=*/true);
    INFO("errmsg: " << r.errmsg);
    CHECK(static_cast<bool>(r));
}

TEST_CASE("live: query_orders/query_trades from a real txid" * doctest::description("private"))
{
    if (!have_credentials())
    {
        MESSAGE("skipped: no credentials");
        return;
    }
    client c;

    // Derive a txid from closed orders; skip if the account has no history.
    const auto closed = c.rest.closed_orders();
    INFO("closed_orders errmsg: " << closed.errmsg);
    REQUIRE(static_cast<bool>(closed));
    if (closed.v.orders.empty())
    {
        MESSAGE("skipped: account has no closed orders to query");
        return;
    }

    const std::string txid = closed.v.orders.begin()->first;

    const auto qo = c.rest.query_orders(txid);
    INFO("query_orders errmsg: " << qo.errmsg);
    CHECK(static_cast<bool>(qo));
    CHECK(qo.v.orders.count(txid) == 1);

    // A txid may have zero or more trades; we only assert the call succeeds.
    const auto qt = c.rest.query_trades(txid);
    INFO("query_trades errmsg: " << qt.errmsg);
    CHECK(static_cast<bool>(qt));
}

TEST_CASE("live: add_order validate=true does not place an order" * doctest::description("private"))
{
    if (!have_credentials())
    {
        MESSAGE("skipped: no credentials");
        return;
    }
    client c;

    // A deliberately low limit price so it would never fill even if it somehow
    // executed; validate=true means Kraken only validates and returns descr.
    const auto r = c.rest.add_order("XBTUSD", "buy", "limit", "0.0001", "1000.0",
                                    /*validate=*/true);
    INFO("errmsg: " << r.errmsg);
    CHECK(static_cast<bool>(r));
    CHECK_FALSE(r.v.descr.empty());
    CHECK(r.v.txid.empty()); // validate-only: no order id
}
