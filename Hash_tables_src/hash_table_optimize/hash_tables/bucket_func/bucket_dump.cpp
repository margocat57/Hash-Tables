#include "bucket_func.h"
#include "bucket_dump.h"
#include <sys/wait.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdarg.h>

#define CHECK_AND_RET_DUMP(bad_condition, msg)\
    if(bad_condition){ \
        fprintf(stderr, msg); \
        return; \
    } \

static filenames_for_dump filename_ctor();

static int check_and_dump_system_work(int sys_res);

static void generate_dot_file(const bucket_t* list, const char* dot_filename);

static void generate_svg_file(const filenames_for_dump* dump);

static void draw_table(const bucket_t* list, FILE* html_output);

static void list_dump_html(const bucket_t* list, const char* img, const char* debug_msg, const char *file, const char *func, int line, va_list args)  __attribute__ ((format (printf, 3, 0)));

void list_dump_func(const bucket_t* list, const char* debug_msg, const char *file, const char *func,  int line, ...){
    filenames_for_dump dump = filename_ctor();
    generate_dot_file(list, dump.dot_filename);
    generate_svg_file(&dump);
    if(dump.svg_filename){
        va_list args = {};
        va_start(args, line);
        list_dump_html(list, dump.svg_filename, debug_msg, file, func, line, args);
        free(dump.svg_filename);
        va_end(args);
    }
    if(dump.dot_filename){
        free(dump.dot_filename);
    }
}

static void list_dump_html(const bucket_t* list, const char* img, const char* debug_msg, const char *file, const char *func, int line, va_list args){
    static int launch_num = 0;
    FILE* html_output = NULL;
    if(launch_num == 0){
        html_output = fopen(LOG_FILE_DUMP, "w");
        CHECK_AND_RET_DUMP(!html_output, "Can't open html file\n");
        launch_num++;
        fprintf(html_output, "<pre style=\"background-color: #FFFAFA; color: #000000;\">");
        fprintf(html_output, "<p style=\"font-size: 50px; text-align: center;\"> LIST DUMP\n");
        fprintf(html_output, "<p style=\"font-size: 30px; \"> Legend:\n");
        fprintf(html_output, "<p style=\"font-size: 25px; \"> <span style=\"color: #82898f;\">Grey</span> - free places(now filled poison elements)\n");
        fprintf(html_output, "<p style=\"font-size: 25px; \"> <span style=\"color: #CD5C5C;\">Red</span> - zero element\n");
        fprintf(html_output, "<p style=\"font-size: 25px; \"> <span style=\"color: #98FB98;\">Green</span> - Ocuppied elememt\n");
        fprintf(html_output, "<p style=\"font-size: 25px; \"> <span style=\"color: #bb0d12;\">Red octagon</span> - index of elemet out of array\n");
        fprintf(html_output, "<p style=\"font-size: 25px; \"> <span style=\"color: #0000FF; font-weight: bold;\">Blue bold</span> or <span style=\"color: #FF4F00; font-weight: bold;\">orange bold</span> - for incorrect connections\n");
    }
    else{
        html_output = fopen(LOG_FILE_DUMP, "a+");
        CHECK_AND_RET_DUMP(!html_output, "Can't open html file\n");
    }
    fprintf(html_output, "<p style=\"font-size: 20px; \">Dump was called at %s function %s line %d\n", file, func, line);
    fprintf(html_output, "<p style=\"font-size: 17.5px; color: #bb0d12;\">");
    vfprintf(html_output, debug_msg, args);
    fprintf(html_output, "\n");

    draw_table(list, html_output);

    fprintf(html_output, "<img src=\"%s\" alt=\"Array visualization\" width=\"95%%\">\n", img);
    fprintf(html_output, "\n");
    fclose(html_output);
}

