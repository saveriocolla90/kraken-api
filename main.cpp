
// ----------------------------------------------------------------------------
//                              Apache License
//                        Version 2.0, January 2004
//                     http://www.apache.org/licenses/
//
// This file is part of the Kraken REST port of binapi.
// ----------------------------------------------------------------------------

// Minimal Kraken REST demo. Public calls run unauthenticated; the private
// Balance call runs only when KRAKEN_API_KEY / KRAKEN_API_SECRET are set.
//
// Build with: cmake -DBINAPI_BUILD_EXAMPLE=ON ...

#include <binapi/api.hpp>

#include <boost/asio/io_context.hpp>

#include <cstdlib>
#include <iostream>

int main() {
    boost::asio::io_context ioctx;

    const char *key    = std::getenv("KRAKEN_API_KEY");
    const char *secret = std::getenv("KRAKEN_API_SECRET");

    binapi::rest::api api{
         ioctx
        ,"api.kraken.com"
        ,"443"
        ,(key ? key : "")
        ,(secret ? secret : "")
        ,10000
    };

    // ---- public: server time ----
    if ( auto r = api.server_time() ) {
        std::cout << "server_time: " << r.v << std::endl;
    } else {
        std::cerr << "server_time error: " << r.errmsg << std::endl;
        return EXIT_FAILURE;
    }

    // ---- public: ticker ----
    if ( auto r = api.ticker("XBTUSD") ) {
        for ( const auto &kv : r.v.tickers ) {
            std::cout << "ticker " << kv.first
                      << ": last=" << kv.second.last
                      << " bid=" << kv.second.bid
                      << " ask=" << kv.second.ask << std::endl;
        }
    } else {
        std::cerr << "ticker error: " << r.errmsg << std::endl;
        return EXIT_FAILURE;
    }

    // ---- public: order book (top of book) ----
    if ( auto r = api.order_book("XBTUSD", 1) ) {
        if ( !r.v.asks.empty() && !r.v.bids.empty() ) {
            std::cout << "order_book " << r.v.pair
                      << ": best_bid=" << r.v.bids[0].price
                      << " best_ask=" << r.v.asks[0].price << std::endl;
        }
    } else {
        std::cerr << "order_book error: " << r.errmsg << std::endl;
        return EXIT_FAILURE;
    }

    // ---- private: balances (requires credentials) ----
    if ( key && secret ) {
        if ( auto r = api.balances() ) {
            std::cout << "balances: " << r.v << std::endl;
        } else {
            std::cerr << "balances error: " << r.errmsg << std::endl;
            return EXIT_FAILURE;
        }
    } else {
        std::cout << "(set KRAKEN_API_KEY / KRAKEN_API_SECRET for private/Balance)"
                  << std::endl;
    }

    return EXIT_SUCCESS;
}
