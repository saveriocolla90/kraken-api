
// ----------------------------------------------------------------------------
//                              Apache License
//                        Version 2.0, January 2004
//                     http://www.apache.org/licenses/
//
// This file is part of binapi(https://github.com/niXman/binapi) project.
//
// Copyright (c) 2019-2021 niXman (github dot nixman dog pm.me). All rights reserved.
// ----------------------------------------------------------------------------

#ifndef __binapi__types_hpp
#define __binapi__types_hpp

#include "double_type.hpp"
#include "enums.hpp"

#include <boost/variant.hpp>

#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include <cassert>

// forward
namespace flatjson {
struct fjson;
} // ns flatjson

namespace binapi {

/*************************************************************************************************/

namespace rest {

// https://docs.kraken.com/rest/#tag/Market-Data/operation/getServerTime
// Kraken `public/Time`: {"error":[],"result":{"unixtime":1616336594,"rfc1123":"..."}}
struct server_time_t {
    std::size_t unixtime;   // seconds since epoch
    std::string rfc1123;    // human-readable server time

    static server_time_t construct(const flatjson::fjson &json);
    friend std::ostream &operator<<(std::ostream &os, const server_time_t &f);
};

// https://docs.kraken.com/rest/#tag/Account-Data/operation/getAccountBalance
// Kraken `private/Balance`: result is an object of asset -> amount, e.g.
//   {"error":[],"result":{"ZUSD":"171.3900","XXBT":"0.0123456700"}}
struct balances_t {
    std::map<std::string, double_type> balances; // asset name -> total balance

    bool has(const std::string &asset) const { return balances.count(asset) != 0; }
    const double_type& get(const std::string &asset) const;

    static balances_t construct(const flatjson::fjson &json);
    friend std::ostream &operator<<(std::ostream &os, const balances_t &f);
};

// https://docs.kraken.com/rest/#tag/Market-Data/operation/getTickerInformation
// Kraken `public/Ticker`: result is pairname -> ticker object, where each value is
//   {"a":[ask,wlv,lv],"b":[bid,wlv,lv],"c":[last,lv],"v":[today,24h],"p":[today,24h],
//    "t":[today,24h],"l":[today,24h],"h":[today,24h],"o":open}
struct tickers_t {
    struct ticker_t {
        std::string pair;          // result key, e.g. "XXBTZUSD"
        double_type ask;           // a[0]
        double_type bid;           // b[0]
        double_type last;          // c[0] last trade price
        double_type volume_today;  // v[0]
        double_type volume_24h;    // v[1]
        double_type vwap_today;    // p[0]
        double_type vwap_24h;      // p[1]
        std::size_t trades_today;  // t[0]
        std::size_t trades_24h;    // t[1]
        double_type low_today;     // l[0]
        double_type low_24h;       // l[1]
        double_type high_today;    // h[0]
        double_type high_24h;      // h[1]
        double_type open;          // o

        static ticker_t construct(const std::string &pair, const flatjson::fjson &json);
        friend std::ostream &operator<<(std::ostream &os, const ticker_t &f);
    };

    std::map<std::string, ticker_t> tickers;

    bool has(const std::string &pair) const { return tickers.count(pair) != 0; }
    const ticker_t& get(const std::string &pair) const;

    static tickers_t construct(const flatjson::fjson &json);
    friend std::ostream &operator<<(std::ostream &os, const tickers_t &f);
};

// https://docs.kraken.com/rest/#tag/Market-Data/operation/getTradableAssetPairs
// Kraken `public/AssetPairs`: result is pairname -> pair metadata object.
struct asset_pairs_t {
    struct asset_pair_t {
        std::string name;            // result key, e.g. "XXBTZUSD"
        std::string altname;         // e.g. "XBTUSD"
        std::string wsname;          // e.g. "XBT/USD" (may be absent)
        std::string base;            // base asset id, e.g. "XXBT"
        std::string quote;           // quote asset id, e.g. "ZUSD"
        std::size_t pair_decimals;   // price scale
        std::size_t lot_decimals;    // volume scale
        double_type ordermin;        // minimum order volume (may be absent)
        std::string status;          // "online", "cancel_only", ...

