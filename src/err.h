#ifndef H_ERR_H
#define H_ERR_H

#include <errno.h>

typedef enum h_err_type
{
	H_ERR_UNKNOWN,
	H_ERR_OTHER,
	H_ERR_ALLOC,
	H_ERR_ACCESS,
	H_ERR_FORMAT_NOTITLE,
	H_ERR_ERRNO
} h_err_type;

typedef struct h_err
{

	h_err_type type;
	int errno_err;

	char* msg;
	int line;
	char* file;
} h_err;

h_err* _h_err_create(h_err_type type, const char* msg, int line, const char* file);
#define h_err_create(type, msg) _h_err_create(type, msg, __LINE__, __FILE__)

h_err* _h_err_from_errno(int err, const char* msg, int line, const char* file);
#define h_err_from_errno(err, msg) _h_err_from_errno(err, msg, __LINE__, __FILE__)

void h_err_print(const h_err* err);

void h_err_free(h_err* err);

#endif
