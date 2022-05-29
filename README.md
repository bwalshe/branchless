# Branchless Programming

This project shows a simple demonstration for branchless programming, and
how it affects the running time of program. To illustrate the difference I am
going to implement a string capitalisation function three ways and measure
how long they take to process a string of one billion characters. 

I'll start with a simple, unoptimised version which scans through the string
linearly updating any lower case alphabetic characters it finds. Then I'll show a "branchless" version which still scans through linearly, but uses special
conditional instructions to avoid needing to jump around the place. Finally, 
I will show a SIMD version which uses vector instructions to process strings
in blocks of 32 characters at a time.

I'll get to describing those soon, but first I should explain why we hate
branches so much. 

## Why is branching bad?
When CPUs execute instructions, each instruction isn't really executed as an
atomic operation. What actually happens is that first the instruction is 
fetched, then it is decoded, then it is actually executed, and finally the 
result is written back. Actually, depending on the CPU architecture, there may
be more stages, and it's possible that the CPU might execute some instructions
in parallel or even out of order if it thinks can do this without changing the
result.

Branches kill the CPU's ability to do stuff like this. Assuming that the CPU
has a four stage pipeline, at instruction *n*, the CPU will already start 
loading instruction *n+3*. If instruction n says to jump to some other 
instruction, then instructions *n+1*, *n+2*, and *n+3* will have to be flushed
from the pipeline so the new string of instructions can be processed.

## Capitalising UTF-8 Strings

The ASCII/UTF-8 character encoding is set up in a way that you can capitalise 
a character by subtracting 32 from it. At least if it is a lower-case 
alphabetic character, you can  do this. For upper-case or punctuation 
characters, subtracting 32 will result in gibberish, so first you have to check
the character is something between 'a' and 'z'. In C, to capitalise the string 
`s` with length `len` you could do something like this:

```c
for(int i=0; i < len; ++i) {
   if(s[i] >= 'a' && s[i] <= 'z') {
     s[i] -= 32;
   }
}
```

## A crash course in assembly
The compiler has a lot of leeway in how to convert this into instructions for 
the CPU so in order to show exactly where the bracing occurs, I'm going to 
have to go one level down and write things in assembler where there is 
complete control over which instructions are used. 

Assembly is a pretty daunting subject, but in order to understand these 
examples there are only a few things you need to know. First of all, in order
to change any values in memory, you need to first move them into a register
using a `mov` instruction, manipulate the value in the register and then write
it back out to memory. So adding one to the value stored at memory address
`x` would require something like this

```asm
    mov rax, [x]    ; Move the value at x into rax
    inc rax         ; Increment the value in rax
    mov [x], rax    ; Write the value back out to memory
```

The next thing you need to know is that flow of control is done with jump
instructions, and there are a few different variants of these instructions that
jump under different conditions. For example `je` will jump if the last 
comparison operation was equal, `jne` will jump if the last comparison was 
*not* equal, and `jnz` will jump if the last arithmetic operation did not 
result in zero. For example you could do a loop that counts down from 10 as
follows
```asm
    mov rax, 10
topOfLoop:
    dec rax
    jnz topOfLoop
```

I won't list all the jump instructions. The only thing you need to remember
about them is that they begin with the letter 'j', and everybody hates them
because they mess up the instruction pipeline. 

## V1.0: a very jumpy capitalisation function
OK if I was to implement the capitalisation algorithm shown above in assembler
I would go for something like this:

```asm
updateChar:
        movzx eax, BYTE [rdi + rcx] ; Move a char from the input into eax
        cmp al, 'a'                 ; Check if the value is less than 'a'
        jl incrementIndex
        cmp al, 'z'                 ; Check if the value is greater than 'z'
        jg incrementIndex 
        add al, 'A' - 'a'           ; The current char is lower case, make it
                                    ; uppercase
        mov BYTE [rdi + rcx], al    ; write the data back out to memoy
incrementIndex:
        add rcx, 1
        cmp rcx, rsi        ; Check if we have reached the end of the string
        jne updateChar
```

Basically, this implementation runs by first loading a byte from memory, 
then checking if the value is in between 'a' and 'z'. If it isn't then we just
increment the array index stored in `rcx` and start from the top of the loop. 
If the value *is* between 'a' and 'z' then we subtract 32 and write it back to 
memory, before incrementing the index and looping. 

