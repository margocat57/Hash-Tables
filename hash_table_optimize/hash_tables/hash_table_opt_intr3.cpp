#include "hash_table.h"


extern "C" unsigned int my_strcmp(const char* s1, const char* s2);


int find_node(bucket_t* bucket, const uint32_t hash, const char* key){
    assert(key);

    int mask_new = 0;

    uint32_t* hashes = bucket->hashes;
    if(!hashes) return bucket->capacity;

    char** keys = bucket->keys;
    if(!keys) return bucket->capacity;

    int size_bucket = bucket->first_free;

    asm(".intel_syntax noprefix\n\t" 
        "vmovd   xmm0, %2\n\t"           
        "vpbroadcastd    ymm0, xmm0\n\t"
        "vlddqu   ymm1, [%1]\n\t"             
        "vpcmpeqd ymm0, ymm0, ymm1\n\t"
        "vmovmskps  %0, ymm0\n\t"
        "vzeroupper\n\t"
        ".att_syntax prefix\n\t"
        :"=r"(mask_new)                    
        :"r"(hashes), "r"(hash)           
        : "ymm0", "ymm1"                     
    );  

    while(mask_new){
        int index = __builtin_ctz(mask_new); // младший установленный бит
        if(keys[index] && my_strcmp(keys[index], key) == 0xFFFFFFFF){
            return index;
        }
        mask_new &= ~(1 << index); // сбраиываем младший установленный бит
    }

    size_bucket -= 8;

    for(int i = 8; i < size_bucket + 8; i++){
        if(hashes[i] == hash && my_strcmp(keys[i], key) == 0xFFFFFFFF){
            return i;
        }
    }
    return bucket->capacity;
}



__attribute__((noinline))
uint32_t hash_func(const char* s){
    assert(s);

    uint32_t crc = 0;

    crc = _mm_crc32_u64(crc, *((const uint64_t*)(s + 0)));
    crc = _mm_crc32_u64(crc, *((const uint64_t*)(s + 8)));
    crc = _mm_crc32_u64(crc, *((const uint64_t*)(s + 16)));
    crc = _mm_crc32_u64(crc, *((const uint64_t*)(s + 24)));

    return crc;

}