        global capitalise

        section .text 
capitalise:
        push rbp            ; Preserve the return address
        mov rbp, rsp        ; Set up the stack (these two lines aren't really neded
        xor rax, rax        ; Clear out rax as we will only use the lower bytes
.updateChar:
        mov al, BYTE [rdi]  ; Move a char from the input into rax
        lea ecx, [eax - 97] ; Find the index of the character in relation to 'A'
        lea edx, [eax - 32] ; Find the index of the lower case version of the 
                            ; char, assuming the current one is uppercase
        cmp cl, 26          ; Treat the value in ecx as an unsigned value. 
                            ; If the value in eax was less than 97 then this 
                            ; will now be some nonsense high number, so we 
                            ; just need to check x - 97 < 26 
        cmovb eax, edx      ; Conditinally move the upper case letter into the
                            ; array
        mov BYTE [rdi], al 
        inc rdi
        cmp al, 0           ; Check if we have reached the end of the string
        jne .updateChar
.end:
        pop rbp
        ret
