        global capitalise

        section .text 
capitalise:
        push rbp            ; preserve the return address
        mov rbp, rsp        ; set up the stack (these two lines aren't really neded
        xor rax, rax        ; clear out rax as we will only use the lower bytes
.updateChar:
        mov al, BYTE [rdi]  ; move a char from the input into rax
        lea ecx, [eax - 97] ; find the index of the character in relation to 'A'
        lea edx, [eax - 32] ; find the index of the lower case version of the 
                            ; char, assuming the current one is uppercase
        cmp cl, 26          ; treat the value in ecx as an unsigned value. 
                            ; If the value in eax was less than 97 then this 
                            ; will now be some nonsense high number, so we 
                            ; just need to x - 97 < 26 
        cmovb eax, edx      ; conditinally move the lower case letter into the
                            ; array
        mov BYTE [rdi], al 
        inc rdi
        cmp al, 0           ; Check if we have reached the end of the string
        jne .updateChar
.end:
        pop rbp
        ret
