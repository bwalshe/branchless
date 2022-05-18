        global capitalise

        section .text 
capitalise:
        push rbp            ; Preserve the return address
        mov rbp, rsp        ; Set up the stack (these two lines aren't really neded
        xor rax, rax        ; Clear out rax as we will only use the lower bytes
        xor rcx, rcx        ; Zero rcx so we can use it to count how many chars
                            ; have been processed
.updateChar:
        movzx eax, BYTE [rdi + rcx]  ; Move a char from the input into rax
        lea r8d, [eax - 97] ; Find the index of the character in relation to 'A'
        lea r9d, [eax - 32] ; Find the index of the upper case version of the 
                            ; char, assuming the current one is lowercase
        cmp r8b, 26         ; We will treat the value in r8 as an unsigned 
                            ; number, hich means that if the value in eax was 
                            ; less than 97 then this will appear to be some 
                            ; nonsense high number, and we just need to check 
                            ; that x - 97 < 26 
        cmovb eax, r9d      ; Conditinally move the upper case letter into the
                            ; array
        mov BYTE [rsi + rcx], al 
        add rcx, 1
        cmp rcx, rdx        ; Check if we have reached the end of the string
        jne .updateChar
.end:
        pop rbp
        ret
