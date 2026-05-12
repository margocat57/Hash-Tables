#include "bucket_func/bucket_func.h"
#include "hash_table.h"
#include <climits>

extern "C" unsigned int find_node_first_eight(const uint32_t* hashes, const uint32_t hash, const char* keys, const char* key);

__attribute__((noinline))
int find_node_optimized(const bucket_t* bucket, const uint32_t hash, const char* key){
    assert(key);
    assert(bucket);

    uint32_t* hashes = bucket->hashes;
    char* keys = bucket->keys;
    int size_bucket = bucket->size;

    int idx = find_node_first_eight(hashes, hash, keys, key);
    if(idx != -1) return idx;

    for(int i = BLOCK_DATA_AMOUNT; i < size_bucket; i++){
        char* key_in_hashtable = keys + i * SIZE_WORD;
        if(hashes[i] == hash && !strcmp(key_in_hashtable, key)){
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