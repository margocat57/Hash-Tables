#include <assert.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cstdint>

typedef uint32_t (*hash_func_t)(const char*);

const int NOT_FOUND = -6666;

struct node_t {
    node_t* next;
    char* key;
    uint32_t hash;
};


struct hash_table {
    int size;
    node_t** elements;
};

hash_table* hash_table_ctor();

void hash_table_insert(const char* key, const int len, hash_table* ht);

bool hash_table_find(const char* key, const int len, const hash_table* ht);

void hash_table_dtor(hash_table* ht);

void hash_table_dump(const hash_table* ht);

uint32_t hash_polynomial(const char* s, const int len);

node_t* find_node(node_t* node, const uint32_t hash, const char* key);

uint32_t hash_crc32(const char* s);