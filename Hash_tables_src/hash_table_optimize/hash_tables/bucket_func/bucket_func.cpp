#include "bucket_func.h"
#include <cstdint>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <limits.h>


// Helping func --------------------------------------------------------------------------------------

void fill_arrays(bucket_t* bucket, int start, int end){
    if (start >= end) return;

    for(int i = start; i < end; i++){
        bucket->next[i] = i + 1;
        bucket->prev[i] = -1;
    }

    bucket->next[end - 1] = -1;
}

// --------------------------------------------------------------------------------------------------

// Bucket ctor --------------------------------------------------------------------------------------

static bucket_err_t calloc_arrays(bucket_t* bucket, char** keys, uint32_t** hashes, int** next, int** prev, int capacity);

bucket_err_t bucket_ctor(bucket_t* bucket, int capacity){
    assert(capacity > 0);
    if(!bucket) return BUCKET_NOT_EXISTS;

    char* keys = NULL;
    uint32_t* hashes = NULL;
    int* next = NULL;
    int* prev = NULL;

    bucket_err_t err = calloc_arrays(bucket,&keys, &hashes, &next, &prev, capacity);
    if(err != NO_MISTAKE) return err;
    
    bucket->capacity = capacity;
    bucket->first_free = 0;
    bucket->size = 0;
    bucket->list_head = 0;

    bucket->hashes = hashes;
    bucket->keys = keys;
    bucket->next = next;
    bucket->prev = prev;
    bucket->is_linearized = true;

    bucket->left_canary = BUCKET_CANARY;
    bucket->right_canary = BUCKET_CANARY;

    fill_arrays(bucket, 0, capacity);
    bucket->prev[0] = 0;

    return NO_MISTAKE;
}

// --------------------------------------------------------------------------------------------------

// (Re)calloc arrays ----------------------------------------------------------------------------------


static bucket_err_t calloc_arrays(bucket_t* bucket, char** keys_dst, uint32_t** hashes_dst, int** next_dst, int** prev_dst, int capacity){
    assert(keys_dst);
    assert(hashes_dst);
    assert(next_dst);
    assert(prev_dst);

    char* keys = (char*)canary_calloc(sizeof(char) * SIZE_WORD, capacity);
    if(!keys){
        fprintf(stderr, "Realloc keys in hash_table_insert error\n");
        return DATA_ALLOC_ERR;
    }

    uint32_t* hashes = (uint32_t*)canary_calloc(sizeof(uint32_t), capacity);
    if(!hashes){
        canary_free(keys, sizeof(char) * SIZE_WORD * capacity);
        fprintf(stderr, "Realloc hashes in hash_table_insert error\n");
        return HASHES_ALLOC_ERR;
    }

    int* next = (int*)canary_calloc(sizeof(int), capacity);
    if(!next){
        canary_free(keys, sizeof(char) * SIZE_WORD * capacity);
        canary_free(hashes, sizeof(uint32_t) * capacity);
        fprintf(stderr, "Realloc next in hash_table_insert error\n");
        return NEXT_ALLOC_ERR;
    }

    int* prev = (int*)canary_calloc(sizeof(int), capacity);
    if(!prev){
        canary_free(keys, sizeof(char) * SIZE_WORD * capacity);
        canary_free(hashes, sizeof(uint32_t) * capacity);
        canary_free(next, sizeof(int) * capacity);
        fprintf(stderr, "Realloc next in hash_table_insert error\n");
        return PREV_ALLOC_ERR;
    }

    *keys_dst = keys;
    *hashes_dst = hashes;
    *next_dst = next;
    *prev_dst = prev;

    bucket->is_aligned = false;
    bucket->capacity_more_or_eqthan_eight = (capacity >= 8) ? true : false;

    return NO_MISTAKE;
}

