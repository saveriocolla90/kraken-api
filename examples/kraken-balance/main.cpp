
// ----------------------------------------------------------------------------
// Kraken REST proof-of-concept.
//
// Public endpoints (no auth): Time, Ticker, AssetPairs.
// Private endpoint (auth):    Balance.
//
// Usage:
//   ./kraken-balance                       # runs the public calls
//   KRAKEN_API_KEY=... KRAKEN_API_SECRET=...  ./kraken-balance   # also Balance
// ----------------------------------------------------------------------------

#include <krapi/api.hpp>

#include <boost/asio/io_context.hpp>

#include <cstdlib>
#include <iostream>

int main()
{
    boost::asio::io_context ioctx;

    const char *key = std::getenv("KRAKEN_API_KEY");
    const char *secret = std::getenv("KRAKEN_API_SECRET");

    krapi::rest::api api{
        ioctx, "api.kraken.com", "443", (key ? key : ""), (secret ? secret : ""), 10000 // timeout (unused by Kraken signing, kept for ctor compat)
    };

    // ---- public: server time ----
    auto time = api.server_time();
    if (!time)
    {
        std::cerr << "server_time error: " << time.errmsg << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "server_time: " << time.v << std::endl;

    // ---- public: ticker for a single pair ----
    auto tick = api.ticker("XBTUSD");
    if (!tick)
    {
        std::cerr << "ticker error: " << tick.errmsg << std::endl;
        return EXIT_FAILURE;
    }
    for (const auto &kv : tick.v.tickers)
    {
        std::cout << "ticker " << kv.first
                  << ": last=" << kv.second.last
                  << " bid=" << kv.second.bid
                  << " ask=" << kv.second.ask << std::endl;
    }

    // ---- public: asset pairs (single pair to keep output small) ----
    auto pairs = api.asset_pairs("XBTUSD");
    if (!pairs)
    {
        std::cerr << "asset_pairs error: " << pairs.errmsg << std::endl;
        return EXIT_FAILURE;
    }
    for (const auto &kv : pairs.v.pairs)
    {
        std::cout << "pair " << kv.first
                  << ": altname=" << kv.second.altname
                  << " base=" << kv.second.base
                  << " quote=" << kv.second.quote
                  << " status=" << kv.second.status << std::endl;
    }

    // ---- private: balances (only if credentials were provided) ----
    if (key && secret)
    {
        auto res = api.balances();
        if (!res)
        {
            std::cerr << "balances error: " << res.errmsg << std::endl;
            return EXIT_FAILURE;
        }
        std::cout << "balances: " << res.v << std::endl;
        for (const auto &kv : res.v.balances)
        {
            std::cout << "  " << kv.first << " = " << kv.second << std::endl;
        }
    }
    else
    {
        std::cout << "(set KRAKEN_API_KEY / KRAKEN_API_SECRET to test private/Balance)"
                  << std::endl;
    }

    return EXIT_SUCCESS;
}