One slightly weird thing here is that `eax` and `al` are the same register. 
With `al` being the lower 8 bits of `eax`. It is a little more efficient to
read the data into `eax`, zeroing the upper bits, and then writing out using
`al`, than it is to read and write using `al` on its own.  

On my computer, this takes about 3.32 seconds to process 10^9 characters. This
isn't great, and it's probably due to those two branches used to determine if
the character was between 'a' and 'z'.

## V1.5: A slightly less jumpy implementation

*You can skip this section if you want, it describes how to make things a bit
more efficient, but it doesn't introduce branchless instructions yet.*

If I take that snippet of C code shown above and turn it into a well-formed 
function, I can get the compiler to generate an optimized version. Using 
`gcc -O2`, I got an interesting optimisation that is worth talking about.

This version uses a hack to remove one of the jump instructions as follows:

```asm
.updateChar:
        movzx eax, BYTE [rdi + rcx] 
        lea r9d, [eax - 'a']        
        cmp r9b, 26 
        jnb incrementIndex
        add al, 'A' - 'a'           
        mov BYTE [rdi + rcx], al 
incrementIndex:
        cmp rcx, rsi        ; Check if we have reached the end of the string
        jne updateChar
```
OK, I call this a "hack" but actually there are two things being done here, and
they are both considered fairly normal practice in the world of x86 assembly.

The first thing is that the `lea` instruction is being used to perform 
arithmetic. This instruction - load effective address - was originally intended
for making working with pointers more efficient, but there is nothing about it
that actually forces you to use it on a pointer. `lea r9d, [eax - 'a']` will
subtract 'a' from the value in `eax` and store the result in `rd9` using a 
single instruction.

The next bit, the really weird bit, is that it will compere the result to 26,
and jump if the value is not lower. So if the original value was greater than 
'z' this will result in a jump. What if the value was less than 'a'? Where is
the test for that. Well here is the thing `jnb` treats the value as *unsigned*.
If you have a negative signed value, then the bits in the register are going 
to be equivalent to some large unsigned number. If the value in `eax` was less
than `a`, then the unsigned result of subtracting 'a' will be larger than 26.
I find this pretty confusing, but I have seen it described in introductory 
texts for x86 assembler, so I guess it is pretty common. It definitely cuts
out one of the jump instructions and it does give you a significant performance
improvement, but there is still one jump left, and there is a way we can 
remove that.

## A branch-free implementation
It's possible to remove the last jump instruction in the range check by using 
a *conditional* move instruction. These behave like a regular move instruction
except if their condition is not met, then they do nothing. Like the jump
instructions, there are several variants of conditional move, that use different
conditions. `cmove` will be triggered if the previous comparison was equal, 
`cmovz` will be triggered if the last arithmetic operation resulted in zero, 
and `cmovb` will be triggered if the last comparison was below an *unsigned*
value. 

Combining `cmovb` with the trick described in the section above, the 
capitalisation loop can be implemented as follows:

```
.updateChar:
        movzx eax, BYTE [rdi + rcx] ; Move a char from the input into rax
        lea r8d, [eax - 32]         ; Convert the char to uppercase, assuming
                                    ; the current char is lowercase
        lea r9d, [eax - 'a']        ; Where is the character in relation to 'A'
        cmp r9b, 26         ; We will treat the value in r8 as an 
                            ; unsigned number, which means that if the value in
                            ; eax was less than 97 then this will appear to be
                            ; some nonsense high number, and we just need to 
                            ; check that x - 'a' < 26 
        cmovb eax, r8d      ; Conditinally move the upper case letter into the
                            ; array
        mov BYTE [rdi + rcx], al
        add rcx, 1
        cmp rcx, rsi        ; Check if we have reached the end of the string
        jne .updateChar
```

This version always subtracts 32 from the value loaded form memory, for 
characters that are not lower case letters, this will result in a nonsense 
value, but that doesn't matter at this point. After the value has been 
calculated, the check to see if the original value was between 'a' and 'z' is 
performed. If this comparison is successful, *then* the value is written back
by the conditional move. If the comparison failed, then nothing is written.

This version of the algorithm actually does more work than the version that 
uses jump instructions. It performs an add operation that is likely to be 
thrown away, and the `cmovb` operation takes more clock cycles than a 
non-conditional `mov` operation. Still though, this results in far fewer
pipeline flushes, and on my computer it can process 10^9 characters in about
0.65 seconds. That is a bit more than a 5x improvement in running time. 

