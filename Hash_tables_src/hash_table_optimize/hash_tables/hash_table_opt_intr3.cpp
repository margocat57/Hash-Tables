#include "hash_table.h"
#include <cstdint>
#include <limits.h>

extern "C" unsigned int find_node_first_eight(const uint32_t* hashes, const uint32_t hash, const char* keys, const char* key);

__attribute__((always_inline))
extern "C" int my_strcmp(const char* key1, const char* key2){

    uint32_t cmp = 0;

    asm(".intel_syntax noprefix\n\t" 
            "vmovdqa ymm2, [%2]\n\t"       
            "vmovdqa ymm1, [%1]\n\t"           
            "vpcmpeqb ymm0, ymm1, ymm2\n\t" 
            "vpmovmskb %0, ymm0\n\t" 
            ".att_syntax prefix\n\t"
            :"=r"(cmp)                    
            :"r"(key1), "r"(key2)           
            : "ymm0", "ymm1", "ymm2"    
    );

    return cmp != 0xFFFFFFFF; // чтобы логика как у стркмп сохранялась
}

__attribute__((noinline))
int find_node_optimized(const bucket_t* bucket, const uint32_t hash, const char* key){
    assert(key);
    assert(bucket);

    int mask_new = 0;

    uint32_t* hashes = bucket->hashes;
    char* keys = bucket->keys;
    int size_bucket = bucket->size;

    int idx = find_node_first_eight(hashes, hash, keys, key);
    if(idx != -1) return idx;

    for(int i = BLOCK_DATA_AMOUNT; i < size_bucket; i++){
        char* key_in_hashtable = keys + i * SIZE_WORD;
        if(hashes[i] == hash && !my_strcmp(key_in_hashtable, key)){
            return i;
        }
    }
    return INT_MAX;
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