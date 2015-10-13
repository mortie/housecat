#include "h_err.h"
#include "h_section.h"
#include "h_post.h"
#include "h_build.h"
#include "h_file.h"
#include "h_util.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

h_err* build(char* path)
{
	h_err* err;

	char* inpath = h_util_path_join(path, H_FILE_INPUT);
	char* outpath = h_util_path_join(path, H_FILE_OUTPUT);
	if (inpath == NULL || outpath == NULL)
		return h_err_create(H_ERR_ALLOC, NULL);

	h_section* root = malloc(sizeof(h_section));
	err = h_section_init_from_dir(root, inpath);
	if (err)
		return err;

	if (mkdir(inpath, 0777) == -1 && errno != EEXIST)
		return h_err_create(errno, inpath);

	char* index_path = h_util_path_join(path, H_FILE_THEME "/index.html");
	char* index_str = h_util_file_read(index_path);
	if (index_str == NULL) return h_err_from_errno(errno, index_path);
	free(index_path);

	char* section_path = h_util_path_join(path, H_FILE_THEME "/section.html");
	char* section_str = h_util_file_read(section_path);
	if (section_str == NULL) return h_err_from_errno(errno, section_path);
	free(section_path);

	char* post_path = h_util_path_join(path, H_FILE_THEME "/post.html");
	char* post_str = h_util_file_read(post_path);
	if (post_str == NULL) return h_err_from_errno(errno, post_path);
	free(post_path);

	struct h_build_strs strs =
	{
		index_str,
		section_str,
		post_str
	};

	err = h_build(root, outpath, strs);
	if (err)
		return err;

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
