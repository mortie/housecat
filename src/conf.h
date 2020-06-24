#ifndef H_CONF_H
#define H_CONF_H

#include "err.h"

// rss defines are here to avoid a circular include
// of conf.h-rss.h
enum h_rsslevel
{
	H_RSS_GLOBAL,
	H_RSS_SECTION,
	H_RSS_SUBSECTION
};

typedef struct h_conf
{
	char* title;
	int posts_per_page;
	int logo;
	char* root;

	int rss;
	int rss_drafts;
	int rss_fullcontent;
	char* url;
	enum h_rsslevel rss_level;
	int use_guid;
	int use_pubdate;
	char* webmaster;
} h_conf;

typedef void (*h_confpair_func)(void*, const char*, const char*);

void h_conf_build(void* c, const char* key, const char* val);
h_err* h_conf_parse(char* str, int len, void* data, h_confpair_func pair);
h_conf* h_conf_create();
void h_conf_free(h_conf* conf);

#endif
