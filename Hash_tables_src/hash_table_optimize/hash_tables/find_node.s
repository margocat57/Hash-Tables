section .text

SIZE_WORD equ 32

extern strcmp
global find_node_first_eight
;
; hashes, hash, strrings, key

find_node_first_eight:
                        push rbx
                        push r12
                        push r13

                        mov r12, rdx
                        mov r13, rcx

                        vmovd  xmm0, esi       
                        vpbroadcastd    ymm0, xmm0
                        vmovdqa  ymm1, [rdi]        
                        vpcmpeqd ymm0, ymm0, ymm1 
                        vmovmskps  ebx, ymm0 
     

                        test rbx, 1
                        jz .check_second_bit

                        lea rdi, qword [r12 + 0 * SIZE_WORD]
                        mov rsi, r13
                        call strcmp wrt ..plt

                        mov r9, 0
                        test rax, rax
                        je .exit_find

.check_second_bit:      test rbx, 2
                        jz .check_third_bit

                        lea rdi, qword [r12 + 1 * SIZE_WORD]
                        mov rsi, r13
                        call strcmp wrt ..plt

                        mov r9, 1
                        test rax, rax
                        je .exit_find

.check_third_bit:       test rbx, 4
                        jz .check_fourth_bit

                        lea rdi, qword [r12 + 2 * SIZE_WORD]
                        mov rsi, r13
                        call strcmp wrt ..plt

                        mov r9, 2
                        test rax, rax
                        je .exit_find

.check_fourth_bit:      test rbx, 8
                        jz .check_fifth_bit

                        lea rdi, qword [r12 + 3 * SIZE_WORD]
                        mov rsi, r13
                        call strcmp wrt ..plt

                        mov r9, 3
                        test rax, rax
                        je .exit_find

.check_fifth_bit:       test rbx, 16
                        jz .check_sixth_bit

                        lea rdi, qword [r12 + 4 * SIZE_WORD]
                        mov rsi, r13
                        call strcmp wrt ..plt

                        mov r9, 4
                        test rax, rax
                        je .exit_find

.check_sixth_bit:       test rbx, 32
                        jz .check_seventh_bit

                        lea rdi, qword [r12 + 5 * SIZE_WORD]
                        mov rsi, r13
                        call strcmp wrt ..plt

                        mov r9, 5
                        test rax, rax
                        je .exit_find

.check_seventh_bit:     test rbx, 64
                        jz .check_eighth_bit

                        lea rdi, qword [r12 + 6 * SIZE_WORD]
                        mov rsi, r13
                        call strcmp wrt ..plt

                        mov r9, 6
                        test rax, rax
                        je .exit_find

.check_eighth_bit:      test rbx, 128
                        jz .exit

                        lea rdi, qword [r12 + 7 * SIZE_WORD]
                        mov rsi, r13
                        call strcmp wrt ..plt

                        mov r9, 7
                        test rax, rax
                        je .exit_find

.exit:                  pop r13
                        pop r12
                        pop rbx
                        mov rax, -1
                        ret

.exit_find:             pop r13
                        pop r12
                        pop rbx
                        mov rax, r9
                        ret

