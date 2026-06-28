cmake_minimum_required(VERSION 3.10)
project(websockets)

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

set(KRAPI_HEADERS
    krapi/api.hpp
    krapi/flatjson.hpp
    krapi/dtf.hpp
    krapi/double_type.hpp
    krapi/enums.hpp
    krapi/errors.hpp
    krapi/invoker.hpp
    krapi/message.hpp
    krapi/tools.hpp
    krapi/types.hpp
    krapi/websocket.hpp
)

set(KRAPI_SOURCES
    ../../src/api.cpp
    ../../src/enums.cpp
    ../../src/errors.cpp
    ../../src/tools.cpp
    ../../src/types.cpp
    ../../src/websocket.cpp
)

add_executable(
    ${PROJECT_NAME}
    #
    main.cpp
    #
    ${KRAPI_SOURCES}
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
