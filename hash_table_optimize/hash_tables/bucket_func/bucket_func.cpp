#include "bucket_func.h"
#include <cstdint>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>


// Helping static func --------------------------------------------------------------------------------

void* aligned_calloc(size_t size){
    void* ptr = NULL;

    if(posix_memalign(&ptr, ALIGN, size) != 0){
        return NULL;
    }

    memset(ptr, 0, size);
    return ptr;
}

static void fill_arrays(bucket_t* bucket, int start, int end){
    for(int i = start; i < end; i++){
        bucket->next[i] = i + 1;
        bucket->prev[i] = -1;
    }
}

// --------------------------------------------------------------------------------------------------

// Bucket ctor --------------------------------------------------------------------------------------

bucket_err_t bucket_ctor(bucket_t* bucket){
    if(!bucket) return BUCKET_NOT_EXISTS;

    int capacity = 10;
    char* keys = (char*)calloc(capacity, sizeof(char) * SIZE_WORD);
    if(!keys){
        fprintf(stderr, "Realloc keys in hash_table_insert error\n");
        return DATA_ALLOC_ERR;
    }

    uint32_t* hashes = (uint32_t*)calloc(capacity, sizeof(uint32_t));
    if(!hashes){
        fprintf(stderr, "Realloc hashes in hash_table_insert error\n");
        return HASHES_ALLOC_ERR;
    }

    int* next = (int*)calloc(capacity, sizeof(int));
    if(!next){
        fprintf(stderr, "Realloc next in hash_table_insert error\n");
        return NEXT_ALLOC_ERR;
    }

    int* prev = (int*)calloc(capacity, sizeof(int));
    if(!next){
        fprintf(stderr, "Realloc next in hash_table_insert error\n");
        return PREV_ALLOC_ERR;
    }
    
    bucket->capacity = capacity;
    bucket->first_free = 0;
    bucket->size = 0;
    bucket->list_head = 0;

    bucket->hashes = hashes;
    bucket->keys = keys;
    bucket->next = next;
    bucket->prev = prev;
    bucket->is_linearized = true;

    fill_arrays(bucket, 0, capacity);
    bucket->prev[0] = 0;

    return NO_MISTAKE;
}

// --------------------------------------------------------------------------------------------------

// Recalloc arrays ----------------------------------------------------------------------------------

bucket_err_t recalloc_arrays(bucket_t* bucket, int capacity, int new_capacity){
    if(!bucket) return BUCKET_NOT_EXISTS;

    char* keys = (char*)recallocarray(bucket->keys, capacity, new_capacity, sizeof(char) * SIZE_WORD);
    if(!keys){
        fprintf(stderr, "Realloc keys in hash_table_insert error\n");
        return DATA_ALLOC_ERR;
    }

    uint32_t* hashes = (uint32_t*)recallocarray(bucket->hashes, capacity, new_capacity, sizeof(uint32_t));
    if(!hashes){
        fprintf(stderr, "Realloc hashes in hash_table_insert error\n");
        return HASHES_ALLOC_ERR;
    }

    int* next = (int*)recallocarray(bucket->next, capacity, new_capacity, sizeof(int));
    if(!next){
        fprintf(stderr, "Realloc next in hash_table_insert error\n");
        return NEXT_ALLOC_ERR;
    }

    int* prev = (int*)recallocarray(bucket->prev, capacity, new_capacity, sizeof(int));
    if(!next){
        fprintf(stderr, "Realloc next in hash_table_insert error\n");
        return PREV_ALLOC_ERR;
    }

    bucket->capacity = new_capacity;

    bucket->keys = keys;
    bucket->hashes = hashes;
    bucket->next = next;
    bucket->prev = prev;

    fill_arrays(bucket, capacity, bucket->capacity);

    return NO_MISTAKE;
}

// --------------------------------------------------------------------------------------------------

// Bucket insert ------------------------------------------------------------------------------------

void bucket_insert(bucket_t* bucket, const char* key, const uint64_t hash){
    assert(key);
    assert(bucket);

    int first_free = bucket->first_free;
    bucket->first_free = bucket->next[first_free];

    int last_elem = bucket->prev[bucket->list_head];

    bucket->hashes[first_free] = hash;
    memcpy(bucket->keys + first_free * SIZE_WORD, key, strlen(key));

    bucket->next[last_elem] = first_free;
    bucket->prev[bucket->list_head] = first_free;

    bucket->next[first_free] =  bucket->list_head;
    bucket->prev[first_free] = last_elem;

    bucket->size++;

    bucket->is_linearized = false;
}

