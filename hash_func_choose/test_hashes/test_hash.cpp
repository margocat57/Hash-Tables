#include <stdlib.h>
#include <stdio.h>
#include "../hash_tables/hash_func.h"
#include "../hash_tables/hash_table.h"

#define NUM_OF_WORDS 20679

struct hash_func_and_name{
    hash_func_t hash_func;
    const char* hash_name;
};

#define func_and_name(x) {x, #x}

void ht_dump_buckets_tests(hash_table* ht, hash_func_and_name func_and_name);

char** create_keys_array();

void test_hashes(char** keys);

int main(){
    char** keys = create_keys_array();
    if(!keys) return 0;

    test_hashes(keys);

    for(int i = 0; i < NUM_OF_WORDS; i++){
        free(keys[i]);
    }

    free(keys);

    return 0;

}

void ht_dump_buckets_tests(hash_table* ht, hash_func_and_name func_and_name){
    assert(ht);
    for(int idx = 0; idx < ht->size; idx++){
        int sum = 0;
        node_t* node = ht->elements[idx];
        if(!node){
            printf("%s;%d;0\n", func_and_name.hash_name, idx);
            continue;
        } 

        sum++;

        while(node->next){
            node = node->next;
            sum++;
        }

        printf("%s;%d;%d\n", func_and_name.hash_name, idx, sum);
    }
}

char** create_keys_array(){
    FILE* fp = fopen("tests_src/output.txt", "r");
    if(!fp){
        fprintf(stderr, "Can't open file\n");
        return NULL;
    }
    char** keys = (char**)calloc(NUM_OF_WORDS,  sizeof(char*));

    for (int i = 0; i < NUM_OF_WORDS; i++) {
        keys[i] = (char*)calloc(18, sizeof(char)); 
        fscanf(fp, "%s", keys[i]);
    }
    return keys;
}

void test_hashes(char** keys){
    hash_func_and_name func[] = {
        func_and_name(hash_length),
        func_and_name(hash_first_letter),
        func_and_name(hash_sum_letters),
        func_and_name(hash_polynomial),
        func_and_name(hash_crc32),
        func_and_name(hash_jenkins_one_at_a_time32), 
        func_and_name(elf_hash), 
        func_and_name(fnv1a_hash),
        func_and_name(murmur3_hash)
    };

    size_t num_of_hash_func = sizeof(func) / sizeof(func[0]);

    printf("hash_func;bucket;size\n");

    for(size_t i = 0; i < num_of_hash_func; i++){
        hash_table* ht = hash_table_ctor(func[i].hash_func);
        for (int j = 0; j < NUM_OF_WORDS; j++){
            hash_table_insert(keys[j], ht);
        }
        ht_dump_buckets_tests(ht, func[i]);
        hash_table_dtor(ht);
    }
}