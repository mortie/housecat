#include "h_build.h"
#include "h_util.h"
#include "h_file.h"

#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

h_err* cp_dir(char* dir1, char* dir2, char* start)
{
	DIR* d1 = opendir(dir1);
	if (d1 == NULL)
		return h_err_from_errno(errno, dir1);

	if (mkdir(dir2, 0777) == -1 && errno != EEXIST)
		return h_err_from_errno(errno, dir2);

	DIR* d2 = opendir(dir2);
	if (d2 == NULL)
		return h_err_from_errno(errno, dir2);
	closedir(d2);

	struct dirent* dp = readdir(d1);
	while (dp != NULL)
	{
		if (dp->d_name[0] == '.')
		{
			dp = readdir(d1);
			continue;
		}

		char* p1 = h_util_path_join(dir1, dp->d_name);
		char* p2 = h_util_path_join(dir2, dp->d_name);
		FILE* f1 = fopen(p1, "r");
		FILE* f2 = fopen(p2, "w+");

		free(p1);
		free(p2);

		fputs(start, f2);
		h_util_file_copy(f1, f2);

		fclose(f1);
		fclose(f2);

		dp = readdir(d1);
	}

	return NULL;
}

static h_err* build_plugin(
		char* dirpath,
		char* outdir,
		h_build_outfiles outfiles,
		h_conf* conf)
{
	char* confjsonpath = h_util_path_join(dirpath, H_FILE_PLUGIN_CONF);
	if (confjsonpath == NULL)
		return h_err_create(H_ERR_ALLOC, NULL);

	errno = 0;
	char* confjson = h_util_file_read(confjsonpath);
	if (confjson == NULL && errno)
		return h_err_from_errno(errno, confjsonpath);
	if (confjson == NULL)
		return h_err_create(H_ERR_ALLOC, NULL);
	free(confjsonpath);

	int jsonlen = strlen(confjson);
	int rootlen = strlen(conf->root);

	//Copy things to script.js
	{
		char* starttemplate =
			"(function(){\n"
			"conf = %s;\n"
			"conf.root = '%s';\n";

		int len = sizeof(char) * (strlen(starttemplate) + jsonlen + rootlen);
		char* start = malloc(len);
		snprintf(start, len, starttemplate, confjson, conf->root);

		char* end = "})();";
		char* jspath = h_util_path_join(dirpath, H_FILE_PLUGIN_JS);

		h_util_cp_dir_to_file_se(jspath, outfiles.js, start, end);

		free(jspath);
		free(start);
	}

	//Copy PHP files
	{
		const char* starttemplate =
			"<?php\n"
			"$conf = <<<EOT\n"
			"%s\n"
			"EOT;\n"
			"$conf = json_decode($conf);\n"
			"$conf->root = '%s';\n"
			"?>\n";

		int len = sizeof(char) * (strlen(starttemplate) + jsonlen) + rootlen;
		char* start = malloc(len);
		snprintf(start, len, starttemplate, confjson, conf->root);

		char* phppath = h_util_path_join(dirpath, H_FILE_PLUGIN_PHP);

		h_err* err = cp_dir(phppath, outdir, start);
		if (err)
			return err;

		free(phppath);
		free(start);
	}

	free(confjson);

	return NULL;
}

h_err* h_build_plugins(char* rootdir, h_build_outfiles outfiles, h_conf* conf)
{
	char* pluginsdir = h_util_path_join(rootdir, H_FILE_PLUGINS);
	char* outpluginsdir = h_util_path_join(
		rootdir,
		H_FILE_OUTPUT "/" H_FILE_OUT_META "/" H_FILE_OUT_PHP
	);

	//Check status of rootdir/plugins, returning if it doesn't exist
	{
		int err = h_util_file_err(pluginsdir);
		if (err == EEXIST)
			return NULL;
		else if (err)
			return h_err_from_errno(err, pluginsdir);
	}

	//Create rootdir/public/_/plugins if it doesn't exist
	if (mkdir(outpluginsdir, 0777) == -1 && errno != EEXIST)
		return h_err_from_errno(errno, outpluginsdir);

	//Loop through plugins, building them
	struct dirent** namelist;
	int n = scandir(pluginsdir, &namelist, NULL, alphasort);
	int i;
	for (i = 0; i < n; ++i)
	{
		struct dirent* ent = namelist[i];
		if (ent->d_name[0] == '.')
			continue;

		char* dirpath = h_util_path_join(pluginsdir, ent->d_name);
		char* outdir = h_util_path_join(outpluginsdir, ent->d_name);

		h_err* err;
		err = build_plugin(dirpath, outdir, outfiles, conf);
		if (err)
			return err;

		free(dirpath);
		free(outdir);
		free(ent);
	}
	free(namelist);

	return NULL;
}
