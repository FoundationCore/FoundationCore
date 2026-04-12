# Project Definition
cmake_minimum_required(VERSION 4.3.1)

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

# TODO: Add dependencies down below
find_package(libsodium REQUIRED)

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

# Install
install(TARGETS ${PROJECT_NAME} DESTINATION bin)