bucket_err_t recalloc_arrays(bucket_t* bucket, int capacity, int new_capacity){
    if(!bucket) return BUCKET_NOT_EXISTS;

    char* keys = (char*)canary_recalloc(bucket->keys, capacity, new_capacity, sizeof(char) * SIZE_WORD);
    if(!keys){
        fprintf(stderr, "Realloc keys in hash_table_insert error\n");
        return DATA_ALLOC_ERR;
    }

    uint32_t* hashes = (uint32_t*)canary_recalloc(bucket->hashes, capacity, new_capacity, sizeof(uint32_t));
    if(!hashes){
        canary_free(keys, sizeof(char) * SIZE_WORD * new_capacity);
        fprintf(stderr, "Realloc hashes in hash_table_insert error\n");
        return HASHES_ALLOC_ERR;
    }

    int* next = (int*)canary_recalloc(bucket->next, capacity, new_capacity, sizeof(int));
    if(!next){
        canary_free(keys, sizeof(char) * SIZE_WORD * new_capacity);
        canary_free(hashes, sizeof(uint32_t) * new_capacity);
        fprintf(stderr, "Realloc next in hash_table_insert error\n");
        return NEXT_ALLOC_ERR;
    }

    int* prev = (int*)canary_recalloc(bucket->prev, capacity, new_capacity, sizeof(int));
    if(!prev){
        canary_free(keys, sizeof(char) * SIZE_WORD * new_capacity);
        canary_free(hashes, sizeof(uint32_t) * new_capacity);
        canary_free(next, sizeof(int) * new_capacity);
        fprintf(stderr, "Realloc next in hash_table_insert error\n");
        return PREV_ALLOC_ERR;
    }

    bucket->capacity = new_capacity;

    bucket->keys = keys;
    bucket->hashes = hashes;
    bucket->next = next;
    bucket->prev = prev;

    bucket->is_aligned = false;
    bucket->capacity_more_or_eqthan_eight = (new_capacity >= 8) ? true : false;

    fill_arrays(bucket, capacity, bucket->capacity);

    return NO_MISTAKE;
}

// --------------------------------------------------------------------------------------------------

// Bucket insert ------------------------------------------------------------------------------------


bool bucket_insert_to_end(bucket_t* bucket, const char* key, const uint32_t hash){
    assert(key);
    assert(bucket);

    int capacity = bucket->capacity;

    if(capacity == 0){
        if(bucket_ctor(bucket, 10) != NO_MISTAKE) return false;
    }
    else if(bucket->size == capacity){
        if(recalloc_arrays(bucket, capacity,capacity * 2) != NO_MISTAKE){
            return false;
        }

        bucket->first_free = bucket->size;
    }

    assert(bucket_verify(bucket) == NO_MISTAKE);

    assert(bucket->first_free != -1);

    int first_free = bucket->first_free;
    bucket->first_free = bucket->next[first_free];

    int last_elem = bucket->prev[bucket->list_head];
    assert(last_elem != -1);

    bucket->hashes[first_free] = hash;
    memcpy(bucket->keys + first_free * SIZE_WORD, key, strlen(key));

    bucket->next[last_elem] = first_free;
    bucket->prev[bucket->list_head] = first_free;

    bucket->next[first_free] =  bucket->list_head;
    bucket->prev[first_free] = last_elem;

    bucket->size++;

    bucket->is_linearized = false;

    assert(bucket_verify(bucket) == NO_MISTAKE);

    return true;
}

// --------------------------------------------------------------------------------------------------

// Bucket find --------------------------------------------------------------------------------------

int bucket_find_phys_index(bucket_t* bucket, int logical_index){
    int j = bucket->list_head;

    for(int i = 0; i < logical_index; i++){
        if(j == -1){
            return -1;
        }
        j = bucket->next[j];
    }

    return j;
}

int bucket_find_phys_index_optimized(bucket_t* bucket, int logical_index){
    if(logical_index == 0) return 0;
    return bucket->next[logical_index - 1]; // по факту тут физический индекс равен логическому
}

// --------------------------------------------------------------------------------------------------

// Bucket delete ------------------------------------------------------------------------------------

// test delete and insert

