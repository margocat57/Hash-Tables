#ifndef BUCKET
#define BUCKET

#include <bsd/stdlib.h>
#include <cstdint>
#include "../../../../my_canary_std_func/canary_std_func.h"

typedef uint64_t bucket_err_t;

enum bucket_mistakes{
    NO_MISTAKE                = 0,
    DATA_ALLOC_ERR            = 1 << 0,
    NEXT_ALLOC_ERR            = 1 << 1,
    PREV_ALLOC_ERR            = 1 << 2,
    HASHES_ALLOC_ERR          = 1 << 3,
    BUCKET_NOT_EXISTS         = 1 << 4,
    KEYS_CORRUPTED            = 1 << 5,
    HASHES_CORRUPTED          = 1 << 6,
    NEXT_CORRUPTED            = 1 << 7,
    INCORR_LIST_HEAD          = 1 << 8,
    INCORR_SIZE               = 1 << 9,
    INCORR_CAPACITY           = 1 << 10,
    PREV_CORRUPTED            = 1 << 11,
    LEFT_CORRUPTED            = 1 << 12,
    RIGHT_CORRUPTED           = 1 << 13,
    INCORR_ORDER              = 1 << 14,
    INCORR_FIRST_FREE         = 1 << 16
};

const int SIZE_WORD = 32;
const int ALIGN = 32;
const int BLOCK_DATA_AMOUNT = 8;
static constexpr uint64_t BUCKET_CANARY     = 0xBADC0FFEE0DDF00Dull;

struct bucket_t{
    uint64_t left_canary;

    char* keys;
    uint32_t* hashes;
    int* next;
    int* prev;
    int list_head;   // чтобы безопасно удалять нулевой элемент
    bool is_linearized;
    bool capacity_more_or_eqthan_eight;
    bool is_aligned; // нужно хэш таблице - выносим туда
    int capacity;
    int first_free;
    int size;

    uint64_t right_canary;
};

void fill_arrays(bucket_t* bucket, int start, int end);

bucket_err_t bucket_ctor(bucket_t* bucket);

bucket_err_t recalloc_arrays(bucket_t* bucket, int capacity, int new_capacity);

bool bucket_insert_to_end(bucket_t* bucket, const char* key, const uint32_t hash);

void bucket_delete_by_physical_index(bucket_t* bucket, int physical_index);

bucket_err_t bucket_linearize(bucket_t* bucket);

void bucket_dtor(bucket_t* bucket);

bucket_err_t bucket_verify(const bucket_t* bucket);

void bucket_resize_up(bucket_t* bucket, size_t new_size);

void bucket_keys_hashes_align(bucket_t* bucket, size_t alignment);

#endif //BUCKET