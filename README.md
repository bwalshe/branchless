# Branchless Programming

This project shows a simple use-case for branchless programming, scanning 
through a string to capitalise al the alphabetic characters. This shows
the difference between a naive, unoptimised version, a branchless version
which uses conditional move CPU instructions, and an SIMD version which uses
vector instructions to process strings 16 characters at a time.