void bucket_delete_by_physical_index(bucket_t* bucket, int physical_index){
    assert(bucket);
    assert(physical_index != -1);

    assert(bucket_verify(bucket) == NO_MISTAKE);

    memset(bucket->keys+ SIZE_WORD * physical_index, 0, SIZE_WORD);
    bucket->hashes[physical_index] = 0;

    if(physical_index == bucket->list_head) bucket->list_head = bucket->next[physical_index];

    int prev = bucket->prev[physical_index];
    assert(prev != -1);

    int next = bucket->next[physical_index];
    assert(next != -1);

    bucket->next[prev] = next;
    bucket->prev[next] = prev;

    bucket->next[physical_index] = bucket->first_free;
    bucket->prev[physical_index] = -1;
    bucket->first_free = physical_index;

    bucket->size--;
    bucket->is_linearized = false;

    if(bucket->size == 0){
        // иначе мы бы пошли в элемент -1, если бы удалили все элемент а потом захотели вставлять
        bucket->next[physical_index] = physical_index;
        bucket->prev[physical_index] = physical_index;
    }

    assert(bucket_verify(bucket) == NO_MISTAKE);
}

void bucket_delete_by_logical_index(bucket_t* bucket, int logical_index){
    assert(bucket);

    assert(bucket_verify(bucket) == NO_MISTAKE);

    int index = 0;
    if(bucket->is_linearized){
        index = bucket_find_phys_index_optimized(bucket, logical_index);
    }
    else{
        index = bucket_find_phys_index(bucket, logical_index);
    }
    assert(index != -1);

    bucket_delete_by_physical_index(bucket, index);

    assert(bucket_verify(bucket) == NO_MISTAKE);
}

// --------------------------------------------------------------------------------------------------

// Bucket linearize ---------------------------------------------------------------------------------

bucket_err_t bucket_linearize(bucket_t* bucket){
    assert(bucket);
    assert(bucket_verify(bucket) == NO_MISTAKE);

    int size = bucket->size;

    char* keys = NULL;
    uint32_t* hashes = NULL;
    int* next = NULL;
    int* prev = NULL;

    bucket_err_t err = calloc_arrays(bucket,&keys, &hashes, &next, &prev, size);
    if(err != NO_MISTAKE) return err;

    int i = bucket->list_head;
    for(int count = 0; count < size; count++){
        memcpy(keys + count * SIZE_WORD, bucket->keys + i * SIZE_WORD, SIZE_WORD);
        hashes[count] = bucket->hashes[i];

        prev[count] = (count == 0) ? size - 1 : count - 1;
        next[count] = (count == size - 1) ? 0 : count + 1;

        i = bucket->next[i];
    }

    bucket->list_head = 0;
    bucket->first_free = -1;

    bucket_dtor(bucket);

    bucket->capacity = size;
    bucket->hashes = hashes;
    bucket->keys = keys;
    bucket->next = next;
    bucket->prev = prev;

    bucket->is_linearized = true;

    assert(bucket_verify(bucket) == NO_MISTAKE);

    return NO_MISTAKE;
}

// --------------------------------------------------------------------------------------------------

// Bucket verify ------------------------------------------------------------------------------------

bucket_err_t bucket_verify(const bucket_t* bucket){
    assert(bucket);

    if(bucket->left_canary != BUCKET_CANARY){
        fprintf(stderr,"Left canary corrupted\n");
        return LEFT_CORRUPTED;
    }
    if(bucket->right_canary != BUCKET_CANARY){
        fprintf(stderr,"Right canary corrupted\n");
        return RIGHT_CORRUPTED;
    }

    if(canary_verify(bucket->keys, sizeof(char) * SIZE_WORD * bucket->capacity) != NO_MISTAKE_CANARY){
        fprintf(stderr,"Keys canaries corrupted\n");
        return KEYS_CORRUPTED;
    }
    if(canary_verify(bucket->hashes, sizeof(uint32_t) * bucket->capacity) != NO_MISTAKE_CANARY){
        fprintf(stderr,"Hashes canaries corrupted\n");
        return HASHES_CORRUPTED;
    }
    if(canary_verify(bucket->prev, sizeof(int) * bucket->capacity) != NO_MISTAKE_CANARY){
        fprintf(stderr,"Hashes canaries corrupted\n");
        return PREV_CORRUPTED;
    }
    if(canary_verify(bucket->next, sizeof(int) * bucket->capacity) != NO_MISTAKE_CANARY){
        fprintf(stderr,"Hashes canaries corrupted\n");
        return NEXT_CORRUPTED;
    }

    // check order
    int idx = bucket->list_head;
    for(int data_idx = 1; data_idx < bucket->size; data_idx++){
        if(bucket->next[bucket->prev[idx]] != idx || bucket->prev[bucket->next[idx]] != idx){
            fprintf(stderr,"Bucket order corrupted\n");
            return INCORR_ORDER;
        }
        idx = bucket->next[idx];
    }

    if(bucket->list_head < 0){
        fprintf(stderr,"bucket->list_head < 0\n");
        return INCORR_LIST_HEAD;
    }
    if(bucket->size < 0 || bucket->size > bucket->capacity){
        fprintf(stderr,"bucket->size < 0\n");
        return INCORR_SIZE;
    }
    if(bucket->capacity < 0){
        fprintf(stderr,"bucket->capacity < 0\n");
        return INCORR_CAPACITY;
    }
    if(bucket->first_free >= bucket->capacity || bucket->first_free < -1){
        fprintf(stderr,"Incorrect first free\n");
        return INCORR_FIRST_FREE;
    }

    return NO_MISTAKE;

}

