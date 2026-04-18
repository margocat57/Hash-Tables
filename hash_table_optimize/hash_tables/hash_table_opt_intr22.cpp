#include "hash_table22.h"
#include <cstdint>
#include <immintrin.h>
#include <bits/types.h>
#include <immintrin.h>
#include <stdlib.h>
#include <bsd/stdlib.h>

void init_masks();

hash_table* hash_table_ctor(){
    hash_table* ht = (hash_table*)calloc(1, sizeof(hash_table));
    if(!ht){
        fprintf(stderr, "Memory allocation error in ht");
        return NULL;
    }

    ht->size = 70001;
    ht->elements = (bucket_t*)calloc(ht->size, sizeof(bucket_t));
    if(!ht->elements){
        fprintf(stderr, "Memory allocation error in ht->elements");
        return NULL;
    }

    return ht;
}

void hash_table_insert(const char* key, hash_table* ht){
    assert(ht);
    assert(key);

    uint32_t hash = hash_crc32(key);
    uint32_t idx = hash % ht->size;

    if(!ht->elements[idx].keys && !ht->elements[idx].hashes){
        ht->elements[idx].keys = (char**)calloc(10, sizeof(char*));
        if(!ht->elements[idx].keys){
            fprintf(stderr, "Calloc keys in hash_table_insert error");
            return;
        }

        ht->elements[idx].hashes = (uint32_t*)calloc(10, sizeof(uint32_t));
        if(!ht->elements[idx].hashes){
            fprintf(stderr, "Calloc hashes in hash_table_insert error");
            return;
        }
        ht->elements[idx].capacity = 10;
    }
    int capacity = ht->elements[idx].capacity;
    if(ht->elements[idx].first_free >= capacity){
        char** keys = (char**)recallocarray(ht->elements[idx].keys, capacity, capacity * 2, sizeof(char*));
        if(!keys){
            fprintf(stderr, "Realloc keys in hash_table_insert error");
            return;
        }

        uint32_t* hashes = (uint32_t*)recallocarray(ht->elements[idx].hashes, capacity, capacity * 2, sizeof(uint32_t));
        if(!hashes){
            fprintf(stderr, "Realloc hashes in hash_table_insert error");
            return;
        }

        ht->elements[idx].capacity*=2;
        ht->elements[idx].keys = keys;
        ht->elements[idx].hashes = hashes;
    }

    int first_free = ht->elements[idx].first_free;

    ht->elements[idx].hashes[first_free] = hash;
    ht->elements[idx].keys[first_free] = (char*)calloc(32, sizeof(char));
    memcpy(ht->elements[idx].keys[first_free], key, 32);

    ht->elements[idx].first_free++;

}

bool hash_table_find(const char* key, const hash_table* ht){
    assert(ht);
    assert(key);

    uint32_t hash = hash_crc32(key);
    uint32_t idx = hash % ht->size;

    int index = find_node(&ht->elements[idx], hash, key);

    if(index != ht->elements[idx].capacity){
        return true;
    }
    
    return false;

}


__attribute__((noinline))
int find_node(bucket_t* bucket, const uint32_t hash, const char* key){
    assert(key);

    uint32_t* hashes = bucket->hashes;
    char** keys = bucket->keys;
    int size_bucket = bucket->first_free;

    for(int i = 0; i < size_bucket; i++){
        if(hashes[i] == hash && !strcmp(keys[i], key)){
            return i;
        }
    }
    return bucket->capacity;
}

// __m128i _mm_set1_epi32 (int a)



int find_node_(bucket_t* bucket, const uint32_t hash, const char* key){
    assert(key);

    uint32_t* hashes = bucket->hashes;
    char** keys = bucket->keys;
    int size_bucket = bucket->first_free;

    __m256i hash_intr = _mm256_set1_epi32(hash);

    __m256i first_four = _mm256_lddqu_si256((__m256i const*)hashes);

    __m256i mask =  _mm256_cmpeq_epi32 (hash_intr, first_four);

    int mask_new = _mm256_movemask_ps((__m256)mask);

    while(mask_new){
        int index = __builtin_ctz(mask_new); // младший установленный бит
        if(keys[index] && !strcmp(keys[index], key)){
            return index;
        }
        mask_new &= ~(1 << index); // сбраиываем младший установленный бит
    }

    size_bucket -= 4;

    for(int i = 4; i < size_bucket + 4; i++){
        if(hashes[i] == hash && !strcmp(keys[i], key)){
            return i;
        }
    }
    return bucket->capacity;
}


/*
int find_node_(bucket_t* bucket, const uint32_t hash, const char* key){
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
*/


void hash_table_dtor(hash_table* ht){
    if(!ht) return;
    
    for(int idx = 0; idx < ht->size; idx++){
        char** keys = ht->elements[idx].keys;
        if(!keys){
            if(ht->elements[idx].hashes){
                free(ht->elements[idx].hashes);
            }
            continue;
        } 

        int fact_size = ht->elements[idx].first_free;
        for(int i = 0; i < fact_size; i++){
            free(keys[i]);
        }

        free(ht->elements[idx].keys);

        if(ht->elements[idx].hashes){
            free(ht->elements[idx].hashes);
        }

    }

    free(ht->elements);
    free(ht);
}


__attribute__((noinline))
uint32_t hash_crc32(const char* s){
    assert(s);

    uint32_t crc = 0;
    const uint64_t* p = (const uint64_t*)s;

    crc = _mm_crc32_u64(crc, p[0]);
    crc = _mm_crc32_u64(crc, p[1]);
    crc = _mm_crc32_u64(crc, p[2]);
    crc = _mm_crc32_u64(crc, p[3]);
    
    return crc;

}