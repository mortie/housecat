#include "h_build.h"
#include "h_template.h"
#include "h_util.h"

#include <stdlib.h>
#include <string.h>

static char* make_rel_path(int depth)
{
	char* str = malloc(depth * 3);

	int i;
	for (i = 0; i < depth; ++i)
	{
		memcpy(str + (i * 3), "../", 3);
	}

	str[(depth * 3) - 1] = '\0';

	return str;
}

static char* h_build_menu_node(h_section* root, h_section* current, char* relpath, h_build_strs strs)
{
	char* subs = NULL;

	int i;
	for (i = 0; i < root->numsubs; ++i)
	{
		h_section* sub = root->subs[i];
		char* relpath = make_rel_path(sub->depth);
		char* s = h_util_str_join(
			subs,
			h_build_menu_node(sub, current, relpath, strs)
		);
		free(relpath);
		free(subs);
		subs = s;
	}

	h_template_args* args = h_template_args_create();
	h_template_args_append(args, "relpath", relpath);
	h_template_args_append(args, "title", root->title);
	h_template_args_append(args, "url", root->path);
	h_template_args_append(args, "subs", subs);
	char* res = h_templateify(strs.menu_section, args);
	h_template_args_free(args);

	free(subs);

	return res;
}

char* h_build_menu(h_section* root, h_section* current, h_build_strs strs)
{
	char* relpath = make_rel_path(current->depth);
	char* sections = NULL;

	int i;
	for (i = 0; i < root->numsubs; ++i)
	{
		char* s = h_util_str_join(
			sections,
			h_build_menu_node(root->subs[i], current, relpath, strs)
		);
		free(sections);
		sections = s;
	}

	h_template_args* args = h_template_args_create();
	h_template_args_append(args, "relpath", relpath);
	h_template_args_append(args, "sections", sections);
	char* res = h_templateify(strs.menu, args);
	h_template_args_free(args);

	free(relpath);
	free(sections);

	return res;
}