// --------------------------------------------------------------------------------------------------

// Bucket delete ------------------------------------------------------------------------------------

void bucket_delete(bucket_t* bucket, const char* key, const uint64_t hash){
    assert(key);
    assert(bucket);

    int index = find_node(bucket, hash, key);
    if(index == bucket->capacity){
        return; // значит не получилось удалить
    }

    memset(bucket->keys+ SIZE_WORD * index, 0, SIZE_WORD);
    bucket->hashes[index] = 0;

    if(index == 0) bucket->list_head = bucket->next[index];

    int prev = bucket->prev[index];
    bucket->next[prev] = bucket->next[index];
    bucket->prev[bucket->next[index]] = prev;

    bucket->next[index] = bucket->first_free;
    bucket->prev[index] = -1;
    bucket->first_free = index;

    bucket->size--;
    bucket->is_linearized = false;
}

// --------------------------------------------------------------------------------------------------

// Bucket linearize ---------------------------------------------------------------------------------

bucket_err_t bucket_linearize(bucket_t* bucket){
    assert(bucket);

    int size = bucket->size;
    if(size < BLOCK_DATA_AMOUNT) size = BLOCK_DATA_AMOUNT;

    char* keys = (char*)aligned_calloc(size * sizeof(char) * SIZE_WORD);
    if(!keys) return DATA_ALLOC_ERR;

    uint32_t* hashes = (uint32_t*)aligned_calloc(size * sizeof(uint32_t));
    if(!hashes) return HASHES_ALLOC_ERR;

    int* next = (int*)calloc(size, sizeof(int));
    if(!next) return NEXT_ALLOC_ERR;

    int* prev = (int*)calloc(size, sizeof(int));
    if(!prev) return PREV_ALLOC_ERR;

    int i = bucket->list_head;
    for(int count = 0; count < bucket->size; count++){
        memcpy(keys + count * SIZE_WORD, bucket->keys + i * SIZE_WORD, SIZE_WORD);
        hashes[count] = bucket->hashes[i];

        prev[count] = (count == 0) ? bucket->size - 1 : count - 1;
        next[count] = (count == bucket->size - 1) ? 0 : count + 1;

        i = bucket->next[i];
    }

    bucket->capacity = size;
    bucket->list_head = 0;
    if(size != bucket->size) bucket->first_free = bucket->size;
    else bucket->first_free = 0;

    bucket_dtor(bucket);

    bucket->hashes = hashes;
    bucket->keys = keys;
    bucket->next = next;
    bucket->prev = prev;

    fill_arrays(bucket, bucket->size, size);

    bucket->is_linearized = true;

    return NO_MISTAKE;
}

// --------------------------------------------------------------------------------------------------

// Find node ----------------------------------------------------------------------------------------

__attribute__((noinline))
int find_node(bucket_t* bucket, const uint32_t hash, const char* key){
    assert(key);

    uint32_t* hashes = bucket->hashes;
    char* keys = bucket->keys;
    int size_bucket = bucket->size;

    int i = bucket->list_head;
    for(int idx = 0; idx < size_bucket; idx++){
        char* key_in_hashtable = keys + i * SIZE_WORD;
        if(hashes[i] == hash  && key_in_hashtable[0] && !strcmp(key_in_hashtable, key)){
            return i;
        }
        i = bucket->next[i];
    }
    return bucket->capacity;
}

// --------------------------------------------------------------------------------------------------

// Bucket dtor --------------------------------------------------------------------------------------

void bucket_dtor(bucket_t* bucket){
    if(!bucket) return;

    if(bucket->keys){
        free(bucket->keys);
        bucket->keys = NULL;
    } 

    if(bucket->hashes){
        free(bucket->hashes);
        bucket->hashes = NULL;
    }

    if(bucket->next){
        free(bucket->next);
        bucket->next = NULL;
    }

    if(bucket->prev){
        free(bucket->prev);
        bucket->prev = NULL;
    }
}

// --------------------------------------------------------------------------------------------------