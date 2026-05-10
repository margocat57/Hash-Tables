#include "canary_std_func.h"
#include <stdlib.h>


char* canary_calloc(size_t size, size_t num_of_elements){
    assert(size <= 0.8 * SIZE_MAX);
    assert(num_of_elements <= 0.8 * SIZE_MAX);

    size_t size_full = size * num_of_elements + 2 * CANARY_SIZE;
    assert(size <= size_full);

    char* array_and_canaries = (char*)calloc(1, size_full);
    if(!array_and_canaries) return NULL;

    memcpy(array_and_canaries, CANARY, CANARY_SIZE);

    memcpy(array_and_canaries + size_full - CANARY_SIZE, CANARY, CANARY_SIZE);

    return array_and_canaries + CANARY_SIZE; 
}

char* canary_alligned_calloc(size_t size, size_t num_of_elements, size_t align){
    assert(size <= 0.8 * SIZE_MAX);
    assert(num_of_elements <= 0.8 * SIZE_MAX);

    assert(align <= 0.8 * SIZE_MAX);
    assert(align % sizeof(void*) == 0); // нужно для posix_memalign - кратен размеру указателя
    assert(align != 0 && align <= 32);  // потому что канарейка 32-байтная
    assert((align & (align - 1)) == 0); // нужно для posix_memalign - степень двойки

    size_t size_full = size * num_of_elements + 2 * CANARY_SIZE;
    assert(size <= size_full);

    void* data = NULL;
    if(posix_memalign(&data, align, size_full) != 0) return NULL;
    char* array_and_canaries = (char*) data;

    memset(array_and_canaries, 0, size_full);

    memcpy(array_and_canaries, CANARY, CANARY_SIZE);
    
    memcpy(array_and_canaries + size_full - CANARY_SIZE, CANARY, CANARY_SIZE);

    return array_and_canaries + CANARY_SIZE; 
}

char* canary_recalloc(void* data, size_t capacity, size_t new_capacity, size_t size_of_elem){
    if(!data){
        return canary_calloc(size_of_elem, new_capacity);
    }

    assert(capacity <= 0.8*SIZE_MAX);
    assert(new_capacity <= 0.8*SIZE_MAX);
    assert(size_of_elem <= 0.8*SIZE_MAX);

    if(new_capacity == 0){
        canary_free(data,capacity * size_of_elem); 
        return NULL;
    } 

    assert(canary_verify(data, capacity * size_of_elem) == NO_MISTAKE_CANARY);

    char* array_and_canaries = (char*)data - CANARY_SIZE;

    size_t old_size_in_bytes = capacity * size_of_elem + 2 * CANARY_SIZE;
    assert(size_of_elem <= old_size_in_bytes);

    size_t new_size_in_bytes = new_capacity * size_of_elem + 2 * CANARY_SIZE;
    assert(size_of_elem <= new_size_in_bytes);

    char* array_and_canaries_new = (char*)recallocarray(array_and_canaries, old_size_in_bytes, new_size_in_bytes, 1);
    if(!array_and_canaries_new) return NULL;

    if(new_capacity > capacity) memset(array_and_canaries_new + old_size_in_bytes - CANARY_SIZE, 0, CANARY_SIZE);

    memcpy(array_and_canaries_new + new_size_in_bytes - CANARY_SIZE, CANARY, CANARY_SIZE);

    return array_and_canaries_new + CANARY_SIZE;
}

canary_err_t canary_verify(void* data, size_t user_size){
    if(!data){
        fprintf(stderr, "Null data ptr was given\n");
        return NULL_DATA_PTR_CANARY;
    }

    char* array_and_canaries = (char*)data - CANARY_SIZE;

    if(memcmp(array_and_canaries, CANARY, CANARY_SIZE) != 0){
        fprintf(stderr, "Left canary corrupted\n");
        return LEFT_CANARY_NOT_IN_PLACE;
    }

    if(memcmp(array_and_canaries + CANARY_SIZE + user_size, CANARY, CANARY_SIZE) != 0){
        fprintf(stderr, "Right canary corrupted\n");
        return RIGHT_CANARY_NOT_IN_PLACE;
    }

    return NO_MISTAKE_CANARY;
}

void canary_full_array_memcpy(void* src, void* dst, size_t user_size){ 
    assert(src);
    assert(dst);

    assert(canary_verify(src, user_size) == NO_MISTAKE_CANARY);
    assert(canary_verify(dst, user_size) == NO_MISTAKE_CANARY);

    char* array_and_canaries_src = (char*)src - CANARY_SIZE;
    char* array_and_canaries_dst = (char*)dst - CANARY_SIZE;

    memcpy(array_and_canaries_dst, array_and_canaries_src, user_size + 2 * CANARY_SIZE);

}

void canary_free(void* data, size_t user_size){
    if(!data) return;

    assert(canary_verify(data, user_size) == NO_MISTAKE_CANARY);

    char* array_and_canaries = (char*)data - CANARY_SIZE;
    free(array_and_canaries);
}

