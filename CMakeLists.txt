cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
else()
    if (DEFINED ENV{"GITHUB_ACTIONS"})
        set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
    elseif (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "arm64")
        set(CMAKE_OSX_ARCHITECTURES "arm64")
    else()
        set(CMAKE_OSX_ARCHITECTURES "x86_64")
    endif()
endif()
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

option(CR_DEBUG "Debug mode" OFF)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CR_DEBUG ON)
    add_compile_definitions(CR_DEBUG=1)
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_compile_definitions(WIN32_LEAN_AND_MEAN=1)
    target_link_libraries(${PROJECT_NAME} Crypt32)
endif()

set(USE_ZLIB OFF)
if (NOT APPLE)
    set(USE_MBED_TLS ON)
endif()

set(USE_OPEN_SSL OFF)
set(USE_TLS ON)
SET(CMAKE_C_SIMULATE_ID "Clang")
set(ENABLE_PROGRAMS OFF)
set(ENABLE_TESTING OFF)
set(MBEDTLS_AS_SUBPROJECT OFF)
set(DISABLE_PACKAGE_CONFIG_AND_INSTALL OFF)
set(MBEDTLS_FATAL_WARNINGS OFF)

CPMAddPackage("gh:TechStudent10/mbedtls#fc79213")

set(MBEDTLS_INCLUDE_DIRS "${mbedtls_SOURCE_DIR}/include")
set(MBEDTLS_LIBRARY "mbedtls")
set(MBEDX509_LIBRARY "mbedx509")
set(MBEDCRYPTO_LIBRARY "mbedcrypto")

CPMAddPackage("gh:machinezone/IXWebSocket#dc8807e")
install(TARGETS mbedtls mbedx509 mbedcrypto EXPORT mbedtls-targets)

CPMAddPackage("gh:LimeGradient/argon#main")

if (WIN32)
    if (CR_DEBUG)
        target_compile_definitions(mbedtls PRIVATE _HAS_ITERATOR_DEBUGGING=0)
        target_compile_definitions(ixwebsocket PRIVATE _HAS_ITERATOR_DEBUGGING=0)
    endif()
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE CR_EXPORTING)

target_include_directories(${PROJECT_NAME} PRIVATE ${ixwebsocket_SOURCE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/ext/cereal/include")

target_link_libraries(${PROJECT_NAME} ixwebsocket argon)

setup_geode_mod(${PROJECT_NAME})
