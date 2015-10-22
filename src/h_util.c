#include "h_util.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>

char* h_util_path_join(char* p1, char* p2)
{
	if (p1 == NULL)
		return NULL;
	else if (p2 == NULL)
	{
		int len = strlen(p1) + 1;
		char* path = malloc(len);
		memcpy(path, p1, (len - 1) * sizeof(char));
		path[len - 1] = '\0';
		return path;
	}

	int len1 = strlen(p1);
	int len2 = strlen(p2);

	//Don't add '/' between the strings if it's already there
	if (p1[len1 - 1] == '/')
	{
		int len = len1 + len2 + 1;
		char* path = malloc(len);

		memcpy(path, p1, len1);
		memcpy(path + len1, p2, len2);

		path[len - 1] = '\0';
		return path;
	}

	//Add '/' between the strings if it's not there
	else
	{
		int len = len1 + len2 + 2;
		char* path = malloc(len);

		memcpy(path, p1, len1);
		memcpy(path + len1, "/", 1);
		memcpy(path + len1 + 1, p2, len2);

		path[len - 1] = '\0';
		return path;
	}
}

char* h_util_str_join(char* s1, char* s2)
{
	int len1, len2;
	if (s1 == NULL) len1 = 0;
	else len1 = strlen(s1);
	if (s2 == NULL) len2 = 0;
	else len2 = strlen(s2);

	int len = len1 + len2 + 1;
	char* str = malloc(len);

	memcpy(str, s1, len1);
	memcpy(str + len1, s2, len2);

	str[len - 1] = '\0';
	return str;
}

char* h_util_file_read(char* path)
{
	FILE* f = fopen(path, "r");

	if (f == NULL)
		return NULL;

	fseek(f, 0L, SEEK_END);
	long len = ftell(f) + 1;
	fseek(f, 0L, SEEK_SET);

	char* str = malloc(len);
	if (str == NULL)
		return NULL;

	fread(str, sizeof(char), len - 1, f);

	fclose(f);

	str[len - 1] = '\0';

	return str;
}

int h_util_file_err(char* path)
{
	FILE* f = fopen(path, "r");

	if (f == NULL)
		return errno;

	fclose(f);
	return 0;
}

void h_util_file_copy(FILE* f1, FILE* f2)
{
	fseek(f1, 0L, SEEK_END);
	long len = ftell(f1);
	fseek(f1, 0L, SEEK_SET);

	char* str = malloc(len + 1);
	fread(str, sizeof(char), len, f1);
	str[len] = '\0';

	fputs(str, f2);
}

void h_util_cp_dir_to_file(char* dirpath, FILE* file)
{
	h_util_cp_dir_to_file_se(dirpath, file, "", "");
}

void h_util_cp_dir_to_file_se(char* dirpath, FILE* file, char* start, char* end)
{
	struct dirent** namelist;
	int n = scandir(dirpath, &namelist, NULL, alphasort);
	if (n <= 0)
		return;

	int i;
	for (i = 0; i < n; ++i)
	{
		struct dirent* ent = namelist[i];
		if (ent->d_name[0] == '.')
			continue;

		char* path = h_util_path_join(dirpath, ent->d_name);
		FILE* f = fopen(path, "r");
		free(path);

		fputs(start, file);
		h_util_file_copy(f, file);
		fputs(end, file);

		fclose(f);
		free(ent);
	}
	free(namelist);
}
