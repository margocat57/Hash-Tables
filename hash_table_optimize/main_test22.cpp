#include <cstdlib>
#include <ctime>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>
#include "hash_tables/hash_table22.h"

#define NUM_OF_WORDS 376024
#define NUM_OF_WORDS_TEST 2504096
#define WORD_LEN 32


char* get_string_array(const char* filename);

char* words_ctor(char* buffer, size_t size);

hash_table* prepare_hashtable(char* words);

void test_hashtable(hash_table* ht, int num_of_tests, int heat_tests, char* words);

int main(int argc, char *argv[]){
    if(argc < 3){
        fprintf(stderr, "Can't get name of input file for keys");
        return 1;
    }

    int num_of_tests = atoi(argv[1]);
    int heat_tests = atoi(argv[2]);

    char* buffer = get_string_array("tests_src/words_alpha.txt");
    char* words = words_ctor(buffer, NUM_OF_WORDS);
    free(buffer);

    char* test_buffer = get_string_array("tests_src/words.txt");
    char* words_test = words_ctor(test_buffer, NUM_OF_WORDS_TEST);
    free(test_buffer);

    hash_table* ht = prepare_hashtable(words);

    test_hashtable(ht, num_of_tests, heat_tests, words_test);

    hash_table_dtor(ht);

    free(words);

    free(words_test);

    return 0;
}

bool incorr_work_with_stat(const char *name_of_file, struct stat *all_info_about_file){
    assert(name_of_file != NULL);
    assert(all_info_about_file != NULL);

    if (stat(name_of_file, all_info_about_file) == -1)
    {
        perror("Stat error");
        fprintf(stderr, "Error code: %d\n", errno);
        return true;
    }
    return false;
}


char* get_string_array(const char* filename){
    assert(filename);

    FILE* fp = fopen(filename, "r");
    if(!fp){
        fprintf(stderr, "Can't open file");
        return NULL;
    }

    struct stat file_info = {};
    bool is_stat_err = incorr_work_with_stat(filename, &(file_info));
    if (is_stat_err){
        return NULL;
    }

    char* buffer = (char*)calloc((file_info.st_size + 1), sizeof(char));

    fread(buffer, 1, file_info.st_size, fp);
    fclose(fp);

    return buffer;
}

char* words_ctor(char* buffer, size_t size){
    char* words = (char*)aligned_alloc(WORD_LEN, WORD_LEN * size);
    memset(words, 0, WORD_LEN * size);
    char* p = buffer;

    int i = 0;

    while (*p) {
        const char* start = p;
        while (*p && *p != '\n') p++;

        size_t len = p - start - 1;
        if(len >= 30){printf("%.*s\n", (int)len, start);}

        char* block = words + i * WORD_LEN;
        memcpy(block, start, len);

        if (*p == '\n') p++; 
        i++;
    }

    return words;
}

hash_table* prepare_hashtable(char* words){
    assert(words);

    hash_table* ht = hash_table_ctor();
    for (int j = 0; j < NUM_OF_WORDS; j++){
        hash_table_insert(words + j * WORD_LEN, ht);
    }

    return ht;
}


void test_hashtable(hash_table* ht, int num_of_tests, int heat_tests, char* words){
    assert(ht);

    double* tests_results = (double*)calloc(num_of_tests, sizeof(double));

    timespec start = {};
    timespec end = {};

    for(int i = 0; i < num_of_tests + heat_tests; i++){
        clock_gettime(CLOCK_MONOTONIC_RAW, &start);

        for(int j = 0; j < NUM_OF_WORDS_TEST; j++){
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
