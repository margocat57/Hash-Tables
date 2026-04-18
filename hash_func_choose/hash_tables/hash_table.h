#include <assert.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cstdint>

typedef uint32_t (*hash_func_t)(const char*);


struct node_t {
    node_t* next;
    node_t* prev;
    const char* key;
    int val;
    uint32_t hash;
};


struct hash_table {
    int size;
    node_t** elements;
    hash_func_t hash_func;
};

hash_table* hash_table_ctor(hash_func_t hash_func);

void hash_table_insert(const char* key, hash_table* ht);

bool hash_table_find(const char* key, const hash_table* ht);

void hash_table_dtor(hash_table* ht);

void hash_table_dump(const hash_table* ht);
