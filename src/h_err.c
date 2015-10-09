#include "h_err.h"

#include <stdlib.h>
#include <stdio.h>

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

static char* str_from_errno(int err)
{
	switch (err)
	{
	case EPERM:
		return "Operation not permitted";
	case ENOENT:
		return "No such file or directory";
	case EIO:
		return "I/O error";
	case EACCES:
		return "Permission denied";
	case EEXIST:
		return "File exists";
	case ENOTDIR:
		return "Not a directory";
	case EISDIR:
		return "Is a directory";
	case EFBIG:
		return "File too large";
	case ENOSPC:
		return "No space left on device";
	case EMLINK:
		return "Too many links";
	case EPIPE:
		return "Broken pipe";
	default:
		return "Unknown error";
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
	err->errno_err = 0;
	err->msg = msg;
	err->line = line;
	err->file = file;

	return err;
}

h_err* _h_err_from_errno(int errno_err, char* msg, int line, char* file)
{
	h_err* err = _h_err_create(H_ERR_ERRNO, msg, line, file);
	err->errno_err = errno_err;
	return err;
}

void h_err_print(h_err* err)
{
	if (!err)
		return;

#ifdef DEBUG
	fprintf(stderr, "File %s, line %i:\n\t", err->file, err->line);
#endif

	if (err->type == H_ERR_ERRNO)
		fprintf(stderr, "Error: %s", str_from_errno(err->errno_err));
	else
		fprintf(stderr, "Error: %s", str_from_err(err));

	if (err->msg)
		fprintf(stderr, ": %s\n", err->msg);
	else
		fprintf(stderr, "\n");
}
