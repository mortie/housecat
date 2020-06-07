#include "rss.h"
#include "util.h"

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

typedef struct strbuf {
	char* str;
	size_t len;
} strbuf;

#define APPEND(x, y) \
	do \
	{ \
		const size_t tmp = strlen(y); \
		if (x.str) \
		{ \
			x.str = realloc(x.str, x.len + tmp + 1); \
			strcat(x.str, y); \
			x.len += tmp; \
		} \
		else \
		{ \
			x.str = malloc(tmp + 1); \
			strcpy(x.str, y); \
			x.len = tmp; \
		} \
	} while(0);

static void build_channel_data(void* r, char* key, char* val)
{
	h_rss_section* rss = r;
	if (h_util_streq(key, "description"))
	{
		rss->description = val;
	}
	else if (h_util_streq(key, "title"))
	{
		rss->title = val;
	}
	else if (h_util_streq(key, "language"))
	{
		rss->language = val;
	}
	else if (h_util_streq(key, "editor"))
	{
		rss->editor = val;
	}
	else if (h_util_streq(key, "copyright"))
	{
		rss->copyright = val;
	}
	else if (h_util_streq(key, "img"))
	{
		rss->img = val;
	}
	else if (h_util_streq(key, "ttl"))
	{
		rss->ttl = val;
	}
	else if (h_util_streq(key, "category"))
	{
		size_t category_length;
		for (category_length = 0; rss->category[category_length] != NULL; ++category_length);

		rss->category = realloc(rss->category, (category_length+2)*sizeof(*rss->category));
		rss->category[category_length] = val;
		rss->category[category_length+1] = NULL;
	}
}

static h_err* print_channel(FILE *f, h_section* section)
{
	if (section->rss)
		fputs(section->rss, f);
	return NULL;
}

static void print_end(FILE *f)
{
	fputs("</rss>\n", f);
}

static void print_recursive(FILE* f, h_section* section)
{
	if (!f || !section)
		return;

	print_channel(f, section);
	for (int i=0; i < section->numsubs; ++i)
		print_recursive(f, section->subs[i]);
}

static void print_start(FILE *f)
{
	fputs("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n", f);
	fputs("<rss version=\"2.0\">\n", f);
}

static h_err* build_feed_global(h_section* root, const h_conf* conf)
{
	if (root == NULL || conf == NULL)
		return NULL;
	h_err* err = h_rss_init_channel(root, conf, 1);
	if (err)
		return err;

	char* feed_path = h_util_path_join(conf->root, "feed.rss");
	if (feed_path == NULL)
		return h_err_from_errno(errno, conf->root);

	FILE* f = fopen(feed_path, "w");
	if (f == NULL)
		return h_err_from_errno(errno, feed_path);
	free(feed_path);

	print_start(f);
	print_channel(f, root);
	print_end(f);
	fclose(f);
	return NULL;
}

static h_err* build_feed_section(h_section* root, const h_conf* conf)
{
	if (!root || !conf)
		return NULL;

	// init the channels of the main sections
	for (int i=0; i < root->numsubs; ++i)
	{
		h_err* err = h_rss_init_channel(root->subs[i], conf, 1);
		if (err)
			return err;
	}

	// Get the rss data for any top level posts
	h_err* err = h_rss_init_channel(root, conf, 0);
	if (err)
		return err;

	// Print out to the appropiate files
	if (conf->combine_feeds)
	{
		char* feed_path = h_util_path_join(conf->root, "feed.rss");
		if (feed_path == NULL)
			return h_err_from_errno(errno, conf->root);

		FILE* f = fopen(feed_path, "w");
		if (f == NULL)
			return h_err_from_errno(errno, feed_path);
		free(feed_path);

		print_start(f);
		print_channel(f, root);
		for (int i=0; i < root->numsubs; ++i)
			print_channel(f, root->subs[i]);
		print_end(f);
		fclose(f);
	}
	else
	{
		char* feed_path = h_util_path_join(conf->root, "feed.rss");
		if (feed_path == NULL)
			return h_err_from_errno(errno, conf->root);
		FILE* f = fopen(feed_path, "w");
		if (f == NULL)
			return h_err_from_errno(errno, feed_path);
		free(feed_path);

		print_start(f);
		print_channel(f, root);
		print_end(f);
		fclose(f);

		for (int i=0; i < root->numsubs; ++i)
		{
			char* section_path = h_util_path_join(conf->root, root->subs[i]->path);
			if (section_path == NULL)
				return h_err_from_errno(errno, conf->root);
			feed_path = h_util_path_join(section_path, "feed.rss");
			if (feed_path == NULL)
				return h_err_from_errno(errno, section_path);
			free(section_path);

			f = fopen(feed_path, "w");
			if (f == NULL)
				return h_err_from_errno(errno, feed_path);
			free(feed_path);

			print_start(f);
			print_channel(f, root->subs[i]);
			print_end(f);
			fclose(f);
		}
	}

	return NULL;
}

