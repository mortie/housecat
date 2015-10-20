#include "h_err.h"
#include "h_section.h"
#include "h_post.h"
#include "h_build.h"
#include "h_file.h"
#include "h_util.h"
#include "h_conf.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

h_err* build(char* path)
{
	h_err* err;

	if (h_util_file_err(path))
		return h_err_from_errno(errno, path);

	char* inpath = h_util_path_join(path, H_FILE_INPUT);
	char* outpath = h_util_path_join(path, H_FILE_OUTPUT);
	if (inpath == NULL || outpath == NULL)
		return h_err_create(H_ERR_ALLOC, NULL);

	h_section* root = malloc(sizeof(h_section));
	err = h_section_init_from_dir(root, inpath);
	if (err)
		return err;

	if (mkdir(inpath, 0777) == -1 && errno != EEXIST)
		return h_err_from_errno(errno, inpath);

	if (mkdir(outpath, 0777) == -1 && errno != EEXIST)
		return h_err_from_errno(errno, outpath);

	//Get config
	char* conf_path = h_util_path_join(path, H_FILE_CONF);
	char* conf_str = h_util_file_read(conf_path);
	if (conf_str == NULL) return h_err_from_errno(errno, conf_path);
	free(conf_path);
	h_conf* conf = h_conf_parse(conf_str, strlen(conf_str));

	//index template
	char* index_path = h_util_path_join(path, H_FILE_THEME_HTML "/index.html");
	char* index_str = h_util_file_read(index_path);
	if (index_str == NULL) return h_err_from_errno(errno, index_path);
	free(index_path);

	//post template
	char* post_path = h_util_path_join(path, H_FILE_THEME_HTML "/post.html");
	char* post_str = h_util_file_read(post_path);
	if (post_str == NULL) return h_err_from_errno(errno, post_path);
	free(post_path);

	//menu template
	char* menu_path = h_util_path_join(path, H_FILE_THEME_HTML "/menu.html");
	char* menu_str = h_util_file_read(menu_path);
	if (menu_str == NULL) return h_err_from_errno(errno, menu_path);
	free(menu_path);

	//menu_section template
	char* menu_section_path = h_util_path_join(path, H_FILE_THEME_HTML "/menu_section.html");
	char* menu_section_str = h_util_file_read(menu_section_path);
	if (menu_section_str == NULL) return h_err_from_errno(errno, menu_section_path);
	free(menu_section_path);

	//menu_logo template
	char* menu_logo_path = h_util_path_join(path, H_FILE_THEME_HTML "/menu_logo.html");
	char* menu_logo_str = h_util_file_read(menu_logo_path);
	if (menu_logo_str == NULL) return h_err_from_errno(errno, menu_logo_path);
	free(menu_logo_path);

	h_build_strs strs =
	{
		index_str,
		post_str,
		menu_str,
		menu_section_str,
		menu_logo_str
	};

	err = h_build(root, outpath, strs, conf);
	if (err)
		return err;

	free(inpath);
	free(outpath);
	free(conf);
	free(conf_str);
	free(index_str);
	free(post_str);
	free(menu_str);
	free(menu_section_str);

	return NULL;
}

int main(int argc, char** argv)
{
#define usage() fprintf(stderr, "Usage: %s <directory>\n", argv[0])

	if (argc != 2) {
		usage();
		return 1;
	}

	h_err* err = build(argv[1]);
	if (err)
		h_err_print(err);

	return 0;
}
