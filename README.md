# Understanding Branchless and SIMD Optimisations

A while ago I saw [this video](https://www.youtube.com/watch?v=bVJ-mWWL7cE),
which demonstrates how conditional move and SIMD instructions can be used to
greatly improve the performance of a function which capitalises letters in a
string. 

The video starts with a simple, unoptimised version which scans through the 
string linearly, updating any lower case alphabetic characters it finds. Then 
it shows a "branchless" version which still scans through linearly, but uses 
special conditional-move instructions to avoid needing to use as many jump
instructions. Finally, it shows a SIMD version which uses vector instructions 
to process strings in blocks of 32 characters at a time.

Throughout the video the presenter talks about the comparative performance of
his hand-coded assembly and what a C compiler would produce. I also think he
kind of implies that the SIMD version can only be achieved by writing assembly
or using intrinsics in C, and because of this, I was interested to see how well
the compiler could optimise the problem automatically.

In this document I am going to spend most of my time describing the 
optimisations and why they work, then at the end I will explore how to get the GNU compiler to do the optimisations automatically, and how to diagnose issues 
with its optimizer.

Before I get to that though, I am going to have to give a very brief 
introduction to reading assembly language, and describe some key components of 
computer architecture that the optimisations described here will take 
advantages of.

## A Crash Course in Assembly
A C compiler has a lot of leeway in how to convert code into instructions for 
the CPU, so in order to show exactly what is being executed, I'm going to 
have to go one level down and write things in assembler, as this gives complete
control over which instructions are used. 

Assembly is a pretty daunting subject, but in order to understand these 
examples here there are only a few things you need to know. First of all, in 
order to change any values in memory, you need to first move them into a 
register using a `mov` instruction, manipulate the value in the register and 
then write it back out to memory. So adding one to the value stored at memory 
address `x` would require something like this

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

I won't list all the jump instructions. The only thing you need to know is that
any instruction that begins with the letter 'j' is a jump, and everybody hates 
them because they mess up the instruction pipeline. 

## Why Do Modern CPUs Hate Jump Instructions So Much Anyway?

Back when I was a kid, CPU clock-speeds kept shooting up year on year. That is 
how you could tell computers were getting better, there was a number on a chart
that kept getting bigger and bigger. Then clock speeds started to flatten out,
and more recently the magic number that has been increasing is the number of 
cores on the CPU. 

Clock speeds and multiple cores aren't the only innovations that have happened
to computer architecture over the years, they are just some of the more easy to
understand concepts that fit nicely into advertisements. Other important 
innovations include Instruction Level Parallelism (ILP) which allows a CPU
to execute multiple instructions at the same time, and SIMD instructions which
can process multiple pieces of data with a single instruction. These are 
quite different techniques, but neither of them mix well with branching code.

### Instruction Pipelining 

Pipelining is one of the oldest forms of ILP, and has actually been around 
since the mid-80s 

When CPUs execute instructions, each instruction isn't really executed as an
atomic operation. What actually happens is that first the instruction is 
fetched, then it is decoded, then it is actually executed, and finally the 
result is written back. Actually, depending on the CPU architecture, there may
be more stages, but the same basic idea applies. In a pipelined architecture,
the CPU will start loading future instructions before it has finished processing
the current instruction.

Branches kill the CPU's ability to use a pipeline efficiently. For example, 
assuming that the CPU has a four stage pipeline, at instruction *n*, the CPU 
will already start loading instruction *n+3*. If instruction n says to jump to 
some other instruction, then instructions *n+1*, *n+2*, and *n+3* will have to 
be flushed from the pipeline so the new string of instructions can be processed.

### SIMD Instructions

SIMD stands for Single Instruction Multiple Data. When working wiht SIMD, you
use arrays of data instead of individual values. There are a few restrictions 
on what you can do with these data - the arrays have to be a spefific size,
and you have to apply the exact same operation to every element in the array.
Branching can make this diffuclt, as you might not know how many elements you 
have to deal with, and you might need to apply different operations to the 
elements of your array. There are ways around this, as will be shown in the 
examples coming up. 

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

### V1.0: A Very Jumpy Capitalisation Function
If I was to implement the capitalisation algorithm shown above, I would go for
something like this:

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

Basically, this implementation first loads a byte from memory, then it checks
if the value is between 'a' and 'z'. If it isn't then it just
increments the array index stored in `rcx` and starts from the top of the loop. 
If the value *is* between 'a' and 'z' then it subtracts 32 from the value and 
writes it back to memory, before incrementing the index and looping. 

One slightly weird thing here is that `eax` and `al` are the same register. 
With `al` being the lower 8 bits of `eax`. It's a little more efficient to read
the data into `eax`, zeroing the upper bits, and then writing out using
`al`, than it is to read and write using `al` on its own.  

On my computer, this takes about 3.32 seconds to process 10^9 characters. This
isn't great, and it's probably due to those two branches used to determine if
the character was between 'a' and 'z'.


## V1.5: A Slightly Less Jumpy Implementation

*You can skip this section if you want, it describes how to make things a bit
more efficient, but it doesn't introduce branchless instructions yet.*

If I take that snippet of C code shown above, turn it into a well-formed 
function, and get the compiler to generate an optimized version, it produces
an interesting optimisation that is worth talking about.

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
OK, I call this a "hack" but actually there are two separate things happening
here, and they are both considered fairly normal practice in the world of x86
assembly.

The first thing is that the `lea` instruction is used to perform arithmetic. 
This instruction - load effective address - was originally intended
for making pointer arithmetic more efficient, but there is nothing about it
that actually forces you to use it on a pointer. `lea r9d, [eax - 'a']` will
subtract 'a' from the value in `eax` and store the result in `rd9` using a 
single instruction.

The next bit, the really weird bit, is that it will compere the result to 26,
and jump if the value is not lower. As we have already subtracted 'a' from 
the original and as 'z' is 26 places away from 'a' if the original value was
greater than 'z' this test will result in a jump. What if the value was less
than 'a'? Where is the test for that? Well here is the thing `jnb` treats the
 value as *unsigned*. If you have a negative signed value, then the bits in the
register are going to be equivalent to some large unsigned number. If the value
in `eax` was less than `a`, then the unsigned result of subtracting 'a' will be
larger than 26. I find this pretty confusing, but I have seen it described in
introductory texts for x86 assembler, so I guess it is pretty common. It
definitely cuts out one of the jump instructions and it does give you a 
significant performance improvement, but there is still one jump left, so there
are still gains to be made.

### V2.0: A Branch-free Implementation
It's possible to remove the last jump instruction in the range check by using 
a *conditional* move instruction. These behave like a regular move instruction,
except that if their condition is not met, then they do nothing. Like the jump
instructions, there are several variants of conditional move that use different
conditions. `cmove` will be triggered if the previous comparison was equal, 
`cmovz` will be triggered if the last arithmetic operation resulted in zero, 
and `cmovb` will be triggered if the last comparison was below an *unsigned*
value. 

Combining `cmovb` with the trick described in the section above, the 
capitalisation loop can be implemented as follows:

```asm
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
characters that are not lower-case letters, this will result in a nonsense 
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


### V3.0 SIMD Implementation
One of the interesting things about the branchless implementation, is that no
mater what data are being processed, the exact same set of instructions are 
executed each iteration of the loop. Some of the instructions might not do
anything, depending on the data, but they always get executed. This means that
the algorithm is a good candidate for running as a Single Instruction, 
Multiple Data (SIMD) program. 

The x86 family of chips have had a few different SIMD facilities over the 
years. I think it started with MMX, and then went through a few flavours
of AVX. The most modern being AVX-512, but on my CPU I only have the 
slightly older AVX-2.

The SIMD functionality consist of large registers that are capable of holding
arrays of values and instructions which can be applied to these values in 
parallel. The AVX-2 registers on my computer are 256 bits wide, which means
they can hold 32 8-bit characters at once. The full contents of the SIMD version
of the capitalisation function is listed in 
[simd_capitalise.asm](src/simd_capitalise.asm), and includes code that figures
out how many 32 character blocks are in the input string, as well as the code
to capitalise any left-over characters if the string length is not an exact 
multiple of 32. I won't go into detail on those parts, and instead will just
focus on the SIMD loop body. 

This uses a slightly different formulation of the loop described at the start 
of the article. Instead of using an `if` statement, it is possible to do the 
capitalisation as follows:

```c
for(int i=0; i < len; ++i) {
    s[i] -= 32 * (s[i] >= 'a' && s[i] <= 'z')
}
```

This relies on the fact that C will convert the result of 
`(s[i] >= 'a' && s[i] <= 'z')` to either 1 or 0 depending on if it evaluates
to true or false. 

The assembler implementation of this is as follows:

```asm
        vmovdqu ymm1, [rel diff_vec] ; set up the constants that will be 
        vmovdqu ymm2, [rel a_vec]    ; used in the SIMD calculations
        vmovdqu ymm3, [rel z_vec]    
.updateBlock:
        vmovdqu ymm0, [rdi + rcx]   ; move the next block of input into a reg
        vpcmpgtb ymm4, ymm0, ymm2   ; test input >= 'a'
        vpcmpgtb ymm5, ymm3, ymm0   ; test input <= 'z'
        vpand ymm6, ymm1, ymm4      ; mask the diff based on >= 'a'
        vpand ymm7, ymm5, ymm6      ; mask the diff based om <= 'z'
        vpaddb ymm0, ymm7           ; add the masked diff vector to the input
        vmovdqu [rdi + rcx], ymm0   ; save the result
        add rcx,  block_size        ; index of next block
        dec rax
        jnz .updateBlock
```

This starts off by loading some constants into the `ymm` registers so that they
can be used later on. `diff_vec` is an array of 32 `-32` values, `a_vec` 
contains 32 ('a' -1) values and `z_vec` contains 32 ('z' + 1) values. The reason 
for not using `a` and `z` in the vectors, is because the SIMD instructions
only let us test for greater-than, and we want to test greater-than-or-equal and 
less-than-or-equal

Once the constants have been loaded, the loop begins. First it loads 256 bytes 
from the input array into `ymm0`. Then it does two comparison operations to test 
for the values that are >= 'a', and the values that are <= 'z'. This leaves two 
bit-masks in `ymm4` and `ymm5` which are ANDed with the vector of -32s in 
`ymm1` to give a vector that contains -32 in the positions where a lower-case 
character was seen and 0 were some other character was seen. Finally this
vector is added to the input, and written back out to memory and before moving
on to the next block of characters.

This implementation processes 10^9 characters in about 0.14 seconds on my 
computer, which is about 4.6 times as fast as the version which used 
conditional moves to avoid branching, and almost 21 times as fast as the 
original, branching implementation.

## Automatically Generating SIMD Code
Having devoted so much space to describing how these optimisations work, this
section on how to get `gcc` to automatically generate them is going to be 
pretty brief. There is not much to say besides which switches to use and how to
diagnose the problem when you are not getting the optimisation you are looking
for.

In an earlier section I said that `gcc` wasn't able to optimize away the extra 
jump instruction in the non-SIMD version of my capitalisation function. This 
wasn't the full truth. In this document I have shown two versions of the C 
function, the first version used an `if` statement and the second version 
multiplied the number being added to the values by a boolean. 

If I were to use the following version of the function:
```c
void capitalise(char *s, size_t l) {
    for(int i=0; i < len; ++i) {
        s[i] -= 32 * (s[i] >= 'a' && s[i] <= 'z')
    }
}
```

and if I compile that using `gcc -O2`, it would actually produce code very
similar to the 
[branch-free assembly implementation](#v20-a-branch-free-implementation) of 
the function. I believe that `-fif-conversion` is the specific optimisation 
flag that will cause `gcc` to use the conditional move instructions, but this 
flag is included in `-02`.

If you want to get `gcc` to produce AVX2 SIMD code, then you need to enable
`-ftree-vectorize` and also you need to tell `gcc` that it is OK to use AVX2
operations. By default, `gcc` will avoid using AVX2, and will use the older 
MMX instructions instead as not all x86 CPUs have AVX2. 

Using the C example above, and compiling with `gcc -O2 -ftree-vectorize -mavx2` will produce a result very similar to the 
[SIMD assembler implementation](#v30-simd-implementation) of the capitalize 
function. So for this case, at least, you do not need to use intrinsics to
get a good SIMD optimisation out of your C code.

### Diagnosing Issues With Optimizations
As shown above, the optimization can be sensitive to the initial C code that 
is supplied. Even if the implementation is logically equivalent, use of an `if`
statement can result in the optimizer failing to find the right optimization.
So just because you turn on an optimization flag, does not guarantee that it
will be applied. 

There are several ways you can diagnose this. The most obvious thing is to test
if there was positive change in performance. You should have some kind of a 
test harness in place before you even start making performance optimizations to
your code, so this should be easy to check. The second thing you can do is to 
inspect the instructions being produced by using the `-S` flag. Of course, 
this relies on you already having a very good idea of how the optimization 
should be implemented, and this isn't always obvious. 

Fortunately `gcc` provides some diagnostics that help you to understand what
optimisations have been applied, and which gives reasons for the ones which 
could not be applied. If you use the `-fopt-info-all` flag it will output
this information to the console while compiling. For example, in the case where `capitalise.c` uses an `if` statement:

```
gcc -O2 -ftree-vectorize -mavx2  -fopt-info-all -S capitalise.c
Unit growth for small function inlining: 13->13 (0%)

Inlined 0 calls, eliminated 0 functions

capitalise.c:4:5: missed: couldn't vectorize loop
capitalise.c:4:5: missed: not vectorized: control flow in loop.
capitalise.c:3:6: note: vectorized 0 loops in function.
```

Or in the case where `capitalise.c` uses multiplication by a boolean instead
of using `if`:

```
gcc -O2 -ftree-vectorize -mavx2  -fopt-info-all -S capitalise.c
Unit growth for small function inlining: 13->13 (0%)

Inlined 0 calls, eliminated 0 functions

capitalise.c:4:5: optimized: loop vectorized using 32 byte vectors
capitalise.c:3:6: note: vectorized 1 loops in function.
```

Here I can see that the loop was optimized and that the 32 byte AVX-2 
instructions were used. If I saw that only 16 byte vectors were being used, 
then I would know the older MMX instructions were being used instead. 

