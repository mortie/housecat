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
	section->rpath = malloc(strlen(path) + 1);
	if (section->rpath == NULL)
		return h_err_create(H_ERR_ALLOC, path);
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
		{
			free(ent);
			continue;
		}
		if (ent->d_name[0] == '_')
		{
			free(ent);
			continue;
		}

		//If entry is a file, add it to either posts or drafts
		if (ent->d_type == DT_REG)
		{
			if (strcmp(ent->d_name, "rss.conf") == 0)
			{
				free(ent);
				continue; // Don't read rss configu
			}

			h_post* post = h_post_create();
			if (post == NULL)
			{
				free(ent);
				free(namelist);
				return h_err_create(H_ERR_ALLOC, NULL);
			}

			char* p = h_util_path_join(path, ent->d_name);
			if (p == NULL)
			{
				free(ent);
				free(namelist);
				return h_err_create(H_ERR_ALLOC, NULL);
			}

			h_err* err = NULL;
			err = h_post_init_from_file(post, p, spath, depth);
			if (err)
			{
				free(p);
				free(ent);
				free(namelist);
				return err;
			}
			free(p);

			if (post->isdraft)
				err = h_section_add_draft(section, post);
			else
				err = h_section_add_post(section, post);

			if (err)
			{
				free(ent);
				free(namelist);
				free(p);
				return err;
			}
		}

		//If it's a directory, it's a section and should be added to subs
		else if (ent->d_type == DT_DIR)
		{
			h_section* sub = h_section_create();
			if (sub == NULL)
			{
				free(ent);
				free(namelist);
				return h_err_create(H_ERR_ALLOC, NULL);
			}

			char* p = h_util_path_join(path, ent->d_name);
			if (p == NULL)
			{
				free(ent);
				free(namelist);
				return h_err_create(H_ERR_ALLOC, NULL);
			}

			char* sp = h_util_path_join(spath, ent->d_name + 6);
			if (sp == NULL)
			{
				free(ent);
				free(namelist);
				free(p);
				return h_err_create(H_ERR_ALLOC, NULL);
			}
			char* sp_use = sp;
			if (sp_use[0] == '/')
				sp_use = sp + 1;

			h_err* err = init_from_dir(sub, p, sp_use, depth + 1);
			if (err)
			{
				free(ent);
				free(namelist);
				free(p);
				free(sp);
				return err;
			}

			err = h_section_add_sub(section, sub);
			free(p);
			free(sp);
			if (err)
			{
				free(ent);
				free(namelist);
				return err;
			}
		}

		free(ent);
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

	section->path = malloc(strlen(spath) + 1);
	strcpy(section->path, spath);
	section->depth = depth;

	if (chars <= 0) {
		section->slug = NULL;
		section->title = NULL;
		return NULL;
	}

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
	section->posts = realloc(section->posts, section->numposts * sizeof(*section->posts));
	if (section->posts == NULL)
		return h_err_create(H_ERR_ALLOC, NULL);

	section->posts[section->numposts - 1] = post;

	return NULL;
}

h_err* h_section_add_draft(h_section* section, h_post* post)
{
	section->numdrafts += 1;
	section->drafts = realloc(section->drafts, section->numdrafts * sizeof(*section->drafts));
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

h_section* h_section_create()
{
	h_section* section = malloc(sizeof(*section));
	if (section == NULL)
		return NULL;

	section->title = NULL;
	section->slug = NULL;

	section->posts = NULL;
	section->numposts = 0;
	section->drafts = NULL;
	section->numdrafts = 0;
	section->subs = NULL;
	section->numsubs = 0;
	section->path = NULL;
	section->rpath = NULL;
	section->rss_metadata = h_rss_section_create();
	if (section->rss_metadata == NULL)
	{
		free(section);
		return NULL;
	}
	section->rss = NULL;
	return section;
}

void h_section_free(h_section* section)
{
	if (section == NULL)
		return;

	free(section->title);
	free(section->slug);
	free(section->path);
	free(section->rpath);
	free(section->rss);
	h_rss_section_free(section->rss_metadata);

	for (int i=0; i < section->numposts; ++i)
		h_post_free(section->posts[i]);
	free(section->posts);

	for (int i=0; i < section->numdrafts; ++i)
		h_post_free(section->drafts[i]);
	free(section->drafts);

	for (int i=0; i < section->numsubs; ++i)
		h_section_free(section->subs[i]);
	free(section->subs);

	free(section);
}
