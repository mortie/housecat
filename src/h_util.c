#include "h_util.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

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
	int len = len1 + len2 + 2;
	char* path = malloc(len);

	memcpy(path, p1, len1);
	memcpy(path + len1, "/", 1);
	memcpy(path + len1 + 1, p2, len2);

	path[len - 1] = '\0';
	return path;
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
	long len = ftell(f);
	fseek(f, 0L, SEEK_SET);

	char* str = malloc(len);

	char c;
	int i = 0;
	while((c = getc(f)) != EOF)
	{
		str[i++] = c;
	}

	fclose(f);

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