// Recursively go through sections making channels, not
// recursing on the channel creation so each channel only has
// its top level posts
static h_err* rss_channel_recurse(h_section* section, const h_conf* conf, int write)
{
	if (!section || !conf)
		return NULL;

	h_err* err = h_rss_init_channel(section, conf, 0);
	if (err)
		return err;

	for (int i=0; i < section->numsubs; ++i)
	{
		err = rss_channel_recurse(section->subs[i], conf, write);
		if (err)
			return err;
	}

	if (write)
	{
		char* section_path = h_util_path_join(conf->root, section->path);
		if (section_path == NULL)
			return h_err_from_errno(errno, conf->root);
		char* feed_path = h_util_path_join(section_path, "feed.rss");
		if (feed_path == NULL)
			return h_err_from_errno(errno, section_path);
		free(section_path);

		FILE* f = fopen(feed_path, "w");
		if (f == NULL)
			return h_err_from_errno(errno, feed_path);
		free(feed_path);

		print_start(f);
		print_channel(f, section);
		print_end(f);
		fclose(f);
	}

	return NULL;
}

static h_err* build_feed_subsection(h_section* root, const h_conf* conf)
{
	if (!root || !conf)
		return NULL;

	h_err* err = rss_channel_recurse(root, conf, !conf->combine_feeds);
	if (err)
		return err;

	if (conf->combine_feeds)
	{
		char* feed_path = h_util_path_join(conf->root, "feed.rss");
		if (feed_path == NULL)
			return h_err_from_errno(errno, conf->root);

		FILE* f = fopen(feed_path, "w");
		if (f == NULL)
			return h_err_from_errno(errno, feed_path);
		free(feed_path);

		print_start(f);
		print_recursive(f, root);
		print_end(f);
		fclose(f);
	}

	return NULL;
}

h_err* h_rss_aggregate(h_section* section)
{
	if (!section)
		return NULL;

	strbuf builder = {NULL, 0};

	for (int i=0; i < section->numsubs; ++i)
	{
		h_err* err = h_rss_aggregate(section->subs[i]);
		if (err)
		{
			free(builder.str);
			return err;
		}
		if (section->subs[i]->rss)
			APPEND(builder, section->subs[i]->rss);
	}

	for (int i=0; i < section->numposts; ++i)
	{
		if (section->posts[i]->rss)
			APPEND(builder, section->posts[i]->rss)
	}

	// rss will be NULL if rss_drafts is false
	for (int i=0; i < section->numdrafts; ++i)
	{
		if (section->drafts[i]->rss)
			APPEND(builder, section->drafts[i]->rss)
	}

	section->rss = builder.str;
	return NULL;
}

h_err* h_rss_build(h_section* root, const h_conf* conf)
{
	switch (conf->rss_level)
	{
	case H_RSS_SUBSECTION:
		return build_feed_subsection(root, conf);
	case H_RSS_SECTION:
		return build_feed_section(root, conf);
	default:
		return build_feed_global(root, conf);
	}
}

