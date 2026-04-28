#include "bucket_func/bucket_func.h"
#include "hash_table.h"
#include <cstdio>

static void buckets_dtor(bucket_t* buckets, int size);

// Hash table ctor ----------------------------------------------------------------------------------

hash_table* hash_table_ctor(int capacity){
    hash_table* ht = (hash_table*)calloc(1, sizeof(hash_table));
    if(!ht){
        fprintf(stderr, "Memory allocation error in ht");
        return NULL;
    }

    ht->capacity = capacity;
    ht->elements = (bucket_t*)calloc(ht->capacity, sizeof(bucket_t));
    if(!ht->elements){
        fprintf(stderr, "Memory allocation error in ht->elements");
        return NULL;
    }

    return ht;
}

// --------------------------------------------------------------------------------------------------

// Hash table insert --------------------------------------------------------------------------------

static void hash_table_rehash(hash_table* ht);

void hash_table_insert(const char* key, hash_table* ht){
    assert(ht);
    assert(key);

    if((ht->size + 1)/ht->capacity > 10){
        hash_table_rehash(ht);
    }

    uint32_t hash = hash_func(key);
    uint32_t idx = hash % ht->capacity;
    bucket_t* bucket = &(ht->elements[idx]);

    buckets_insert(bucket, key, hash);

    ht->size++;
}

static void hash_table_rehash(hash_table* ht){
    assert(ht);

    int new_capacity = ht->capacity * 2 + 1;
    bucket_t* buckets = (bucket_t*)calloc(new_capacity, sizeof(bucket_t));

    for(int idx = 0; idx < ht->capacity; idx++){
        bucket_t* bucket = &(ht->elements[idx]);

        int j = bucket->list_head;
        for(int i = 0; i < bucket->size; i++){

            char* key = bucket->keys + SIZE_WORD * j;
            uint32_t hash = hash_func(key);
            uint32_t new_idx = hash % new_capacity;

            if(!buckets_insert(&(buckets[new_idx]), key, hash)){
                buckets_dtor(buckets, new_capacity);
                return;
            }

            j = bucket->next[j];
        }

    }

    buckets_dtor(ht->elements, ht->capacity);

    ht->elements = buckets;
    ht->capacity = new_capacity;

}

// --------------------------------------------------------------------------------------------------

// Hash table linearize --------------------------------------------------------------------------------

bool hash_table_linearize(hash_table* ht){
   for(int idx=0; idx < ht->capacity; idx++){
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
    uint32_t idx = hash % ht->capacity;
    bucket_t* bucket = &(ht->elements[idx]);

    if(!bucket->keys || !bucket->hashes || !bucket->next|| !bucket->prev) return false;

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
    uint32_t idx = hash % ht->capacity;

    bucket_t* bucket = &(ht->elements[idx]);
    if(!bucket) return;

    bucket_delete(bucket, key, hash);

    ht->size--;
}

// --------------------------------------------------------------------------------------------------

// Hash table dtor --------------------------------------------------------------------------------


void hash_table_dtor(hash_table* ht){
    if(!ht) return;
    
    for(int idx = 0; idx < ht->capacity; idx++){
        bucket_t* bucket = &(ht->elements[idx]);
        bucket_dtor(bucket);

    }

    free(ht->elements);
    free(ht);
}



static void buckets_dtor(bucket_t* buckets, int size){
    if(!buckets) return;
    
    for(int idx = 0; idx < size; idx++){
        bucket_t* bucket = &(buckets[idx]);
        bucket_dtor(bucket);

    }

    free(buckets);
}


// --------------------------------------------------------------------------------------------------

// Hash table dump ----------------------------------------------------------------------------------

void hash_table_dump(const hash_table* ht){
    assert(ht);
    
    for(int idx = 0; idx < ht->capacity; idx++){
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