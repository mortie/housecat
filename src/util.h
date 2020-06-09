#ifndef H_UTIL_H
#define H_UTIL_H

#include <stdio.h>

char* h_util_str_join(const char* s1, const char* s2);
char* h_util_path_join(const char* p1, const char* p2);
char* h_util_file_read(const char* path);
int h_util_file_err(const char* path);
void h_util_file_copy(FILE* f1, FILE* f2);
void h_util_cp_dir_to_file(const char* dirpath, FILE* file);
void h_util_cp_dir_to_file_se(const char* dirpath, FILE* file, const char* start, const char* end);
void h_util_cp_dir(const char* dir1, const char* dir2);
void h_util_cp_dir_se(const char* dir1, const char* dir2, const char* start, const char* end);

#define h_util_streq(x, y) (strcmp(x, y) == 0)

#endif
