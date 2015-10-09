#ifndef H_PAGE_H
#define H_PAGE_H

#include "h_err.h"

typedef struct h_post
{
	char* _fstr;
	char* title;
	char* slug;
	char* html;
} h_post;

//Initiate a post
void h_post_init(h_post* post, char* title, char* html, char* slug);

//Initiate a post from file
h_err* h_post_init_from_file(h_post* post, char* path);

#endif
