#include "hash_table.h"
#include <cstdint>
#include <immintrin.h>
#include <bits/types.h>

__attribute__((noinline))
int find_node(bucket_t* bucket, const uint32_t hash, const char* key){
    assert(key);

    uint32_t* hashes = bucket->hashes;
    char* keys = bucket->keys;
    int size_bucket = bucket->first_free;

    for(int i = 0; i < size_bucket; i++){
        if(hashes[i] == hash && !strcmp(keys + i *size_word, key)){
            return i;
        }
    }
    return bucket->capacity;
}


__attribute__((noinline))
uint32_t hash_func(const char* s){
    assert(s);
    uint32_t  poly = 0;
    uint32_t  p = 31;
    unsigned char* data= (unsigned char*)s;

    for(int i = 0; i < 32; i++){
        poly = poly * p + data[i];
    }
    return poly;
}
