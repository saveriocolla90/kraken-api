# ---------------------------------------------------------------------------
# krapi test suite
#
#   - krapi_tests_offline : deterministic parser tests over tests/fixtures/*.json
#   - krapi_tests_live    : opt-in integration tests against api.kraken.com
#
# Enable with -DKRAPI_BUILD_TESTS=ON from the top-level build.
# Offline (no network) builds can point FetchContent at a local doctest:
#   -DFETCHCONTENT_SOURCE_DIR_DOCTEST=/path/to/doctest
# ---------------------------------------------------------------------------

include(FetchContent)

# doctest: single-header test framework (no system dependency).
set(DOCTEST_WITH_TESTS OFF CACHE BOOL "" FORCE)
set(DOCTEST_NO_INSTALL ON CACHE BOOL "" FORCE)
# doctest v2.4.11 ships a cmake_minimum_required() below what modern CMake
# accepts; relax the policy floor just for the fetched subproject.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "" FORCE)
FetchContent_Declare(
  doctest
  GIT_REPOSITORY https://github.com/doctest/doctest.git
  GIT_TAG v2.4.11
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(doctest)

# Shared helpers (fixture loading, .env / credentials handling).
add_library(krapi_test_helpers STATIC test_helpers.cpp)
target_link_libraries(krapi_test_helpers PUBLIC krapi::krapi)
target_compile_definitions(krapi_test_helpers PUBLIC
  KRAPI_FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
  KRAPI_TESTS_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)
target_include_directories(krapi_test_helpers PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Offline parser tests.
add_executable(krapi_tests_offline test_main.cpp test_parsers.cpp)
target_link_libraries(krapi_tests_offline PRIVATE
  krapi_test_helpers krapi::krapi doctest::doctest)
target_compile_options(krapi_tests_offline PRIVATE -Wall -Wextra)

# Live integration tests.
add_executable(krapi_tests_live test_main.cpp test_live.cpp)
target_link_libraries(krapi_tests_live PRIVATE
  krapi_test_helpers krapi::krapi doctest::doctest)
target_compile_options(krapi_tests_live PRIVATE -Wall -Wextra)

add_test(NAME offline COMMAND krapi_tests_offline)
add_test(NAME live COMMAND krapi_tests_live)
# Label the live test so it can be selected (-L live) or excluded (-LE live).
set_tests_properties(live PROPERTIES LABELS live)