h_err* h_rss_configure(h_section* section, const h_rss_section* inherit)
{
	section->rss_metadata = malloc(sizeof(*section->rss_metadata));
	section->rss_metadata->title = NULL;
	section->rss_metadata->description = NULL;
	section->rss_metadata->language = NULL;
	section->rss_metadata->editor = NULL;
	section->rss_metadata->copyright = NULL;
	section->rss_metadata->img = NULL;
	section->rss_metadata->ttl = NULL;
	section->rss_metadata->category = malloc(sizeof(*section->rss_metadata->category));
	section->rss_metadata->category[0] = NULL;
	if (inherit != NULL)
	{
		if (inherit->title)
			section->rss_metadata->title = inherit->title;
		if (inherit->description)
			section->rss_metadata->description = inherit->description;
		if (inherit->language)
			section->rss_metadata->language = inherit->language;
		if (inherit->editor)
			section->rss_metadata->editor = inherit->editor;
		if (inherit->copyright)
			section->rss_metadata->copyright = inherit->copyright;
		if (inherit->img)
			section->rss_metadata->img = inherit->img;
		if (inherit->ttl)
			section->rss_metadata->ttl = inherit->ttl;
		if (inherit->category)
		{
			size_t category_length;
			for (category_length = 0; inherit->category[category_length] != NULL; ++category_length);
			section->rss_metadata->category =
				realloc(section->rss_metadata->category, sizeof(*section->rss_metadata->category)*(category_length+1));
			for (size_t i=0; i <= category_length; ++i)
				section->rss_metadata->category[i] = inherit->category[i];
		}
	}

	char* rsspath = h_util_path_join(section->rpath, "rss.conf");
	char* rssdata = h_util_file_read(rsspath);
	free(rsspath);
	if (!rssdata)
		return NULL; // rss.conf is optional

	h_err* err = h_conf_parse(rssdata, strlen(rssdata)+1, (void *)section->rss_metadata, build_channel_data);
	if (err)
		return err;

	for (int i=0; i < section->numsubs; ++i)
		h_rss_configure(section->subs[i], section->rss_metadata);

	return NULL;
}

