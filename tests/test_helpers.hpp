
// ----------------------------------------------------------------------------
// Shared helpers for the krapi test suite (offline parser tests + live tests).
// ----------------------------------------------------------------------------

#ifndef __krapi_tests__test_helpers_hpp
#define __krapi_tests__test_helpers_hpp

#include <string>

namespace krapi_test
{

    // Reads a fixture file by name (e.g. "server_time.json") from the fixtures
    // directory baked in at configure time (KRAPI_FIXTURES_DIR). Throws
    // std::runtime_error if the file cannot be opened.
    std::string read_fixture(const std::string &name);

    // Parses a `KEY=VALUE` file (blank lines and `#` comments ignored) and
    // exports each pair into the environment *without* overwriting variables that
    // are already set. Missing file is a no-op. Surrounding quotes on the value
    // are stripped. Used to load tests/.env for the live tests.
    void load_dotenv(const std::string &path);

    // Loads tests/.env (KRAPI_TESTS_DIR/.env) once. Safe to call repeatedly.
    void load_test_env();

    // Returns the value of an environment variable, or an empty string.
    std::string env_or_empty(const char *name);

    // True when both KRAKEN_API_KEY and KRAKEN_API_SECRET are set and non-empty.
    bool have_credentials();

} // ns krapi_test

#endif // __krapi_tests__test_helpers_hpp
