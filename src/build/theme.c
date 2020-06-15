#include "../build.h"
#include "../util.h"
#include "../file.h"

#include <stdlib.h>

h_err* h_build_theme(const char* dirpath, h_build_outfiles outfiles)
{
	//Copy things to script.js
	{
		char* start = "(function(){\n";
		char* end = "})();\n";
		char* path = h_util_path_join(dirpath, H_FILE_THEME_JS);
		if (path == NULL)
			return h_err_from_errno(errno, dirpath);

		h_util_cp_dir_to_file_se(path, outfiles.js, start, end);

		free(path);
	}

	//Copy things to style.css
	{
		char* path = h_util_path_join(dirpath, H_FILE_THEME_CSS);
		if (path == NULL)
			return h_err_from_errno(errno, dirpath);

		h_util_cp_dir_to_file(path, outfiles.css);

		free(path);
	}

	return NULL;
}
