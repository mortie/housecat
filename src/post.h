#ifndef H_PAGE_H
#define H_PAGE_H

#include "err.h"

#include <sys/stat.h>

// Forward declarations
struct h_rss_post;

typedef struct h_post
{
	char* _fstr;
	char* title;
	char* slug;
	char* html;
	char* path;
	int depth;
	int isdraft;
	char* rss;
	struct h_rss_post* rss_metadata;
} h_post;

//Initiate a post from file
h_err* h_post_init_from_file(h_post* post, char* path, char* spath, int depth);

#endif
