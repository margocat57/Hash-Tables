#include <cstdlib>
#include <ctime>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <time.h>
#include "hash_tables/hash_table.h"

#define WORD_LEN 32


char* get_string_array(const char* filename, size_t* num_of_words);

hash_table* prepare_hashtable(char* words, size_t num_of_words);

void test_hashtable(hash_table* ht, int num_of_tests, int heat_tests, char* words, size_t num_of_words);

int main(int argc, char *argv[]){
    if(argc < 3){
        fprintf(stderr, "Can't get name of input file for keys");
        return 1;
    }

    int num_of_tests = atoi(argv[1]);
    int heat_tests = atoi(argv[2]);
    size_t num_of_words_buffer = 0;
    size_t num_of_words_test = 0;

    char* buffer = get_string_array("tests_src/words_alpha.bin", &num_of_words_buffer);
    char* test_buffer = get_string_array("tests_src/words1.bin", &num_of_words_test);

    hash_table* ht = prepare_hashtable(buffer, num_of_words_buffer/WORD_LEN);

    test_hashtable(ht, num_of_tests, heat_tests, test_buffer, num_of_words_test/WORD_LEN);

    hash_table_dtor(ht);

    free(buffer);
    free(test_buffer);

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

    hash_table* ht = hash_table_ctor(70001);
    for (int j = 0; j < num_of_words; j++){
        bool is_success = hash_table_insert(words + j * WORD_LEN, ht);
        assert(is_success);
    }

    return ht;
}

bool hash_table_prefare_for_test(hash_table* ht){
    if(!hash_table_linearize(ht)){
        printf("Can't linearize and use fast optimizations");
        return false;
    }

    if(!hash_table_buckets_resize_up(ht)){
        printf("Can't resize and use fast optimizations");
        return false;
    }

    return true;
}


void test_hashtable(hash_table* ht, int num_of_tests, int heat_tests, char* words, size_t num_of_words){
    assert(ht);

    if(!hash_table_prefare_for_test(ht)){
        fprintf(stderr, "Ht can't be prepared for test\n");
        return;
    }

    double* tests_results = (double*)calloc(num_of_tests, sizeof(double));

    timespec start = {};
    timespec end = {};

    for(int i = 0; i < num_of_tests + heat_tests; i++){
        clock_gettime(CLOCK_MONOTONIC_RAW, &start);

        for(int j = 0; j < num_of_words; j++){
            hash_table_find(words + j * WORD_LEN, ht);
        }

        clock_gettime(CLOCK_MONOTONIC_RAW, &end);

        if(i >= heat_tests)
            tests_results[i - heat_tests] = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

        fprintf(stderr, "test %d ended\n", i);

    }

    for(int i = 0; i < num_of_tests; i++){
        printf("%lg\n", tests_results[i]);
    }

    free(tests_results);

    

}
