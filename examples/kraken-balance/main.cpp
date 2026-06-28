
// ----------------------------------------------------------------------------
// Kraken REST proof-of-concept: signed call to private/Balance.
//
// Usage:
//   export KRAKEN_API_KEY="<your api key>"
//   export KRAKEN_API_SECRET="<your base64 private key>"
//   ./kraken-balance
// ----------------------------------------------------------------------------

#include <binapi/api.hpp>

#include <boost/asio/io_context.hpp>

#include <cstdlib>
#include <iostream>

int main() {
    const char *key    = std::getenv("KRAKEN_API_KEY");
    const char *secret = std::getenv("KRAKEN_API_SECRET");
    if ( !key || !secret ) {
        std::cerr << "set KRAKEN_API_KEY and KRAKEN_API_SECRET in the environment\n";
        return EXIT_FAILURE;
    }

    boost::asio::io_context ioctx;
    binapi::rest::api api{
         ioctx
        ,"api.kraken.com"
        ,"443"
        ,key
        ,secret
        ,10000 // timeout (unused by Kraken signing, kept for ctor compat)
    };

    // ---- synchronous call ----
    auto res = api.balances();
    if ( !res ) {
        std::cerr << "balances error: " << res.errmsg << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "balances: " << res.v << std::endl;
    for ( const auto &kv : res.v.balances ) {
        std::cout << "  " << kv.first << " = " << kv.second << std::endl;
    }

    return EXIT_SUCCESS;
}
