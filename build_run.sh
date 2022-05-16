#!/bin/sh
nasm -felf64 capitalise.asm && \ 
gcc capitalise.o run_capitalise.c && \
./a.out
