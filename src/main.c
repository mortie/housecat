#include "h_err.h"
#include "h_post.h"

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char** argv)
{
	if (argc < 0)
		return 1;

	h_post* post = malloc(sizeof(h_post));
	h_err* err = h_post_init_from_file(post, argv[1]);
	if (err)
	{
		h_err_print(err);
		exit(1);
	}

	return 0;
}
