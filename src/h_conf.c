#include "h_conf.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <stdio.h>

static int iskey(char c)
{
	return (isalnum(c) || c == '_');
}

static void put_pair(h_conf* conf, char* key, char* val)
{
	if (strcmp(key, "title") == 0)
		conf->title = val;
}

h_conf* h_conf_parse(char* str, int len)
{
	h_conf* conf = malloc(sizeof(h_conf));

	int offset_key = -1;
	int offset_key_end = -1;
	int offset_val = -1;

	int i;
	for (i = 0; i < len; ++i)
	{
		char c = str[i];

		if (offset_key == -1 && iskey(c))
		{
			offset_key = i;
		}
		else if (c == ':')
		{
			str[i] = '\0';
			offset_key_end = i;
		}
		else if (offset_key_end != -1 && iskey(c))
		{
			offset_val = i;
			offset_key_end = -1;
		}
		else if (c == '\n')
		{
			str[i] = '\0';

			put_pair(conf, str + offset_key, str + offset_val);

			offset_key = -1;
			offset_val = -1;
		}
	}

	return conf;
}
