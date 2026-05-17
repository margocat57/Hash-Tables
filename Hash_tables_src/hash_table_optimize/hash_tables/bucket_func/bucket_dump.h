#ifndef LIST_DUMP_H
#define LIST_DUMP_H
#include <string.h>
#include <stdio.h>
#include "bucket_func.h"

struct filenames_for_dump{
    char* dot_filename;
    char* svg_filename;
};

const char* const LOG_FILE_DUMP = "log.htm";

void list_dump_func(const bucket_t* bucket, const char* debug_msg, const char *file, const char *func, int line, ...) __attribute__ ((format (printf, 2, 6)));
#endif // LIST_DUMP_H