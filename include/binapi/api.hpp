
// ----------------------------------------------------------------------------
//                              Apache License
//                        Version 2.0, January 2004
//                     http://www.apache.org/licenses/
//
// This file is part of binapi(https://github.com/niXman/binapi) project.
//
// Copyright (c) 2019-2021 niXman (github dot nixman dog pm.me). All rights reserved.
// ----------------------------------------------------------------------------

#ifndef __binapi__api_hpp
#define __binapi__api_hpp

#include "types.hpp"
#include "enums.hpp"

#include <cstdint>
#include <memory>
#include <functional>

namespace boost {
namespace asio {

class io_context;

} // ns asio
} // ns boost

namespace binapi {
namespace rest {

/*************************************************************************************************/

struct api {
    template<typename T>
    struct result {
        result()
            :ec{0}
        {}

        int ec;
        std::string errmsg;
        std::string reply;
        T v;

        // returns FALSE when error
        explicit operator bool() const { return errmsg.empty(); }
    };

    api(
         boost::asio::io_context &ioctx
        ,std::string host
        ,std::string port
        ,std::string pk
        ,std::string sk
        ,std::size_t timeout
        ,std::string client_api_string = "binapi-0.0.1"
    );
    virtual ~api();

    api(const api &) = delete;
    api(api &&) = default;

    // https://docs.kraken.com/rest/#tag/Market-Data/operation/getServerTime
    using server_time_cb = std::function<bool(const char *fl, int ec, std::string errmsg, server_time_t res)>;
    result<server_time_t>
    server_time(server_time_cb cb = {});

    // https://docs.kraken.com/rest/#tag/Account-Data/operation/getAccountBalance
    // Kraken `private/Balance` — account balances per asset.
    using balances_cb = std::function<bool(const char *fl, int ec, std::string errmsg, balances_t res)>;
    result<balances_t>
    balances(balances_cb cb = {});

    // https://docs.kraken.com/rest/#tag/Market-Data/operation/getTickerInformation
    // Kraken `public/Ticker` — ticker for one pair, or all pairs when omitted.
    using tickers_cb = std::function<bool(const char *fl, int ec, std::string errmsg, tickers_t res)>;
    result<tickers_t>
    ticker(const std::string &pair, tickers_cb cb = {}) { return ticker(pair.c_str(), std::move(cb)); }
    result<tickers_t>
    ticker(const char *pair, tickers_cb cb = {});
    result<tickers_t>
    tickers(tickers_cb cb = {});

    // https://docs.kraken.com/rest/#tag/Market-Data/operation/getTradableAssetPairs
    // Kraken `public/AssetPairs` — tradable pair metadata (one pair, or all).
    using asset_pairs_cb = std::function<bool(const char *fl, int ec, std::string errmsg, asset_pairs_t res)>;
    result<asset_pairs_t>
    asset_pairs(asset_pairs_cb cb = {});
    result<asset_pairs_t>
    asset_pairs(const std::string &pair, asset_pairs_cb cb = {}) { return asset_pairs(pair.c_str(), std::move(cb)); }
    result<asset_pairs_t>
    asset_pairs(const char *pair, asset_pairs_cb cb = {});

    // https://docs.kraken.com/rest/#tag/Market-Data/operation/getOrderBook
    // Kraken `public/Depth` — order book for a pair (count limits depth per side).
    using order_book_cb = std::function<bool(const char *fl, int ec, std::string errmsg, order_book_t res)>;
    result<order_book_t>
    order_book(const std::string &pair, std::size_t count = 0, order_book_cb cb = {}) { return order_book(pair.c_str(), count, std::move(cb)); }
    result<order_book_t>
    order_book(const char *pair, std::size_t count = 0, order_book_cb cb = {});

    // https://docs.kraken.com/rest/#tag/Market-Data/operation/getOHLCData
    // Kraken `public/OHLC` — candles for a pair. interval is in minutes
    // (1,5,15,30,60,240,1440,10080,21600); since is an optional cursor.
    using ohlc_cb = std::function<bool(const char *fl, int ec, std::string errmsg, ohlc_t res)>;
    result<ohlc_t>
    ohlc(const std::string &pair, std::size_t interval, std::size_t since = 0, ohlc_cb cb = {}) { return ohlc(pair.c_str(), interval, since, std::move(cb)); }
    result<ohlc_t>
    ohlc(const char *pair, std::size_t interval, std::size_t since = 0, ohlc_cb cb = {});

    // https://docs.kraken.com/rest/#tag/Market-Data/operation/getRecentTrades
    // Kraken `public/Trades` — recent trades for a pair; since is an optional cursor.
    using recent_trades_cb = std::function<bool(const char *fl, int ec, std::string errmsg, recent_trades_t res)>;
    result<recent_trades_t>
    recent_trades(const std::string &pair, const std::string &since = std::string{}, recent_trades_cb cb = {}) { return recent_trades(pair.c_str(), since.empty() ? nullptr : since.c_str(), std::move(cb)); }
    result<recent_trades_t>
    recent_trades(const char *pair, const char *since = nullptr, recent_trades_cb cb = {});