h_err* h_rss_init_channel(h_section* section, const h_conf* conf, int recurse)
{
	strbuf builder = {NULL, 0};

	// Required tags
	APPEND(builder, "<channel>\n\t<title>")
	if (section->rss_metadata->title != NULL)
		APPEND(builder, section->rss_metadata->title)
	APPEND(builder, "</title>\n\t<link>")
	if (conf->url != NULL)
	{
		char* urlpath = h_util_path_join(conf->url, section->path);
		APPEND(builder, urlpath)
		free(urlpath);
	}
	else
		APPEND(builder, section->path)
	APPEND(builder, "</link>\n\t<description>")
	if (section->rss_metadata->description != NULL)
		APPEND(builder, section->rss_metadata->description)
	APPEND(builder, "</description>\n")

	// optional tags
	if (section->rss_metadata->language != NULL)
	{
		APPEND(builder, "\t<language>")
		APPEND(builder, section->rss_metadata->language)
		APPEND(builder, "</language>\n")
	}

	if (section->rss_metadata->editor != NULL)
	{
		APPEND(builder, "\t<managingEditor>")
		APPEND(builder, section->rss_metadata->editor)
		APPEND(builder, "</managingEditor>\n")
	}

	if (section->rss_metadata->copyright != NULL)
	{
		APPEND(builder, "\t<copyright>")
		APPEND(builder, section->rss_metadata->copyright)
		APPEND(builder, "</copyright>\n")
	}

	if (section->rss_metadata->ttl != NULL)
	{
		APPEND(builder, "\t<ttl>")
		APPEND(builder, section->rss_metadata->ttl)
		APPEND(builder, "</ttl>\n")
	}

	if (section->rss_metadata->img != NULL)
	{
		APPEND(builder, "\t<image>")
		APPEND(builder, section->rss_metadata->img)
		APPEND(builder, "</image>\n")
	}

	for (int i=0; section->rss_metadata->category[i] != NULL; ++i)
	{
		APPEND(builder, "\t<category>")
		APPEND(builder, section->rss_metadata->category[i])
		APPEND(builder, "</category>\n")
	}

	struct timespec ts;
	if (conf->use_pubdate && (clock_gettime(CLOCK_REALTIME, &ts) == 0))
	{
		APPEND(builder, "\t<pubDate>")
		// Conforming to https://www.ietf.org/rfc/rfc822.txt , section 5
		// (if local settings are right)
		tzset();
		struct tm* ltime = localtime(&ts.tv_sec);
		char timebuffer[64];
		strftime(
			&timebuffer[0],
			sizeof(timebuffer) / sizeof(timebuffer[0]),
			"%a, %e %B %y %H:%M:%S %Z",
			ltime
		);

		APPEND(builder, &timebuffer[0])
		APPEND(builder, "</pubDate>\n")
	}

	if (conf->webmaster != NULL)
	{
		APPEND(builder, "\t<webMaster>")
		APPEND(builder, conf->webmaster)
		APPEND(builder, "</webMaster>\n")
	}

	APPEND(builder, "\t<generator>housecat</generator>\n\t<docs>https://cyber.harvard.edu/rss/rss.html</docs>\n")

	// Now that the channel settings are set up, generate the actual items
	if (recurse)
		for (int i=0; i < section->numsubs; ++i)
		{
			h_err* err = h_rss_aggregate(section->subs[i]);
			if (err)
			{
				free(builder.str);
				return err;
			}
			if (section->subs[i]->rss != NULL)
				APPEND(builder, section->subs[i]->rss)
		}

	for (int i=0; i < section->numposts; ++i)
	{
		if (section->posts[i]->rss != NULL)
			APPEND(builder, section->posts[i]->rss)
	}

	for (int i=0; i < section->numdrafts; ++i)
	{
		if (section->drafts[i]->rss != NULL)
			APPEND(builder, section->drafts[i]->rss)
	}

	APPEND(builder, "</channel>\n")
	section->rss = builder.str;
	return NULL;
}

h_err* h_rss_init_item(h_post* post, const h_conf* conf)
{
	strbuf builder = {NULL, 0};

	APPEND(builder, "\t<item>\n\t\t<title>")
	APPEND(builder, post->title)
	APPEND(builder, "</title>\n\t\t<link>")
	if (conf->url != NULL)
	{
		char* urlpath = h_util_path_join(conf->url, post->path);
		APPEND(builder, urlpath);
		free(urlpath);
	}
	else
	{
		APPEND(builder, post->path);
	}
	APPEND(builder, "</link>\n\t\t<description>")

	if (post->rss_metadata->description)
	{
		APPEND(builder, post->rss_metadata->description);
	}
	APPEND(builder, "</description>\n")

	if (post->rss_metadata->author)
	{
		APPEND(builder, "\t\t<author>")
		APPEND(builder, post->rss_metadata->author)
		APPEND(builder, "</author>\n")
	}

	if (conf->use_guid)
	{
		if (conf->url != NULL)
		{
			char* urlpath = h_util_path_join(conf->url, post->path);
			APPEND(builder, "\t\t<guid isPermaLink=\"true\">")
			APPEND(builder, urlpath)
			free(urlpath);
		}
		else
		{
			APPEND(builder, "\t\t<guid isPermaLink=\"false\">")
			APPEND(builder, post->path)
		}
		APPEND(builder, "</guid>\n")
	}

	if (post->rss_metadata->date)
	{
		APPEND(builder, "\t\t<pubDate>")
		APPEND(builder, post->rss_metadata->date)
		APPEND(builder, "</pubDate>\n")
	}

	for (size_t i = 0; post->rss_metadata->category[i] != NULL; ++i)
	{
		APPEND(builder, "\t\t<category>")
		APPEND(builder, post->rss_metadata->category[i])
		APPEND(builder, "</category>\n")
	}

	APPEND(builder, "\t</item>\n")
  post->rss = builder.str;
	return NULL;
}
