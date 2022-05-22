        global simd_capitalise
        section .text 

simd_capitalise:
        push rdx            ; rdx currently holds the size of the string
        mov rax, rdx        ; calculate the number of 32 char blocks in he input
        xor rdx, rdx
        mov rcx, 32
        div rcx
        pop rdx             ; restore the input size to rdx
        xor rcx, rcx        ; clear out rcx because it will be used as a counter
        cmp rax, 0          ; if there were less than 32 chars, skip the simd section
        je .updateChar        
        vmovdqu ymm1, [rel diff]    ; this is the difference between 'A' and 'a'
        vmovdqu ymm2, [rel avec]    ; used for testing if the input >= 'a'
        vmovdqu ymm3, [rel zvec]    ; usef for testing if the input <= 'z'
.updateBlock:
        vmovdqu ymm0, [rdi + rcx]   ; move the next block of input into a reg
        vpcmpgtb ymm4, ymm0, ymm2   ; test input >= 'a'
        vpand ymm5, ymm1, ymm4      ; mask the diff vector that will be added
        vpcmpgtb ymm4, ymm3, ymm0   ; test input <= 'z'
        vpand ymm6, ymm5, ymm4      ; mask the diff vector again
        vpaddb ymm0, ymm6           ; add the masked diff vector to the input
        vmovdqu [rsi + rcx], ymm0   ; save the result
        add rcx,  32                ; index of next block
        dec rax
        jnz .updateBlock
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

        ret

        section .data
diff: times 32 db 'A' - 'a'
avec: times 32 db 'a' - 1
zvec: times 32 db 'z' + 1 
