set(CMAKE_C_COMPILER clang)
set(CMAKE_ASM_NASM_COMPILER nasm)

add_link_options(-fuse-ld=lld)

add_compile_options(
        $<$<COMPILE_LANGUAGE:C>:-Wall>
        $<$<COMPILE_LANGUAGE:C>:-Wextra>
        $<$<COMPILE_LANGUAGE:C>:-Werror>
        $<$<COMPILE_LANGUAGE:C>:-Wpedantic>
        $<$<COMPILE_LANGUAGE:C>:-Wconversion>
        $<$<COMPILE_LANGUAGE:C>:-Wshadow>
        $<$<COMPILE_LANGUAGE:C>:-fcolor-diagnostics>
)