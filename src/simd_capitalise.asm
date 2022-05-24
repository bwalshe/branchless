        global simd_capitalise
 
%define diff 'A' - 'a'
%define alphabet_len 'z' - 'a' + 1
%define block_size 32

       section .text 

simd_capitalise:
        cmp rdx, 0          ; rdx currently holds the size of the string
        je .end
        push rdx                    
        mov rax, rdx        ; calculate the number of 32 char blocks in he input
        xor rdx, rdx
        mov rcx, block_size
        div rcx
        pop rdx             ; restore the input size to rdx
        xor rcx, rcx        ; clear out rcx because it will be used as a counter
        cmp rax, 0          ; if there were less than 32 chars, skip the simd section
        je .updateChar        
        vmovdqu ymm1, [rel diff_vec] ; set up the constants that will be 
        vmovdqu ymm2, [rel a_vec]    ; used in the SIMD calculations
        vmovdqu ymm3, [rel z_vec]    
.updateBlock:
        vmovdqu ymm0, [rdi + rcx]   ; move the next block of input into a reg
        vpcmpgtb ymm4, ymm0, ymm2   ; test input >= 'a'
        vpand ymm5, ymm1, ymm4      ; mask the diff vector that will be added
        vpcmpgtb ymm4, ymm3, ymm0   ; test input <= 'z'
        vpand ymm6, ymm5, ymm4      ; mask the diff vector again
        vpaddb ymm0, ymm6           ; add the masked diff vector to the input
        vmovdqu [rsi + rcx], ymm0   ; save the result
        add rcx,  block_size        ; index of next block
        dec rax
        jnz .updateBlock
        cmp rcx, rdx
        je .writeNull
.updateChar:
        movzx eax, BYTE [rdi + rcx]  ; Move a char from the input into rax
        lea r8d, [eax - 'a']    ; Where is the character in relation to 'A'
        lea r9d, [eax + diff]   ; Connvnert  to upper case, assuming the 
                                ; current char is lowercase
        cmp r8b, alphabet_len   ; We will treat the value in r8 as an unsigned 
                            ; number, hich means that if the value in eax was 
                            ; less than 'a' then this will appear to be some 
                            ; nonsense high number, and we just need to check 
                            ; that x - 'a' < 26 
        cmovb eax, r9d      ; Conditinally move the upper case letter into the
                            ; array
        mov BYTE [rsi + rcx], al 
        add rcx, 1
        cmp rcx, rdx 
        jne .updateChar
.writeNull:
        mov BYTE [rsi + rcx], 0
.end:
        ret

        section .data
diff_vec: times 32 db 'A' - 'a'
a_vec: times 32 db 'a' - 1
z_vec: times 32 db 'z' + 1 
