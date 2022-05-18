#!/bin/bash
set -e

nasm -felf64 capitalise.asm
gcc -c -Wall -Werror naive_capitalise.c
gcc -O2 -c -Wall -Werror simd_capitalise.c
gcc -Wall -Werror capitalise.o simd_capitalise.o naive_capitalise.o run_capitalise.c

./a.out
