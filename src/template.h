#ifndef CMS_TEMPLATE_H
#define CMS_TEMPLATE_H

#include "err.h"

typedef struct h_template_arg
{
	char* key;
	char* val;
} h_template_arg;

typedef struct h_template_args
{
	h_template_arg* arguments;
	int argnum;
	int allocd;
} h_template_args;

h_template_args* h_template_args_create();

h_err* h_template_args_append(h_template_args* args, const char* key, const char* val);

void h_template_args_free(h_template_args* args);

char* h_templateify(const char* str, h_template_args* args);

#endif