// --------------------------------------------------------------------------------------------------

// Bucket resize ------------------------------------------------------------------------------------

/*
Для 2 и 3 оптимизаций надо чтобы размер бакета был хотя бы 8, поэтому я написала функцию, с помощью
которой можно увеличить размер бакета
*/

void bucket_resize_up(bucket_t* bucket, size_t new_size){
    if(new_size < bucket->size) return;

    if(recalloc_arrays(bucket, bucket->capacity, new_size) != NO_MISTAKE){
        return;
    }

    bucket->capacity_more_or_eqthan_eight = true;
}

// --------------------------------------------------------------------------------------------------

// Bucket align --------------------------------------------------------------------------------------

void bucket_keys_hashes_align(bucket_t* bucket, size_t alignment){

    char* aligned_keys = canary_alligned_calloc(sizeof(char) * SIZE_WORD, bucket->capacity,  alignment);
    if(!aligned_keys){
        fprintf(stderr, "Align keys alloc error\n");
        return;
    }

    uint32_t* aligned_hashes = (uint32_t*)canary_alligned_calloc(sizeof(uint32_t), bucket->capacity,  alignment);
    if(!aligned_hashes){
        canary_free(aligned_keys, sizeof(char) * SIZE_WORD * bucket->capacity);
        fprintf(stderr, "Align hashes alloc error\n");
        return;
    }

    canary_full_array_memcpy(bucket->keys, aligned_keys, sizeof(char) * SIZE_WORD * bucket->capacity);
    canary_full_array_memcpy(bucket->hashes, aligned_hashes, sizeof(uint32_t) * bucket->capacity);

    canary_free(bucket->keys, sizeof(char) * SIZE_WORD * bucket->capacity);
    canary_free(bucket->hashes, sizeof(uint32_t) * bucket->capacity);

    bucket->keys = aligned_keys;
    bucket->hashes = aligned_hashes;

    bucket->is_aligned = true;
}

// --------------------------------------------------------------------------------------------------

// Bucket dtor --------------------------------------------------------------------------------------

void bucket_dtor(bucket_t* bucket){
    if(!bucket) return;
    assert(bucket->left_canary == BUCKET_CANARY && bucket->right_canary == BUCKET_CANARY);

    if(bucket->keys){
        canary_free(bucket->keys, sizeof(char) * SIZE_WORD * bucket->capacity);
        bucket->keys = NULL;
    } 

    if(bucket->hashes){
        canary_free(bucket->hashes, sizeof(uint32_t) * bucket->capacity);
        bucket->hashes = NULL;
    }

    if(bucket->next){
        canary_free(bucket->next, sizeof(int) * bucket->capacity);
        bucket->next = NULL;
    }

    if(bucket->prev){
        canary_free(bucket->prev, sizeof(int) * bucket->capacity);
        bucket->prev = NULL;
    }

    bucket->capacity = 0;

    assert(bucket->left_canary == BUCKET_CANARY && bucket->right_canary == BUCKET_CANARY);
}

// --------------------------------------------------------------------------------------------------
