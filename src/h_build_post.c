#include "h_build.h"
#include "h_template.h"
#include "h_util.h"

#include <stdlib.h>

static char* build_post(h_post* post, h_build_strs strs)
{
	char* relpath = h_util_make_rel_path(post->depth);

	h_template_args* args = h_template_args_create();
	h_template_args_append(args, "title", post->title);
	h_template_args_append(args, "html", post->html);
	h_template_args_append(args, "relpath", relpath);
	h_template_args_append(args, "url", post->path);
	char* res = h_templateify(strs.post, args);
	h_template_args_free(args);

	free(relpath);

	return res;
}

h_err* h_build_post(
		h_section* root,
		h_section* section,
		h_post* post,
		FILE* file,
		h_build_strs strs,
		h_conf* conf)
{
	char* menu_str = h_build_menu(root, section, strs, conf);
	char* post_str = build_post(post, strs);

	h_template_args* args = h_template_args_create();
	h_template_args_append(args, "title", post->title);
	h_template_args_append(args, "s_title", conf->title);
	h_template_args_append(args, "menu", menu_str);
	h_template_args_append(args, "posts", post_str);
	char* res = h_templateify(strs.index, args);
	h_template_args_free(args);

	fputs(res, file);

	free(menu_str);
	free(post_str);

	return NULL;
}
