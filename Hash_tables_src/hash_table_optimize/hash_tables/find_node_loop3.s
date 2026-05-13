section .text

SIZE_WORD equ 32

extern my_strcmp
global find_node_first_eight

; hashes, hash, strrings(r12), key(r13)

find_node_first_eight:
                        push rbx
                        push r12
                        push r13
                        push r14
                        sub rsp, 8

                        mov r12, rdx
                        mov r13, rcx

                        vmovd  xmm0, esi       
                        vpbroadcastd    ymm0, xmm0
                        vmovdqa  ymm1, [rdi]        
                        vpcmpeqd ymm0, ymm0, ymm1 
                        vmovmskps  ebx, ymm0 

                        test ebx, ebx
                        jz .exit

.find_loop:
                        tzcnt   edi, ebx
                        mov     r14, rdi
                        sal     rdi, 5
                        lea rdi, qword [r12 + rdi]
                        mov     rsi, r13
                        call    my_strcmp
                        test    eax, eax
                        je      .exit_find

                        blsr    ebx, ebx
                        test    ebx, ebx
                        jne     .find_loop 


.exit:                  mov rax, -1
                        add rsp, 8
                        pop r14
                        pop r13
                        pop r12
                        pop rbx
                        ret

.exit_find:             mov rax, r14
                        add rsp, 8
                        pop r14
                        pop r13
                        pop r12
                        pop rbx
                        ret