static void draw_table(const bucket_t* list, FILE* html_output){
    fprintf(html_output, "<table style=\"text-align: center; font-size: 15px; border: 2px solid #000000; border-collapse: collapse; width: 50%%; color: #FFFFFF;\">\n");

    fprintf(html_output, "<tr>\n");
    fprintf(html_output, "<td style = \"color: #000000;\">Index</td>");
    for (int i = 0; i < list->capacity; i++) {
        fprintf(html_output, "<td style = \"border: 1px solid #000000; color: #000000;\";>%d</td>\n", i);
    }
    fprintf(html_output, "</tr>\n");

    fprintf(html_output, "<tr>\n");
    fprintf(html_output, "<td style = \"color: #000000;\">Data</td>");
    for (int i = 0; i < list->capacity; i++) {
        fprintf(html_output, "<td style = \"border: 1px solid #000000; color: #000000;\";>%s</td>\n", list->keys + i * SIZE_WORD);
    }
    fprintf(html_output, "</tr>\n");

    fprintf(html_output, "<tr>\n");
    fprintf(html_output, "<td style = \"color: #000000;\">Next</td>");
    for (int i = 0; i < list->capacity; i++) {
        fprintf(html_output, "<td style = \"border: 1px solid #000000; color: #000000;\";>%d</td>\n", list->next[i]);
    }
    fprintf(html_output, "</tr>\n");

    fprintf(html_output, "<tr>\n");
    fprintf(html_output, "<td style = \"color: #000000;\">Prev</td>");
    for (int i = 0; i < list->capacity; i++) {
        fprintf(html_output, "<td style = \"border: 1px solid #000000; color: #000000;\";>%d</td>\n", list->prev[i]);
    }
    fprintf(html_output, "</tr>\n");

    fprintf(html_output, "</table>\n"); 
}

static filenames_for_dump filename_ctor(){
    filenames_for_dump dump = {};
    static int num = 0;
    num++;

    char* dot_filename = (char*)calloc(100, sizeof(char));
    if(!dot_filename){
        fprintf(stderr, "Allocation error for string with .dot filename");
        return dump;
    }

    char* svg_filename = (char*)calloc(100, sizeof(char));
    if(!svg_filename){
        fprintf(stderr, "Allocation error for string with .svg filename");
        return dump;
    }

    dump.svg_filename = svg_filename;
    dump.dot_filename = dot_filename;

    time_t now = time(NULL); 
    struct tm *t = localtime(&now); 

    if (snprintf(dump.dot_filename, 100, 
                "images/dump%d_%04d%02d%02d_%02d%02d%02d.dot", num,
                t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                t->tm_hour, t->tm_min, t->tm_sec) == -1) {
        fprintf(stderr, "Can't generate dot filename\n");
        free(dump.svg_filename);
        return {};
    }
    
    if (snprintf(dump.svg_filename, 100, 
                "images/dump%d_%04d%02d%02d_%02d%02d%02d.svg", num,
                t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                t->tm_hour, t->tm_min, t->tm_sec) == -1) {
        fprintf(stderr, "Can't generate svg filename\n");
        free(dump.dot_filename);
        return {};
    }
    return dump;
}

