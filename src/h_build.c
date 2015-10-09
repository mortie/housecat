#include "h_build.h"
#include "h_file.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

static char* path_join(char* p1, char* p2)
{
	if (p1 == NULL || p2 == NULL)
		return NULL;

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

h_err* h_build_post(h_post* post, FILE* f)
{
	fprintf(f, "hey am doing things with %s\n", post->title);

	return NULL;
}

h_err* h_build_section(h_section* section, FILE* f)
{
	int i;
	for (i = 0; i < section->numposts; ++i)
	{
		h_post* post = section->posts[i];
		h_build_post(post, f);
	}

	return NULL;
}

h_err* h_build(h_section* section, char* dirpath)
{
	//Create section's directory
	char* path = path_join(dirpath, section->slug);
	if (path == NULL)
		return h_err_create(H_ERR_ALLOC, NULL);
	if (mkdir(path, 0777) == -1 && errno != EEXIST)
		return h_err_from_errno(errno, path);

	//Create path for index.html file
	char* fpath = path_join(path, FILE_INDEX);
	if (fpath == NULL)
		return h_err_create(H_ERR_ALLOC, NULL);

	//Build section's index.html file
	FILE* f = fopen(fpath, "w");
	if (f == NULL)
		return h_err_from_errno(errno, path);

	h_build_section(section, f);
	fclose(f);

	//Build section's individual posts' files
	int i;
	for (i = 0; i < section->numposts; ++i)
	{
		h_post* post = section->posts[i];

		//Create post's directory
		char* dpath = path_join(path, post->slug);
		if (dpath == NULL)
			return h_err_create(H_ERR_ALLOC, NULL);
		if (mkdir(dpath, 0777) == -1 && errno != EEXIST)
			return h_err_from_errno(errno, dpath);

		//Create path for index.html file
		char* fpath = path_join(dpath, FILE_INDEX);
		if (fpath == NULL)
			return h_err_create(H_ERR_ALLOC, NULL);

		//Build path's index.html file
		FILE* f = fopen(fpath, "w");
		if (f == NULL)
			return h_err_from_errno(errno, path);

		h_build_post(post, f);
		fclose(f);

		free(dpath);
		free(fpath);
	}

	//Recurse through subs
	for (i = 0; i < section->numsubs; ++i)
	{
		h_section* sub = section->subs[i];

		//Build sub
		h_build(sub, path);
	}

	free(path);
	free(fpath);

	return NULL;
}
