#include "err.h"
#include "section.h"
#include "post.h"
#include "build.h"
#include "file.h"
#include "util.h"
#include "conf.h"
#include "strs.h"
#include "rss.h"

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
	{
		free(inpath);
		free(outpath);
		return h_err_create(H_ERR_ALLOC, NULL);
	}

	h_section* root = h_section_create();
	if (root == NULL)
	{
		err = h_err_create(H_ERR_ALLOC, inpath);
		free(inpath);
		free(outpath);
		return err;
	}
	err = h_section_init_from_dir(root, inpath);
	if (err)
	{
		free(inpath);
		free(outpath);
		h_section_free(root);
		return err;
	}

	err = h_rss_configure(root, NULL);
	if (err)
	{
		free(inpath);
		free(outpath);
		h_section_free(root);
		return err;
	}

	if (mkdir(inpath, 0777) == -1 && errno != EEXIST)
	{
		err = h_err_from_errno(errno, inpath);
		free(inpath);
		free(outpath);
		h_section_free(root);
		return err;
	}
	free(inpath);

	if (mkdir(outpath, 0777) == -1 && errno != EEXIST)
	{
		err = h_err_from_errno(errno, outpath);
		free(outpath);
		h_section_free(root);
		return err;
	}

	//Get config
	char* conf_path = h_util_path_join(path, H_FILE_CONF);
	char* conf_str = h_util_file_read(conf_path);
	if (conf_str == NULL)
	{
		err = h_err_from_errno(errno, conf_path);
		free(outpath);
		free(conf_path);
		h_section_free(root);
		return err;
	}

	h_conf* conf = h_conf_create();
	if (conf == NULL)
	{
		err = h_err_from_errno(errno, conf_path);
		free(conf_path);
		free(conf_str);
		free(outpath);
		h_section_free(root);
		return err;
	}


	free(conf_path);

	err = h_conf_parse(conf_str, strlen(conf_str)+1, (void *)conf, h_conf_build);
	if (err)
	{
		free(outpath);
		free(conf_str);
		h_section_free(root);
		h_conf_free(conf);
		return err;
	}
	free(conf_str);

	// append root to the end of the url so links actually work
	if (conf->url != NULL && conf->root != NULL && conf->root[0] != '\0')
	{
		char* old_url = conf->url;
		conf->url = h_util_path_join(conf->url, conf->root);
		free(old_url);
	}

	//index template
	char* index_path = h_util_path_join(path, H_FILE_THEME_HTML "/index.html");
	char* index_str = h_util_file_read(index_path);
	if (index_str == NULL)
	{
		err = h_err_from_errno(errno, index_path);
		free(outpath);
		free(index_path);
		h_section_free(root);
		h_conf_free(conf);
		return err;
	}
	free(index_path);

	//post template
	char* post_path = h_util_path_join(path, H_FILE_THEME_HTML "/post.html");
	char* post_str = h_util_file_read(post_path);
	if (post_str == NULL)
	{
		err = h_err_from_errno(errno, post_path);
		free(outpath);
		free(post_path);
		h_section_free(root);
		h_conf_free(conf);
		return err;
	}
	free(post_path);

	//page template
	char* page_path = h_util_path_join(path, H_FILE_THEME_HTML "/page.html");
	char* page_str = h_util_file_read(page_path);
	if (page_str == NULL)
	{
		err = h_err_from_errno(errno, page_path);
		free(outpath);
		free(page_path);
		h_section_free(root);
		h_conf_free(conf);
		return err;
	}
	free(page_path);

	//menu template
	char* menu_path = h_util_path_join(path, H_FILE_THEME_HTML "/menu.html");
	char* menu_str = h_util_file_read(menu_path);
	if (menu_str == NULL)
	{
		err = h_err_from_errno(errno, menu_path);
		free(outpath);
		free(menu_path);
		free(page_str);
		h_section_free(root);
		h_conf_free(conf);
		return err;
	}
	free(menu_path);

	//menu_section template
	char* menu_section_path = h_util_path_join(path, H_FILE_THEME_HTML "/menu_section.html");
	char* menu_section_str = h_util_file_read(menu_section_path);
	if (menu_section_str == NULL)
	{
		err = h_err_from_errno(errno, menu_section_path);
		free(outpath);
		free(page_str);
		free(menu_section_path);
		h_section_free(root);
		h_conf_free(conf);
		return err;
	}
	free(menu_section_path);

	//menu_logo template
	char* menu_logo_path = h_util_path_join(path, H_FILE_THEME_HTML "/menu_logo.html");
	char* menu_logo_str = h_util_file_read(menu_logo_path);
	if (menu_logo_str == NULL)
	{
		err = h_err_from_errno(errno, menu_logo_path);
		free(outpath);
		free(page_str);
		free(menu_logo_path);
		h_section_free(root);
		h_conf_free(conf);
		return err;
	}
	free(menu_logo_path);

	h_build_strs strs =
	{
		index_str,
		post_str,
		page_str,
		menu_str,
		menu_section_str,
		menu_logo_str
	};

	err = h_build(root, outpath, strs, conf);
	if (err)
	{
		free(index_str);
		free(post_str);
		free(page_str);
		free(menu_str);
		free(menu_section_str);
		free(menu_logo_str);
		free(outpath);
		h_section_free(root);
		h_conf_free(conf);
		return err;
	}

	// Deal with rss
	if (conf->rss)
	{
		err = h_rss_build(root, conf, outpath);
		if (err)
		{
			free(index_str);
			free(post_str);
			free(page_str);
			free(menu_str);
			free(menu_section_str);
			free(menu_logo_str);
			free(outpath);
			h_section_free(root);
			h_conf_free(conf);
			return err;
		}
	}

	h_section_free(root);

	//Deal with meta things

	char* metapath = h_util_path_join(path, H_FILE_OUTPUT "/" H_FILE_OUT_META);
	if (metapath == NULL)
	{
		err = h_err_create(H_ERR_ALLOC, NULL);
		free(index_str);
		free(post_str);
		free(page_str);
		free(menu_str);
		free(menu_section_str);
		free(menu_logo_str);
		free(outpath);
		h_conf_free(conf);
		return err;
	}
	if (mkdir(metapath, 0777) == -1 && errno != EEXIST)
	{
		err = h_err_from_errno(errno, metapath);
		free(index_str);
		free(metapath);
		free(post_str);
		free(page_str);
		free(menu_str);
		free(menu_section_str);
		free(menu_logo_str);
		free(outpath);
		h_conf_free(conf);
		return err;
	}
	free(metapath);

	h_build_outfiles outfiles;

	//public/_/script.js
	char* outjspath = h_util_path_join(
		outpath, H_FILE_OUT_META "/" H_FILE_OUT_JS
	);
	outfiles.js = fopen(outjspath, "w");
	if (outfiles.js == NULL)
	{
		err = h_err_from_errno(errno, outjspath);
		free(outjspath);
		free(index_str);
		free(post_str);
		free(page_str);
		free(menu_str);
		free(menu_section_str);
		free(menu_logo_str);
		free(outpath);
		h_conf_free(conf);
		return err;
	}
	free(outjspath);

	//public/_/style.css
	char* outcsspath = h_util_path_join(
		outpath, H_FILE_OUT_META "/" H_FILE_OUT_CSS
	);
	outfiles.css = fopen(outcsspath, "w");
	if (outfiles.css == NULL)
	{
		err = h_err_from_errno(errno, outcsspath);
		free(outcsspath);
		free(index_str);
		free(post_str);
		free(page_str);
		free(menu_str);
		free(menu_section_str);
		free(menu_logo_str);
		free(outpath);
		h_conf_free(conf);
		return err;
	}
	free(outcsspath);
	free(outpath);

	//Prepare imgs
	err = h_build_imgs(path);
	if (err)
	{
		free(index_str);
		free(post_str);
		free(page_str);
		free(menu_str);
		free(menu_section_str);
		free(menu_logo_str);
		h_conf_free(conf);
		return err;
	}

	//Prepare basic script.js library
	fputs(H_STRS_JS_LIB, outfiles.js);

	//Prepare theme things
	err = h_build_theme(path, outfiles);
	if (err)
	{
		free(index_str);
		free(post_str);
		free(page_str);
		free(menu_str);
		free(menu_section_str);
		free(menu_logo_str);
		h_conf_free(conf);
		return err;
	}

	//Prepare plugins
	err = h_build_plugins(path, outfiles, conf);
	if (err)
	{
		free(index_str);
		free(post_str);
		free(page_str);
		free(menu_str);
		free(menu_section_str);
		free(menu_logo_str);
		h_conf_free(conf);
		return err;
	}

	fclose(outfiles.js);
	fclose(outfiles.css);
	h_conf_free(conf);
	free(index_str);
	free(post_str);
	free(page_str);
	free(menu_str);
	free(menu_section_str);
	free(menu_logo_str);

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
	{
		h_err_print(err);
		h_err_free(err);
		return 2;
	}

	return 0;
}
