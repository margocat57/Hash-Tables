#include "hash_table.h"


int find_node(bucket_t* bucket, const uint32_t hash, const char* key){
    assert(key);

    int mask_new = 0;

    uint32_t* hashes = bucket->hashes;
    if(!hashes) return bucket->capacity;

    char* keys = bucket->keys;
    if(!keys) return bucket->capacity;

    int size_bucket = bucket->first_free;

    asm(".intel_syntax noprefix\n\t" 
        "vmovd   xmm0, %2\n\t"           
        "vpbroadcastd    ymm0, xmm0\n\t" //  __m256i hash_intr = _mm256_set1_epi32(hash);
        "vlddqu   ymm1, [%1]\n\t"        // _m256i first_four = _mm256_lddqu_si256((__m256i const*)hashes);
        "vpcmpeqd ymm0, ymm0, ymm1\n\t"  //  __m256i mask =  _mm256_cmpeq_epi32 (hash_intr, first_four);
        "vmovmskps  %0, ymm0\n\t"        // int mask_new = _mm256_movemask_ps((__m256)mask);
        "vzeroupper\n\t"
        ".att_syntax prefix\n\t"
        :"=r"(mask_new)                    
        :"r"(hashes), "r"(hash)           
        : "ymm0", "ymm1"                     
    );  

    while(mask_new){
        int index = __builtin_ctz(mask_new); // младший установленный бит
        char* key_in_hashtable = keys + index * size_word;
        if(key_in_hashtable && !strcmp(key_in_hashtable, key)){
            return index;
        }
        mask_new &= ~(1 << index); // сбраиываем младший установленный бит
    }

    size_bucket -= 8;

    for(int i = 8; i < size_bucket + 8; i++){
        if(hashes[i] == hash && !strcmp(keys + i *size_word, key)){
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