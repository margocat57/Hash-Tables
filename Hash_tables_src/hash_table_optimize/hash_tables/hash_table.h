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


struct hash_table {
    int capacity;
    int size;
    bucket_t* elements;
};

hash_table* hash_table_ctor(int capacity);

void hash_table_insert(const char* key, hash_table* ht);

bool hash_table_find(const char* key, const hash_table* ht);

void hash_table_dtor(hash_table* ht);

void hash_table_dump(const hash_table* ht);

void hash_table_delete(const char* key, hash_table* ht);

uint32_t hash_func(const char* s);

bool hash_table_linearize(hash_table* ht);