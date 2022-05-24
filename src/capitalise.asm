        global capitalise

        section .text 
capitalise:
        cmp rsi, 0          ; rdx is the length of the string
        je .end
        xor rax, rax        ; Clear out rax as we will only use the lower bytes
        xor rcx, rcx        ; Zero rcx so we can use it to count how many chars
                            ; have been processed
.updateChar:
        mov al, BYTE [rdi + rcx]    ; Move a char from the input into rax
        cmp al, 'a'                 ; Check if the value is less than 'a'
        jl .writeChar 
        cmp al, 'z'                 ; Check if the value is greater than 'z'
        jg .writeChar 
        add al, 'A' - 'a'           ; The current char is lower case, make it
                                    ; uppercase
.writeChar:
        mov BYTE [rdi + rcx], al 
        add rcx, 1
        cmp rcx, rsi        ; Check if we have reached the end of the string
        jne .updateChar
.end:
        ret