        static asset_pair_t construct(const std::string &name, const flatjson::fjson &json);
        friend std::ostream &operator<<(std::ostream &os, const asset_pair_t &f);
    };

    std::map<std::string, asset_pair_t> pairs;

    bool has(const std::string &name) const { return pairs.count(name) != 0; }
    const asset_pair_t& get(const std::string &name) const;

    static asset_pairs_t construct(const flatjson::fjson &json);
    friend std::ostream &operator<<(std::ostream &os, const asset_pairs_t &f);
};

// https://docs.kraken.com/rest/#tag/Trading/operation/addOrder
// Kraken `private/AddOrder` reply: {"descr":{"order":"..."},"txid":["..."]}
// (with validate=true there is no txid, only descr).
struct add_order_t {
    std::string descr;                   // descr.order — human-readable description
    std::vector<std::string> txid;       // transaction ids of the placed order(s)

    static add_order_t construct(const flatjson::fjson &json);
    friend std::ostream &operator<<(std::ostream &os, const add_order_t &f);
};

// https://docs.kraken.com/rest/#tag/Trading/operation/cancelOrder
// Kraken `private/CancelOrder` reply: {"count":1} (+ optional "pending":true)
struct cancel_order_t {
    std::size_t count;   // number of orders canceled
    bool pending;        // true if cancellation is still being processed

    static cancel_order_t construct(const flatjson::fjson &json);
    friend std::ostream &operator<<(std::ostream &os, const cancel_order_t &f);
};

// A single Kraken order, shared by OpenOrders and ClosedOrders.
struct kraken_order_t {
    std::string txid;        // result key (the order's transaction id)
    std::string status;      // open/closed/canceled/pending/expired
    std::size_t userref;     // user reference id (0 if unset)
    double opentm;           // open timestamp (fractional unix seconds)
    double closetm;          // close timestamp (closed orders only; 0 otherwise)
    double starttm;          // scheduled start (0 if immediate)
    double expiretm;         // expiration (0 if none)

    struct descr_t {
        std::string pair;
        std::string type;        // buy/sell
        std::string ordertype;   // limit/market/...
        double_type price;
        double_type price2;
        std::string leverage;
        std::string order;       // human-readable order description
        std::string close;       // conditional close description
    } descr;

    double_type vol;         // order volume
    double_type vol_exec;    // executed volume
    double_type cost;        // total cost
    double_type fee;         // total fee
    double_type price;       // average price
    double_type stopprice;   // stop price
    double_type limitprice;  // triggered limit price
    std::string misc;
    std::string oflags;
    std::string reason;      // close/cancel reason (closed orders; may be empty)

    static kraken_order_t construct(const std::string &txid, const flatjson::fjson &json);
    friend std::ostream &operator<<(std::ostream &os, const kraken_order_t &f);
};

// https://docs.kraken.com/rest/#tag/Account-Data/operation/getOpenOrders
struct open_orders_t {
    std::map<std::string, kraken_order_t> orders; // txid -> order

    static open_orders_t construct(const flatjson::fjson &json);
    friend std::ostream &operator<<(std::ostream &os, const open_orders_t &f);
};

// https://docs.kraken.com/rest/#tag/Account-Data/operation/getClosedOrders
struct closed_orders_t {
    std::map<std::string, kraken_order_t> orders; // txid -> order
    std::size_t count;                            // total count available

    static closed_orders_t construct(const flatjson::fjson &json);
    friend std::ostream &operator<<(std::ostream &os, const closed_orders_t &f);
};

// https://docs.kraken.com/rest/#tag/Account-Data/operation/getTradeHistory
struct trades_history_t {
    struct trade_t {
        std::string txid;        // result key (trade id)
        std::string ordertxid;   // order responsible for the trade
        std::string postxid;     // position id
        std::string pair;
        double time;             // trade timestamp (fractional unix seconds)
        std::string type;        // buy/sell
        std::string ordertype;   // limit/market/...
        double_type price;
        double_type cost;
        double_type fee;
        double_type vol;
        double_type margin;
        std::string misc;

