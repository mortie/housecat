#include "h_conf.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <stdio.h>

static int streq(char* s1, char* s2)
{
	return (strcmp(s1, s2) == 0);
}

static int iskey(char c)
{
	return (isalnum(c) || c == '_');
}

static void put_pair(h_conf* conf, char* key, char* val)
{
	if (streq(key, "title"))
	{
		conf->title = val;
	}
	else if (streq(key, "posts_per_page"))
	{
		conf->posts_per_page = atoi(val);
	}
	else if (streq(key, "logo"))
	{
		if (streq(val, "true"))
			conf->logo = 1;
		else
			conf->logo = 0;
	}
	else if (streq(key, "root"))
	{
		if (val[0] == '/' && val[1] == '\0')
			conf->root = "";
		else
			conf->root = val;
	}
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
		else if (c == ':' && offset_val == -1)
		{
			str[i] = '\0';
			offset_key_end = i;
		}
		else if (offset_key_end != -1 && !isspace(c))
		{
			offset_val = i;
			offset_key_end = -1;
		}
		else if (offset_val != -1 && c == '\n')
		{
			str[i] = '\0';

			put_pair(conf, str + offset_key, str + offset_val);

			offset_key = -1;
			offset_val = -1;
		}
	}

	return conf;
}
