#include <assert.h>
#include <cstring>
#include <cstdio>
#include <cstdint>
#include <immintrin.h>
#include <bits/types.h>
#include <immintrin.h>
#include <stdlib.h>
#include <bsd/stdlib.h>

const int size_word = 32;

const int block_data_amount = 8;

struct bucket_t{
    char* keys;
    uint32_t* hashes;
    int* next;
    int list_head; // чтобы безопасно удалять нулевой элемент
    bool is_linearized;
    int capacity;
    int first_free;
    int size;
};


struct hash_table {
    int size;
    bucket_t* elements;
};

enum ALLOC_MISTAKES {
    NO_MISTAKES,
    HASH_ALLOC_MISTAKE,
    KEY_ALLOC_MISTAKE,
    NEXT_ALLOC_MISTAKE,
    PREV_ALLOC_MISTAKE
};

hash_table* hash_table_ctor(int size);

void hash_table_insert(const char* key, hash_table* ht);

bool hash_table_find(const char* key, const hash_table* ht);

void hash_table_dtor(hash_table* ht);

void hash_table_dump(const hash_table* ht);

int find_node(bucket_t* bucket, const uint32_t hash, const char* key);

int find_node_optimized(bucket_t* bucket, const uint32_t hash, const char* key);

void hash_table_delete(const char* key, hash_table* ht);

uint32_t hash_func(const char* s);

bool hash_table_linearize(hash_table* ht);