#ifndef H_UTIL_H
#define H_UTIL_H

#include <stdio.h>

char* h_util_str_join(char* s1, char* s2);
char* h_util_path_join(char* p1, char* p2);
char* h_util_file_read(char* path);
int h_util_file_err(char* path);
void h_util_file_copy(FILE* f1, FILE* f2);
void h_util_cp_dir_to_file(char* dirpath, FILE* file);
void h_util_cp_dir_to_file_se(char* dirpath, FILE* file, char* start, char* end);

#endif