static void generate_dot_file(const bucket_t* list, const char* dot_filename){
    if(!dot_filename){
        fprintf(stderr, "NULL dot_filename pointer - can't work\n");
        return;
    }

    FILE* dot_file = fopen(dot_filename, "w");
    if(!dot_file){
        fprintf(stderr, "Can't open dot file\n");
        return;
    }

    fprintf(dot_file, "digraph G{\n");
    fprintf(dot_file, " rankdir=LR;\n");
    // fprintf(dot_file, " splines=ortho;\n");
    fprintf(dot_file, " graph [bgcolor=\"#FFFAFA\", nodesep = 0.3, ranksep=0.8];\n");

    for(int idx = 0; idx < list->capacity; idx++){

        if(list->prev[idx]==-1){
            fprintf(dot_file, " %d[shape=\"Mrecord\", style=\"filled\", fillcolor=\"#82898F\", color = \"#000000\", penwidth=1.0, label=\"phys idx = %d | elem = %s(NONE) | {prev = %d | next = %d} \"];\n", idx ,idx, list->keys + idx*SIZE_WORD, list->prev[idx], list->next[idx]);
            continue;
        }
        else if((list->prev[idx] >= list->capacity || (list->prev[idx] < 0 && list->prev[idx]!=-1)) || (list->next[idx] >= list->capacity || list->next[idx] < 0 )){
            if(list->prev[idx] >= list->capacity || list->prev[idx] < 0 ){
                fprintf(dot_file, " %d[shape=\"octagon\", style=\"filled\", fillcolor=\"#bb0d12\", color = \"#000000\", penwidth=2.0, label=\"%d\"];\n", list->prev[idx] ,list->prev[idx]);
            }
            if((list->next[idx] >= list->capacity || list->next[idx] < 0 )){
                fprintf(dot_file, " %d[shape=\"octagon\", style=\"filled\", fillcolor=\"#bb0d12\", color = \"#000000\", penwidth=2.0, label=\"%d\"];\n", list->next[idx], list->next[idx]);
            }
        }
        fprintf(dot_file, " %d[shape=\"Mrecord\", style=\"filled\", fillcolor=\"#98FB98\", color = \"#000000\", penwidth=1.0, label=\"phys idx = %d | elem = %s | {prev = %d | next = %d}\"];\n", idx ,idx, list->keys + idx*SIZE_WORD, list->prev[idx], list->next[idx]);
    }
    fprintf(dot_file, " -1[shape=\"Mrecord\", style=\"filled\", fillcolor=\"#bb0d12\", color = \"#000000\", penwidth=1.0, label=\"phys idx = -1 | elem = NONE | {prev = -1 | next = -1}\"];\n");
    
    fprintf(dot_file, " head_label [shape=\"Mrecord\", style=\"filled\", fillcolor=\"#20B2AA\", color=\"#000000\", fontcolor=\"#000000\", fontsize=12, label=\"HEAD\"]\n");
    fprintf(dot_file, " tail_label [shape=\"Mrecord\", style=\"filled\", fillcolor=\"#20B2AA\", color=\"#000000\", fontcolor=\"#000000\", fontsize=12, label=\"TAIL\"]\n");
    fprintf(dot_file, " free_label [shape=\"Mrecord\", style=\"filled\", fillcolor=\"#20B2AA\", color=\"#000000\", fontcolor=\"#000000\", fontsize=12, label=\"FREE\"]\n");

    // fprintf(dot_file, "{rank=same; %zd; head_label;}\n", list->next[0]);
    // fprintf(dot_file, "{rank=same; %zd; tail_label;}\n", list->prev[0]);
    // fprintf(dot_file, "{rank=same; %zd; free_label;}\n", list->free);

    fprintf(dot_file, "head_label -> tail_label -> free_label[style=\"invis\", weight=500]");

    fprintf(dot_file, "head_label -> %d [color=\"#20B2AA\", penwidth=1, arrowsize=0.85, style=\"solid\"]\n", list->list_head);
    fprintf(dot_file, "tail_label -> %d [color=\"#20B2AA\", penwidth=1, arrowsize=0.85, style=\"solid\"]\n", list->prev[list->list_head]);
    fprintf(dot_file, "free_label -> %d [color=\"#20B2AA\", penwidth=1, arrowsize=0.85, style=\"solid\"]\n", list->first_free);
    

    // connect elements (invis)
    int last_norm_idx = list->list_head;
    for(int idx = 1; idx < list->capacity; idx++){
        if(last_norm_idx == list->list_head){
            fprintf(dot_file," %d -> %d [style=\"invis\", weight=10000]\n", last_norm_idx, idx);
            last_norm_idx = idx;
            continue;
        }
        fprintf(dot_file," %d -> %d [style=\"invis\", weight=10000]\n", last_norm_idx, idx);
        last_norm_idx = idx;
    }
    fprintf(dot_file," %d -> %d [style=\"invis\", weight=10000]\n", last_norm_idx, -1);

    for(int idx = 0; idx < list->capacity; idx++){
        if(list->next[idx] == -1 && idx == list->capacity - 1){
            fprintf(dot_file," %d -> %d [color = \"#FF4F00\", penwidth = 1, arrowsize = 0.85, constraint=false, weight=0]\n", idx, list->next[idx]);
            continue;
        }

        if((list->prev[idx] >= list->capacity || (list->prev[idx] < 0 && list->prev[idx]!=-1)) || (list->next[idx] >= list->capacity || list->next[idx] < 0 )){
            if((list->prev[idx] >= list->capacity || (list->prev[idx] < 0 && list->prev[idx]!=-1)) && (list->next[idx] >= list->capacity || list->next[idx] < 0 )){
                fprintf(dot_file, " %d -> %d [color = \"#FF4F00\", penwidth = 2, arrowsize = 1, constraint=false, weight=0]\n",  list->prev[idx], idx);
                fprintf(dot_file," %d -> %d [color = \"#0000FF\", penwidth = 2, arrowsize = 1, constraint=false, weight=0]\n", idx, list->next[idx]);
                continue;
            }
            if(list->prev[idx] >= list->capacity || (list->prev[idx] < 0 && list->prev[idx]!=-1)){
                fprintf(dot_file, " %d -> %d [color = \"#FF4F00\", penwidth = 2, arrowsize = 1, constraint=false, weight=0]\n", list->prev[idx], idx);
                fprintf(dot_file," %d -> %d [color = \"#0000FF\", penwidth = 2, arrowsize = 1, constraint=false, weight=0]\n", idx, list->next[idx]);
            }
            if(list->next[idx] >= list->capacity || (list->next[idx] < 0 && list->next[idx] != -1)){
                fprintf(dot_file, " %d -> %d [color = \"#FF4F00\", penwidth = 2, arrowsize = 1, constraint=false, weight=0]\n",idx, list->next[idx]);
                fprintf(dot_file," %d -> %d [color = \"#0000FF\", penwidth = 2, arrowsize = 1, constraint=false, weight=0]\n", list->prev[idx], idx);
            }
            continue;
        }

        
        if((list->keys + idx * SIZE_WORD)[0] != '\0' && list->prev[idx] != -1 && list->next[list->prev[idx]] != idx){
            if(list->prev[idx] < idx){
                fprintf(dot_file," %d -> %d [color = \"#FF4F00\", penwidth = 2, arrowsize = 1, dir=back, constraint=false, weight=0]\n", list->prev[idx], idx);
                continue;
            }
            fprintf(dot_file," %d -> %d [color = \"#FF4F00\", penwidth = 2, arrowsize = 1, constraint=false, weight=0]\n", list->prev[idx], idx);
        }
        

        if((list->keys + idx * SIZE_WORD)[0] != '\0' && list->prev[idx] != -1 && list->prev[list->next[idx]] != idx){
            fprintf(dot_file," %d -> %d [color = \"#0000FF\", penwidth = 2, arrowsize = 1, constraint=false, weight=0]\n", idx, list->next[idx]);
            continue;
        }

        if((list->keys + idx * SIZE_WORD)[0] == '\0' && list->prev[idx] == -1 && idx != list->list_head){
            if((list->next[idx] > 0 && list->next[idx] < list->capacity) && ((list->prev[list->next[idx]] >= list->capacity || (list->prev[list->next[idx]] < 0 && list->prev[list->next[idx]]!=-1)) || (list->next[list->next[idx]] >= list->capacity))){
                continue;
            }
            fprintf(dot_file," %d -> %d [color = \"#FF8C00\", penwidth = 1, arrowsize = 0.85, weight=0]\n", idx, list->next[idx]);
        }
    
        if(((list->keys + idx * SIZE_WORD)[0] != '\0' && list->prev[idx] != -1) || idx == list->list_head){
            if((list->next[idx] > 0 && list->next[idx] < list->capacity) && ((list->prev[list->next[idx]] >= list->capacity|| (list->prev[list->next[idx]] < 0 && list->prev[list->next[idx]]!=-1)) || (list->next[list->next[idx]] >= list->capacity || list->next[list->next[idx]] < 0 ))){
                continue;
            }
            if(list->size == 1 && idx != list->list_head){
                continue;
            }
            fprintf(dot_file," %d -> %d [color = \"#000000\", penwidth = 1, arrowsize = 0.85, dir=both, weight=0]\n", idx, list->next[idx]);
        }
    }

    fprintf(dot_file,"}\n");
    fclose(dot_file);
}

static void generate_svg_file(const filenames_for_dump* dump){
    char command[200] = {0};

    if(snprintf(command, sizeof(command) ,"dot -Tsvg %s -o %s", dump->dot_filename, dump->svg_filename) == -1){
        fprintf(stderr, "Can't parse command to do\n");
        return;
    }
    int sys_res = system(command);
    check_and_dump_system_work(sys_res);
}

static int check_and_dump_system_work(int sys_res){
    if(sys_res == 0){
        return 0;
    }
    if(sys_res == -1){
        fprintf(stderr, "Cannot start to do command\n");
        return -1;
    }
    if(WIFEXITED(sys_res)){
        int exit_stat = WEXITSTATUS(sys_res);
        fprintf(stderr, "Creating svg file finished with mistake %d\n", exit_stat);
        return exit_stat;
    }
    return 0;
}

// list->prev[list->next[data_idx]] != data_idx
// list->next[list->prev[data_idx]] != data_idx