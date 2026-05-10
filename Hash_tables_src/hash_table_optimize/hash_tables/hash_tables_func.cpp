#include "bucket_func/bucket_func.h"
#include "hash_table.h"
#include <cstdio>
#include <climits>


// Hash table ctor ----------------------------------------------------------------------------------

hash_table* hash_table_ctor(int capacity){
    assert(capacity > 0);

    hash_table* ht = (hash_table*)calloc(1, sizeof(hash_table));
    if(!ht){
        fprintf(stderr, "Memory allocation error in ht");
        return NULL;
    }

    ht->capacity = capacity;
    ht->elements = (bucket_t*)canary_calloc(sizeof(bucket_t), ht->capacity);
    if(!ht->elements){
        free(ht);
        fprintf(stderr, "Memory allocation error in ht->elements");
        return NULL;
    }

    ht->left_canary = CANARY_VALUE;
    ht->right_canary = CANARY_VALUE;

    return ht;
}

// --------------------------------------------------------------------------------------------------

// Hash table insert --------------------------------------------------------------------------------

static bool hash_table_rehash(hash_table* ht);

bool hash_table_insert(const char* key, hash_table* ht){
    assert(ht);
    assert(key);
    assert(hash_table_verify(ht) == NO_MISTAKE_HT);

    if((ht->size + 1)/ht->capacity > 10){
        if(!hash_table_rehash(ht)) return false;
    }

    uint32_t hash = hash_func(key);
    uint32_t idx = hash % ht->capacity;
    bucket_t* bucket = &(ht->elements[idx]);

    bucket_insert_to_end(bucket, key, hash);

    ht->size++;

    assert(hash_table_verify(ht) == NO_MISTAKE_HT);

    return true;
}

static bool hash_table_rehash(hash_table* ht){
    assert(ht);

    assert(hash_table_verify(ht) == NO_MISTAKE_HT);

    int new_capacity = ht->capacity * 2 + 1;
    bucket_t* buckets = (bucket_t*)canary_calloc(sizeof(bucket_t), new_capacity);
    if(!buckets) return false;

    for(int idx = 0; idx < ht->capacity; idx++){
        bucket_t* bucket = &(ht->elements[idx]);

        int j = bucket->list_head;
        for(int i = 0; i < bucket->size; i++){

            char* key = bucket->keys + SIZE_WORD * j;
            uint32_t hash = hash_func(key);
            uint32_t new_idx = hash % new_capacity;

            if(!bucket_insert_to_end(&(buckets[new_idx]), key, hash)){
                buckets_dtor(buckets, new_capacity);
                return false;
            }

            j = bucket->next[j];
        }

    }

    buckets_dtor(ht->elements, ht->capacity);

    ht->elements = buckets;
    ht->capacity = new_capacity;

    assert(hash_table_verify(ht) == NO_MISTAKE_HT);

    return true;
}

// --------------------------------------------------------------------------------------------------

// Hash table linearize --------------------------------------------------------------------------------

bool hash_table_linearize(hash_table* ht){
    assert(hash_table_verify(ht) == NO_MISTAKE_HT);
    for(int idx=0; idx < ht->capacity; idx++){
        bucket_t* bucket = &(ht->elements[idx]);

        if(!bucket->keys || !bucket->hashes || !bucket->next || !bucket->prev) continue;

        if(bucket_linearize(bucket) != NO_MISTAKE){
            return false;
        }
   }

   assert(hash_table_verify(ht) == NO_MISTAKE_HT);

   return true;

}

// --------------------------------------------------------------------------------------------------

// Hash table find --------------------------------------------------------------------------------

bool hash_table_find(const char* key, const hash_table* ht){
    assert(ht);
    assert(key);
    assert(hash_table_verify(ht) == NO_MISTAKE_HT);

    uint32_t hash = hash_func(key);
    uint32_t idx = hash % ht->capacity;
    bucket_t* bucket = &(ht->elements[idx]);

    if(!bucket->keys || !bucket->hashes || !bucket->next|| !bucket->prev) return false;

    int index = 0;
    if(bucket->is_linearized && bucket->is_aligned && bucket->capacity_more_or_eqthan_eight){
        index = find_node_optimized(bucket, hash, key);
    }
    else{
        index = find_node(bucket, hash, key);
    }
    
    if(index != INT_MAX){
        return true;
    }

    assert(hash_table_verify(ht) == NO_MISTAKE_HT);
    
    return false;

}

// --------------------------------------------------------------------------------------------------

// Find node ----------------------------------------------------------------------------------------

