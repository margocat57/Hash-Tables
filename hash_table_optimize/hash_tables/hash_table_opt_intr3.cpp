#include "hash_table.h"


extern "C" unsigned int my_strcmp(const char* s1, const char* s2);


int find_node_optimized(bucket_t* bucket, const uint32_t hash, const char* key){
    assert(key);

    int mask_new = 0;

    uint32_t* hashes = bucket->hashes;
    char* keys = bucket->keys;
    int size_bucket = bucket->size;

    asm(".intel_syntax noprefix\n\t" 
        "vmovd   xmm0, %2\n\t"           
        "vpbroadcastd    ymm0, xmm0\n\t" //  __m256i hash_intr = _mm256_set1_epi32(hash);
        "vmovdqa   ymm1, [%1]\n\t"        // _m256i first_eight = _mm256_load_si256((__m256i const*)(hashes)); 
        "vpcmpeqd ymm0, ymm0, ymm1\n\t"  //  __m256i mask =  _mm256_cmpeq_epi32 (hash_intr, first_eight);
        "vmovmskps  %0, ymm0\n\t"        // int mask_new = _mm256_movemask_ps((__m256)mask);
        ".att_syntax prefix\n\t"
        :"=r"(mask_new)                    
        :"r"(hashes), "r"(hash)           
        : "ymm0", "ymm1"                     
    );  

    while(mask_new){
        int index = __builtin_ctz(mask_new); // младший установленный бит
        char* key_in_hashtable = keys + index * size_word;
        if(key_in_hashtable[0] && my_strcmp(key_in_hashtable, key) == 0xFFFFFFFF){
            return index;
        }
        mask_new &= ~(1 << index); // сбраиываем младший установленный бит
    }

    size_bucket -= block_data_amount;

    for(int i = block_data_amount; i < size_bucket + block_data_amount; i++){
        char* key_in_hashtable = keys + i * size_word;
        if(hashes[i] == hash  && key_in_hashtable[0] && my_strcmp(key_in_hashtable, key) == 0xFFFFFFFF){
            return i;
        }
    }
    return bucket->capacity;
}

__attribute__((noinline))
int find_node(bucket_t* bucket, const uint32_t hash, const char* key){
    assert(key);

    uint32_t* hashes = bucket->hashes;
    char* keys = bucket->keys;
    int size_bucket = bucket->size;

    int i = bucket->list_head;
    for(int idx = 0; idx < size_bucket; idx++){
        char* key_in_hashtable = keys + i * size_word;
        if(hashes[i] == hash && key_in_hashtable[0] && !strcmp(key_in_hashtable, key)){
            return i;
        }
        i = bucket->next[i];
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