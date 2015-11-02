#include "../build.h"
#include "../template.h"
#include "../util.h"

#include <stdlib.h>

static char* build_section_post(h_post* post, h_build_strs strs, h_conf* conf)
{
	h_template_args* args = h_template_args_create();
	h_template_args_append(args, "title", post->title);
	h_template_args_append(args, "html", post->html);
	h_template_args_append(args, "s_root", conf->root);
	h_template_args_append(args, "url", post->path);
	char* res = h_templateify(strs.post, args);
	h_template_args_free(args);

	return res;
}

static char* build_section_page(
		h_section* section,
		int page,
		h_build_strs strs,
		h_conf* conf)
{
	char* posts_str = NULL;
	int ppp = conf->posts_per_page;
	int i;
	for (i = page * ppp; i < (page * ppp) + ppp; ++i)
	{
		if (i >= section->numposts)
			break;

		char* s = build_section_post(section->posts[i], strs, conf);
		char* s2 = h_util_str_join(posts_str, s);
		free(s);
		free(posts_str);
		posts_str = s2;
	}

	int has_prev = (page > 0);
	int has_next = (page < (section->numposts / ppp));
	char* prev_url = NULL;
	char* next_url = NULL;

	if (has_prev)
	{
		char n[16];
		snprintf(n, 16, "%d", page - 1);
		prev_url = h_util_path_join(section->path, n);
	}
	if (has_next)
	{
		char n[16];
		snprintf(n, 16, "%d", page + 1);
		next_url = h_util_path_join(section->path, n);
	}

	h_template_args* args = h_template_args_create();
	h_template_args_append(args, "posts", posts_str);
	h_template_args_append(args, "has-prev", (has_prev ? "has-prev" : ""));
	h_template_args_append(args, "has-next", (has_next ? "has-next" : ""));
	h_template_args_append(args, "prev-url", prev_url);
	h_template_args_append(args, "next-url", next_url);
	h_template_args_append(args, "s_root", conf->root);
	char* res = h_templateify(strs.page, args);
	h_template_args_free(args);

	free(posts_str);
	free(prev_url);
	free(next_url);

	return res;
}

h_err* h_build_section(
		h_section* root,
		h_section* section,
		FILE* file,
		int page,
		h_build_strs strs,
		h_conf* conf)
{
	char* menu_str = h_build_menu(root, section, strs, conf);
	char* page_str = build_section_page(section, page, strs, conf);

	h_template_args* args = h_template_args_create();
	h_template_args_append(args, "title", section->title);
	h_template_args_append(args, "s_title", conf->title);
	h_template_args_append(args, "menu", menu_str);
	h_template_args_append(args, "page", page_str);
	h_template_args_append(args, "s_root", conf->root);
	char* res = h_templateify(strs.index, args);
	h_template_args_free(args);

	fputs(res, file);

	free(menu_str);
	free(page_str);

	return NULL;
}
