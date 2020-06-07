#ifndef H_RSS_H
#define H_RSS_H

#include "err.h"
#include "post.h"
#include "section.h"
#include "conf.h"

typedef struct h_rss_section
{
	char* title;
	char* description;
	char* language;
	char* editor;
	char* copyright;
	char** category;
	char* img;
	char* ttl;
} h_rss_section;

typedef struct h_rss_post
{
	char* author;
	char* description;
	char* date;
	char** category;
} h_rss_post;

// Put all posts under this entire section, including subsections,
// into the section's rss string
h_err* h_rss_aggregate(h_section* section);

// Build the actual rss feeds based on the configuration
h_err* h_rss_build(h_section* root, const h_conf* conf);

// Read in channel configuration file recursively
// Start with the root section and NULL inheritence
h_err* h_rss_configure(h_section* section, const h_rss_section* inherit);

// Fill a section with an rss channel
// aggregates the items in the subsections
h_err* h_rss_init_channel(h_section* section, const h_conf* conf, int recurse);

// Fill a post with an rss <item>
// The final post's html path is *path
h_err* h_rss_init_item(h_post* post, const h_conf* conf);

#endif
