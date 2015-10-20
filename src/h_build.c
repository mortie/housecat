#include "h_build.h"
#include "h_file.h"
#include "h_template.h"
#include "h_util.h"
#include "../settings.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

static h_err* build_node(h_section* root, h_section* current, char* rootdir, h_build_strs strs)
{
	char* dirpath = h_util_path_join(rootdir, current->path);

	if (mkdir(dirpath, 0777) == -1 && errno != EEXIST)
		return h_err_from_errno(errno, dirpath);

	puts(current->path);

	int i;
	for (i = 0; i < current->numsubs; ++i)
	{
		h_err* err = build_node(root, current->subs[i], rootdir, strs);
		if (err)
			return err;
	}

	char* indexpath = h_util_path_join(dirpath, H_FILE_INDEX);
	FILE* file = fopen(indexpath, "w");
	free(indexpath);

	h_err* err = h_build_section(root, current, file, strs);
	if (err)
		return err;

	fclose(file);

	return NULL;
}

h_err* h_build(h_section* root, char* rootdir, h_build_strs strs)
{
	int i;
	for (i = 0; i < root->numsubs; ++i)
	{
		h_err* err = build_node(root, root->subs[i], rootdir, strs);
		if (err)
			return err;
	}

	return NULL;
}
