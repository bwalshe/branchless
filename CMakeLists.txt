project(Branchless)

cmake_minimum_required(VERSION 3.16)

enable_language(C ASM_NASM)

set( SOURCES
    src/capitalise.asm
    src/branchless_capitalise.asm
    src/simd_capitalise.asm
    src/capitalise.h
)

include_directories(src)

add_executable(perf_test test/perf_test.c ${SOURCES})
find_package(Catch2 REQUIRED)
add_executable(unit_test test/unit_test.c ${SOURCES})
target_link_libraries(unit_test Catch2::Catch2)
