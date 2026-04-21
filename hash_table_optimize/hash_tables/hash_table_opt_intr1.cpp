#include "hash_table.h"

__attribute__((noinline))
int find_node(bucket_t* bucket, const uint32_t hash, const char* key){
    assert(key);

    uint32_t* hashes = bucket->hashes;
    char* keys = bucket->keys;
    int size_bucket = bucket->size;

    int i = bucket->list_head;
    for(int idx = 0; idx < size_bucket; idx++){
        char* key_in_hashtable = keys + i * size_word;
        if(hashes[i] == hash  && key_in_hashtable[0] && !strcmp(key_in_hashtable, key)){
            return i;
        }
        i = bucket->next[i];
    }
    return bucket->capacity;
}

__attribute__((noinline))
int find_node_optimized(bucket_t* bucket, const uint32_t hash, const char* key){
    return find_node(bucket, hash, key);
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