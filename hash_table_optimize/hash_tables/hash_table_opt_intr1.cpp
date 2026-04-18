#include "hash_table.h"

__attribute__((noinline))
int find_node(bucket_t* bucket, const uint32_t hash, const char* key){
    assert(key);

    uint32_t* hashes = bucket->hashes;
    char** keys = bucket->keys;
    int size_bucket = bucket->first_free;

    for(int i = 4; i < size_bucket + 4; i++){
        if(hashes[i] == hash && !strcmp(keys[i], key)){
            return i;
        }
    }
    return bucket->capacity;
}


__attribute__((noinline))
uint32_t hash_crc32(const char* s){
    assert(s);

    uint32_t crc = 0;

    crc = _mm_crc32_u64(crc, *((const uint64_t*)(s + 0)));
    crc = _mm_crc32_u64(crc, *((const uint64_t*)(s + 8)));
    crc = _mm_crc32_u64(crc, *((const uint64_t*)(s + 16)));
    crc = _mm_crc32_u64(crc, *((const uint64_t*)(s + 24)));

    return crc;

}