cmake_minimum_required(VERSION 3.10)
project(kraken-balance)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

add_definitions(
    -UNDEBUG
    -DDTF_HEADER_ONLY
)

include_directories(
    ../../include
)

if (DEFINED ${BOOST_INCLUDE_DIR})
    include_directories(
        ${BOOST_INCLUDE_DIR}
    )
endif()

set(BINAPI_SOURCES
    ../../src/errors.cpp
    ../../src/api.cpp
    ../../src/enums.cpp
    ../../src/pairslist.cpp
    ../../src/reports.cpp
    ../../src/tools.cpp
    ../../src/types.cpp
    ../../src/websocket.cpp
)

add_executable(
    ${PROJECT_NAME}
    #
    main.cpp
    #
    ${BINAPI_SOURCES}
)

if (DEFINED ${BOOST_LIB_DIR})
    target_link_libraries(
        ${PROJECT_NAME}
        -L${BOOST_LIB_DIR}
    )
endif()

target_link_libraries(
    ${PROJECT_NAME}
    z
    crypto
    ssl
    pthread
)