        static trade_t construct(const std::string &txid, const flatjson::fjson &json);
        friend std::ostream &operator<<(std::ostream &os, const trade_t &f);
    };

    std::map<std::string, trade_t> trades; // txid -> trade
    std::size_t count;                     // total count available

    static trades_history_t construct(const flatjson::fjson &json);
    friend std::ostream &operator<<(std::ostream &os, const trades_history_t &f);
};

// https://docs.kraken.com/rest/#tag/Market-Data/operation/getOrderBook
// Kraken `public/Depth`: result is pairname -> {"asks":[[price,vol,tm],...],"bids":[...]}
struct order_book_t {
    struct level_t {
        double_type price;
        double_type volume;
        std::size_t timestamp;   // unix seconds

        friend std::ostream &operator<<(std::ostream &os, const level_t &f);
    };

    std::string pair;            // result key, e.g. "XXBTZUSD"
    std::vector<level_t> asks;
    std::vector<level_t> bids;

    static order_book_t construct(const flatjson::fjson &json);
    friend std::ostream &operator<<(std::ostream &os, const order_book_t &f);
};

// https://docs.kraken.com/rest/#tag/Market-Data/operation/getOHLCData
// Kraken `public/OHLC`: result is {pairname:[[time,o,h,l,c,vwap,vol,count],...],"last":id}
struct ohlc_t {
    struct candle_t {
        std::size_t time;        // unix seconds (interval start)
        double_type open;
        double_type high;
        double_type low;
        double_type close;
        double_type vwap;
        double_type volume;
        std::size_t count;       // number of trades

        friend std::ostream &operator<<(std::ostream &os, const candle_t &f);
    };

    std::string pair;            // result key, e.g. "XXBTZUSD"
    std::vector<candle_t> candles;
    std::size_t last;            // id to pass as `since` for the next request

    static ohlc_t construct(const flatjson::fjson &json);
    friend std::ostream &operator<<(std::ostream &os, const ohlc_t &f);
};

// https://docs.kraken.com/rest/#tag/Market-Data/operation/getRecentTrades
// Kraken `public/Trades`: result is {pairname:[[price,vol,time,side,type,misc,id],...],"last":id}
struct recent_trades_t {
    struct trade_t {
        double_type price;
        double_type volume;
        double time;             // fractional unix seconds
        std::string side;        // "b" (buy) / "s" (sell)
        std::string type;        // "m" (market) / "l" (limit)
        std::string misc;
        std::size_t trade_id;

        friend std::ostream &operator<<(std::ostream &os, const trade_t &f);
    };

    std::string pair;            // result key, e.g. "XXBTZUSD"
    std::vector<trade_t> trades;
    std::string last;            // nanosecond id (string) to page from

    static recent_trades_t construct(const flatjson::fjson &json);
    friend std::ostream &operator<<(std::ostream &os, const recent_trades_t &f);
};

/*************************************************************************************************/

} // ns rest

/*************************************************************************************************/
/*************************************************************************************************/
/*************************************************************************************************/

namespace ws {

/*************************************************************************************************/

// https://github.com/binance-exchange/binance-official-api-docs/blob/master/web-socket-streams.md#aggregate-trade-streams
struct agg_trade_t {
    std::string e; // Event type
    std::size_t E; // Event time
    std::string s; // Symbol
    std::size_t a; // Aggregate trade ID
    double_type p; // Price
    double_type q; // Quantity
    std::size_t f; // First trade ID
    std::size_t l; // Last trade ID
    std::size_t T; // Trade time
    bool m; // Is the buyer the market maker?
    bool M; // Ignore

    static agg_trade_t construct(const flatjson::fjson &json);
    friend std::ostream &operator<<(std::ostream &os, const agg_trade_t &o);
};

// https://github.com/binance-exchange/binance-official-api-docs/blob/master/web-socket-streams.md#trade-streams
struct trade_t {
    std::size_t E; // Event time
    std::string s; // Symbol
    std::size_t t; // Trade ID
    double_type p; // Price
    double_type q; // Quantity
    std::size_t T; // Trade time
    bool m; // Is the buyer the market maker?
    bool M; // Ignore

