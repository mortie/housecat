#include "h_template.h"

#include <stdlib.h>
#include <string.h>

#include <stdio.h>

h_template_args* h_template_args_create()
{
	h_template_args* args = malloc(sizeof(h_template_args));

	args->arguments = NULL;
	args->argnum = 0;
	args->allocd = 0;

	return args;
}

h_err* h_template_args_append(h_template_args* args, char* key, char* val)
{
	if (val == NULL)
		return h_err_create(H_ERR_UNKNOWN, key);
	args->argnum += 1;
	if (args->argnum > args->allocd)
	{
		if (!args->allocd)
			args->allocd = 1;
		else
			args->allocd *= 2;
		args->arguments = realloc(
			args->arguments,
			sizeof(h_template_arg) * args->allocd
		);
		if (!args->arguments)
			return h_err_create(H_ERR_ALLOC, NULL);
	}
	//Wrap key in {{ and }}
	int len = strlen(key);
	char* fullkey = malloc(len + 5);
	memcpy(fullkey, "{{", 2);
	memcpy(fullkey + 2, key, len);
	memcpy(fullkey + len + 2, "}}", 2);
	fullkey[len + 4] = '\0';
	h_template_arg arg = {fullkey, val};
	args->arguments[args->argnum - 1] = arg;

	return NULL;
}

void h_template_args_free(h_template_args* args)
{
	int i;
	for (i = 0; i < args->argnum; ++i)
	{
		free(args->arguments[i].key);
	}

	free(args->arguments);
	free(args);
}

//Copied from Stack Overflow, http://stackoverflow.com/a/779960
//Modified to fit my code style
static char* str_replace(char* orig, char* rep, char* with) {
	char* result;  // the return string
	char* ins;     // the next insert point
	char* tmp;     // varies
	int len_rep;   // length of rep
	int len_with;  // length of with
	int len_front; // distance between rep and end of last rep
	int count;     // number of replacements

	if (!orig)
		return NULL;
	if (!rep)
		rep = "";
	len_rep = strlen(rep);
	if (!with)
		with = "";
	len_with = strlen(with);

	ins = orig;
	for (count = 0; (tmp = strstr(ins, rep)); ++count) {
		ins = tmp + len_rep;
	}

	// first time through the loop, all the variable are set correctly
	// from here on,
	//	tmp points to the end of the result string
	//	ins points to the next occurrence of rep in orig
	//	orig points to the remainder of orig after "end of rep"
	tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

	if (!result)
		return NULL;

	while (count--) {
		ins = strstr(orig, rep);
		len_front = ins - orig;
		tmp = strncpy(tmp, orig, len_front) + len_front;
		tmp = strcpy(tmp, with) + len_with;
		orig += len_front + len_rep; // move to next "end of rep"
	}
	strcpy(tmp, orig);
	return result;
}

char* h_templateify(char* str, h_template_args* args)
{
	int i;
	for (i = 0; i < args->argnum; ++i)
	{
		h_template_arg arg = args->arguments[i];
		str = str_replace(str, arg.key, arg.val);
	}

	return str;
}
