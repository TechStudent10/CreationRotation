cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(CreationRotation VERSION 1.0.0)

file(GLOB_RECURSE SOURCES src/*.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

target_include_directories(${PROJECT_NAME} PRIVATE src/)
add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_compile_definitions(WIN32_LEAN_AND_MEAN=1)
endif()

CPMAddPackage("gh:socketio/socket.io-client-cpp#0dc2f7a")

# CPMAddPackage(
#     NAME Boost
#     VERSION 1.84.0
#     URL https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.tar.xz
#     URL_HASH SHA256=2e64e5d79a738d0fa6fb546c6e5c2bd28f88d268a2a080546f74e5ff98f29d0e
#     OPTIONS "BOOST_ENABLE_CMAKE ON" "BOOST_INCLUDE_LIBRARIES describe" # escape with \\\;
# )
# find_package(Boost REQUIRED COMPONENTS filesystem system thread regex)

# CPMAddPackage("gh:zaphoyd/websocketpp#1b11fd3")

# target_include_directories(${PROJECT_NAME} PRIVATE ${websocketpp_SOURCE_DIR})


# Add ZLIB
# set(ZLIB_REPO "https://github.com/madler/zlib.git"
#      CACHE STRING "ZLIB repository location."
# )
# CPMAddPackage(
#     NAME zlib 
#     GIT_REPOSITORY ${ZLIB_REPO}
#     GIT_TAG "v1.2.13"
#     OPTIONS
#     "CMAKE_POSITION_INDEPENDENT_CODE True"
# )
# if(zlib_ADDED)
#     target_include_directories(zlib 
#                                PUBLIC $<BUILD_INTERFACE:${zlib_BINARY_DIR}>
#                                PUBLIC $<INSTALL_INTERFACE:include>)
#     target_include_directories(zlibstatic 
#                                PUBLIC $<BUILD_INTERFACE:${zlib_BINARY_DIR}>
#                                PUBLIC $<INSTALL_INTERFACE:include>)
               
#     set(ZLIB_LIBRARY ${zlib_BINARY_DIR})
#     set(ZLIB_INCLUDE_DIR ${zlib_SOURCE_DIR})
#     message(STATUS "Cmake added local zlib: ${zlib_SOURCE_DIR}")
# else()
#     message(FATAL_ERROR "Cmake failed to add local zlib")
# endif()

# set(ZLIB_LIBRARY "${CMAKE_SOURCE_DIR}/ext/zlib")
# set(ZLIB_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/ext/zlib")
# set(RENAME_ZCONF OFF)

set(USE_ZLIB OFF)

CPMAddPackage("gh:machinezone/IXWebSocket#9884c32")

target_include_directories(${PROJECT_NAME} PRIVATE ${ixwebsocket_SOURCE_DIR})
# target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/ext/zlib")
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/ext/cereal/include")

# add_subdirectory("ext/zlib")

target_link_libraries(${PROJECT_NAME} ixwebsocket sioclient)

setup_geode_mod(${PROJECT_NAME})