    static trade_t construct(const flatjson::fjson &json);
    friend std::ostream &operator<<(std::ostream &os, const trade_t &o);
};

/*************************************************************************************************/

// https://github.com/binance/binance-spot-api-docs/blob/master/web-socket-streams.md#partial-book-depth-streams
struct part_depths_t {
    struct depth_t {
        double_type price;
        double_type amount;

        friend std::ostream &operator<<(std::ostream &os, const depth_t &o);
    };

    std::vector<depth_t> a;
    std::vector<depth_t> b;

    static part_depths_t construct(const flatjson::fjson &json);
    friend std::ostream& operator<<(std::ostream &os, const part_depths_t &o);
};

/*************************************************************************************************/

// https://github.com/binance-exchange/binance-official-api-docs/blob/master/web-socket-streams.md#diff-depth-stream
struct diff_depths_t {
    struct depth_t {
        double_type price;
        double_type amount;

        friend std::ostream &operator<<(std::ostream &os, const depth_t &o);
    };

    std::size_t E;
    std::string s;
    std::size_t u;
    std::size_t U;
    std::vector<depth_t> a;
    std::vector<depth_t> b;

    static diff_depths_t construct(const flatjson::fjson &json);
    friend std::ostream& operator<<(std::ostream &os, const diff_depths_t &o);
};

/*************************************************************************************************/

// https://github.com/binance-exchange/binance-official-api-docs/blob/master/web-socket-streams.md#klinecandlestick-streams
struct kline_t {
    std::size_t E; // Event time
    std::string s; // Symbol
    std::size_t t; // Kline start time
    std::size_t T; // Kline close time
    std::string i; // Interval
    std::size_t f; // First trade ID
    std::size_t L; // Last trade ID
    double_type o; // Open price
    double_type c; // Close price
    double_type h; // High price
    double_type l; // Low price
    double_type v; // Base asset volume
    std::size_t n; // Number of trades
    bool        x; // Is this kline closed?
    double_type q; // Quote asset volume
    double_type V; // Taker buy base asset volume
    double_type Q; // Taker buy quote asset volume

    static kline_t construct(const flatjson::fjson &json);
    friend std::ostream& operator<<(std::ostream &os, const kline_t &o);
    friend bool ohlc_equal(const kline_t &l, const kline_t &r);
};

std::ostream& ohlc(std::ostream &os, const kline_t &o);

/*************************************************************************************************/

// https://github.com/binance/binance-spot-api-docs/blob/master/web-socket-streams.md#individual-symbol-mini-ticker-stream
struct mini_ticker_t {
    std::size_t E; // Event time
    std::string s; // Symbol
    double_type c; // Close price
    double_type o; // Open price
    double_type h; // High price
    double_type l; // Low price
    double_type v; // Total traded base asset volume
    double_type q; // Total traded quote asset volume

    static mini_ticker_t construct(const flatjson::fjson &json);
    friend std::ostream& operator<<(std::ostream &os, const mini_ticker_t &o);
};

// https://github.com/binance/binance-spot-api-docs/blob/master/web-socket-streams.md#all-market-mini-tickers-stream
struct mini_tickers_t {
    std::map<std::string, mini_ticker_t> tickers;

