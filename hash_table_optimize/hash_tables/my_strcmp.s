section .text

global my_strcmp

my_strcmp:  xor eax, eax
            vmovdqu ymm2, [rdi]      
            vmovdqu ymm1, [rsi]          
            vpcmpeqb ymm0, ymm1, ymm2
            vpmovmskb eax, ymm0
            vzeroupper
            ret
            