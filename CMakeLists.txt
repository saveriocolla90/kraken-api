cmake_minimum_required(VERSION 3.10)
project(binapi VERSION 1.0.0 LANGUAGES CXX)

# Options
option(BUILD_SHARED_LIBS "Build shared library" ON)
option(BINAPI_BUILD_EXAMPLE "Build example executable from main.cpp" OFF)
option(BINAPI_ENABLE_ASAN "Enable AddressSanitizer" OFF)

# C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Sanitizer (opt-in)
if(BINAPI_ENABLE_ASAN)
  add_compile_options(-fsanitize=address)
  add_link_options(-fsanitize=address)
endif()

# Include dirs
include(GNUInstallDirs)

# Sources
set(BINAPI_HEADERS
    include/binapi/api.hpp
    include/binapi/flatjson.hpp
    include/binapi/dtf.hpp
    include/binapi/double_type.hpp
    include/binapi/enums.hpp
    include/binapi/errors.hpp
    include/binapi/invoker.hpp
    include/binapi/message.hpp
    include/binapi/pairslist.hpp
    include/binapi/reports.hpp
    include/binapi/tools.hpp
    include/binapi/types.hpp
    include/binapi/websocket.hpp
)

set(BINAPI_SOURCES
    src/api.cpp
    src/enums.cpp
    src/errors.cpp
    src/pairslist.cpp
    src/reports.cpp
    src/tools.cpp
    src/types.cpp
    src/websocket.cpp
)

# Library target
add_library(binapi ${BINAPI_SOURCES} ${BINAPI_HEADERS})
add_library(binapi::binapi ALIAS binapi)

target_compile_options(binapi PRIVATE -Wall -Wextra)
# UNDEFINE NDEBUG using compile OPTIONS, not definitions
target_compile_options(binapi PRIVATE -UNDEBUG)

# Real preprocessor definitions go here (no leading -D / -U)
target_compile_definitions(binapi PUBLIC DTF_HEADER_ONLY)

target_include_directories(binapi
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Optional external include/lib hints (kept from your original)
if(DEFINED BOOST_INCLUDE_DIR)
  target_include_directories(binapi PUBLIC ${BOOST_INCLUDE_DIR})
endif()
if(DEFINED BOOST_LIB_DIR)
  # Prefer modern find_package(Boost ...) in the future; this mirrors your original linkage hint.
  target_link_directories(binapi PRIVATE ${BOOST_LIB_DIR})
endif()

# Link system libs
target_link_libraries(binapi
  PUBLIC
    z
    crypto
    ssl
    pthread
)

# Versioning for shared libs
set_target_properties(binapi PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
  EXPORT_NAME binapi
)

# -------- Installation --------
# 1) Install the library
install(TARGETS binapi
  EXPORT binapiTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}  # in case of DLLs on Windows or tools later
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# 2) Install public headers (preserve folder layout: include/binapi/...)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# 3) Export targets for find_package
install(EXPORT binapiTargets
  NAMESPACE binapi::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/binapi
)

# Generate a basic ConfigVersion file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/binapiConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

# Minimal binapiConfig.cmake that imports the installed targets
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/binapiConfig.cmake"
"include(\"\${CMAKE_CURRENT_LIST_DIR}/binapiTargets.cmake\")\n")

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/binapiConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/binapiConfigVersion.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/binapi
)

# -------- Optional example (main.cpp) --------
if(BINAPI_BUILD_EXAMPLE)
  add_executable(binapi-cli main.cpp)
  target_link_libraries(binapi-cli PRIVATE binapi)
  target_compile_options(binapi-cli PRIVATE -Wall -Wextra -UNDEBUG)
endif()