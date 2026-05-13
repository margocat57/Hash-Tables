#ifndef CANARY_STD
#define CANARY_STD

#include <bsd/stdlib.h>
#include <stdint.h>
#include <cstdint>
#include <cstring>
#include <assert.h>

enum canary_err_t{
    NO_MISTAKE_CANARY,
    LEFT_CANARY_NOT_IN_PLACE,
    RIGHT_CANARY_NOT_IN_PLACE,
    NULL_DATA_PTR_CANARY
};

const uint8_t CANARY[] = {
    0xCA, 0xFE, 0xBA, 0xBE, 0xDE, 0xAD, 0xBE, 0xEF,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
    0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00,
    0x13, 0x37, 0x42, 0x24, 0xAB, 0xCD, 0xEF, 0x01
};

#define CANARY_SIZE (sizeof CANARY)

char* canary_calloc(size_t size, size_t num_of_elements);

char* canary_alligned_calloc(size_t size, size_t num_of_elements, size_t align);

char* canary_recalloc(void* data, size_t capacity, size_t new_capacity, size_t size_of_elem);

char* canary_alligned_recalloc(void* data, size_t capacity, size_t new_capacity, size_t size_of_elem, size_t align);

canary_err_t canary_verify(void* data, size_t user_size);

void canary_full_array_memcpy(void* src, void* dst, size_t user_size);

void canary_free(void* data, size_t user_size);


#endif //CANARY_STD