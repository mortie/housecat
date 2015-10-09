#include "h_err.h"
#include "h_section.h"
#include "h_post.h"
#include "h_build.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>

int main(int argc, char** argv)
{
	if (argc < 2)
		return 1;

	h_section* root = malloc(sizeof(h_section));
	h_err* err;
	err = h_section_init_from_dir(root, argv[1]);
	if (err)
	{
		h_err_print(err);
		return 1;
	}

	if (mkdir(argv[2], 0777) == -1 && errno != EEXIST)
	{
		h_err_print(h_err_create(errno, argv[2]));
		return 1;
	}

	struct h_build_strs strs =
	{
		/* index */ "<!DOCTYPE html><html><body>{{page}}</body></html>",
		/* section */ "<div class='posts'>{{posts}}</div>",
		/* post */ "<div class='post'><h1>{{title}}</h1><div class='content'>{{html}}</div></div>"
	};

	err = h_build(root, argv[2], strs);
	if (err)
	{
		h_err_print(err);
		exit(1);
	}

	return 0;
}
