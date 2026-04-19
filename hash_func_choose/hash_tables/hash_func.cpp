#include "hash_func.h"

uint32_t hash_length(const char* s){
    assert(s);
    return strlen(s);
}

uint32_t hash_first_letter(const char* s){
    assert(s);
    return s[0];
}

uint32_t hash_sum_letters(const char* s){
    assert(s);
    uint32_t  sum = 0;
    unsigned char* data= (unsigned char*)s;
    for(int i = 0; data[i] != '\0'; i++){
        sum += data[i];
    }
    return sum;
}

uint32_t hash_polynomial(const char* s){
    assert(s);
    uint32_t  poly = 0;
    uint32_t  p = 31;
    for(int i = 0; s[i] != '\0'; i++){
        poly = poly * p + s[i];
    }
    return poly;
}

uint32_t hash_crc32(const char* s){
    assert(s);
    const uint32_t POLY = 0xEDB88320;
    uint32_t crc = 0xFFFFFFFF;
    unsigned char* data= (unsigned char*)s;

    for (size_t i = 0; data[i] != '\0'; i++) {
        crc ^= data[i]; 

        for (int bit = 0; bit < 8; bit++) {
            if (crc & 1)
                crc = (crc >> 1) ^ POLY;
            else
                crc >>= 1;
        }
    }

    return crc ^ 0xFFFFFFFF; 
}


uint32_t hash_jenkins_one_at_a_time32(const char* s){
    assert(s);
    uint32_t hash = 0;
    unsigned char* data= (unsigned char*)s;

    for (size_t i = 0; data[i] != '\0'; i++) {
        hash += data[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}


// source https://en.wikipedia.org/wiki/PJW_hash_function
uint32_t elf_hash(const char* s){
    uint32_t h = 0;
    uint32_t high = 0;
    unsigned char* data= (unsigned char*)s;

    for (size_t i = 0; data[i] != '\0'; i++){
        h = (h << 4) + data[i];
        high = h & 0xF0000000;
        if(high != 0){
            h = h ^ (high >> 24);
            h = h & ~high;
        }
    }
    return h;
}

// source https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function

uint32_t fnv1a_hash(const char* s){
    uint32_t hash = 2166136261;
    unsigned char* data= (unsigned char*)s;

    for (size_t i = 0; data[i] != '\0'; i++){
        hash = hash ^ data[i];
        hash = hash * 16777619;
    }
    return hash;
}

// source https://en.wikipedia.org/wiki/MurmurHash

uint32_t murmur3_hash(const char* s){
    uint32_t c1 = 0xcc9e2d51;
    uint32_t c2 = 0x1b873593;
    uint32_t r1 = 15;
    uint32_t r2 = 13;
    uint32_t m = 5;
    uint32_t n = 0xe6546b64;

    uint32_t seed = 0x9747b28c;
    uint32_t hash = seed;

    uint32_t k = 0;
    size_t len = strlen(s);
    const size_t len_new = len;

    unsigned char* data= (unsigned char*)s;

    while(len >= 4){
        k = data[0];
        k |= data[1] << 8;
        k |= data[2] << 16;
        k |= data[3] << 24;

        k *= c1;
        k = (k << r1) | (k >> (32 - r1));
        k *= c2;

        hash = hash ^ k;
        hash = (hash << r2) | (hash >> (32 - r2));
        hash = (hash * m) + n;

        data += 4;
        len -= 4;
    }

    uint32_t rem_bytes = 0;
    switch (len){
        case 3: 
            rem_bytes ^= data[2] << 16;
        case 2: 
            rem_bytes ^= data[1] << 8;
        case 1: 
            rem_bytes ^= data[0];
            rem_bytes *= c1;
            rem_bytes = (rem_bytes << r1) | (rem_bytes >> (32 - r1));
            rem_bytes *= c2;
            hash ^= rem_bytes;
    }

    hash ^= len_new;
	hash ^= hash >> 16;
	hash *= 0x85ebca6b;
	hash ^= hash >> 13;
	hash *= 0xc2b2ae35;
	hash ^= hash >> 16;

    return hash;
}