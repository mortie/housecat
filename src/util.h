#ifndef H_UTIL_H
#define H_UTIL_H

#include <stdio.h>

char* h_util_str_join(char* s1, char* s2);
char* h_util_path_join(const char* p1, const char* p2);
char* h_util_file_read(const char* path);
int h_util_file_err(char* path);
void h_util_file_copy(FILE* f1, FILE* f2);
void h_util_cp_dir_to_file(char* dirpath, FILE* file);
void h_util_cp_dir_to_file_se(char* dirpath, FILE* file, char* start, char* end);
void h_util_cp_dir(char* dir1, char* dir2);
void h_util_cp_dir_se(char* dir1, char* dir2, char* start, char* end);

#define h_util_streq(x, y) (strcmp(x, y) == 0)

#endif
