
// ----------------------------------------------------------------------------
//                              Apache License
//                        Version 2.0, January 2004
//                     http://www.apache.org/licenses/
//
// This file is part of binapi(https://github.com/niXman/binapi) project.
//
// Copyright (c) 2019-2021 niXman (github dot nixman dog pm.me). All rights reserved.
// ----------------------------------------------------------------------------

#include <krapi/types.hpp>
#include <krapi/flatjson.hpp>
#include <krapi/fnv1a.hpp>

#include <type_traits>
#include <cstdio>

#include <boost/utility/string_view.hpp>

// #include <iostream> // TODO: comment out

namespace krapi
{

    /*************************************************************************************************/

    template <typename T>
    typename std::enable_if<std::is_integral<T>::value>::type
    __get_json(T &v, const char *member, const flatjson::fjson &j)
    {
        v = j.at(member).to<T>();
    }

    template <typename T>
    typename std::enable_if<std::is_same<T, std::string>::value>::type
    __get_json(T &v, const char *member, const flatjson::fjson &j)
    {
        const auto &o = j.at(member);
        v = (o.is_null() ? std::string{} : o.to_string());
    }

    template <typename T>
    typename std::enable_if<std::is_same<T, double_type>::value>::type
    __get_json(T &v, const char *member, const flatjson::fjson &j)
    {
        const auto s = j.at(member).to_string();
        v.assign(s);
    }

#define __KRAPI_GET2(obj, member, json) \
    __get_json(obj.member, #member, json)

#define __KRAPI_GET(member) __KRAPI_GET2(res, member, json)

    /*************************************************************************************************/

    namespace rest
    {

        /*************************************************************************************************/
        /*************************************************************************************************/
        /*************************************************************************************************/

        server_time_t server_time_t::construct(const flatjson::fjson &json)
        {
            assert(json.is_valid());

            server_time_t res{};
            const auto result = json.at("result");
            __KRAPI_GET2(res, unixtime, result);
            __KRAPI_GET2(res, rfc1123, result);

            return res;
        }

        std::ostream &operator<<(std::ostream &os, const server_time_t &o)
        {
            os
                << "{"
                << "\"unixtime\":" << o.unixtime << ","
                << "\"rfc1123\":\"" << o.rfc1123 << "\""
                << "}";

            return os;
        }

        /*************************************************************************************************/

        const double_type &balances_t::get(const std::string &asset) const
        {
            static const double_type zero{0};
            const auto it = balances.find(asset);

            return it == balances.end() ? zero : it->second;
        }

        balances_t balances_t::construct(const flatjson::fjson &json)
        {
            assert(json.is_valid());
            assert(json.is_object());

            balances_t res{};

            const auto result = json.at("result");
            const auto keys = result.get_keys();
            for (const auto &k : keys)
            {
                const std::string asset{k.data(), k.size()};
                double_type amount{};
                amount.assign(result.at(asset.c_str()).to_string());
                res.balances.emplace(asset, std::move(amount));
            }

            return res;
        }

        std::ostream &operator<<(std::ostream &os, const balances_t &o)
        {
            os << "{";
            for (auto it = o.balances.begin(); it != o.balances.end(); ++it)
            {
                os << "\"" << it->first << "\":\"" << it->second << "\"";
                if (std::next(it) != o.balances.end())
                {
                    os << ",";
                }
            }
            os << "}";

            return os;
        }

        /*************************************************************************************************/

        // reads a double_type from an array element, e.g. ticker "a"[0]
        static double_type __get_arr_double(const flatjson::fjson &arr, std::size_t idx)
        {
            double_type v{};
            v.assign(arr.at(idx).to_string());

            return v;
        }

        tickers_t::ticker_t
        tickers_t::ticker_t::construct(const std::string &pair, const flatjson::fjson &json)
        {
            assert(json.is_valid());
            assert(json.is_object());

            tickers_t::ticker_t res{};
            res.pair = pair;

            const auto a = json.at("a");
            const auto b = json.at("b");
            const auto c = json.at("c");
            const auto v = json.at("v");
            const auto p = json.at("p");
            const auto t = json.at("t");
            const auto l = json.at("l");
            const auto h = json.at("h");

            res.ask = __get_arr_double(a, 0u);
            res.bid = __get_arr_double(b, 0u);
            res.last = __get_arr_double(c, 0u);
            res.volume_today = __get_arr_double(v, 0u);
            res.volume_24h = __get_arr_double(v, 1u);
            res.vwap_today = __get_arr_double(p, 0u);
            res.vwap_24h = __get_arr_double(p, 1u);
            res.trades_today = t.at(0u).to<std::size_t>();
            res.trades_24h = t.at(1u).to<std::size_t>();
            res.low_today = __get_arr_double(l, 0u);
            res.low_24h = __get_arr_double(l, 1u);
            res.high_today = __get_arr_double(h, 0u);
            res.high_24h = __get_arr_double(h, 1u);
            res.open.assign(json.at("o").to_string());

            return res;
        }

        std::ostream &operator<<(std::ostream &os, const tickers_t::ticker_t &o)
        {
            os
                << "{"
                << "\"pair\":\"" << o.pair << "\","
                << "\"ask\":\"" << o.ask << "\","
                << "\"bid\":\"" << o.bid << "\","
                << "\"last\":\"" << o.last << "\","
                << "\"volume_today\":\"" << o.volume_today << "\","
                << "\"volume_24h\":\"" << o.volume_24h << "\","
                << "\"vwap_today\":\"" << o.vwap_today << "\","
                << "\"vwap_24h\":\"" << o.vwap_24h << "\","
                << "\"trades_today\":" << o.trades_today << ","
                << "\"trades_24h\":" << o.trades_24h << ","
                << "\"low_today\":\"" << o.low_today << "\","
                << "\"low_24h\":\"" << o.low_24h << "\","
                << "\"high_today\":\"" << o.high_today << "\","
                << "\"high_24h\":\"" << o.high_24h << "\","
                << "\"open\":\"" << o.open << "\""
                << "}";

            return os;
        }

        const tickers_t::ticker_t &tickers_t::get(const std::string &pair) const
        {
            static const ticker_t empty{};
            const auto it = tickers.find(pair);

            return it == tickers.end() ? empty : it->second;
        }

        tickers_t tickers_t::construct(const flatjson::fjson &json)
        {
            assert(json.is_valid());
            assert(json.is_object());

            tickers_t res{};
            const auto result = json.at("result");
            const auto keys = result.get_keys();
            for (const auto &k : keys)
            {
                const std::string pair{k.data(), k.size()};
                auto item = tickers_t::ticker_t::construct(pair, result.at(pair.c_str()));
                res.tickers.emplace(pair, std::move(item));
            }

            return res;
        }

        std::ostream &operator<<(std::ostream &os, const tickers_t &o)
        {
            os << "[";
            for (auto it = o.tickers.begin(); it != o.tickers.end(); ++it)
            {
                os << it->second;
                if (std::next(it) != o.tickers.end())
                {
                    os << ",";
                }
            }
            os << "]";

            return os;
        }

        /*************************************************************************************************/

        asset_pairs_t::asset_pair_t
        asset_pairs_t::asset_pair_t::construct(const std::string &name, const flatjson::fjson &json)
        {
            assert(json.is_valid());
            assert(json.is_object());

            asset_pairs_t::asset_pair_t res{};
            res.name = name;

            __KRAPI_GET2(res, altname, json);
            if (json.contains("wsname"))
            {
                __KRAPI_GET2(res, wsname, json);
            }
            __KRAPI_GET2(res, base, json);
            __KRAPI_GET2(res, quote, json);
            __KRAPI_GET2(res, pair_decimals, json);
            __KRAPI_GET2(res, lot_decimals, json);
            if (json.contains("ordermin"))
            {
                __KRAPI_GET2(res, ordermin, json);
            }
            if (json.contains("status"))
            {
                __KRAPI_GET2(res, status, json);
            }

            return res;
        }

        std::ostream &operator<<(std::ostream &os, const asset_pairs_t::asset_pair_t &o)
        {
            os
                << "{"
                << "\"name\":\"" << o.name << "\","
                << "\"altname\":\"" << o.altname << "\","
                << "\"wsname\":\"" << o.wsname << "\","
                << "\"base\":\"" << o.base << "\","
                << "\"quote\":\"" << o.quote << "\","
                << "\"pair_decimals\":" << o.pair_decimals << ","
                << "\"lot_decimals\":" << o.lot_decimals << ","
                << "\"ordermin\":\"" << o.ordermin << "\","
                << "\"status\":\"" << o.status << "\""
                << "}";

            return os;
        }

        const asset_pairs_t::asset_pair_t &asset_pairs_t::get(const std::string &name) const
        {
            static const asset_pair_t empty{};
            const auto it = pairs.find(name);

            return it == pairs.end() ? empty : it->second;
        }

        asset_pairs_t asset_pairs_t::construct(const flatjson::fjson &json)
        {
            assert(json.is_valid());
            assert(json.is_object());

            asset_pairs_t res{};
            const auto result = json.at("result");
            const auto keys = result.get_keys();
            for (const auto &k : keys)
            {
                const std::string name{k.data(), k.size()};
                auto item = asset_pairs_t::asset_pair_t::construct(name, result.at(name.c_str()));
                res.pairs.emplace(name, std::move(item));
            }

            return res;
        }

        std::ostream &operator<<(std::ostream &os, const asset_pairs_t &o)
        {
            os << "[";
            for (auto it = o.pairs.begin(); it != o.pairs.end(); ++it)
            {
                os << it->second;
                if (std::next(it) != o.pairs.end())
                {
                    os << ",";
                }
            }
            os << "]";

            return os;
        }

        /*************************************************************************************************/

        add_order_t add_order_t::construct(const flatjson::fjson &json)
        {
            assert(json.is_valid());

            add_order_t res{};
            const auto result = json.at("result");

            if (result.contains("descr") && !result.at("descr").is_null())
            {
                const auto d = result.at("descr");
                if (d.contains("order"))
                {
                    res.descr = d.at("order").to_string();
                }
            }
            if (result.contains("txid"))
            {
                const auto t = result.at("txid");
                for (auto idx = 0u; idx < t.size(); ++idx)
                {
                    res.txid.push_back(t.at(idx).to_string());
                }
            }

            return res;
        }

        std::ostream &operator<<(std::ostream &os, const add_order_t &o)
        {
            os
                << "{"
                << "\"descr\":\"" << o.descr << "\","
                << "\"txid\":[";
            for (auto it = o.txid.begin(); it != o.txid.end(); ++it)
            {
                os << "\"" << *it << "\"";
                if (std::next(it) != o.txid.end())
                {
                    os << ",";
                }
            }
            os << "]}";

            return os;
        }

        /*************************************************************************************************/

        cancel_order_t cancel_order_t::construct(const flatjson::fjson &json)
        {
            assert(json.is_valid());

            cancel_order_t res{};
            const auto result = json.at("result");
            __KRAPI_GET2(res, count, result);
            if (result.contains("pending"))
            {
                res.pending = result.at("pending").to_bool();
            }

            return res;
        }

        std::ostream &operator<<(std::ostream &os, const cancel_order_t &o)
        {
            os
                << "{"
                << "\"count\":" << o.count << ","
                << "\"pending\":" << (o.pending ? "true" : "false")
                << "}";

            return os;
        }

        /*************************************************************************************************/

        // Kraken timestamps are fractional unix seconds; format without scientific
        // notation and without losing the sub-second part.
        static std::string fmt_seconds(double v)
        {
            char buf[32];
            std::snprintf(buf, sizeof(buf), "%.4f", v);

            return buf;
        }

        kraken_order_t kraken_order_t::construct(const std::string &txid, const flatjson::fjson &json)
        {
            assert(json.is_valid());
            assert(json.is_object());

            kraken_order_t res{};
            res.txid = txid;

            __KRAPI_GET2(res, status, json);
            if (json.contains("userref") && !json.at("userref").is_null())
            {
                __KRAPI_GET2(res, userref, json);
            }
            if (json.contains("opentm"))
            {
                res.opentm = json.at("opentm").to_double();
            }
            if (json.contains("closetm"))
            {
                res.closetm = json.at("closetm").to_double();
            }
            if (json.contains("starttm"))
            {
                res.starttm = json.at("starttm").to_double();
            }
            if (json.contains("expiretm"))
            {
                res.expiretm = json.at("expiretm").to_double();
            }

            const auto d = json.at("descr");
            __KRAPI_GET2(res.descr, pair, d);
            __KRAPI_GET2(res.descr, type, d);
            __KRAPI_GET2(res.descr, ordertype, d);
            __KRAPI_GET2(res.descr, price, d);
            __KRAPI_GET2(res.descr, price2, d);
            __KRAPI_GET2(res.descr, leverage, d);
            __KRAPI_GET2(res.descr, order, d);
            if (d.contains("close") && !d.at("close").is_null())
            {
                __KRAPI_GET2(res.descr, close, d);
            }

            __KRAPI_GET2(res, vol, json);
            __KRAPI_GET2(res, vol_exec, json);
            __KRAPI_GET2(res, cost, json);
            __KRAPI_GET2(res, fee, json);
            __KRAPI_GET2(res, price, json);
            __KRAPI_GET2(res, stopprice, json);
            __KRAPI_GET2(res, limitprice, json);
            if (json.contains("misc"))
            {
                __KRAPI_GET2(res, misc, json);
            }
            if (json.contains("oflags"))
            {
                __KRAPI_GET2(res, oflags, json);
            }
            if (json.contains("reason") && !json.at("reason").is_null())
            {
                __KRAPI_GET2(res, reason, json);
            }

            return res;
        }

        std::ostream &operator<<(std::ostream &os, const kraken_order_t &o)
        {
            os
                << "{"
                << "\"txid\":\"" << o.txid << "\","
                << "\"status\":\"" << o.status << "\","
                << "\"userref\":" << o.userref << ","
                << "\"opentm\":" << fmt_seconds(o.opentm) << ","
                << "\"closetm\":" << fmt_seconds(o.closetm) << ","
                << "\"pair\":\"" << o.descr.pair << "\","
                << "\"type\":\"" << o.descr.type << "\","
                << "\"ordertype\":\"" << o.descr.ordertype << "\","
                << "\"descr\":\"" << o.descr.order << "\","
                << "\"vol\":\"" << o.vol << "\","
                << "\"vol_exec\":\"" << o.vol_exec << "\","
                << "\"cost\":\"" << o.cost << "\","
                << "\"fee\":\"" << o.fee << "\","
                << "\"price\":\"" << o.price << "\","
                << "\"oflags\":\"" << o.oflags << "\""
                << "}";

            return os;
        }

        /*************************************************************************************************/

        open_orders_t open_orders_t::construct(const flatjson::fjson &json)
        {
            assert(json.is_valid());

            open_orders_t res{};
            const auto result = json.at("result");
            if (!result.contains("open"))
            {
                return res;
            }

            const auto open = result.at("open");
            const auto keys = open.get_keys();
            for (const auto &k : keys)
            {
                const std::string txid{k.data(), k.size()};
                res.orders.emplace(txid, kraken_order_t::construct(txid, open.at(txid.c_str())));
            }

            return res;
        }

        std::ostream &operator<<(std::ostream &os, const open_orders_t &o)
        {
            os << "[";
            for (auto it = o.orders.begin(); it != o.orders.end(); ++it)
            {
                os << it->second;
                if (std::next(it) != o.orders.end())
                {
                    os << ",";
                }
            }
            os << "]";

            return os;
        }

        /*************************************************************************************************/

        closed_orders_t closed_orders_t::construct(const flatjson::fjson &json)
        {
            assert(json.is_valid());

            closed_orders_t res{};
            const auto result = json.at("result");
            if (result.contains("count"))
            {
                __KRAPI_GET2(res, count, result);
            }
            if (!result.contains("closed"))
            {
                return res;
            }

            const auto closed = result.at("closed");
            const auto keys = closed.get_keys();
            for (const auto &k : keys)
            {
                const std::string txid{k.data(), k.size()};
                res.orders.emplace(txid, kraken_order_t::construct(txid, closed.at(txid.c_str())));
            }

            return res;
        }

        std::ostream &operator<<(std::ostream &os, const closed_orders_t &o)
        {
            os
                << "{"
                << "\"count\":" << o.count << ","
                << "\"orders\":[";
            for (auto it = o.orders.begin(); it != o.orders.end(); ++it)
            {
                os << it->second;
                if (std::next(it) != o.orders.end())
                {
                    os << ",";
                }
            }
            os << "]}";

            return os;
        }

        /*************************************************************************************************/

        trades_history_t::trade_t
        trades_history_t::trade_t::construct(const std::string &txid, const flatjson::fjson &json)
        {
            assert(json.is_valid());
            assert(json.is_object());

            trades_history_t::trade_t res{};
            res.txid = txid;

            __KRAPI_GET2(res, ordertxid, json);
            if (json.contains("postxid"))
            {
                __KRAPI_GET2(res, postxid, json);
            }
            __KRAPI_GET2(res, pair, json);
            res.time = json.at("time").to_double();
            __KRAPI_GET2(res, type, json);
            __KRAPI_GET2(res, ordertype, json);
            __KRAPI_GET2(res, price, json);
            __KRAPI_GET2(res, cost, json);
            __KRAPI_GET2(res, fee, json);
            __KRAPI_GET2(res, vol, json);
            __KRAPI_GET2(res, margin, json);
            if (json.contains("misc"))
            {
                __KRAPI_GET2(res, misc, json);
            }

            return res;
        }

        std::ostream &operator<<(std::ostream &os, const trades_history_t::trade_t &o)
        {
            os
                << "{"
                << "\"txid\":\"" << o.txid << "\","
                << "\"ordertxid\":\"" << o.ordertxid << "\","
                << "\"pair\":\"" << o.pair << "\","
                << "\"time\":" << fmt_seconds(o.time) << ","
                << "\"type\":\"" << o.type << "\","
                << "\"ordertype\":\"" << o.ordertype << "\","
                << "\"price\":\"" << o.price << "\","
                << "\"cost\":\"" << o.cost << "\","
                << "\"fee\":\"" << o.fee << "\","
                << "\"vol\":\"" << o.vol << "\","
                << "\"margin\":\"" << o.margin << "\""
                << "}";

            return os;
        }

        trades_history_t trades_history_t::construct(const flatjson::fjson &json)
        {
            assert(json.is_valid());

            trades_history_t res{};
            const auto result = json.at("result");
            if (result.contains("count"))
            {
                __KRAPI_GET2(res, count, result);
            }
            if (!result.contains("trades"))
            {
                return res;
            }

            const auto trades = result.at("trades");
            const auto keys = trades.get_keys();
            for (const auto &k : keys)
            {
                const std::string txid{k.data(), k.size()};
                res.trades.emplace(txid, trades_history_t::trade_t::construct(txid, trades.at(txid.c_str())));
            }

            return res;
        }

        std::ostream &operator<<(std::ostream &os, const trades_history_t &o)
        {
            os
                << "{"
                << "\"count\":" << o.count << ","
                << "\"trades\":[";
            for (auto it = o.trades.begin(); it != o.trades.end(); ++it)
            {
                os << it->second;
                if (std::next(it) != o.trades.end())
                {
                    os << ",";
                }
            }
            os << "]}";

            return os;
        }

        /*************************************************************************************************/

        // Kraken keys market-data results by pair name and may append a "last" cursor;
        // the pair key is the one that isn't "last".
        static std::string find_pair_key(const flatjson::fjson &result)
        {
            const auto keys = result.get_keys();
            for (const auto &k : keys)
            {
                const std::string key{k.data(), k.size()};
                if (key != "last")
                {
                    return key;
                }
            }

            return {};
        }

        order_book_t::level_t __parse_level(const flatjson::fjson &l)
        {
            order_book_t::level_t lvl{};
            lvl.price.assign(l.at(0u).to_string());
            lvl.volume.assign(l.at(1u).to_string());
            lvl.timestamp = l.at(2u).to<std::size_t>();

            return lvl;
        }

        std::ostream &operator<<(std::ostream &os, const order_book_t::level_t &o)
        {
            os
                << "[\"" << o.price << "\",\"" << o.volume << "\"," << o.timestamp << "]";

            return os;
        }

        order_book_t order_book_t::construct(const flatjson::fjson &json)
        {
            assert(json.is_valid());

            order_book_t res{};
            const auto result = json.at("result");
            res.pair = find_pair_key(result);

            const auto book = result.at(res.pair.c_str());
            const auto asks = book.at("asks");
            for (auto idx = 0u; idx < asks.size(); ++idx)
            {
                res.asks.push_back(__parse_level(asks.at(idx)));
            }
            const auto bids = book.at("bids");
            for (auto idx = 0u; idx < bids.size(); ++idx)
            {
                res.bids.push_back(__parse_level(bids.at(idx)));
            }

            return res;
        }

        std::ostream &operator<<(std::ostream &os, const order_book_t &o)
        {
            os
                << "{"
                << "\"pair\":\"" << o.pair << "\","
                << "\"asks\":[";
            for (auto it = o.asks.begin(); it != o.asks.end(); ++it)
            {
                os << *it;
                if (std::next(it) != o.asks.end())
                {
                    os << ",";
                }
            }
            os << "],\"bids\":[";
            for (auto it = o.bids.begin(); it != o.bids.end(); ++it)
            {
                os << *it;
                if (std::next(it) != o.bids.end())
                {
                    os << ",";
                }
            }
            os << "]}";

            return os;
        }

        /*************************************************************************************************/

        std::ostream &operator<<(std::ostream &os, const ohlc_t::candle_t &o)
        {
            os
                << "{"
                << "\"time\":" << o.time << ","
                << "\"open\":\"" << o.open << "\","
                << "\"high\":\"" << o.high << "\","
                << "\"low\":\"" << o.low << "\","
                << "\"close\":\"" << o.close << "\","
                << "\"vwap\":\"" << o.vwap << "\","
                << "\"volume\":\"" << o.volume << "\","
                << "\"count\":" << o.count
                << "}";

            return os;
        }

        ohlc_t ohlc_t::construct(const flatjson::fjson &json)
        {
            assert(json.is_valid());

            ohlc_t res{};
            const auto result = json.at("result");
            if (result.contains("last"))
            {
                res.last = result.at("last").to<std::size_t>();
            }
            res.pair = find_pair_key(result);

            const auto arr = result.at(res.pair.c_str());
            for (auto idx = 0u; idx < arr.size(); ++idx)
            {
                const auto c = arr.at(idx);
                ohlc_t::candle_t cd{};
                cd.time = c.at(0u).to<std::size_t>();
                cd.open.assign(c.at(1u).to_string());
                cd.high.assign(c.at(2u).to_string());
                cd.low.assign(c.at(3u).to_string());
                cd.close.assign(c.at(4u).to_string());
                cd.vwap.assign(c.at(5u).to_string());
                cd.volume.assign(c.at(6u).to_string());
                cd.count = c.at(7u).to<std::size_t>();
                res.candles.push_back(std::move(cd));
            }

            return res;
        }

        std::ostream &operator<<(std::ostream &os, const ohlc_t &o)
        {
            os
                << "{"
                << "\"pair\":\"" << o.pair << "\","
                << "\"last\":" << o.last << ","
                << "\"candles\":[";
            for (auto it = o.candles.begin(); it != o.candles.end(); ++it)
            {
                os << *it;
                if (std::next(it) != o.candles.end())
                {
                    os << ",";
                }
            }
            os << "]}";

            return os;
        }

        /*************************************************************************************************/

        std::ostream &operator<<(std::ostream &os, const recent_trades_t::trade_t &o)
        {
            os
                << "{"
                << "\"price\":\"" << o.price << "\","
                << "\"volume\":\"" << o.volume << "\","
                << "\"time\":" << fmt_seconds(o.time) << ","
                << "\"side\":\"" << o.side << "\","
                << "\"type\":\"" << o.type << "\","
                << "\"misc\":\"" << o.misc << "\","
                << "\"trade_id\":" << o.trade_id
                << "}";

            return os;
        }

        recent_trades_t recent_trades_t::construct(const flatjson::fjson &json)
        {
            assert(json.is_valid());

            recent_trades_t res{};
            const auto result = json.at("result");
            if (result.contains("last"))
            {
                res.last = result.at("last").to_string();
            }
            res.pair = find_pair_key(result);

            const auto arr = result.at(res.pair.c_str());
            for (auto idx = 0u; idx < arr.size(); ++idx)
            {
                const auto t = arr.at(idx);
                recent_trades_t::trade_t tr{};
                tr.price.assign(t.at(0u).to_string());
                tr.volume.assign(t.at(1u).to_string());
                tr.time = t.at(2u).to_double();
                tr.side = t.at(3u).to_string();
                tr.type = t.at(4u).to_string();
                tr.misc = t.at(5u).to_string();
                tr.trade_id = t.at(6u).to<std::size_t>();
                res.trades.push_back(std::move(tr));
            }

            return res;
        }

        std::ostream &operator<<(std::ostream &os, const recent_trades_t &o)
        {
            os
                << "{"
                << "\"pair\":\"" << o.pair << "\","
                << "\"last\":\"" << o.last << "\","
                << "\"trades\":[";
            for (auto it = o.trades.begin(); it != o.trades.end(); ++it)
            {
                os << *it;
                if (std::next(it) != o.trades.end())
                {
                    os << ",";
                }
            }
            os << "]}";

            return os;
        }

        /*************************************************************************************************/

        system_status_t system_status_t::construct(const flatjson::fjson &json)
        {
            assert(json.is_valid());

            system_status_t res{};
            const auto result = json.at("result");
            __KRAPI_GET2(res, status, result);
            __KRAPI_GET2(res, timestamp, result);

            return res;
        }

        std::ostream &operator<<(std::ostream &os, const system_status_t &o)
        {
            os
                << "{"
                << "\"status\":\"" << o.status << "\","
                << "\"timestamp\":\"" << o.timestamp << "\""
                << "}";

            return os;
        }

        /*************************************************************************************************/

        query_orders_t query_orders_t::construct(const flatjson::fjson &json)
        {
            assert(json.is_valid());

            query_orders_t res{};
            const auto result = json.at("result");
            const auto keys = result.get_keys();
            for (const auto &k : keys)
            {
                const std::string txid{k.data(), k.size()};
                res.orders.emplace(txid, kraken_order_t::construct(txid, result.at(txid.c_str())));
            }

            return res;
        }

        std::ostream &operator<<(std::ostream &os, const query_orders_t &o)
        {
            os << "[";
            for (auto it = o.orders.begin(); it != o.orders.end(); ++it)
            {
                os << it->second;
                if (std::next(it) != o.orders.end())
                {
                    os << ",";
                }
            }
            os << "]";

            return os;
        }

        /*************************************************************************************************/

        query_trades_t query_trades_t::construct(const flatjson::fjson &json)
        {
            assert(json.is_valid());

            query_trades_t res{};
            const auto result = json.at("result");
            const auto keys = result.get_keys();
            for (const auto &k : keys)
            {
                const std::string txid{k.data(), k.size()};
                res.trades.emplace(txid, trades_history_t::trade_t::construct(txid, result.at(txid.c_str())));
            }

            return res;
        }

        std::ostream &operator<<(std::ostream &os, const query_trades_t &o)
        {
            os << "[";
            for (auto it = o.trades.begin(); it != o.trades.end(); ++it)
            {
                os << it->second;
                if (std::next(it) != o.trades.end())
                {
                    os << ",";
                }
            }
            os << "]";

            return os;
        }

        /*************************************************************************************************/

        open_positions_t::position_t
        open_positions_t::position_t::construct(const std::string &txid, const flatjson::fjson &json)
        {
            assert(json.is_valid());
            assert(json.is_object());

            open_positions_t::position_t res{};
            res.txid = txid;

            __KRAPI_GET2(res, ordertxid, json);
            __KRAPI_GET2(res, posstatus, json);
            __KRAPI_GET2(res, pair, json);
            res.time = json.at("time").to_double();
            __KRAPI_GET2(res, type, json);
            __KRAPI_GET2(res, ordertype, json);
            __KRAPI_GET2(res, cost, json);
            __KRAPI_GET2(res, fee, json);
            __KRAPI_GET2(res, vol, json);
            if (json.contains("vol_closed"))
            {
                __KRAPI_GET2(res, vol_closed, json);
            }
            __KRAPI_GET2(res, margin, json);
            if (json.contains("value"))
            {
                __KRAPI_GET2(res, value, json);
            }
            if (json.contains("net"))
            {
                __KRAPI_GET2(res, net, json);
            }
            if (json.contains("terms"))
            {
                __KRAPI_GET2(res, terms, json);
            }
            if (json.contains("rollovertm"))
            {
                __KRAPI_GET2(res, rollovertm, json);
            }
            if (json.contains("misc"))
            {
                __KRAPI_GET2(res, misc, json);
            }
            if (json.contains("oflags"))
            {
                __KRAPI_GET2(res, oflags, json);
            }

            return res;
        }

        std::ostream &operator<<(std::ostream &os, const open_positions_t::position_t &o)
        {
            os
                << "{"
                << "\"txid\":\"" << o.txid << "\","
                << "\"ordertxid\":\"" << o.ordertxid << "\","
                << "\"posstatus\":\"" << o.posstatus << "\","
                << "\"pair\":\"" << o.pair << "\","
                << "\"time\":" << fmt_seconds(o.time) << ","
                << "\"type\":\"" << o.type << "\","
                << "\"ordertype\":\"" << o.ordertype << "\","
                << "\"cost\":\"" << o.cost << "\","
                << "\"fee\":\"" << o.fee << "\","
                << "\"vol\":\"" << o.vol << "\","
                << "\"vol_closed\":\"" << o.vol_closed << "\","
                << "\"margin\":\"" << o.margin << "\","
                << "\"value\":\"" << o.value << "\","
                << "\"net\":\"" << o.net << "\""
                << "}";

            return os;
        }

        open_positions_t open_positions_t::construct(const flatjson::fjson &json)
        {
            assert(json.is_valid());

            open_positions_t res{};
            const auto result = json.at("result");
            const auto keys = result.get_keys();
            for (const auto &k : keys)
            {
                const std::string txid{k.data(), k.size()};
                res.positions.emplace(txid, open_positions_t::position_t::construct(txid, result.at(txid.c_str())));
            }

            return res;
        }

        std::ostream &operator<<(std::ostream &os, const open_positions_t &o)
        {
            os << "[";
            for (auto it = o.positions.begin(); it != o.positions.end(); ++it)
            {
                os << it->second;
                if (std::next(it) != o.positions.end())
                {
                    os << ",";
                }
            }
            os << "]";

            return os;
        }

        /*************************************************************************************************/

        cancel_all_after_t cancel_all_after_t::construct(const flatjson::fjson &json)
        {
            assert(json.is_valid());

            cancel_all_after_t res{};
            const auto result = json.at("result");
            if (result.contains("currentTime"))
            {
                __KRAPI_GET2(res, currentTime, result);
            }
            if (result.contains("triggerTime"))
            {
                __KRAPI_GET2(res, triggerTime, result);
            }

            return res;
        }

        std::ostream &operator<<(std::ostream &os, const cancel_all_after_t &o)
        {
            os
                << "{"
                << "\"currentTime\":\"" << o.currentTime << "\","
                << "\"triggerTime\":\"" << o.triggerTime << "\""
                << "}";

            return os;
        }

    } // ns rest

    /*************************************************************************************************/
    /*************************************************************************************************/
    /*************************************************************************************************/

    namespace ws
    {

        agg_trade_t agg_trade_t::construct(const flatjson::fjson &json)
        {
            assert(json.is_valid());

            agg_trade_t res{};
            __KRAPI_GET(e);
            __KRAPI_GET(E);
            __KRAPI_GET(s);
            __KRAPI_GET(a);
            __KRAPI_GET(p);
            __KRAPI_GET(q);
            __KRAPI_GET(f);
            __KRAPI_GET(l);
            __KRAPI_GET(T);
            __KRAPI_GET(m);
            __KRAPI_GET(M);

            return res;
        }

        std::ostream &operator<<(std::ostream &os, const agg_trade_t &o)
        {
            os
                << "{"
                << "\"e\":\"" << o.e << "\","
                << "\"E\":" << o.E << ","
                << "\"s\":\"" << o.s << "\","
                << "\"a\":" << o.a << ","
                << "\"p\":\"" << o.p << "\","
                << "\"q\":\"" << o.q << "\","
                << "\"f\":" << o.f << ","
                << "\"l\":" << o.l << ","
                << "\"T\":" << o.T << ","
                << "\"m\":" << (o.m ? "true" : "false") << ","
                << "\"M\":" << (o.M ? "true" : "false")
                << "}";

            return os;
        }

        /*************************************************************************************************/

        trade_t trade_t::construct(const flatjson::fjson &json)
        {
            assert(json.is_valid());

            trade_t res{};
            __KRAPI_GET(E);
            __KRAPI_GET(s);
            __KRAPI_GET(t);
            __KRAPI_GET(p);
            __KRAPI_GET(q);
            __KRAPI_GET(T);
            __KRAPI_GET(m);
            __KRAPI_GET(M);

            return res;
        }

        std::ostream &operator<<(std::ostream &os, const trade_t &o)
        {
            os
                << "{"
                << "\"E\":" << o.E << ","
                << "\"s\":\"" << o.s << "\","
                << "\"t\":" << o.t << ","
                << "\"p\":\"" << o.p << "\","
                << "\"q\":\"" << o.q << "\","
                << "\"T\":" << o.T << ","
                << "\"m\":" << (o.m ? "true" : "false") << ","
                << "\"M\":" << (o.M ? "true" : "false")
                << "}";

            return os;
        }

        /*************************************************************************************************/

        std::ostream &operator<<(std::ostream &os, const part_depths_t::depth_t &o)
        {
            (void)o;

            return os;
        }

        part_depths_t part_depths_t::construct(const flatjson::fjson &json)
        {
            assert(json.is_valid());

            part_depths_t res{};
            const auto a = json.at("asks");
            for (auto idx = 0u; idx < a.size(); ++idx)
            {
                part_depths_t::depth_t item{};
                const auto it = a.at(idx);
                item.price.assign(it.at(0).to_string());
                item.amount.assign(it.at(1).to_string());

                res.a.push_back(std::move(item));
            }
            const auto b = json.at("bids");
            for (auto idx = 0u; idx < b.size(); ++idx)
            {
                part_depths_t::depth_t item{};
                const auto it = b.at(idx);
                item.price.assign(it.at(0).to_string());
                item.amount.assign(it.at(1).to_string());

                res.b.push_back(std::move(item));
            }

            return res;
        }

        std::ostream &operator<<(std::ostream &os, const part_depths_t &o)
        {
            os
                << "{"
                << "\"asks\":[";
            for (auto it = o.a.begin(); it != o.a.end(); ++it)
            {
                os << "[\"" << it->price << "\", \"" << it->amount << "\"]";
                if (std::next(it) != o.a.end())
                {
                    os << ",";
                }
            }
            os
                << "],"
                << "\"bids\":[";
            for (auto it = o.b.begin(); it != o.b.end(); ++it)
            {
                os << "[\"" << it->price << "\", \"" << it->amount << "\"]";
                if (std::next(it) != o.b.end())
                {
                    os << ",";
                }
            }
            os
                << "]}";

            return os;
        }

        /*************************************************************************************************/

        std::ostream &operator<<(std::ostream &os, const diff_depths_t::depth_t &o)
        {
            (void)o;

            return os;
        }

        diff_depths_t diff_depths_t::construct(const flatjson::fjson &json)
        {
            assert(json.is_valid());

            diff_depths_t res{};
            __KRAPI_GET(E);
            __KRAPI_GET(s);
            __KRAPI_GET(u);
            __KRAPI_GET(U);
            const auto a = json.at("a");
            for (auto idx = 0u; idx < a.size(); ++idx)
            {
                diff_depths_t::depth_t item{};
                const auto it = a.at(idx);
                item.price.assign(it.at(0).to_string());
                item.amount.assign(it.at(1).to_string());

                res.a.push_back(std::move(item));
            }
            const auto b = json.at("b");
            for (auto idx = 0u; idx < b.size(); ++idx)
            {
                diff_depths_t::depth_t item{};
                const auto it = b.at(idx);
                item.price.assign(it.at(0).to_string());
                item.amount.assign(it.at(1).to_string());

                res.b.push_back(std::move(item));
            }

            return res;
        }

        std::ostream &operator<<(std::ostream &os, const diff_depths_t &o)
        {
            os
                << "{"
                << "\"E\":" << o.E << ","
                << "\"s\":\"" << o.s << "\","
                << "\"u\":" << o.u << ","
                << "\"U\":" << o.U << ","
                << "\"a\":[";
            for (auto it = o.a.begin(); it != o.a.end(); ++it)
            {
                os << "[\"" << it->price << "\", \"" << it->amount << "\"]";
                if (std::next(it) != o.a.end())
                {
                    os << ",";
                }
            }
            os
                << "],"
                << "\"b\":[";
            for (auto it = o.b.begin(); it != o.b.end(); ++it)
            {
                os << "[\"" << it->price << "\", \"" << it->amount << "\"]";
                if (std::next(it) != o.b.end())
                {
                    os << ",";
                }
            }
            os
                << "]}";

            return os;
        }

        /*************************************************************************************************/

        kline_t kline_t::construct(const flatjson::fjson &json)
        {
            assert(json.is_valid());

            kline_t res{};
            __KRAPI_GET(E);
            __KRAPI_GET(s);

            const auto k = json.at("k");
            __KRAPI_GET2(res, t, k);
            __KRAPI_GET2(res, T, k);
            __KRAPI_GET2(res, i, k);
            __KRAPI_GET2(res, f, k);
            __KRAPI_GET2(res, L, k);
            __KRAPI_GET2(res, o, k);
            __KRAPI_GET2(res, c, k);
            __KRAPI_GET2(res, h, k);
            __KRAPI_GET2(res, l, k);
            __KRAPI_GET2(res, v, k);
            __KRAPI_GET2(res, n, k);
            __KRAPI_GET2(res, x, k);
            __KRAPI_GET2(res, q, k);
            __KRAPI_GET2(res, V, k);
            __KRAPI_GET2(res, Q, k);

            return res;
        }

        std::ostream &operator<<(std::ostream &os, const kline_t &o)
        {
            os
                << "{"
                << "\"E\": " << o.E << ","
                << "\"s\": \"" << o.s << "\","
                << "\"t\": " << o.t << ","
                << "\"T\": " << o.T << ","
                << "\"i\": \"" << o.i << "\","
                << "\"f\": " << o.f << ","
                << "\"L\": " << o.L << ","
                << "\"o\": \"" << o.o << "\","
                << "\"c\": \"" << o.c << "\","
                << "\"h\": \"" << o.h << "\","
                << "\"l\": \"" << o.l << "\","
                << "\"v\": \"" << o.v << "\","
                << "\"n\": " << o.n << ","
                << "\"x\": " << (o.x ? "true" : "false") << ","
                << "\"q\": \"" << o.q << "\","
                << "\"V\": \"" << o.V << "\","
                << "\"Q\": \"" << o.Q << "\""
                << "}";

            return os;
        }

        bool ohlc_equal(const kline_t &l, const kline_t &r)
        {
            return l.o == r.o && l.h == r.h && l.l == r.l && l.c == r.c;
        }

        std::ostream &ohlc(std::ostream &os, const kline_t &o)
        {
            os
                << "{"
                << "\"o\": \"" << o.o << "\","
                << "\"h\": \"" << o.h << "\","
                << "\"l\": \"" << o.l << "\","
                << "\"c\": \"" << o.c
                << "}";

            return os;
        }

        /*************************************************************************************************/

        mini_ticker_t mini_ticker_t::construct(const flatjson::fjson &json)
        {
            mini_ticker_t res{};
            __KRAPI_GET(E);
            __KRAPI_GET(s);
            __KRAPI_GET(c);
            __KRAPI_GET(o);
            __KRAPI_GET(h);
            __KRAPI_GET(l);
            __KRAPI_GET(v);
            __KRAPI_GET(q);

            return res;
        }

        std::ostream &operator<<(std::ostream &os, const mini_ticker_t &o)
        {
            os
                << "{"
                << "\"E:\"" << o.E << ","
                << "\"s\":\"" << o.s << "\","
                << "\"c\":\"" << o.c << "\","
                << "\"o\":\"" << o.o << "\","
                << "\"h\":\"" << o.h << "\","
                << "\"l\":\"" << o.l << "\","
                << "\"v\":\"" << o.v << "\","
                << "\"q\":\"" << o.q << "\""
                << "}";

            return os;
        }

        mini_tickers_t mini_tickers_t::construct(const flatjson::fjson &json)
        {
            assert(json.is_array());

            mini_tickers_t res{};
            for (auto idx = 0u; idx < json.size(); ++idx)
            {
                const auto it = json.at(idx);
                std::string symbol = it.at("s").to_string();
                mini_ticker_t item = mini_ticker_t::construct(it);
                res.tickers.emplace(std::move(symbol), std::move(item));
            }

            return res;
        }

        std::ostream &operator<<(std::ostream &os, const mini_tickers_t &o)
        {
            os << "[";
            for (auto it = o.tickers.begin(); it != o.tickers.end(); ++it)
            {
                os << it->second;
                if (std::next(it) != o.tickers.end())
                {
                    os << ",";
                }
            }
            os << "]";

            return os;
        }

        /*************************************************************************************************/

        market_ticker_t market_ticker_t::construct(const flatjson::fjson &json)
        {
            market_ticker_t res{};
            __KRAPI_GET(E);
            __KRAPI_GET(s);
            __KRAPI_GET(p);
            __KRAPI_GET(P);
            __KRAPI_GET(w);
            __KRAPI_GET(x);
            __KRAPI_GET(c);
            __KRAPI_GET(Q);
            __KRAPI_GET(b);
            __KRAPI_GET(B);
            __KRAPI_GET(a);
            __KRAPI_GET(A);
            __KRAPI_GET(o);
            __KRAPI_GET(h);
            __KRAPI_GET(l);
            __KRAPI_GET(v);
            __KRAPI_GET(q);
            __KRAPI_GET(O);
            __KRAPI_GET(C);
            __KRAPI_GET(F);
            __KRAPI_GET(L);
            __KRAPI_GET(n);

            return res;
        }

        std::ostream &operator<<(std::ostream &os, const market_ticker_t &o)
        {
            os
                << "{"
                << "\"E\":" << o.E << ","
                << "\"s\":\"" << o.s << "\","
                << "\"p\":\"" << o.p << "\","
                << "\"P\":\"" << o.P << "\","
                << "\"w\":\"" << o.w << "\","
                << "\"x\":\"" << o.x << "\","
                << "\"c\":\"" << o.c << "\","
                << "\"Q\":\"" << o.Q << "\","
                << "\"b\":\"" << o.b << "\","
                << "\"B\":\"" << o.B << "\","
                << "\"a\":\"" << o.a << "\","
                << "\"A\":\"" << o.A << "\","
                << "\"o\":\"" << o.o << "\","
                << "\"h\":\"" << o.h << "\","
                << "\"l\":\"" << o.l << "\","
                << "\"v\":\"" << o.v << "\","
                << "\"q\":\"" << o.q << "\","
                << "\"O\":" << o.O << ","
                << "\"C\":" << o.C << ","
                << "\"F\":" << o.F << ","
                << "\"L\":" << o.L << ","
                << "\"n\":" << o.n
                << "}";

            return os;
        }

        /*************************************************************************************************/

        markets_tickers_t markets_tickers_t::construct(const flatjson::fjson &json)
        {
            assert(json.is_array());

            markets_tickers_t res{};
            for (auto idx = 0u; idx < json.size(); ++idx)
            {
                market_ticker_t item{};
                const auto it = json.at(idx);
                __KRAPI_GET2(item, E, it);
                __KRAPI_GET2(item, s, it);
                __KRAPI_GET2(item, p, it);
                __KRAPI_GET2(item, P, it);
                __KRAPI_GET2(item, w, it);
                __KRAPI_GET2(item, x, it);
                __KRAPI_GET2(item, c, it);
                __KRAPI_GET2(item, Q, it);
                __KRAPI_GET2(item, b, it);
                __KRAPI_GET2(item, B, it);
                __KRAPI_GET2(item, a, it);
                __KRAPI_GET2(item, A, it);
                __KRAPI_GET2(item, o, it);
                __KRAPI_GET2(item, h, it);
                __KRAPI_GET2(item, l, it);
                __KRAPI_GET2(item, v, it);
                __KRAPI_GET2(item, q, it);
                __KRAPI_GET2(item, O, it);
                __KRAPI_GET2(item, C, it);
                __KRAPI_GET2(item, F, it);
                __KRAPI_GET2(item, L, it);
                __KRAPI_GET2(item, n, it);

                std::string symbol = item.s;
                res.tickers.emplace(std::move(symbol), std::move(item));
            }

            return res;
        }

        std::ostream &operator<<(std::ostream &os, const markets_tickers_t &o)
        {
            os << "[";
            for (auto it = o.tickers.begin(); it != o.tickers.end(); ++it)
            {
                os << it->second;
                if (std::next(it) != o.tickers.end())
                {
                    os << ",";
                }
            }
            os << "]";

            return os;
        }

        /*************************************************************************************************/

        book_ticker_t book_ticker_t::construct(const flatjson::fjson &json)
        {
            assert(json.is_valid());

            book_ticker_t res{};
            __KRAPI_GET(u);
            __KRAPI_GET(s);
            __KRAPI_GET(b);
            __KRAPI_GET(B);
            __KRAPI_GET(a);
            __KRAPI_GET(A);

            return res;
        }

        std::ostream &operator<<(std::ostream &os, const book_ticker_t &o)
        {
            os
                << "{"
                << "\"u\":" << o.u << ","
                << "\"s\":\"" << o.s << "\","
                << "\"b\":\"" << o.b << "\","
                << "\"B\":\"" << o.B << "\","
                << "\"a\":\"" << o.a << "\","
                << "\"A\":\"" << o.A << "\""
                << "}";

            return os;
        }

        /*************************************************************************************************/
        /*************************************************************************************************/
        /*************************************************************************************************/

    } // ns ws

    /*************************************************************************************************/
    /*************************************************************************************************/
    /*************************************************************************************************/

    namespace userdata
    {

        std::ostream &operator<<(std::ostream &os, const account_update_t::balance_t &o)
        {
            os
                << "{"
                << "\"a\":\"" << o.a << "\","
                << "\"f\":\"" << o.f << "\","
                << "\"l\":\"" << o.l << "\""
                << "}";

            return os;
        }

        account_update_t account_update_t::construct(const flatjson::fjson &json)
        {
            assert(json.is_valid());

            account_update_t res{};
            __KRAPI_GET(e);
            __KRAPI_GET(E);
            __KRAPI_GET(u);
            const auto B = json.at("B");
            for (auto idx = 0u; idx < B.size(); ++idx)
            {
                account_update_t::balance_t item{};
                const auto it = B.at(idx);
                __KRAPI_GET2(item, a, it);
                __KRAPI_GET2(item, l, it);
                __KRAPI_GET2(item, f, it);

                res.B[item.a] = std::move(item);
            }

            return res;
        }

        std::ostream &operator<<(std::ostream &os, const account_update_t &o)
        {
            os
                << "{"
                << "\"e\":\"" << o.e << "\","
                << "\"E\":" << o.E << ","
                << "\"u\":" << o.u << ","
                << "\"B\":[";
            for (auto it = o.B.begin(); it != o.B.end(); ++it)
            {
                os << it->second;
                if (std::next(it) != o.B.end())
                {
                    os << ",";
                }
            }

            os << "]}";

            return os;
        }

        /*************************************************************************************************/

        balance_update_t balance_update_t::construct(const flatjson::fjson &json)
        {
            assert(json.is_valid());

            balance_update_t res{};
            __KRAPI_GET(e);
            __KRAPI_GET(E);
            __KRAPI_GET(a);
            __KRAPI_GET(d);
            __KRAPI_GET(T);

            return res;
        }

        std::ostream &operator<<(std::ostream &os, const balance_update_t &o)
        {
            os
                << "{"
                << "\"e\":\"" << o.e << "\","
                << "\"E\":" << o.E << ","
                << "\"a\":\"" << o.a << "\","
                << "\"d\":\"" << o.d << "\","
                << "\"T\":" << o.T
                << "}";

            return os;
        }

        /*************************************************************************************************/

        order_update_t order_update_t::construct(const flatjson::fjson &json)
        {
            assert(json.is_valid());

            order_update_t res{};
            __KRAPI_GET(e);
            __KRAPI_GET(E);
            __KRAPI_GET(s);
            __KRAPI_GET(c);
            __KRAPI_GET(S);
            __KRAPI_GET(o);
            __KRAPI_GET(f);
            __KRAPI_GET(q);
            __KRAPI_GET(p);
            __KRAPI_GET(P);
            __KRAPI_GET(F);
            __KRAPI_GET(C);
            __KRAPI_GET(x);
            __KRAPI_GET(X);
            __KRAPI_GET(r);
            __KRAPI_GET(i);
            __KRAPI_GET(l);
            __KRAPI_GET(z);
            __KRAPI_GET(L);
            __KRAPI_GET(n);
            __KRAPI_GET(N);
            __KRAPI_GET(T);
            __KRAPI_GET(t);
            __KRAPI_GET(I);
            __KRAPI_GET(w);
            __KRAPI_GET(m);
            __KRAPI_GET(M);
            __KRAPI_GET(O);
            __KRAPI_GET(Z);

            return res;
        }

        std::ostream &operator<<(std::ostream &os, const order_update_t &o)
        {
            os
                << "{"
                << "\"e\":\"" << o.e << "\","
                << "\"E\":" << o.E << ","
                << "\"s\":\"" << o.s << "\","
                << "\"c\":\"" << o.c << "\","
                << "\"S\":\"" << o.S << "\","
                << "\"o\":\"" << o.o << "\","
                << "\"f\":\"" << o.f << "\","
                << "\"q\":\"" << o.q << "\","
                << "\"p\":\"" << o.p << "\","
                << "\"P\":\"" << o.P << "\","
                << "\"F\":\"" << o.F << "\","
                << "\"C\":\"" << o.C << "\","
                << "\"x\":\"" << o.x << "\","
                << "\"X\":\"" << o.X << "\","
                << "\"r\":\"" << o.r << "\","
                << "\"i\":" << o.i << ","
                << "\"l\":\"" << o.l << "\","
                << "\"z\":\"" << o.z << "\","
                << "\"L\":\"" << o.L << "\","
                << "\"n\":\"" << o.n << "\","
                << "\"N\":\"" << o.N << "\","
                << "\"T\":" << o.T << ","
                << "\"t\":" << o.t << ","
                << "\"I\":" << o.I << ","
                << "\"w\":" << (o.w ? "true" : "false") << ","
                << "\"m\":" << (o.m ? "true" : "false") << ","
                << "\"M\":" << (o.M ? "true" : "false") << ","
                << "\"O\":" << o.O << ","
                << "\"Z\":\"" << o.Z << "\""
                << "}";

            return os;
        }

        /*************************************************************************************************/

        userdata_stream_t userdata_stream_t::construct(const flatjson::fjson &json)
        {
            userdata_stream_t res{};
            const auto json_src = json.get_source_data();
            res.data.assign(json_src.first, json_src.second);

            return res;
        }

        std::ostream &operator<<(std::ostream &os, const userdata_stream_t &o)
        {
            (void)o;

            return os;
        }

        /*************************************************************************************************/
        /*************************************************************************************************/
        /*************************************************************************************************/

    } // ns userdata
} // ns krapi
