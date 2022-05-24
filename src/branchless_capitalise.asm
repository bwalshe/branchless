        global branchless_capitalise

%define diff 'A' - 'a'
%define alphabet_len 'z' - 'a' + 1

        section .text 
branchless_capitalise:
        cmp rsi, 0          ; If the length of the input is 0, jump to the end
        je .end
        xor rax, rax        ; Clear out rax as we will only use the lower bytes
        xor rcx, rcx        ; Zero rcx so we can use it to count how many chars
                            ; have been processed
.updateChar:
        movzx eax, BYTE [rdi + rcx] ; Move a char from the input into rax
        lea r8d, [eax - 'a']        ; Where is the character in relation to 'A'
        lea r9d, [eax + diff]       ; Convert the char to uppercase, assuming
                                    ; the current char is lowercase
        cmp r8b, alphabet_len       ; We will treat the value in r8 as an unsigned 
                            ; number, which means that if the value in eax was 
                            ; less than 97 then this will appear to be some 
                            ; nonsense high number, and we just need to check 
                            ; that x - 'a' < 26 
        cmovb eax, r9d      ; Conditinally move the upper case letter into the
                            ; array
        mov BYTE [rdi + rcx], al 
        add rcx, 1
        cmp rcx, rsi        ; Check if we have reached the end of the string
        jne .updateChar
.end:
        ret
