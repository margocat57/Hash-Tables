#ifndef BUCKET
#define BUCKET

#include <bsd/stdlib.h>

typedef uint64_t bucket_err_t;

enum bucket_mistakes{
    NO_MISTAKE                = 0,
    DATA_ALLOC_ERR            = 1 << 0,
    NEXT_ALLOC_ERR            = 1 << 1,
    PREV_ALLOC_ERR            = 1 << 2,
    HASHES_ALLOC_ERR          = 1 << 3,
    BUCKET_NOT_EXISTS         = 1 << 4
};

const int SIZE_WORD = 32;
const int ALIGN = 32;
const int BLOCK_DATA_AMOUNT = 8;

struct bucket_t{
    char* keys;
    uint32_t* hashes;
    int* next;
    int* prev;
    int list_head; // чтобы безопасно удалять нулевой элемент
    bool is_linearized;
    int capacity;
    int first_free;
    int size;
};

bucket_err_t bucket_ctor(bucket_t* bucket);

bucket_err_t recalloc_arrays(bucket_t* bucket, int capacity, int new_capacity);

bool buckets_insert(bucket_t* bucket, const char* key, const uint32_t hash);

void bucket_insert(bucket_t* bucket, const char* key, const uint32_t hash);

void bucket_delete(bucket_t* bucket, const char* key, const uint32_t hash);

bucket_err_t bucket_linearize(bucket_t* bucket);

__attribute__((noinline))
int find_node(bucket_t* bucket, const uint32_t hash, const char* key);

int find_node_optimized(bucket_t* bucket, const uint32_t hash, const char* key);

void bucket_dtor(bucket_t* bucket);

void buckets_dtor(bucket_t* buckets, int size);

#endif //BUCKET