__attribute__((noinline))
int find_node(const bucket_t* bucket, const uint32_t hash, const char* key){
    assert(key);

    uint32_t* hashes = bucket->hashes;
    char* keys = bucket->keys;
    int size_bucket = bucket->size;

    int i = bucket->list_head;
    for(int idx = 0; idx < size_bucket; idx++){
        char* key_in_hashtable = keys + i * SIZE_WORD;
        if(hashes[i] == hash && !strcmp(key_in_hashtable, key)){
            return i;
        }
        i = bucket->next[i];
    }
    return INT_MAX;
}

// --------------------------------------------------------------------------------------------------

// Hash table verify --------------------------------------------------------------------------------

hash_table_err_t hash_table_verify(const hash_table* ht){
    assert(ht);

    if(ht->left_canary != CANARY_VALUE){
        fprintf(stderr,"Left canary corrupted\n");
        return LEFT_CORRUPTED_HT;
    }

    if(ht->right_canary != CANARY_VALUE){
        fprintf(stderr,"Right canary corrupted\n");
        return RIGHT_CORRUPTED_HT;
    }

    if(ht->size < 0 && ht->size > ht->capacity){
        fprintf(stderr, "Incorr hash table size\n");
        return INCORR_SIZE_HT;
    }

    if(ht->capacity < 0){
        fprintf(stderr, "Incorr hash table size\n");
        return INCORR_CAPACITY_HT;
    }

    if(canary_verify(ht->elements, sizeof(bucket_t) * ht->capacity) != NO_MISTAKE_CANARY){
        fprintf(stderr, "Buckets array corrupted\n");
        return BUCKETS_CORR_HT;
    }

    for(int idx = 0; idx < ht->capacity; idx++){
        if(ht->elements[idx].capacity == 0) continue; // бакет пустой - память под него выделена а он не заполнен
    
        hash_table_err_t err = bucket_verify(&ht->elements[idx]);
        if(err) return err;
    }

    return NO_MISTAKE_HT;
}

// --------------------------------------------------------------------------------------------------

// Hash table resize up -----------------------------------------------------------------------------

bool hash_table_buckets_resize_up(hash_table* ht){
    assert(ht);

    for(int idx = 0; idx < ht->capacity; idx++){
        bucket_t* bucket = &(ht->elements[idx]);

        if(bucket->capacity < 8){
            bucket_resize_up(bucket, 8);
            if(!bucket->capacity_more_or_eqthan_eight){
                return false;
            }
        }
    }
    return true;
}

// --------------------------------------------------------------------------------------------------

// Hash table align up -----------------------------------------------------------------------------

bool hash_table_buckets_align(hash_table* ht){
    assert(ht);

    for(int idx = 0; idx < ht->capacity; idx++){
        bucket_t* bucket = &(ht->elements[idx]);

        if(bucket->capacity < 8){
            bucket_keys_hashes_align(bucket, 32);
            if(!bucket->is_aligned){
                return false;
            }
        }
    }
    return true;
}

// --------------------------------------------------------------------------------------------------

// Hash table delete --------------------------------------------------------------------------------

void hash_table_delete(const char* key, hash_table* ht){
    assert(key);
    assert(ht);
    assert(hash_table_verify(ht) == NO_MISTAKE_HT);

    uint32_t hash = hash_func(key);
    uint32_t idx = hash % ht->capacity;

    bucket_t* bucket = &(ht->elements[idx]);
    if(!bucket) return;

    int index = 0;
    if(bucket->is_linearized && bucket->is_aligned && bucket->capacity_more_or_eqthan_eight){
        // здесь именно ассемблерная оптимизация требует чтобы данные лежали плотно, то есть надо хэш таблице
        index = find_node_optimized(bucket, hash, key); 
    }
    else{
        index = find_node(bucket, hash, key);
    }

    if(index == INT_MAX) return; // значит не получилось удалить

    bucket_delete_by_physical_index(bucket, index); // хэш таблица дает именно физический индекс

    ht->size--;

    assert(hash_table_verify(ht) == NO_MISTAKE_HT);
}

// --------------------------------------------------------------------------------------------------

// Hash table dtor --------------------------------------------------------------------------------


void buckets_dtor(bucket_t* buckets, int size){
    if(!buckets) return;
    
    for(int idx = 0; idx < size; idx++){
        bucket_t* bucket = &(buckets[idx]);
        bucket_dtor(bucket);

    }

    canary_free(buckets, size* sizeof(bucket_t));
}

void hash_table_dtor(hash_table* ht){
    if(!ht) return;

    buckets_dtor(ht->elements, ht->capacity);

    free(ht);
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