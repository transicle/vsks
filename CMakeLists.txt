cmake_minimum_required(VERSION 3.20)

project(vsks LANGUAGES CXX)

if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(FATAL_ERROR "vsks requires GCC. Found: ${CMAKE_CXX_COMPILER_ID}")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenSSL REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(SQLITE3     REQUIRED sqlite3)
pkg_check_modules(LIBSECRET   REQUIRED libsecret-1)

add_executable(vsks
    src/main.cc
    src/cli.cc
    src/crypto.cc
    src/database.cc
    src/io.cc
    src/paths.cc
    src/repl.cc
)

target_include_directories(vsks PRIVATE
    src
    ${SQLITE3_INCLUDE_DIRS}
    ${LIBSECRET_INCLUDE_DIRS}
)

target_link_libraries(vsks PRIVATE
    OpenSSL::Crypto
    ${SQLITE3_LIBRARIES}
    ${LIBSECRET_LIBRARIES}
)

target_compile_options(vsks PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    ${SQLITE3_CFLAGS_OTHER}
    ${LIBSECRET_CFLAGS_OTHER}
)