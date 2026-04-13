# Project Definition
cmake_minimum_required(VERSION 3.25)

set(CMAKE_ASM_NASM_COMPILER nasm)

project(FoundationCore
        VERSION 0.1.0
        LANGUAGES C ASM_NASM
)

# Tooling
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Standards
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# NASM
set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
set(CMAKE_ASM_NASM_FLAGS "-f elf64 -g -F dwarf")

# Conan
list(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}")
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}")

# Dependencies
find_package(libsodium REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(fmt REQUIRED)
find_package(ZLIB REQUIRED)
find_package(libuv REQUIRED)
find_package(liburing REQUIRED)
find_package(CURL REQUIRED)
find_package(cJSON REQUIRED)
find_package(xxHash REQUIRED)
find_package(mimalloc REQUIRED)
find_package(zstd REQUIRED)
find_package(lmdb REQUIRED)
find_package(libuuid REQUIRED)
find_package(cglm REQUIRED)

# Sources
file(GLOB_RECURSE C_SOURCES CONFIGURE_DEPENDS "src/*.c")
file(GLOB_RECURSE ASM_SOURCES CONFIGURE_DEPENDS "src/*.asm")

# Binary
add_executable(
        ${PROJECT_NAME}
        ${C_SOURCES}
        ${ASM_SOURCES}
)

target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# TODO: Link the dependencies down below
target_link_libraries(${PROJECT_NAME} libsodium::libsodium)
target_link_libraries(${PROJECT_NAME} fmt::fmt)
target_link_libraries(${PROJECT_NAME} openssl::openssl)
target_link_libraries(${PROJECT_NAME} ZLIB::ZLIB)
target_link_libraries(${PROJECT_NAME} libuv::uv_a)
target_link_libraries(${PROJECT_NAME} liburing::liburing)
target_link_libraries(${PROJECT_NAME} CURL::libcurl)
target_link_libraries(${PROJECT_NAME} cjson::cjson)
target_link_libraries(${PROJECT_NAME} xxHash::xxhash)
target_link_libraries(${PROJECT_NAME} mimalloc-static)
target_link_libraries(${PROJECT_NAME} zstd::libzstd)
target_link_libraries(${PROJECT_NAME} lmdb::lmdb)
target_link_libraries(${PROJECT_NAME} libuuid::libuuid)
target_link_libraries(${PROJECT_NAME} cglm::cglm)

# Install
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
