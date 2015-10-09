#include "h_template.h"

#include <stdlib.h>
#include <string.h>

#include <stdio.h>

h_template_args* h_template_args_create()
{
	h_template_args* args = malloc(sizeof(h_template_args));
	if (!args)
		return NULL;

	args->arguments = NULL;
	args->argnum = 0;
	args->allocd = 0;

	return args;
}

h_err* h_template_args_append(h_template_args* args, char* key, char* val)
{
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

static char* str_insert(
		char* str1,
		char* str2,
		int start,
		int end,
		int str1len)
{
	int str2len;
	if (str2 == NULL)
		str2len = 0;
	else
		str2len = strlen(str2);

	int targetlen = str1len + str2len - (end - start);

	char* str = malloc(targetlen * sizeof(char));
	if (str1 == NULL)
		return NULL;
	str = memcpy(str, str1, start);
	if (str == NULL)
		return NULL;

	str[targetlen - 1] = '\0';

	memmove(str + start + str2len, str1 + end, str1len - end - 1);
	if (str2len > 0)
		memcpy(str + start, str2, str2len);

	return str;
}

static char* templateify_arg(char* str, h_template_arg arg, int len)
{
	#define OFFSET i - match_start

	int match_start = 0;
	int in_match = 0;

	char c;
	int i;
	for (i = 0; i < len; ++i)
	{
		c = str[i];

		if (!in_match && arg.key[0] == c)
		{
			match_start = i;
			in_match = 1;
		}

		if (in_match && arg.key[OFFSET] != 0)
		{
			if (arg.key[OFFSET] != c)
				in_match = 0;
		}
		else if (in_match)
		{
			char* str2 = str_insert(str, arg.val, match_start, i, len);
			free(str);
			str = str2;

			if (str == NULL)
				return NULL;

			len = strlen(str);

			in_match = 0;
		}
	}

	#undef OFFSET

	return str;
}

char* h_templateify(char* fstr, h_template_args* args)
{
	int len = strlen(fstr) + 1;

	char* str = malloc(len);
	if (str == NULL)
		return NULL;

	memcpy(str, fstr, len);
	str[len - 1] = '\0';

	int i;
	for (i = 0; i < args->argnum; ++i)
	{
		str = templateify_arg(str, args->arguments[i], len);
		if (str == NULL)
			return NULL;

		len = strlen(str);
	}
	return str;
}
