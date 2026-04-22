section .text

global my_strcmp

my_strcmp:  xor eax, eax
            vmovdqa ymm2, [rdi]      
            vmovdqa ymm1, [rsi]          
            vpcmpeqb ymm0, ymm1, ymm2
            vpmovmskb eax, ymm0
            ret
            