    // https://docs.kraken.com/rest/#tag/Trading/operation/addOrder
    // Kraken `private/AddOrder`. Pass validate=true to only validate (no execution).
    using add_order_cb = std::function<bool(const char *fl, int ec, std::string errmsg, add_order_t res)>;
    result<add_order_t>
    add_order(
         const std::string &pair
        ,const std::string &type        // "buy" / "sell"
        ,const std::string &ordertype   // "market" / "limit" / "stop-loss" / ...
        ,const std::string &volume
        ,const std::string &price = std::string{}   // required for limit/stop orders
        ,bool validate = false
        ,add_order_cb cb = {}
    );

    // https://docs.kraken.com/rest/#tag/Trading/operation/cancelOrder
    // Kraken `private/CancelOrder` — cancel by transaction id (or userref).
    using cancel_order_resp_cb = std::function<bool(const char *fl, int ec, std::string errmsg, cancel_order_t res)>;
    result<cancel_order_t>
    cancel_order(const std::string &txid, cancel_order_resp_cb cb = {});

    // https://docs.kraken.com/rest/#tag/Account-Data/operation/getOpenOrders
    using open_orders_resp_cb = std::function<bool(const char *fl, int ec, std::string errmsg, open_orders_t res)>;
    result<open_orders_t>
    open_orders(open_orders_resp_cb cb = {});

    // https://docs.kraken.com/rest/#tag/Account-Data/operation/getClosedOrders
    using closed_orders_cb = std::function<bool(const char *fl, int ec, std::string errmsg, closed_orders_t res)>;
    result<closed_orders_t>
    closed_orders(closed_orders_cb cb = {});

    // https://docs.kraken.com/rest/#tag/Account-Data/operation/getTradeHistory
    using trades_history_cb = std::function<bool(const char *fl, int ec, std::string errmsg, trades_history_t res)>;
    result<trades_history_t>
    trades_history(trades_history_cb cb = {});

    // https://docs.kraken.com/rest/#tag/Market-Data/operation/getSystemStatus
    // Kraken `public/SystemStatus` — exchange availability.
    using system_status_cb = std::function<bool(const char *fl, int ec, std::string errmsg, system_status_t res)>;
    result<system_status_t>
    system_status(system_status_cb cb = {});

    // https://docs.kraken.com/rest/#tag/Account-Data/operation/getOrdersInfo
    // Kraken `private/QueryOrders` — query specific orders by txid (comma-separated, up to 50).
    using query_orders_cb = std::function<bool(const char *fl, int ec, std::string errmsg, query_orders_t res)>;
    result<query_orders_t>
    query_orders(const std::string &txid, query_orders_cb cb = {});

    // https://docs.kraken.com/rest/#tag/Account-Data/operation/getTradesInfo
    // Kraken `private/QueryTrades` — query specific trades by txid (comma-separated, up to 20).
    using query_trades_cb = std::function<bool(const char *fl, int ec, std::string errmsg, query_trades_t res)>;
    result<query_trades_t>
    query_trades(const std::string &txid, query_trades_cb cb = {});

    // https://docs.kraken.com/rest/#tag/Account-Data/operation/getOpenPositions
    // Kraken `private/OpenPositions` — open margin positions (docalcs adds value/net P&L).
    using open_positions_cb = std::function<bool(const char *fl, int ec, std::string errmsg, open_positions_t res)>;
    result<open_positions_t>
    open_positions(bool docalcs = false, open_positions_cb cb = {});

    // https://docs.kraken.com/rest/#tag/Trading/operation/cancelAllOrders
    // Kraken `private/CancelAll` — cancel all open orders.
    using cancel_all_cb = std::function<bool(const char *fl, int ec, std::string errmsg, cancel_order_t res)>;
    result<cancel_order_t>
    cancel_all(cancel_all_cb cb = {});

    // https://docs.kraken.com/rest/#tag/Trading/operation/cancelAllOrdersAfter
    // Kraken `private/CancelAllOrdersAfter` — dead-man's switch; timeout in seconds (0 disables).
    using cancel_all_after_cb = std::function<bool(const char *fl, int ec, std::string errmsg, cancel_all_after_t res)>;
    result<cancel_all_after_t>
    cancel_all_after(std::size_t timeout, cancel_all_after_cb cb = {});

private:
    struct impl;
    std::unique_ptr<impl> pimpl;
};

/*************************************************************************************************/

} // ns rest
} // ns binapi

#endif // __binapi__api_hpp
