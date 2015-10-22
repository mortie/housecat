#include "h_build.h"
#include "h_util.h"
#include "h_file.h"

#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

h_err* h_build_theme(char* dirpath, h_build_outfiles outfiles)
{
	//Copy things to script.js
	{
		char* start = "(function(){\n";
		char* end = "})();\n";
		char* path = h_util_path_join(dirpath, H_FILE_THEME_JS);

		h_util_cp_dir_to_file_se(path, outfiles.js, start, end);

		free(path);
	}

	//Copy things to style.css
	{
		char* path = h_util_path_join(dirpath, H_FILE_THEME_CSS);

		h_util_cp_dir_to_file(path, outfiles.css);

		free(path);
	}

	return NULL;
}
