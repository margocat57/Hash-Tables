#include <cstdint>
#include <cstring>
#include <cstdio>
#include <assert.h>

uint32_t hash_length(const char* s);

uint32_t hash_first_letter(const char* s);

uint32_t hash_sum_letters(const char* s);

uint32_t hash_polynomial(const char* s);

uint32_t hash_crc32(const char* s);

uint32_t hash_jenkins_one_at_a_time32(const char* s);

uint32_t elf_hash(const char* s);

uint32_t fnv1a_hash(const char* s);

uint32_t murmur3_hash(const char* s);
