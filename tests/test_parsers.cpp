
// ----------------------------------------------------------------------------
// Offline parser tests.
//
// Each case feeds a captured Kraken JSON payload (tests/fixtures/*.json) into the
// matching `T::construct(const flatjson::fjson&)` parser and asserts the parsed
// fields. This mirrors exactly how src/api.cpp turns a raw reply into a
// `result<T>`, but without any network or credentials.
// ----------------------------------------------------------------------------

#include "test_helpers.hpp"

#include <krapi/types.hpp>
#include <krapi/errors.hpp>
#include <krapi/flatjson.hpp>

#include <doctest/doctest.h>

using namespace krapi::rest;
using krapi::double_type;
using krapi_test::read_fixture;

namespace
{
    // Parses a fixture and returns its raw text (kept alive by the caller) plus a
    // validated fjson view over it. The returned string MUST outlive any use of
    // the fjson, so callers hold the string in a local and pass it in.
    flatjson::fjson parse(const std::string &text)
    {
        flatjson::fjson json{text.c_str(), text.length()};
        REQUIRE(json.error() == flatjson::FJ_EC_OK);

        return json;
    }
} // anon ns

TEST_CASE("server_time parses unixtime and rfc1123")
{
    const std::string s = read_fixture("server_time.json");
    const auto r = server_time_t::construct(parse(s));

    CHECK(r.unixtime == 1616336594u);
    CHECK(r.rfc1123 == "Sun, 21 Mar 21 14:23:14 +0000");
}

TEST_CASE("system_status parses status and timestamp")
{
    const std::string s = read_fixture("system_status.json");
    const auto r = system_status_t::construct(parse(s));

    CHECK(r.status == "online");
    CHECK(r.timestamp == "2021-03-21T15:33:02Z");
}

TEST_CASE("ticker maps the a/b/c/v/p/t/l/h/o arrays onto fields")
{
    const std::string s = read_fixture("ticker.json");
    const auto r = tickers_t::construct(parse(s));

    REQUIRE(r.has("XXBTZUSD"));
    const auto &t = r.get("XXBTZUSD");
    CHECK(t.pair == "XXBTZUSD");
    CHECK(t.ask == double_type("30300.10000"));
    CHECK(t.bid == double_type("30300.00000"));
    CHECK(t.last == double_type("30303.20000"));
    CHECK(t.volume_today == double_type("4083.67001100"));
    CHECK(t.volume_24h == double_type("4412.73601799"));
    CHECK(t.vwap_today == double_type("30706.77771"));
    CHECK(t.trades_today == 34619u);
    CHECK(t.trades_24h == 38907u);
    CHECK(t.low_24h == double_type("29868.30000"));
    CHECK(t.high_today == double_type("31631.00000"));
    CHECK(t.open == double_type("30502.80000"));
}

TEST_CASE("asset_pairs parses pair metadata")
{
    const std::string s = read_fixture("asset_pairs.json");
    const auto r = asset_pairs_t::construct(parse(s));

    REQUIRE(r.has("XXBTZUSD"));
    const auto &p = r.get("XXBTZUSD");
    CHECK(p.name == "XXBTZUSD");
    CHECK(p.altname == "XBTUSD");
    CHECK(p.wsname == "XBT/USD");
    CHECK(p.base == "XXBT");
    CHECK(p.quote == "ZUSD");
    CHECK(p.pair_decimals == 1u);
    CHECK(p.lot_decimals == 8u);
    CHECK(p.ordermin == double_type("0.0001"));
    CHECK(p.status == "online");
}

TEST_CASE("order_book parses ask/bid levels")
{
    const std::string s = read_fixture("order_book.json");
    const auto r = order_book_t::construct(parse(s));

    CHECK(r.pair == "XXBTZUSD");
    REQUIRE(r.asks.size() == 2);
    REQUIRE(r.bids.size() == 2);
    CHECK(r.asks[0].price == double_type("30384.10000"));
    CHECK(r.asks[0].volume == double_type("2.059"));
    CHECK(r.asks[0].timestamp == 1615560030u);
    CHECK(r.bids[0].price == double_type("30297.00000"));
}

TEST_CASE("ohlc parses candles and the last cursor")
{
    const std::string s = read_fixture("ohlc.json");
    const auto r = ohlc_t::construct(parse(s));

    CHECK(r.pair == "XXBTZUSD");
    CHECK(r.last == 1616662920u);
    REQUIRE(r.candles.size() == 2);
    CHECK(r.candles[0].time == 1616662740u);
    CHECK(r.candles[0].open == double_type("52591.9"));
    CHECK(r.candles[0].high == double_type("52599.9"));
    CHECK(r.candles[0].close == double_type("52599.1"));
    CHECK(r.candles[0].count == 5u);
    CHECK(r.candles[1].count == 17u);
}

TEST_CASE("recent_trades parses price/volume/side/type and the last cursor")
{
    const std::string s = read_fixture("recent_trades.json");
    const auto r = recent_trades_t::construct(parse(s));

    CHECK(r.pair == "XXBTZUSD");
    CHECK(r.last == "1616663618722575631");
    REQUIRE(r.trades.size() == 2);
    CHECK(r.trades[0].price == double_type("52560.10000"));
    CHECK(r.trades[0].volume == double_type("0.05272010"));
    CHECK(r.trades[0].time == doctest::Approx(1616663618.0529));
    CHECK(r.trades[0].side == "s");
    CHECK(r.trades[0].type == "m");
    CHECK(r.trades[0].trade_id == 1234567u);
    CHECK(r.trades[1].side == "b");
}

