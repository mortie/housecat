#include "h_section.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>

static char* path_join(char* p1, char* p2)
{
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

void h_section_init(h_section* section, char* title, char* slug)
{
	section->title = title;
	section->slug = slug;
	section->posts = NULL;
	section->numposts = 0;
	section->subs = NULL;
	section->numsubs = 0;
}

h_err* h_section_init_from_dir(h_section* section, char* path)
{
	section->posts = NULL;
	section->numposts = 0;
	section->subs = NULL;
	section->numsubs = 0;

	struct dirent** namelist;
	int n;

	n = scandir(path, &namelist, 0, alphasort);
	if (n == -1)
		return h_err_from_errno(errno, path);

	//Loop through directory, adding posts and sub sections
	while (n--)
	{
		struct dirent* ent = namelist[n];

		//Skip hidden entries
		if (ent->d_name[0] == '.')
			continue;

		//If entry is a file, add it to the posts array
		if (ent->d_type == DT_REG)
		{
			h_post* post = malloc(sizeof(h_post));
			if (post == NULL)
				return h_err_create(H_ERR_ALLOC, NULL);

			char* p = path_join(path, ent->d_name);
			if (p == NULL)
				return h_err_create(H_ERR_ALLOC, NULL);

			h_err* err = NULL;
			err = h_post_init_from_file(post, p);
			free(p);
			if (err)
				return err;

			err = h_section_add_post(section, post);
			if (err)
				return err;
		}

		//If it's a directory, it's a section and should be added to subs
		else if (ent->d_type == DT_DIR)
		{
			h_section* sub = malloc(sizeof(h_section));
			if (sub == NULL)
				return h_err_create(H_ERR_ALLOC, NULL);

			char* p = path_join(path, ent->d_name);
			if (p == NULL)
				return h_err_create(H_ERR_ALLOC, NULL);

			h_err* err = NULL;
			err = h_section_init_from_dir(sub, p);
			if (err)
				return err;

			err = h_section_add_sub(section, sub);
			free(p);
			if (err)
				return err;
		}

		free(namelist[n]);
	}

	free(namelist);

	//Get a section's slug from the path

	int lastslash = 0;
	int length = strlen(path);
	int i;
	for  (i = 0; i < length; ++i)
	{
		if (path[i] == '/')
			lastslash = i;
	}

	int chars = length - lastslash - 6;
	section->slug = malloc(chars * sizeof(char));
	if (section->slug == NULL)
		return h_err_create(H_ERR_ALLOC, NULL);

	memcpy(section->slug, path + lastslash + 1 + 6, chars * sizeof(char));
	section->slug[chars - 1] = '\0';

	//Make a title from the slug

	length = strlen(section->slug) + 1;
	section->title = malloc(length * sizeof(char));
	memcpy(section->title, section->slug, (length - 1) * sizeof(char));
	for (i = 0; i < length - 1; ++i)
	{
		char c = section->title[i];

		if (c == '-' || c == '_')
			section->title[i] = ' ';
	}

	section->title[length - 1] = '\0';

	return NULL;
}

h_err* h_section_add_post(h_section* section, h_post* post)
{
	section->numposts += 1;
	section->posts = realloc(section->posts, section->numposts * sizeof(h_post));
	if (section->posts == NULL)
		return h_err_create(H_ERR_ALLOC, NULL);

	section->posts[section->numposts - 1] = post;

	return NULL;
}

h_err* h_section_add_sub(h_section* section, h_section* sub)
{
	section->numsubs += 1;
	section->subs = realloc(section->subs, section->numsubs * sizeof(h_section));
	if (section->subs == NULL)
		return h_err_create(H_ERR_ALLOC, NULL);

	section->subs[section->numsubs - 1] = sub;

	return NULL;
}
