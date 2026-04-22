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

static void fill_arrays(bucket_t* bucket, int start, int end);

static ALLOC_MISTAKES recalloc_arrays(bucket_t* bucket, int capacity, int new_capacity);

static int find_prev_elem(bucket_t* bucket, int elem_idx);

void hash_table_insert(const char* key, hash_table* ht){
    assert(ht);
    assert(key);

    uint32_t hash = hash_func(key);
    uint32_t idx = hash % ht->size;
    bucket_t* bucket = &(ht->elements[idx]);

    int capacity = bucket->capacity;

    if(bucket->size == capacity){

        if(recalloc_arrays(bucket, capacity,(capacity == 0)? 10 : capacity * 2)){
            return;
        }

        fill_arrays(bucket, capacity, ht->elements[idx].capacity - 1);

        bucket->first_free = bucket->size;

    }

    int first_free = bucket->first_free;
    bucket->first_free = bucket->next[first_free];

    int last_elem = find_prev_elem(bucket, bucket->list_head);

    bucket->hashes[first_free] = hash;
    memcpy(bucket->keys + first_free * size_word, key, strlen(key));

    bucket->next[last_elem] = first_free;
    bucket->next[first_free] =  bucket->list_head;

    bucket->size++;

    bucket->is_linearized = false;

}

static int find_prev_elem(bucket_t* bucket, int elem_idx){
    int i = bucket->list_head;
    for(int idx = 0; idx < bucket->size; idx++){
        if(bucket->next[i] == elem_idx){
            return i;
        }
        i = bucket->next[i];
    }
    return 0;
}

static void fill_arrays(bucket_t* bucket, int start, int end){

    for(int i = start; i < end; i++){
        bucket->next[i] = i + 1;
    }
    bucket->next[end] = -1;

}

static ALLOC_MISTAKES recalloc_arrays(bucket_t* bucket, int capacity, int new_capacity){
    char* keys = (char*)recallocarray(bucket->keys, capacity, new_capacity, sizeof(char) * size_word);
    if(!keys){
        fprintf(stderr, "Realloc keys in hash_table_insert error\n");
        return KEY_ALLOC_MISTAKE;
    }

    uint32_t* hashes = (uint32_t*)recallocarray(bucket->hashes, capacity, new_capacity, sizeof(uint32_t));
    if(!hashes){
        fprintf(stderr, "Realloc hashes in hash_table_insert error\n");
        return HASH_ALLOC_MISTAKE;
    }

    int* next = (int*)recallocarray(bucket->next, capacity, new_capacity, sizeof(int));
    if(!next){
        fprintf(stderr, "Realloc next in hash_table_insert error\n");
        return NEXT_ALLOC_MISTAKE;
    }

    bucket->capacity = new_capacity;

    bucket->keys = keys;
    bucket->hashes = hashes;
    bucket->next = next;

    return NO_MISTAKES;
}

// --------------------------------------------------------------------------------------------------


// Hash table linearize --------------------------------------------------------------------------------

bool hash_table_linearize(hash_table* ht){
   for(int idx=0; idx < ht->size; idx++){
        bucket_t* bucket = &(ht->elements[idx]);

        if(!bucket->keys || !bucket->hashes || !bucket->next) continue;

        int size = bucket->size;
        if(size < block_data_amount) size = block_data_amount;

        char* keys = (char*)calloc(size, sizeof(char) * size_word);
        if(!keys) return false;

        uint32_t* hashes = (uint32_t*)calloc(size, sizeof(uint32_t));
        if(!hashes) return false;

        int* next = (int*)calloc(size, sizeof(int));
        if(!next) return false;

        int i = bucket->list_head;
        for(int count = 0; count < size; count++){
            memcpy(keys + count * size_word, bucket->keys + i * size_word, size_word);
            hashes[count] = bucket->hashes[i];

            i = bucket->next[i];
        }

        bucket->capacity = size;
        bucket->list_head = 0;
        if(size != bucket->size){
            bucket->first_free = size;
        }
        else{
            bucket->first_free = 0;
        }

        free(bucket->hashes);
        bucket->hashes = hashes;

        free(bucket->keys);
        bucket->keys = keys;

        free(bucket->next);
        bucket->next = next;

        fill_arrays(bucket, 0, size - 1);
        bucket->next[bucket->size - 1] = 0;

        bucket->is_linearized = true;
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

    int capacity = bucket->capacity;

    int index = find_node(bucket, hash, key);
    if(index == capacity){
        return; // значит не получилось удалить
    }

    memset(bucket->keys+ size_word * index, 0, size_word);
    bucket->hashes[index] = 0;

    if(index == 0) bucket->list_head = bucket->next[index];

    int prev = find_prev_elem(bucket, index);
    bucket->next[prev] = bucket->next[index];

    bucket->next[index] = bucket->first_free;
    bucket->first_free = index;

    bucket->size--;
    bucket->is_linearized = false;
}

// --------------------------------------------------------------------------------------------------

// Hash table dtor --------------------------------------------------------------------------------


void hash_table_dtor(hash_table* ht){
    if(!ht) return;
    
    for(int idx = 0; idx < ht->size; idx++){
        if(ht->elements[idx].keys){
            free(ht->elements[idx].keys);
        } 

        if(ht->elements[idx].hashes){
            free(ht->elements[idx].hashes);
        }

        if(ht->elements[idx].next){
            free(ht->elements[idx].next);
        }

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
            fprintf(stderr, "[%4d : %d]: key = %s, next = %d\n", idx, i, bucket->keys + i * size_word, bucket->next[i]);
            i = bucket->next[i];
        }
    }
}

// --------------------------------------------------------------------------------------------------