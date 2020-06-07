#include "rss.h"
#include "section.h"
#include "util.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>

static void reverse_subs(h_section* section)
{
	h_section** newsubs = malloc(section->numsubs * sizeof(h_section**));

	int i;
	for (i = 0; i < section->numsubs; ++i)
	{
		newsubs[section->numsubs - i - 1] = section->subs[i];
	}

	free(section->subs);
	section->subs = newsubs;
}

static h_err* init_from_dir(h_section* section, char* path, char* spath, int depth)
{
	section->posts = NULL;
	section->numposts = 0;
	section->drafts = NULL;
	section->numdrafts = 0;
	section->subs = NULL;
	section->numsubs = 0;
	section->rpath = malloc(strlen(path) + 1);
	strcpy(section->rpath, path);

	struct dirent** namelist;
	int n;

	n = scandir(path, &namelist, 0, alphasort);
	if (n == -1)
		return h_err_from_errno(errno, path);

	//Loop through directory, adding posts and sub sections
	while (n--)
	{
		struct dirent* ent = namelist[n];

		//Skip hidden entries,
		//or entries starting with _
		if (ent->d_name[0] == '.')
			continue;
		if (ent->d_name[0] == '_')
			continue;

		//If entry is a file, add it to either posts or drafts
		if (ent->d_type == DT_REG)
		{
			if (strcmp(ent->d_name, "rss.conf") == 0)
				continue; // Don't read rss configu

			h_post* post = malloc(sizeof(h_post));
			if (post == NULL)
				return h_err_create(H_ERR_ALLOC, NULL);

			char* p = h_util_path_join(path, ent->d_name);
			if (p == NULL)
				return h_err_create(H_ERR_ALLOC, NULL);

			h_err* err = NULL;
			err = h_post_init_from_file(post, p, spath, depth);
			if (err)
				return err;
			free(p);

			if (post->isdraft)
				err = h_section_add_draft(section, post);
			else
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

			char* p = h_util_path_join(path, ent->d_name);
			if (p == NULL)
				return h_err_create(H_ERR_ALLOC, NULL);

			char* sp = h_util_path_join(spath, ent->d_name + 6);
			if (sp == NULL)
				return h_err_create(H_ERR_ALLOC, NULL);
			if (sp[0] == '/')
				sp = sp + 1;

			h_err* err = NULL;
			err = init_from_dir(sub, p, sp, depth + 1);
			if (err)
				return err;

			err = h_section_add_sub(section, sub);
			free(p);
			if (err)
				return err;
		}

		free(namelist[n]);
	}

	//Both posts and subs are reversed now, as posts should be reversed;
	//we have to reverse the subs again though, as they shouldn't be reversed.
	reverse_subs(section);

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

	if (chars <= 0) {
		section->slug = NULL;
		section->title = NULL;
		section->path = spath;
		section->depth = depth;
		return NULL;
	}

	section->path = spath;
	section->depth = depth;

	section->slug = malloc(chars * sizeof(char));
	if (section->slug == NULL)
		return h_err_create(H_ERR_ALLOC, NULL);

	memcpy(section->slug, path + lastslash + 1 + 6, chars * sizeof(char));
	section->slug[chars - 1] = '\0';

	//Make a title from the slug

	length = chars;
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

h_err* h_section_init_from_dir(h_section* section, char* path)
{
	return init_from_dir(section, path, "", 0);
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

h_err* h_section_add_draft(h_section* section, h_post* post)
{
	section->numdrafts += 1;
	section->drafts = realloc(section->drafts, section->numdrafts * sizeof(h_post));
	if (section->drafts == NULL)
		return h_err_create(H_ERR_ALLOC, NULL);

	section->drafts[section->numdrafts - 1] = post;

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
