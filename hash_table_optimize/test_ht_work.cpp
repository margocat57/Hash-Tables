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

#define NUM_OF_WORDS 206
#define WORD_LEN 32


char* get_string_array(const char* filename);

char* words_ctor(char* buffer, size_t size);

hash_table* prepare_hashtable(char* words);

void test_hashtable(hash_table* ht, int num_of_tests, int heat_tests, char* words);

int main(){
    char* buffer = get_string_array("tests_src/small.txt");
    char* words = words_ctor(buffer, NUM_OF_WORDS);
    free(buffer);

    hash_table* ht = prepare_hashtable(words);

    hash_table_dump(ht);

    for(int i = 0; i < NUM_OF_WORDS; i++){
        assert(hash_table_find(words + i * WORD_LEN, ht) == true);
    }

    for(int i = 0; i < NUM_OF_WORDS; i++){
        if(i % 8 == 0){
            hash_table_delete(words + i * WORD_LEN, ht);
            hash_table_dump(ht);
        }
    }

    hash_table_linearize(ht);
    hash_table_dump(ht);

    hash_table_dtor(ht);

    free(words);

    return 0;
}



char* get_string_array(const char* filename){
    assert(filename);

    FILE* fp = fopen(filename, "r");
    if(!fp){
        fprintf(stderr, "Can't open file");
        return NULL;
    }

    struct stat file_info = {};
    if(stat(filename, &file_info)){
        fprintf(stderr, "Error getting file info\n");
    }

    char* buffer = (char*)calloc((file_info.st_size + 1), sizeof(char));

    fread(buffer, 1, file_info.st_size, fp);
    fclose(fp);

    return buffer;
}

char* words_ctor(char* buffer, size_t size){
    char* words = (char*)aligned_alloc(align, WORD_LEN * size);
    memset(words, 0, WORD_LEN * size);
    char* p = buffer;

    for(int i =0; *p; i++){
        char* start = p;
        while(*p && *p != '\n') p++;

        int len = p - start;
        memcpy(words + i * WORD_LEN, start, len);

        while(*p && isspace(*p)) p++;
    }

    return words;
}

hash_table* prepare_hashtable(char* words){
    assert(words);

    hash_table* ht = hash_table_ctor(13);
    for (int j = 0; j < NUM_OF_WORDS; j++){
        hash_table_insert(words + j * WORD_LEN, ht);
    }

    return ht;
}

