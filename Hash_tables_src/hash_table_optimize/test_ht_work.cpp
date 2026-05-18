#include <cstdlib>
#include <ctime>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <time.h>
#include <ctype.h>
#include "hash_tables/hash_table.h"

#define WORD_LEN 32


char* get_string_array(const char* filename, size_t* num_of_words);

hash_table* prepare_hashtable(char* words, size_t num_of_words);

void test_hashtable(hash_table* ht, int num_of_tests, int heat_tests, char* words);

void test_ht_work(char* words, hash_table* ht, size_t num_of_words);

void test_ht_crash(hash_table* ht);

int main(){
    size_t num_of_words_test = 0;
    char* buffer = get_string_array("tests_src/small.bin", &num_of_words_test);

    hash_table* ht = prepare_hashtable(buffer, num_of_words_test/WORD_LEN);

    test_ht_work(buffer, ht, num_of_words_test/WORD_LEN);

    hash_table_dtor(ht);

    free(buffer);

    return 0;
}



char* get_string_array(const char* filename, size_t* num_of_words){
    assert(filename);

    FILE* fp = fopen(filename, "rb");
    if(!fp){
        fprintf(stderr, "Can't open file");
        return NULL;
    }

    struct stat file_info = {};
    if(stat(filename, &file_info)){
        fprintf(stderr, "Error getting file info\n");
    }

    char* buffer = (char*)aligned_alloc(ALIGN, (file_info.st_size)*sizeof(char));
    *num_of_words = file_info.st_size;
    fread(buffer, 1, file_info.st_size, fp);
    fclose(fp);

    return buffer;
}


hash_table* prepare_hashtable(char* words, size_t num_of_words){
    assert(words);

    hash_table* ht = hash_table_ctor(13);
    for (size_t j = 0; j < num_of_words; j++){
        bool is_success = hash_table_insert(words + j * WORD_LEN, ht);
        assert(is_success);
    }

    return ht;
}
void test_ht_work(char* words, hash_table* ht, size_t num_of_words){
    hash_table_dump(ht, "Dumping when made");

    for(size_t i = 0; i < num_of_words; i++){
        assert(hash_table_find(words + i * WORD_LEN, ht) == true);
    }

    for(size_t i = 0; i < num_of_words; i++){
        hash_table_delete(words + i * WORD_LEN, ht);
        if(i % 50 == 0){
            hash_table_dump(ht, "Dump while deleting");
        }
    }

    for(size_t j = 0; j < num_of_words; j++){
        bool is_success = hash_table_insert(words + j * WORD_LEN, ht);
        assert(is_success);
    }

    hash_table_linearize(ht);
    hash_table_buckets_resize_up(ht);
    hash_table_dump(ht, "Dump after linearize");

    test_ht_crash(ht);

}

void test_ht_crash(hash_table* ht){
    assert(ht->elements[0].prev[ht->elements[0].first_free]);
    int prev = ht->elements[0].prev[ht->elements[0].first_free];
    ht->elements[0].prev[ht->elements[0].first_free] = -2;
    hash_table_verify(ht);
    ht->elements[0].prev[ht->elements[0].first_free] = prev;

    int size = ht->size;
    ht->size = (LOAD_FACTOR + 1) * ht->capacity;
    hash_table_verify(ht);
    ht->size = size;

    char* keys = ht->elements[0].keys;
    ht->elements[0].keys = NULL;
    hash_table_verify(ht);
    ht->elements[0].keys = keys;

    int next = ht->elements[0].next[0];
    ht->elements[0].next[0] = 99999;
    hash_table_verify(ht);
    ht->elements[0].next[0] = next;
}
