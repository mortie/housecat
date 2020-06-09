#include "conf.h"
#include "util.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <stdio.h>

static int iskey(char c)
{
	return (isalnum(c) || c == '_');
}

void h_conf_build(void* c, const char* key, const char* val)
{
	h_conf* conf = c;
	if (h_util_streq(key, "title"))
	{
    conf->title = realloc(conf->title, strlen(val) + 1);
    strcpy(conf->title, val);
	}
	else if (h_util_streq(key, "posts_per_page"))
	{
		conf->posts_per_page = atoi(val);
	}
	else if (h_util_streq(key, "logo"))
	{
		if (h_util_streq(val, "true"))
			conf->logo = 1;
		else
			conf->logo = 0;
	}
	else if (h_util_streq(key, "root"))
	{
		if (val[0] == '/' && val[1] == '\0')
		{
			conf->root = realloc(conf->root, 1);
			conf->root[0] = '\0';
		}
		else
		{
			conf->root = realloc(conf->root, strlen(val) + 1);
			strcpy(conf->root, val);
		}
	}
	else if (h_util_streq(key, "rss"))
	{
		if (h_util_streq(val, "true"))
			conf->rss = 1;
		else
			conf->rss = 0;
	}
	else if (h_util_streq(key, "rss_drafts"))
	{
		if (h_util_streq(val, "true"))
			conf->rss_drafts = 1;
		else
			conf->rss_drafts = 0;
	}
	else if (h_util_streq(key, "rss_level"))
	{
		if (h_util_streq(val, "subsection"))
			conf->rss_level = H_RSS_SUBSECTION;
		else if (h_util_streq(val, "section"))
			conf->rss_level = H_RSS_SECTION;
		else
			conf->rss_level = H_RSS_GLOBAL;
	}
	else if (h_util_streq(key, "url"))
	{
		if (val[0] == '\0')
		{
			if (conf->url != NULL)
				free(conf->url);
			conf->url = NULL;
		}
		else
		{
			const size_t urlsize = strlen(val);
			if (urlsize >= 1 && val[urlsize - 1] == '/')
			{
				conf->url = realloc(conf->url, urlsize);
				memcpy(conf->url, val, urlsize);
				conf->url[urlsize - 1] = '\0';
			}
			else
			{
				conf->url = realloc(conf->url, urlsize + 1);
				strcpy(conf->url, val);
			}
		}
	}
	else if (h_util_streq(key, "use_guid"))
	{
		if (h_util_streq(val, "true"))
			conf->use_guid = 1;
		else
			conf->use_guid = 0;
	}
	else if (h_util_streq(key, "use_pubdate"))
	{
		if (h_util_streq(val, "true"))
			conf->use_pubdate = 1;
		else
			conf->use_pubdate = 0;
	}
	else if (h_util_streq(key, "webmaster"))
	{
		if (val[0] == '\0')
			conf->webmaster = NULL;
		else
		{
			conf->webmaster = realloc(conf->webmaster, strlen(val) + 1);
			strcpy(conf->webmaster, val);
		}
	}
}

h_err* h_conf_parse(char* str, int len, void* data, h_confpair_func pair)
{
	if (!str || !data)
		return h_err_create(H_ERR_OTHER, NULL);

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
		else if (offset_val != -1 && (c == '\n' || c == '\0'))
		{
			str[i] = '\0';

			pair(data, str + offset_key, str + offset_val);

			offset_key = -1;
			offset_val = -1;
		}
	}

	return NULL;
}

h_conf* h_conf_create()
{
	h_conf* conf = malloc(sizeof(*conf));
	if (conf == NULL)
		return NULL;

	conf->title = NULL;
	conf->posts_per_page = 5;
	conf->logo = 0;
	conf->root = NULL;
	conf->rss = 0;
	conf->rss_drafts = 0;
	conf->url = NULL;
	conf->rss_level = H_RSS_GLOBAL;
	conf->use_guid = 1;
	conf->use_pubdate = 0;
	conf->webmaster = NULL;
	return conf;
}

void h_conf_free(h_conf* conf)
{
	if (conf == NULL)
		return;

	free(conf->title);
	free(conf->root);
	free(conf->url);
	free(conf->webmaster);
	free(conf);
}
