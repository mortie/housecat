#include "../build.h"
#include "../util.h"
#include "../file.h"
#include "../strs.h"

#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

static h_err* build_plugin(
		char* dirpath,
		char* outdirphp,
		char* outdirmisc,
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
	char* jspath = h_util_path_join(dirpath, H_FILE_PLUGIN_JS);
	char* phppath = h_util_path_join(dirpath, H_FILE_PLUGIN_PHP);
	char* miscpath = h_util_path_join(dirpath, H_FILE_PLUGIN_MISC);

	//Copy things to script.js
	if (h_util_file_err(jspath) != ENOENT)
	{
		char* starttemplate = H_STRS_JS_START;

		int len = sizeof(char) * (strlen(starttemplate) + jsonlen + rootlen);
		char* start = malloc(len);
		snprintf(start, len, starttemplate, confjson, conf->root);

		char* end = H_STRS_JS_END;

		h_util_cp_dir_to_file_se(jspath, outfiles.js, start, end);

		free(start);
	}

	//Copy PHP files
	if (h_util_file_err(phppath) != ENOENT)
	{
		const char* starttemplate = H_STRS_PHP_START;

		int len = sizeof(char) * (strlen(starttemplate) + jsonlen) + rootlen;
		char* start = malloc(len);
		snprintf(start, len, starttemplate, confjson, conf->root);

		//Make sure dirs are okay
		DIR* d1 = opendir(phppath);
		if (d1 == NULL)
			return h_err_from_errno(errno, phppath);
		closedir(d1);
		if (mkdir(outdirphp, 0777) == -1 && errno != EEXIST)
			return h_err_from_errno(errno, outdirphp);

		h_util_cp_dir_se(phppath, outdirphp, start, "");

		free(start);
	}

	//Copy misc files
	if (h_util_file_err(miscpath) != ENOENT)
	{
		//Make sure dirs are okay
		DIR* d1 = opendir(miscpath);
		if (d1 == NULL)
			return h_err_from_errno(errno, miscpath);
		closedir(d1);
		if (mkdir(outdirmisc, 0777) == -1 && errno != EEXIST)
			return h_err_from_errno(errno, outdirmisc);

		h_util_cp_dir(miscpath, outdirmisc);
	}

	free(confjson);
	free(phppath);
	free(jspath);
	free(miscpath);

	return NULL;
}

h_err* h_build_plugins(char* rootdir, h_build_outfiles outfiles, h_conf* conf)
{
	char* pluginsdir = h_util_path_join(rootdir, H_FILE_PLUGINS);
	char* outpluginsdirphp = h_util_path_join(
		rootdir,
		H_FILE_OUTPUT "/" H_FILE_OUT_META "/" H_FILE_OUT_PHP
	);
	char* outpluginsdirmisc = h_util_path_join(
		rootdir,
		H_FILE_OUTPUT "/" H_FILE_OUT_META "/" H_FILE_OUT_MISC
	);

	//Check status of rootdir/plugins, returning if it doesn't exist
	{
		int err = h_util_file_err(pluginsdir);
		if (err == ENOENT)
			return NULL;
		if (err && err != EEXIST)
			return h_err_from_errno(err, pluginsdir);
	}

	//Create dirs if they don't exist
	if (mkdir(outpluginsdirphp, 0777) == -1 && errno != EEXIST)
		return h_err_from_errno(errno, outpluginsdirphp);
	if (mkdir(outpluginsdirmisc, 0777) == -1 && errno != EEXIST)
		return h_err_from_errno(errno, outpluginsdirmisc);

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
		char* outdirphp = h_util_path_join(outpluginsdirphp, ent->d_name);
		char* outdirmisc = h_util_path_join(outpluginsdirmisc, ent->d_name);

		h_err* err;
		err = build_plugin(dirpath, outdirphp, outdirmisc, outfiles, conf);
		if (err)
			return err;

		free(dirpath);
		free(outdirphp);
		free(outdirmisc);
		free(ent);
	}
	free(namelist);

	return NULL;
}