TEST_CASE("balances parses asset -> amount with full precision")
{
    const std::string s = read_fixture("balances.json");
    const auto r = balances_t::construct(parse(s));

    CHECK(r.balances.size() == 3);
    REQUIRE(r.has("ZUSD"));
    REQUIRE(r.has("XXBT"));
    CHECK(r.get("ZUSD") == double_type("171.3900"));
    CHECK(r.get("XXBT") == double_type("0.0123456700"));
    CHECK_FALSE(r.has("NOPE"));
}

TEST_CASE("open_orders parses the order incl. nested descr")
{
    const std::string s = read_fixture("open_orders.json");
    const auto r = open_orders_t::construct(parse(s));

    REQUIRE(r.orders.size() == 1);
    const auto &o = r.orders.at("OQCLML-BW3P3-BUCMWZ");
    CHECK(o.txid == "OQCLML-BW3P3-BUCMWZ");
    CHECK(o.status == "open");
    CHECK(o.descr.pair == "XBTUSD");
    CHECK(o.descr.type == "buy");
    CHECK(o.descr.ordertype == "limit");
    CHECK(o.descr.price == double_type("30010.0"));
    CHECK(o.vol == double_type("1.25"));
    CHECK(o.vol_exec == double_type("0.375"));
    CHECK(o.oflags == "fciq");
    CHECK(o.opentm == doctest::Approx(1616666559.8974));
}

TEST_CASE("closed_orders parses count, reason and userref")
{
    const std::string s = read_fixture("closed_orders.json");
    const auto r = closed_orders_t::construct(parse(s));

    CHECK(r.count == 1u);
    REQUIRE(r.orders.size() == 1);
    const auto &o = r.orders.at("O37652-RJWRT-IMO74O");
    CHECK(o.status == "canceled");
    CHECK(o.reason == "User requested");
    CHECK(o.userref == 1u);
    CHECK(o.closetm == doctest::Approx(1616148610.0482));
}

TEST_CASE("trades_history parses count and trade fields")
{
    const std::string s = read_fixture("trades_history.json");
    const auto r = trades_history_t::construct(parse(s));

    CHECK(r.count == 1u);
    REQUIRE(r.trades.size() == 1);
    const auto &t = r.trades.at("THVRQM-33VKH-UCI7BS");
    CHECK(t.ordertxid == "OQCLML-BW3P3-BUCMWZ");
    CHECK(t.pair == "XXBTZUSD");
    CHECK(t.type == "buy");
    CHECK(t.price == double_type("30010.00000"));
    CHECK(t.vol == double_type("0.02000000"));
}

TEST_CASE("query_orders parses txid -> order directly under result")
{
    const std::string s = read_fixture("query_orders.json");
    const auto r = query_orders_t::construct(parse(s));

    REQUIRE(r.orders.size() == 1);
    const auto &o = r.orders.at("OQCLML-BW3P3-BUCMWZ");
    CHECK(o.status == "closed");
    CHECK(o.vol_exec == double_type("1.25"));
    CHECK(o.descr.type == "buy");
}

TEST_CASE("query_trades parses txid -> trade directly under result")
{
    const std::string s = read_fixture("query_trades.json");
    const auto r = query_trades_t::construct(parse(s));

    REQUIRE(r.trades.size() == 1);
    const auto &t = r.trades.at("THVRQM-33VKH-UCI7BS");
    CHECK(t.pair == "XXBTZUSD");
    CHECK(t.type == "buy");
    CHECK(t.cost == double_type("600.20000"));
}

TEST_CASE("open_positions parses position incl. docalcs value/net")
{
    const std::string s = read_fixture("open_positions.json");
    const auto r = open_positions_t::construct(parse(s));

    REQUIRE(r.positions.size() == 1);
    const auto &p = r.positions.at("TF5GVO-T7ZZ5-UNTOF6");
    CHECK(p.ordertxid == "OLWNFG-LLH4R-D6SFFP");
    CHECK(p.posstatus == "open");
    CHECK(p.pair == "XXBTZUSD");
    CHECK(p.type == "buy");
    CHECK(p.vol == double_type("8.82412861"));
    CHECK(p.vol_closed == double_type("0.00000000"));
    CHECK(p.value == double_type("258797.5"));
    CHECK(p.net > double_type("0")); // "+154186.9728"
}

TEST_CASE("add_order with validate=true returns descr and no txid")
{
    const std::string s = read_fixture("add_order_validate.json");
    const auto r = add_order_t::construct(parse(s));

    CHECK_FALSE(r.descr.empty());
    CHECK(r.txid.empty());
}

TEST_CASE("add_order returns descr and txid")
{
    const std::string s = read_fixture("add_order.json");
    const auto r = add_order_t::construct(parse(s));

    CHECK_FALSE(r.descr.empty());
    REQUIRE(r.txid.size() == 1);
    CHECK(r.txid[0] == "OUF4EM-FRGI2-MQMWZD");
}

TEST_CASE("cancel_order parses count and defaults pending to false")
{
    const std::string s = read_fixture("cancel_order.json");
    const auto r = cancel_order_t::construct(parse(s));

    CHECK(r.count == 1u);
    CHECK(r.pending == false);
}

TEST_CASE("cancel_all_after parses currentTime and triggerTime")
{
    const std::string s = read_fixture("cancel_all_after.json");
    const auto r = cancel_all_after_t::construct(parse(s));

    CHECK(r.currentTime == "2023-03-24T17:41:56Z");
    CHECK(r.triggerTime == "2023-03-24T17:42:56Z");
}

TEST_CASE("error replies are detected and surfaced")
{
    const std::string s = read_fixture("error.json");
    const auto json = parse(s);

    REQUIRE(is_api_error(json));
    const auto err = construct_error(json);
    CHECK(err.second == "EAPI:Invalid key");
}
