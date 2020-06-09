#include "build.h"
#include "file.h"
#include "template.h"
#include "util.h"
#include "rss.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

static h_err* build_post(
		h_section* root,
		h_section* current,
		char* rootdir,
		h_post* post,
		h_build_strs strs,
		h_conf* conf)
{
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

	if (conf->rss && !(post->isdraft && !conf->rss_drafts))
	{
		err = h_rss_init_item(post, conf);
		if (err)
			return err;
	}

	free(dpath);

	return NULL;
}

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

	//Recurse through sub sections
	for (i = 0; i < current->numsubs; ++i)
	{
		h_err* err = build_node(root, current->subs[i], rootdir, strs, conf);
		if (err)
			return err;
	}

	//Go through individual posts and build them
	for (i = 0; i < current->numposts; ++i)
	{
		h_post* post = current->posts[i];
		h_err* err = build_post(root, current, rootdir, post, strs, conf);
		if (err)
			return err;
	}

	//Go through drafts and build them
	for (i = 0; i < current->numdrafts; ++i)
	{
		h_post* post = current->drafts[i];
		h_err* err = build_post(root, current, rootdir, post, strs, conf);
		if (err)
			return err;
	}

	char* indexpath = h_util_path_join(dirpath, H_FILE_INDEX);
	FILE* file = fopen(indexpath, "w");
	free(indexpath);

	h_err* err = h_build_section(root, current, file, 0, strs, conf);
	if (err)
		return err;

	fclose(file);

	int ppp = conf->posts_per_page;
	for (i = 0; i < (current->numposts / ppp) + 1; ++i)
	{
		char n[16];
		snprintf(n, 16, "%d", i);
		char* dpath = h_util_path_join(dirpath, n);
		char* indexpath = h_util_path_join(dpath, H_FILE_INDEX);

		if (mkdir(dpath, 0777) == -1 && errno != EEXIST)
			return h_err_from_errno(errno, dpath);

		FILE* file = fopen(indexpath, "w");
		free(indexpath);

		h_err* err = h_build_section(root, current, file, i, strs, conf);
		if (err)
			return err;

		fclose(file);
		free(dpath);
	}

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

		h_err* err = h_build_section(root, root->subs[0], file, 0, strs, conf);
		if (err)
			return err;

		fclose(file);
	}

	return NULL;
}
