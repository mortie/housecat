#include "h_err.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

static char* str_from_err(h_err* err)
{
	switch (err->type)
	{
	case H_ERR_UNKNOWN:
		return "Unknown error";
	case H_ERR_OTHER:
		return "";
	case H_ERR_ALLOC:
		return "Allocation error";
	case H_ERR_ACCESS:
		return "Permission denied";
	default:
		return "Unknown error";
	}
}

static h_err_type type_from_errno(int err)
{
	switch (err)
	{
	case EACCES:
		return H_ERR_ACCESS;
	default:
		return H_ERR_UNKNOWN;
	}
}

h_err* _h_err_create(h_err_type type, char* msg, int line, char* file)
{
	h_err* err = malloc(sizeof(h_err));
	if (err == NULL) {
		puts("Fatal error: failed to allocate memory.");
		exit(1);
	}

	err->type = type;
	err->msg = msg;
	err->line = line;
	err->file = file;

	return err;
}

h_err* _h_err_from_errno(int err, char* msg, int line, char* file)
{
	printf("creating from errno, %i\n", err);
	return _h_err_create(type_from_errno(err), msg, line, file);
}

void h_err_print(h_err* err)
{
	if (!err)
		return;

#ifdef DEBUG
	fprintf(stderr, "File %s, line %i:\n\t", err->file, err->line);
#endif

	if (err->msg)
		fprintf(stderr, "Error: %s: %s\n", str_from_err(err), err->msg);
	else
		fprintf(stderr, "Error: %s\n", str_from_err(err));
}
