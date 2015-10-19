#ifndef H_SECTION_H
#define H_SECTION_H

#include "h_err.h"
#include "h_post.h"

typedef struct h_section
{
	char* title;
	char* slug;
	h_post** posts;
	int numposts;
	struct h_section** subs;
	int numsubs;
	char* path;
	int depth;
} h_section;

//Initiate a section from directory
h_err* h_section_init_from_dir(h_section* section, char* path);

//Add a post
h_err* h_section_add_post(h_section* section, h_post* post);

//Add a sub section
h_err* h_section_add_sub(h_section* section, h_section* sub);

#endif
