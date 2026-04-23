#include "hash_table.h"
#include <cstdint>
#include <immintrin.h>
#include <bits/types.h>

__attribute__((noinline))
int find_node_optimized(bucket_t* bucket, const uint32_t hash, const char* key){
    return find_node(bucket, hash, key);
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
