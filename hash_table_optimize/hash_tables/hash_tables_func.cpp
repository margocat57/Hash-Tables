#include "hash_table.h"



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

    uint32_t hash = hash_func(key);
    uint32_t idx = hash % ht->size;

    if(!ht->elements[idx].keys && !ht->elements[idx].hashes){
        ht->elements[idx].keys = (char*)calloc(10, sizeof(char) * size_word);
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
        char* keys = (char*)recallocarray(ht->elements[idx].keys, capacity, capacity * 2, sizeof(char) * size_word);
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
    memcpy(ht->elements[idx].keys + first_free * size_word, key, 32);

    ht->elements[idx].first_free++;

}

bool hash_table_find(const char* key, const hash_table* ht){
    assert(ht);
    assert(key);

    uint32_t hash = hash_func(key);
    uint32_t idx = hash % ht->size;

    int index = find_node(&ht->elements[idx], hash, key);

    if(index != ht->elements[idx].capacity){
        return true;
    }
    
    return false;

}


void hash_table_dtor(hash_table* ht){
    if(!ht) return;
    
    for(int idx = 0; idx < ht->size; idx++){
        if(ht->elements[idx].keys){
            free(ht->elements[idx].keys);
        } 

        if(ht->elements[idx].hashes){
            free(ht->elements[idx].hashes);
        }

    }

    free(ht->elements);
    free(ht);
}


