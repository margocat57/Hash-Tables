#include <assert.h>
#include <cstring>
#include <cstdio>
#include <cstdint>
#include <immintrin.h>
#include <bits/types.h>
#include <immintrin.h>
#include <stdlib.h>
#include <bsd/stdlib.h>
#include "bucket_func/bucket_func.h"
#include "bucket_func/bucket_dump.h"

const uint64_t CANARY_VALUE = 0xBADC0FFEE0DDF00Dull;
const uint64_t LOAD_FACTOR = 10;

typedef uint64_t hash_table_err_t;

enum ht_mistakes{
    NO_MISTAKE_HT                = 0,
    INCORR_SIZE_HT               = 1 << 17,
    INCORR_CAPACITY_HT           = 1 << 18,
    LEFT_CORRUPTED_HT            = 1 << 19,
    RIGHT_CORRUPTED_HT           = 1 << 20,
    BUCKETS_CORR_HT              = 1 << 21
};

struct hash_table {
    uint64_t left_canary;

    int capacity;
    int size;
    bucket_t* elements;

    uint64_t right_canary;
};

hash_table* hash_table_ctor(int capacity);

bool hash_table_insert(const char* key, hash_table* ht);

bool hash_table_find(const char* key, const hash_table* ht);

__attribute__((noinline))
int find_node(const bucket_t* bucket, const uint32_t hash, const char* key);

__attribute__((noinline))
int find_node_optimized(const bucket_t* bucket, const uint32_t hash, const char* key);

void hash_table_dtor(hash_table* ht);

void hash_table_dump(const hash_table* ht, const char* msg);

void hash_table_delete(const char* key, hash_table* ht);

uint32_t hash_func(const char* s);

bool hash_table_linearize(hash_table* ht);

void buckets_dtor(bucket_t* buckets, int size);

hash_table_err_t hash_table_verify(const hash_table* ht);

bool hash_table_buckets_resize_up(hash_table* ht);

bool hash_table_buckets_align(hash_table* ht);
