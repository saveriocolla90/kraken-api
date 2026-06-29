
// ----------------------------------------------------------------------------
// Shared helpers for the krapi test suite.
// ----------------------------------------------------------------------------

#include "test_helpers.hpp"

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <stdexcept>

#ifndef KRAPI_FIXTURES_DIR
#define KRAPI_FIXTURES_DIR "fixtures"
#endif

#ifndef KRAPI_TESTS_DIR
#define KRAPI_TESTS_DIR "."
#endif

namespace krapi_test
{

    std::string read_fixture(const std::string &name)
    {
        const std::string path = std::string{KRAPI_FIXTURES_DIR} + "/" + name;
        std::ifstream in{path, std::ios::binary};
        if (!in)
        {
            throw std::runtime_error("cannot open fixture: " + path);
        }

        std::ostringstream ss;
        ss << in.rdbuf();

        return ss.str();
    }

    static std::string trim(const std::string &s)
    {
        const auto b = s.find_first_not_of(" \t\r\n");
        if (b == std::string::npos)
        {
            return {};
        }
        const auto e = s.find_last_not_of(" \t\r\n");

        return s.substr(b, e - b + 1);
    }

    void load_dotenv(const std::string &path)
    {
        std::ifstream in{path};
        if (!in)
        {
            return; // missing .env is fine
        }

        std::string line;
        while (std::getline(in, line))
        {
            const std::string s = trim(line);
            if (s.empty() || s[0] == '#')
            {
                continue;
            }

            const auto eq = s.find('=');
            if (eq == std::string::npos)
            {
                continue;
            }

            std::string key = trim(s.substr(0, eq));
            std::string val = trim(s.substr(eq + 1));
            if (key.empty())
            {
                continue;
            }

            // strip a single pair of surrounding quotes
            if (val.size() >= 2 &&
                ((val.front() == '"' && val.back() == '"') ||
                 (val.front() == '\'' && val.back() == '\'')))
            {
                val = val.substr(1, val.size() - 2);
            }

            // 0 => do not overwrite an already-exported variable
            ::setenv(key.c_str(), val.c_str(), 0);
        }
    }

    void load_test_env()
    {
        load_dotenv(std::string{KRAPI_TESTS_DIR} + "/.env");
    }

    std::string env_or_empty(const char *name)
    {
        const char *v = std::getenv(name);

        return v ? std::string{v} : std::string{};
    }

    bool have_credentials()
    {
        return !env_or_empty("KRAKEN_API_KEY").empty() &&
               !env_or_empty("KRAKEN_API_SECRET").empty();
    }

} // ns krapi_test
