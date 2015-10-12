#include "h_err.h"
#include "h_section.h"
#include "h_post.h"
#include "h_build.h"
#include "h_file.h"
#include "h_util.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

h_err* init(char* path)
{
	printf("Init isn't implemented yet. %s\n", path);
	return NULL;
}

h_err* build(char* path)
{
	h_err* err;

	char* inpath = h_util_path_join(path, H_FILE_INPUT);
	char* outpath = h_util_path_join(path, H_FILE_OUTPUT);
	if (inpath == NULL || outpath == NULL)
		return h_err_create(H_ERR_ALLOC, NULL);

	h_section* root = malloc(sizeof(h_section));
	err = h_section_init_from_dir(root, inpath);
	if (err)
		return err;

	if (mkdir(inpath, 0777) == -1 && errno != EEXIST)
		return h_err_create(errno, inpath);

	struct h_build_strs strs =
	{
		/* index */ "<!DOCTYPE html><html><body>{{page}}</body></html>",
		/* section */ "<div class='posts'>{{posts}}</div>",
		/* post */ "<div class='post'><h1>{{title}}</h1><div class='content'>{{html}}</div></div>"
	};

	err = h_build(root, outpath, strs);
	if (err)
		return err;

	return NULL;
}

int main(int argc, char** argv)
{
#define usage() fprintf(stderr, "Usage: %s <build|init> <directory>\n", argv[0])

	if (argc != 3) {
		usage();
		return 1;
	}

	if (strcmp(argv[1], "build") == 0)
	{
		h_err* err = build(argv[2]);
		if (err)
			h_err_print(err);
	}
	else if (strcmp(argv[1], "init") == 0)
	{
		h_err* err = init(argv[2]);
		if (err)
			h_err_print(err);
	}
	else
	{
		usage();
		return 1;
	}

	return 0;
}
