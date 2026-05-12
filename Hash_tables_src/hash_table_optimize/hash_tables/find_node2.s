section .text

SIZE_WORD equ 32

extern strcmp
global find_node_first_eight

; rdi = hashes
; esi = hash
; rdx = strings
; rcx = key
; return: index 0..7 or -1

find_node_first_eight:
        push rbx
        push r12
        push r13

        mov r12, rdx        ; strings
        mov r13, rcx        ; key

        vmovd xmm0, esi
        vpbroadcastd ymm0, xmm0
        vmovdqu ymm1, [rdi]        ; use vmovdqa only if hashes is 32-byte aligned
        vpcmpeqd ymm0, ymm0, ymm1
        vmovmskps ebx, ymm0

        test ebx, ebx
        je .not_found

        vzeroupper

.loop:
        tzcnt r8d, ebx              ; r8d = index of lowest set bit

        mov rdi, r8
        shl rdi, 5                  ; index * 32
        add rdi, r12                ; &strings[index * SIZE_WORD]

        cmp byte [rdi], 0
        je .next

        mov rsi, r13
        call strcmp wrt ..plt

        test eax, eax
        je .found

.next:
        blsr ebx, ebx               ; clear lowest set bit
        jne .loop

.not_found:
        mov rax, -1
        pop r13
        pop r12
        pop rbx
        ret

.found:
        tzcnt eax, ebx              ; current lowest set bit is the matching index
        pop r13
        pop r12
        pop rbx
        ret
