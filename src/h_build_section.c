#include "h_build.h"
#include "h_template.h"
#include "h_util.h"

#include <stdlib.h>

static char* build_section_post(h_post* post, h_build_strs strs)
{
	h_template_args* args = h_template_args_create();
	h_template_args_append(args, "title", post->title);
	h_template_args_append(args, "html", post->html);
	char* res = h_templateify(strs.post, args);
	h_template_args_free(args);

	return res;
}

h_err* h_build_section(h_section* root, h_section* section, FILE* file, h_build_strs strs, h_conf* conf)
{
	char* posts_str = NULL;
	int i;
	for (i = 0; i < section->numposts; ++i)
	{
		char* s = build_section_post(section->posts[i], strs);
		char* s2 = h_util_str_join(posts_str, s);
		free(s);
		free(posts_str);
		posts_str = s2;
	}

	char* menu_str = h_build_menu(root, section, strs);

	h_template_args* args = h_template_args_create();
	h_template_args_append(args, "title", section->title);
	h_template_args_append(args, "s_title", conf->title);
	h_template_args_append(args, "menu", menu_str);
	h_template_args_append(args, "posts", posts_str);
	char* res = h_templateify(strs.index, args);
	h_template_args_free(args);

	fputs(res, file);

	free(menu_str);
	free(posts_str);

	return NULL;
}
