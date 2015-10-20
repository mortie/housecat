#include "h_build.h"
#include "h_file.h"
#include "h_template.h"
#include "h_util.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

static h_err* build_node(
		h_section* root,
		h_section* current,
		char* rootdir,
		h_build_strs strs,
		h_conf* conf)
{
	char* dirpath = h_util_path_join(rootdir, current->path);

	if (mkdir(dirpath, 0777) == -1 && errno != EEXIST)
		return h_err_from_errno(errno, dirpath);

	int i;
	for (i = 0; i < current->numsubs; ++i)
	{
		h_err* err = build_node(root, current->subs[i], rootdir, strs, conf);
		if (err)
			return err;
	}

	for (i = 0; i < current->numposts; ++i)
	{
		h_post* post = current->posts[i];
		char* dpath = h_util_path_join(rootdir, post->path);
		char* indexpath = h_util_path_join(dpath, H_FILE_INDEX);

		if (mkdir(dpath, 0777) == -1 && errno != EEXIST)
			return h_err_from_errno(errno, dpath);

		FILE* file = fopen(indexpath, "w");
		free(indexpath);

		h_err* err = h_build_post(root, current, post, file, strs, conf);
		if (err)
			return err;

		fclose(file);
		free(dpath);
	}

	char* indexpath = h_util_path_join(dirpath, H_FILE_INDEX);
	FILE* file = fopen(indexpath, "w");
	free(indexpath);

	h_err* err = h_build_section(root, current, file, strs, conf);
	if (err)
		return err;

	fclose(file);
	free(dirpath);

	return NULL;
}

h_err* h_build(
		h_section* root,
		char* rootdir,
		h_build_strs strs,
		h_conf* conf)
{
	//Build subs
	int i;
	for (i = 0; i < root->numsubs; ++i)
	{
		h_err* err = build_node(root, root->subs[i], rootdir, strs, conf);
		if (err)
			return err;
	}

	//Make / be the first section
	if (root->numsubs > 0)
	{
		char* indexpath = h_util_path_join(rootdir, H_FILE_INDEX);
		FILE* file = fopen(indexpath, "w");
		free(indexpath);

		h_err* err = h_build_section(root, root->subs[0], file, strs, conf);
		if (err)
			return err;

		fclose(file);
	}

	return NULL;
}
