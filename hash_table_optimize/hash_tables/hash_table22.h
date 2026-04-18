#include <assert.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cstdint>

typedef uint32_t (*hash_func_t)(const char*);

const int NOT_FOUND = -6666;

struct bucket_t{
    char** keys;
    uint32_t* hashes;
    int capacity;
    int first_free;
};


struct hash_table {
    int size;
    bucket_t* elements;
};

hash_table* hash_table_ctor();

void hash_table_insert(const char* key, hash_table* ht);

bool hash_table_find(const char* key, const hash_table* ht);

void hash_table_dtor(hash_table* ht);

void hash_table_dump(const hash_table* ht);

int find_node(bucket_t* bucket, const uint32_t hash, const char* key);

uint32_t hash_crc32(const char* s);