    static mini_tickers_t construct(const flatjson::fjson &json);
    friend std::ostream& operator<<(std::ostream &os, const mini_tickers_t &o);
};

/*************************************************************************************************/

// https://github.com/binance-exchange/binance-official-api-docs/blob/master/web-socket-streams.md#individual-symbol-ticker-streams
struct market_ticker_t {
    std::size_t E; // Event time
    std::string s; // Symbol
    double_type p; // Price change
    double_type P; // Price change percent
    double_type w; // Weighted average price
    double_type x; // First trade(F)-1 price (first trade before the 24hr rolling window)
    double_type c; // Last price
    double_type Q; // Last quantity
    double_type b; // Best bid price
    double_type B; // Best bid quantity
    double_type a; // Best ask price
    double_type A; // Best ask quantity
    double_type o; // Open price
    double_type h; // High price
    double_type l; // Low price
    double_type v; // Total traded base asset volume
    double_type q; // Total traded quote asset volume
    std::size_t O; // Statistics open time
    std::size_t C; // Statistics close time
    std::size_t F; // First trade ID
    std::size_t L; // Last trade Id
    std::size_t n; // Total number of trades

    static market_ticker_t construct(const flatjson::fjson &json);
    friend std::ostream& operator<<(std::ostream &os, const market_ticker_t &o);
};

// https://github.com/binance-exchange/binance-official-api-docs/blob/master/web-socket-streams.md#all-market-tickers-stream
struct markets_tickers_t {
    std::map<std::string, market_ticker_t> tickers;

    static markets_tickers_t construct(const flatjson::fjson &json);
    friend std::ostream& operator<<(std::ostream &os, const markets_tickers_t &o);
};

/*************************************************************************************************/

// https://github.com/binance/binance-spot-api-docs/blob/master/web-socket-streams.md#individual-symbol-book-ticker-streams
struct book_ticker_t {
    std::size_t u;
    std::string s;
    double_type b;
    double_type B;
    double_type a;
    double_type A;

    static book_ticker_t construct(const flatjson::fjson &json);
    friend std::ostream& operator<<(std::ostream &os, const book_ticker_t &o);
};

/*************************************************************************************************/

} // ns ws

/*************************************************************************************************/
/*************************************************************************************************/
/*************************************************************************************************/

namespace userdata {

/*************************************************************************************************/

// https://github.com/binance/binance-spot-api-docs/blob/master/user-data-stream.md#account-update
struct account_update_t {
    struct balance_t {
        std::string a;
        double_type f;
        double_type l;

        friend std::ostream& operator<<(std::ostream &os, const balance_t &o);
    };

    std::string e;
    std::size_t E;
    std::size_t u;
    std::map<std::string, balance_t> B;

    static account_update_t construct(const flatjson::fjson &json);
    friend std::ostream& operator<<(std::ostream &os, const account_update_t &o);
};

/*************************************************************************************************/

// https://github.com/binance/binance-spot-api-docs/blob/master/user-data-stream.md#balance-update
struct balance_update_t {
    std::string e;
    std::size_t E;
    std::string a;
    double_type d;
    std::size_t T;

    static balance_update_t construct(const flatjson::fjson &json);
    friend std::ostream& operator<<(std::ostream &os, const balance_update_t &o);
};

/*************************************************************************************************/

// https://github.com/binance/binance-spot-api-docs/blob/master/user-data-stream.md#order-update
struct order_update_t {
    std::string e;
    std::size_t E;
    std::string s;
    std::string c;
    std::string S;
    std::string o;
    std::string f;
    double_type q;
    double_type p;
    double_type P;
    double_type F;
    std::string C;
    std::string x;
    std::string X;
    std::string r;
    std::size_t i;
    double_type l;
    double_type z;
    double_type L;
    double_type n;
    std::string N;
    std::size_t T;
    std::size_t t;
    std::size_t I;
    bool        w;
    bool        m;
    bool        M;
    std::size_t O;
    double_type Z;

    static order_update_t construct(const flatjson::fjson &json);
    friend std::ostream& operator<<(std::ostream &os, const order_update_t &o);
};

/*************************************************************************************************/

// wrapper for account_update_t and order_update_t
struct userdata_stream_t {
    std::string data;

    static userdata_stream_t construct(const flatjson::fjson &json);
    friend std::ostream& operator<<(std::ostream &os, const userdata_stream_t &o);
};

/*************************************************************************************************/

} // ns userdata

/*************************************************************************************************/
/*************************************************************************************************/
/*************************************************************************************************/

} // ns binapi

#endif // __binapi__types_hpp
