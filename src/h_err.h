#ifndef H_ERR_H
#define H_ERR_H

typedef enum h_err_type
{
	H_ERR_UNKNOWN,
	H_ERR_OTHER,
	H_ERR_ALLOC,
	H_ERR_ACCESS
} h_err_type;

typedef struct h_err
{
	h_err_type type;
	char* msg;
	int line;
	char* file;
} h_err;

h_err* _h_err_create(h_err_type type, char* msg, int line, char* file);
#define h_err_create(type, msg) _h_err_create(type, msg, __LINE__, __FILE__)

h_err* _h_err_from_errno(int err, char* msg, int line, char* file);
#define h_err_from_errno(err, msg) _h_err_from_errno(err, msg, __LINE__, __FILE__)

void h_err_print(h_err* err);

#endif
