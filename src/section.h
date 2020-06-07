#ifndef H_SECTION_H
#define H_SECTION_H

#include "err.h"
#include "post.h"

// Forward declarations
struct h_rss_section;

typedef struct h_section
{
	char* title;
	char* slug;

	h_post** posts;
	int numposts;

	h_post** drafts;
	int numdrafts;

	struct h_section** subs;
	int numsubs;

	char* path;
	int depth;

	char* rpath;
	struct h_rss_section* rss_metadata;
	char* rss;
} h_section;

//Initiate a section from directory
h_err* h_section_init_from_dir(h_section* section, char* path);

//Add a post
h_err* h_section_add_post(h_section* section, h_post* post);

//Add a draft post
h_err* h_section_add_draft(h_section* section, h_post* post);

//Add a sub section
h_err* h_section_add_sub(h_section* section, h_section* sub);

#endif
