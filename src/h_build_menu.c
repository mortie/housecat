#include "h_build.h"
#include "h_template.h"
#include "h_util.h"

#include <stdlib.h>
#include <string.h>

int iscurrent(h_section* sec1, h_section* sec2)
{
	return sec1 == sec2;
}

static char* build_menu_node(
		h_section* root,
		h_section* current,
		char* relpath,
		h_build_strs strs)
{
	char* subs = NULL;

	int i;
	for (i = 0; i < root->numsubs; ++i)
	{
		h_section* sub = root->subs[i];
		char* relpath = h_util_make_rel_path(sub->depth);
		char* s = h_util_str_join(
			subs,
			build_menu_node(sub, current, relpath, strs)
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
	h_template_args_append(args, "current", iscurrent(root, current) ? "current" : "");
	char* res = h_templateify(strs.menu_section, args);
	h_template_args_free(args);

	free(subs);

	return res;
}

static char* build_logo(char* relpath, h_build_strs strs)
{
	h_template_args* args = h_template_args_create();
	h_template_args_append(args, "relpath", relpath);
	char* res = h_templateify(strs.menu_logo, args);
	h_template_args_free(args);

	return res;
}

char* h_build_menu(h_section* root,
		h_section* current,
		h_build_strs strs,
		h_conf* conf)
{
	char* relpath = h_util_make_rel_path(current->depth);
	char* sections = NULL;

	int i;
	for (i = 0; i < root->numsubs; ++i)
	{
		char* s = h_util_str_join(
			sections,
			build_menu_node(root->subs[i], current, relpath, strs)
		);
		free(sections);
		sections = s;
	}

	h_template_args* args = h_template_args_create();
	h_template_args_append(args, "relpath", relpath);
	h_template_args_append(args, "sections", sections);

	if (conf->logo)
	{
		char* logo = build_logo(relpath, strs);
		h_template_args_append(args, "logo", logo);
	}
	else
	{
		h_template_args_append(args, "logo", "");
	}

	char* res = h_templateify(strs.menu, args);
	h_template_args_free(args);

	free(relpath);
	free(sections);

	return res;
}
