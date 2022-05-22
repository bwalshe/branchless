#!/bin/bash
set -e

CC=gcc-10
nasm -felf64 capitalise.asm
$CC -c -Wall -Werror naive_capitalise.c
nasm -felf64 simd_capitalise.asm
#$CC -O2 -c -Wall -Werror simd_capitalise.c
$CC -O2 -Wall -Werror capitalise.o simd_capitalise.o naive_capitalise.o run_capitalise.c

./a.out
