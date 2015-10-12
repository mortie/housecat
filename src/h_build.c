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

char* h_build_post(h_post* post, struct h_build_strs strs)
{
	h_err* err;

	//Create arguments for the template
	h_template_args* args = h_template_args_create();
	err = h_template_args_append(args, "title", post->title);
	if (err) return NULL;
	err = h_template_args_append(args, "html", post->html);
	if (err) return NULL;

	//Execute the template
	char* str = h_templateify(strs.post, args);

	free(args);

	return str;
}

char* h_build_section(h_section* section, struct h_build_strs strs)
{
	h_err* err;
	char* posts_str = NULL;

	//Assemble posts
	int i;
	for (i = 0; i < section->numposts; ++i)
	{
		h_post* post = section->posts[i];
		char* str = h_build_post(post, strs);

		char* s = h_util_str_join(posts_str, str);
		if (s == NULL)
			return NULL;

		free(posts_str);
		posts_str = s;
	}

	//Create arguments for the template
	h_template_args* args = h_template_args_create();
	err = h_template_args_append(args, "posts", posts_str);
	if (err) return NULL;

	char* str = h_templateify(strs.section, args);

	free(args);
	free(posts_str);

	return str;
}

h_err* h_build(h_section* section, char* dirpath, struct h_build_strs strs)
{
	char* str;

	//Create section's directory
	char* path = h_util_path_join(dirpath, section->slug);
	if (path == NULL)
		return h_err_create(H_ERR_ALLOC, NULL);
	if (mkdir(path, 0777) == -1 && errno != EEXIST)
		return h_err_from_errno(errno, path);

	//Create path for index.html file
	char* fpath = h_util_path_join(path, H_FILE_INDEX);
	if (fpath == NULL)
		return h_err_create(H_ERR_ALLOC, NULL);

	//Build section's index.html file

	str = h_build_section(section, strs);
	if (str == NULL)
		return h_err_create(H_ERR_ALLOC, NULL);

	FILE* f = fopen(fpath, "w");
	if (f == NULL)
		return h_err_from_errno(errno, path);

	fputs(str, f);
	free(str);
	fclose(f);

	//Build section's individual posts' files
	int i;
	for (i = 0; i < section->numposts; ++i)
	{
		h_post* post = section->posts[i];

		//Create post's directory
		char* dpath = h_util_path_join(path, post->slug);
		if (dpath == NULL)
			return h_err_create(H_ERR_ALLOC, NULL);
		if (mkdir(dpath, 0777) == -1 && errno != EEXIST)
			return h_err_from_errno(errno, dpath);

		//Create path for index.html file
		char* fpath = h_util_path_join(dpath, H_FILE_INDEX);
		if (fpath == NULL)
			return h_err_create(H_ERR_ALLOC, NULL);

		//Build path's index.html file

		str = h_build_post(post, strs);
		if (str == NULL)
			return h_err_create(H_ERR_ALLOC, NULL);

		FILE* f = fopen(fpath, "w");
		if (f == NULL)
			return h_err_from_errno(errno, path);

		fputs(str, f);
		free(str);
		fclose(f);

		free(dpath);
		free(fpath);
	}

	//Recurse through subs
	for (i = 0; i < section->numsubs; ++i)
	{
		h_section* sub = section->subs[i];

		//Build sub
		h_build(sub, path, strs);
	}

	free(path);
	free(fpath);

	return NULL;
}
