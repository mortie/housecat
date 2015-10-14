#ifndef H_BUILD_H
#define H_BUILD_H

#include "h_err.h"
#include "h_section.h"

struct h_build_strs
{
	char* index;
	char* section;
	char* post;
	char* menu;
	char* menu_section;
};

h_err* h_build(h_section* root, char* dirpath, struct h_build_strs str);

#endif
