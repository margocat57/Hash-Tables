#include "bucket_func/bucket_func.h"
#include "hash_table.h"
#include <cstdio>

// Hash table ctor ----------------------------------------------------------------------------------

hash_table* hash_table_ctor(int size){
    hash_table* ht = (hash_table*)calloc(1, sizeof(hash_table));
    if(!ht){
        fprintf(stderr, "Memory allocation error in ht");
        return NULL;
    }

    ht->size = size;
    ht->elements = (bucket_t*)calloc(ht->size, sizeof(bucket_t));
    if(!ht->elements){
        fprintf(stderr, "Memory allocation error in ht->elements");
        return NULL;
    }

    return ht;
}

// --------------------------------------------------------------------------------------------------

// Hash table insert --------------------------------------------------------------------------------

void hash_table_insert(const char* key, hash_table* ht){
    assert(ht);
    assert(key);

    uint32_t hash = hash_func(key);
    uint32_t idx = hash % ht->size;
    bucket_t* bucket = &(ht->elements[idx]);

    int capacity = bucket->capacity;
    int size = bucket->size;

    if(!size){
        if(bucket_ctor(bucket)) return;
    }
    else if(bucket->size == capacity){
        if(recalloc_arrays(bucket, capacity,capacity * 2)){
            return;
        }

        bucket->first_free = bucket->size;
    }

    bucket_insert(bucket, key, hash);
}

// --------------------------------------------------------------------------------------------------

// Hash table linearize --------------------------------------------------------------------------------

bool hash_table_linearize(hash_table* ht){
   for(int idx=0; idx < ht->size; idx++){
        bucket_t* bucket = &(ht->elements[idx]);

        if(!bucket->keys || !bucket->hashes || !bucket->next || !bucket->prev) continue;

        if(bucket_linearize(bucket)){
            return false;
        }
   }

   return true;

}

// --------------------------------------------------------------------------------------------------

// Hash table find --------------------------------------------------------------------------------

bool hash_table_find(const char* key, const hash_table* ht){
    assert(ht);
    assert(key);

    uint32_t hash = hash_func(key);
    uint32_t idx = hash % ht->size;
    bucket_t* bucket = &(ht->elements[idx]);

    if(!bucket->keys || !bucket->hashes || !bucket->next) return false;

    int index = 0;
    if(bucket->is_linearized){
        index = find_node_optimized(bucket, hash, key);
    }
    else{
        index = find_node(bucket, hash, key);
    }
    
    if(index != bucket->capacity){
        return true;
    }
    
    return false;

}

// --------------------------------------------------------------------------------------------------

// Hash table delete --------------------------------------------------------------------------------

void hash_table_delete(const char* key, hash_table* ht){
    uint32_t hash = hash_func(key);
    uint32_t idx = hash % ht->size;

    bucket_t* bucket = &(ht->elements[idx]);
    if(!bucket) return;

    bucket_delete(bucket, key, hash);
}

// --------------------------------------------------------------------------------------------------

// Hash table dtor --------------------------------------------------------------------------------


void hash_table_dtor(hash_table* ht){
    if(!ht) return;
    
    for(int idx = 0; idx < ht->size; idx++){
        bucket_t* bucket = &(ht->elements[idx]);
        bucket_dtor(bucket);

    }

    free(ht->elements);
    free(ht);
}

// --------------------------------------------------------------------------------------------------

// Hash table dump ----------------------------------------------------------------------------------

void hash_table_dump(const hash_table* ht){
    assert(ht);
    
    for(int idx = 0; idx < ht->size; idx++){
        bucket_t* bucket = &(ht->elements[idx]);
        if(!bucket->keys || !bucket->hashes || !bucket->next){
            fprintf(stderr, "[%4d]: NO\n", idx);
            continue;
        } 

        int i = bucket->list_head;
        int size = bucket->size;
        for(int count = 0; count < size; count++){
            fprintf(stderr, "[%4d : %d]: key = %s, next = %d, prev = %d\n", idx, i, bucket->keys + i * SIZE_WORD, bucket->next[i], bucket->prev[i]);
            i = bucket->next[i];
        }
    }
}

// --------------------------------------------------------------------------------------------------