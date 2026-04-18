#include "hash_table.h"
#include <cstdint>
#include <immintrin.h>
#include <bits/types.h>

hash_table* hash_table_ctor(){

    hash_table* ht = (hash_table*)calloc(1, sizeof(hash_table));
    if(!ht){
        fprintf(stderr, "Memory allocation error in ht");
        return NULL;
    }

    ht->size = 70001;
    ht->elements = (node_t**)calloc(ht->size, sizeof(node_t*));
    if(!ht->elements){
        fprintf(stderr, "Memory allocation error in ht->elements");
        return NULL;
    }

    return ht;
}

unsigned int my_strcmp(const char* key1, const char* key2){
    int result = 1;

    asm(".intel_syntax noprefix\n\t" 
        "vmovdqu ymm2, [%1]\n\t"           
        "vmovdqu ymm1, [%2]\n\t"            
        "vpcmpeqb ymm0, ymm1, ymm2\n\t"
        "vpmovmskb %0, ymm0\n\t"
        ".att_syntax prefix\n\t"
        :"=r"(result)                    
        :"r"(key1), "r"(key2)             
        : "ymm0", "ymm1", "ymm2"                     
    );    

    return result;
}

void hash_table_insert(const char* key, const int len, hash_table* ht){
    assert(ht);
    assert(key);

    uint32_t hash = hash_crc32(key);
    uint32_t idx = hash % ht->size;

    node_t* node = (node_t*)calloc(1, sizeof(node_t));

    node->next = ht->elements[idx];
    ht->elements[idx] = node;

    node->hash = hash;
    node->key = strdup(key);

}

bool hash_table_find(const char* key, const int len, const hash_table* ht){
    assert(ht);
    assert(key);

    uint32_t hash = hash_crc32(key);
    uint32_t idx = hash % ht->size;

    node_t* node = find_node(ht->elements[idx], hash, key);

    if(node){
        return true;
    }
    
    return false;

}


__attribute__((noinline))
node_t* find_node(node_t* node, const uint32_t hash, const char* key){
    assert(key);

    while(node){
        if(node->hash == hash && my_strcmp(node->key, key) == 0xFFFFFFFF){
            return node;
        }
        node = node->next;
    }

    return node;
}

void hash_table_dtor(hash_table* ht){
    if(!ht) return;
    
    for(int idx = 0; idx < ht->size; idx++){
        node_t* node = ht->elements[idx];
        if(!node) continue;

        while(node->next){

            node_t* next = node->next;

            free((char*)node->key);
            node->key = NULL;

            free(node);

            node = next;
        }

        free((char*)node->key);
        node->key = NULL;

        free(node);

    }


    free(ht->elements);
    free(ht);
}

void hash_table_dump(const hash_table* ht){
    assert(ht);
    
    for(int idx = 0; idx < ht->size; idx++){
        node_t* node = ht->elements[idx];
        if(!node){
            fprintf(stderr, "[%4d]: NO\n", idx);
            continue;
        } 

        fprintf(stderr, "[%4d : %p]: key = %s, next = %p\n", idx, node, node->key, node->next);

        while(node->next){
            node = node->next;
            fprintf(stderr, "[   : %p ]: key = %s, next = %p\n", node, node->key, node->next);
        }
    }
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