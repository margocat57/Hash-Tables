#include <stdlib.h>
#include <stdio.h>
#include "../hash_tables/hash_func.h"
#include "../hash_tables/hash_table.h"

#define NUM_OF_WORDS 20679

struct hash_func_and_name{
    hash_func_t hash_func;
    const char* hash_name;
};

void ht_dump_buckets_tests(hash_table* ht, hash_func_and_name func_and_name);

int main(){
    FILE* fp = fopen("../../tests_src/output.txt", "r");
    char** keys = (char**)calloc(NUM_OF_WORDS,  sizeof(char*));

    for (int i = 0; i < NUM_OF_WORDS; i++) {
        keys[i] = (char*)calloc(18, sizeof(char)); 
        fscanf(fp, "%s", keys[i]);
    }

    hash_func_and_name func[] = {
        { hash_length,                  "hash_length"},
        { hash_first_letter,            "hash_first_letter"},
        { hash_sum_letters,             "hash_sum_letters"},
        { hash_polynomial,              "hash_polynomial"},
        { hash_crc32,                   "hash_crc32"},
        { hash_jenkins_one_at_a_time32, "hash_jenkins_one_at_a_time32"},
        { elf_hash,                     "elf_hash"},
        { fnv1a_hash,                   "fnv1a_hash"},
        { murmur3_hash,                 "murmur3_hash"}

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