#!/bin/bash
set -e

nasm -felf64 capitalise.asm
gcc -c -Wall -Werror naieve_capitalise.c
gcc -O2 -c -Wall -Werror simd_capitalise.c
gcc -Wall -Werror capitalise.o simd_capitalise.o naieve_capitalise.o run_capitalise.c

./a.out
