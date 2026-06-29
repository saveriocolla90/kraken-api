cmake_minimum_required(VERSION 3.10)
project(krapi VERSION 1.0.0 LANGUAGES CXX)

# Options
option(BUILD_SHARED_LIBS "Build shared library" ON)
option(KRAPI_BUILD_EXAMPLE "Build example executable from main.cpp" OFF)
option(KRAPI_BUILD_TESTS "Build unit tests" OFF)
option(KRAPI_ENABLE_ASAN "Enable AddressSanitizer" OFF)

# C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Sanitizer (opt-in)
if(KRAPI_ENABLE_ASAN)
  add_compile_options(-fsanitize=address)
  add_link_options(-fsanitize=address)
endif()

# Include dirs
include(GNUInstallDirs)

# Sources
set(KRAPI_HEADERS
    include/krapi/api.hpp
    include/krapi/flatjson.hpp
    include/krapi/dtf.hpp
    include/krapi/double_type.hpp
    include/krapi/enums.hpp
    include/krapi/errors.hpp
    include/krapi/invoker.hpp
    include/krapi/message.hpp
    include/krapi/tools.hpp
    include/krapi/types.hpp
    include/krapi/websocket.hpp
)

set(KRAPI_SOURCES
    src/api.cpp
    src/enums.cpp
    src/errors.cpp
    src/tools.cpp
    src/types.cpp
    src/websocket.cpp
)

# Library target
add_library(krapi ${KRAPI_SOURCES} ${KRAPI_HEADERS})
add_library(krapi::krapi ALIAS krapi)

target_compile_options(krapi PRIVATE -Wall -Wextra)
# UNDEFINE NDEBUG using compile OPTIONS, not definitions
target_compile_options(krapi PRIVATE -UNDEBUG)

# Real preprocessor definitions go here (no leading -D / -U)
target_compile_definitions(krapi PUBLIC DTF_HEADER_ONLY)

target_include_directories(krapi
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Optional external include/lib hints (kept from your original)
if(DEFINED BOOST_INCLUDE_DIR)
  target_include_directories(krapi PUBLIC ${BOOST_INCLUDE_DIR})
endif()
if(DEFINED BOOST_LIB_DIR)
  # Prefer modern find_package(Boost ...) in the future; this mirrors your original linkage hint.
  target_link_directories(krapi PRIVATE ${BOOST_LIB_DIR})
endif()

# Link system libs
target_link_libraries(krapi
  PUBLIC
    z
    crypto
    ssl
    pthread
)

# Versioning for shared libs
set_target_properties(krapi PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
  EXPORT_NAME krapi
)

# -------- Installation --------
# 1) Install the library
install(TARGETS krapi
  EXPORT krapiTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}  # in case of DLLs on Windows or tools later
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# 2) Install public headers (preserve folder layout: include/krapi/...)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# 3) Export targets for find_package
install(EXPORT krapiTargets
  NAMESPACE krapi::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/krapi
)

# Generate a basic ConfigVersion file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/krapiConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

# Minimal krapiConfig.cmake that imports the installed targets
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/krapiConfig.cmake"
"include(\"\${CMAKE_CURRENT_LIST_DIR}/krapiTargets.cmake\")\n")

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/krapiConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/krapiConfigVersion.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/krapi
)

# -------- Optional example (main.cpp) --------
if(KRAPI_BUILD_EXAMPLE)
  add_executable(krapi-cli main.cpp)
  target_link_libraries(krapi-cli PRIVATE krapi)
  target_compile_options(krapi-cli PRIVATE -Wall -Wextra -UNDEBUG)
endif()

# -------- Optional tests --------
if(KRAPI_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()