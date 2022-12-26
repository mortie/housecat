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

static void append_span(strbuf *buf, const char *str, size_t len)
{
	if (buf->str)
	{
		buf->str = realloc(buf->str, buf->len + len + 1);
		strncat(buf->str, str, len);
		buf->len += len;
		buf->str[buf->len] = '\0';
	}
	else
	{
		buf->str = malloc(len + 1);
		strncpy(buf->str, str, len);
		buf->len = len;
		buf->str[buf->len] = '\0';
	}
}

static void append(strbuf *buf, const char *str)
{
	append_span(buf, str, strlen(str));
}

static void append_truncated_html(strbuf *buf, const char *html)
{
	const char *magic = "<!--RSS_END-->";
	size_t magic_length = strlen(magic);

	size_t line_start = 0;
	char ch;
	for (size_t i = 0; (ch = html[i]); ++i)
	{
		if (ch == '\r' || ch == '\n')
		{
			const char *line = &html[line_start];
			size_t line_length = i - line_start;

			if (magic_length == line_length && strncmp(magic, line, line_length) == 0)
			{
				append_span(buf, html, line_start - 1);
				return;
			}

			if (ch == '\n')
				line_start = i + 1;
		}
	}

	append(buf, html);
}

static void build_channel_data(void* r, const char* key, const char* val)
{
	h_rss_section* rss = r;
	if (h_util_streq(key, "description"))
	{
		rss->description = realloc(rss->description, strlen(val) + 1);
		strcpy(rss->description, val);
	}
	else if (h_util_streq(key, "title"))
	{
		rss->title = realloc(rss->title, strlen(val) + 1);
		strcpy(rss->title, val);
	}
	else if (h_util_streq(key, "language"))
	{
		rss->language = realloc(rss->language, strlen(val) + 1);
		strcpy(rss->language, val);
	}
	else if (h_util_streq(key, "editor"))
	{
		rss->editor = realloc(rss->editor, strlen(val) + 1);
		strcpy(rss->editor, val);
	}
	else if (h_util_streq(key, "copyright"))
	{
		rss->copyright = realloc(rss->copyright, strlen(val) + 1);
		strcpy(rss->copyright, val);
	}
	else if (h_util_streq(key, "img"))
	{
		rss->img = realloc(rss->img, strlen(val) + 1);
		strcpy(rss->img, val);
	}
	else if (h_util_streq(key, "ttl"))
	{
		rss->ttl = realloc(rss->ttl, strlen(val) + 1);
		strcpy(rss->ttl, val);
	}
	else if (h_util_streq(key, "category"))
	{
		size_t category_length;
		for (category_length = 0; rss->category[category_length] != NULL; ++category_length);

		rss->category = realloc(rss->category, (category_length+2)*sizeof(*rss->category));
		rss->category[category_length] = malloc(strlen(val) + 1);
		strcpy(rss->category[category_length], val);
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

static void print_start(FILE *f)
{
	fputs("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n", f);
	fputs("<rss version=\"2.0\" xmlns:atom=\"http://www.w3.org/2005/Atom\">\n", f);
}

static h_err* build_feed_global(h_section* root, const h_conf* conf, const char* path)
{
	if (root == NULL || conf == NULL)
		return NULL;
	h_err* err = h_rss_init_channel(root, conf, 1);
	if (err)
		return err;

	char* feed_path = h_util_path_join(path, "feed.rss");
	if (feed_path == NULL)
		return h_err_from_errno(errno, path);

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

static h_err* build_feed_section(h_section* root, const h_conf* conf, const char* path)
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
	char* feed_path = h_util_path_join(path, "feed.rss");
	if (feed_path == NULL)
		return h_err_from_errno(errno, path);
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
		char* section_path = h_util_path_join(path, root->subs[i]->path);
		if (section_path == NULL)
			return h_err_from_errno(errno, path);
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

	return NULL;
}

// Recursively go through sections making channels, not
// recursing on the channel creation so each channel only has
// its top level posts
static h_err* rss_channel_recurse(h_section* section, const h_conf* conf, const char* path)
{
	if (!section || !conf)
		return NULL;

	h_err* err = h_rss_init_channel(section, conf, 0);
	if (err)
		return err;

	for (int i=0; i < section->numsubs; ++i)
	{
		err = rss_channel_recurse(section->subs[i], conf, path);
		if (err)
			return err;
	}

	char* section_path = h_util_path_join(path, section->path);
	if (section_path == NULL)
		return h_err_from_errno(errno, path);
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

	return NULL;
}

static h_err* build_feed_subsection(h_section* root, const h_conf* conf, const char* path)
{
	if (!root || !conf)
		return NULL;

	h_err* err = rss_channel_recurse(root, conf, path);
	if (err)
		return err;

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
			append(&builder, section->subs[i]->rss);
	}

	for (int i=0; i < section->numposts; ++i)
	{
		if (section->posts[i]->rss)
			append(&builder, section->posts[i]->rss);
	}

	// rss will be NULL if rss_drafts is false
	for (int i=0; i < section->numdrafts; ++i)
	{
		if (section->drafts[i]->rss)
			append(&builder, section->drafts[i]->rss);
	}

	section->rss = builder.str;
	return NULL;
}

void h_rss_arg(h_section* section, h_template_args* args, const h_conf* conf)
{
	if (conf->rss)
	{
		char* feed = NULL;
		switch(conf->rss_level)
		{
		case H_RSS_SUBSECTION:
			feed = h_util_path_join(section->path, "feed.rss");
			h_template_args_append(args, "feed", feed);
			break;
		case H_RSS_SECTION:
			// ; is needed so case statement is satisfied
			;const size_t path_len = strlen(section->path);
			size_t first_slash;
			for (first_slash = 0; first_slash < path_len && section->path[first_slash] != '/'; ++first_slash);
			if (path_len == 0)
			{
				h_template_args_append(args, "feed", "feed.rss");
			}
			else if (first_slash == path_len)
			{
				feed = h_util_path_join(section->path, "feed.rss");
				h_template_args_append(args, "feed", feed);
			}
			else
			{
				section->path[first_slash] = '\0';
				feed = h_util_path_join(section->path, "feed.rss");
				section->path[first_slash] = '/';
				h_template_args_append(args, "feed", feed);
			}
			break;
		default:
			h_template_args_append(args, "feed", "feed.rss");
		}
		free(feed);
	}
}

h_err* h_rss_build(h_section* root, const h_conf* conf, const char* path)
{
	switch (conf->rss_level)
	{
	case H_RSS_SUBSECTION:
		return build_feed_subsection(root, conf, path);
	case H_RSS_SECTION:
		return build_feed_section(root, conf, path);
	default:
		return build_feed_global(root, conf, path);
	}
}

h_err* h_rss_configure(h_section* section, const h_rss_section* inherit)
{
	if (inherit != NULL)
	{
		if (inherit->title)
		{
			section->rss_metadata->title = malloc(strlen(inherit->title) + 1);
			strcpy(section->rss_metadata->title, inherit->title);
		}

		if (inherit->description)
		{
			section->rss_metadata->description = malloc(strlen(inherit->description) + 1);
			strcpy(section->rss_metadata->description, inherit->description);
		}

		if (inherit->language)
		{
			section->rss_metadata->language = malloc(strlen(inherit->language) + 1);
			strcpy(section->rss_metadata->language, inherit->language);
		}

		if (inherit->editor)
		{
			section->rss_metadata->editor = malloc(strlen(inherit->editor) + 1);
			strcpy(section->rss_metadata->editor, inherit->editor);
		}

		if (inherit->copyright)
		{
			section->rss_metadata->copyright = malloc(strlen(inherit->copyright) + 1);
			strcpy(section->rss_metadata->copyright, inherit->copyright);
		}

		if (inherit->img)
		{
			section->rss_metadata->img = malloc(strlen(inherit->img) + 1);
			strcpy(section->rss_metadata->img, inherit->img);
		}

		if (inherit->ttl)
		{
			section->rss_metadata->ttl = malloc(strlen(inherit->ttl) + 1);
			strcpy(section->rss_metadata->ttl, inherit->ttl);
		}

		if (inherit->category)
		{
			size_t category_length;
			for (category_length = 0; inherit->category[category_length] != NULL; ++category_length);

			section->rss_metadata->category =
				realloc(section->rss_metadata->category, sizeof(*section->rss_metadata->category)*(category_length+1));

			for (size_t i=0; i < category_length; ++i)
			{
				section->rss_metadata->category[i] = malloc(strlen(inherit->category[i]) + 1);
				strcpy(section->rss_metadata->category[i], inherit->category[i]);
			}
			section->rss_metadata->category[category_length] = NULL;
		}
	}

	char* rsspath = h_util_path_join(section->rpath, "rss.conf");
	if (rsspath == NULL)
		return h_err_create(H_ERR_ALLOC, NULL);
	char* rssdata = h_util_file_read(rsspath);
	free(rsspath);
	if (rssdata == NULL)
		return NULL;

	h_err* err = h_conf_parse(rssdata, strlen(rssdata)+1, (void *)section->rss_metadata, build_channel_data);
	free(rssdata);
	if (err)
		return err;

	for (int i=0; i < section->numsubs; ++i)
	{
		err = h_rss_configure(section->subs[i], section->rss_metadata);
		if (err)
			return err;
	}

	return NULL;
}

h_err* h_rss_init_channel(h_section* section, const h_conf* conf, int recurse)
{
	strbuf builder = {NULL, 0};

	// Required tags
	append(&builder, "<channel>\n\t<title>");
	if (section->rss_metadata->title != NULL)
		append(&builder, section->rss_metadata->title);
	append(&builder, "</title>\n\t<link>");
	if (conf->url != NULL)
	{
		char* urlpath = h_util_path_join(conf->url, section->path);
		append(&builder, urlpath);
		free(urlpath);
	}
	else
		append(&builder, section->path);
	append(&builder, "</link>\n\t<description>");
	if (section->rss_metadata->description != NULL)
		append(&builder, section->rss_metadata->description);

	append(&builder, "</description>\n\t<atom:link href=\"");

	if (conf->url != NULL)
	{
		char* urlpath = h_util_path_join(conf->url, section->path);
		char* rsspath = h_util_path_join(urlpath, "feed.rss");
		append(&builder, rsspath);
		free(urlpath);
		free(rsspath);
	}
	else
		append(&builder, section->path);
	append(&builder, "\" rel=\"self\" type=\"application/rss+xml\" />\n");

	// optional tags
	if (section->rss_metadata->language != NULL)
	{
		append(&builder, "\t<language>");
		append(&builder, section->rss_metadata->language);
		append(&builder, "</language>\n");
	}

	if (section->rss_metadata->editor != NULL)
	{
		append(&builder, "\t<managingEditor>");
		append(&builder, section->rss_metadata->editor);
		append(&builder, "</managingEditor>\n");
	}

	if (section->rss_metadata->copyright != NULL)
	{
		append(&builder, "\t<copyright>");
		append(&builder, section->rss_metadata->copyright);
		append(&builder, "</copyright>\n");
	}

	if (section->rss_metadata->ttl != NULL)
	{
		append(&builder, "\t<ttl>");
		append(&builder, section->rss_metadata->ttl);
		append(&builder, "</ttl>\n");
	}

	if (section->rss_metadata->img != NULL)
	{
		append(&builder, "\t<image>");
		append(&builder, section->rss_metadata->img);
		append(&builder, "</image>\n");
	}

	for (int i=0; section->rss_metadata->category[i] != NULL; ++i)
	{
		append(&builder, "\t<category>");
		append(&builder, section->rss_metadata->category[i]);
		append(&builder, "</category>\n");
	}

	struct timespec ts;
	if (conf->use_pubdate && (clock_gettime(CLOCK_REALTIME, &ts) == 0))
	{
		append(&builder, "\t<pubDate>");
		// Conforming to https://www.ietf.org/rfc/rfc822.txt , section 5
		// Not using strftime for most things since it depends on locale, which
		// may not be compliant with weekday/month
		struct tm* ltime = localtime(&ts.tv_sec);
		char timebuffer[64];
		memset(&timebuffer[0], 0, sizeof(timebuffer) / sizeof(timebuffer[0]));

		switch (ltime->tm_wday)
		{
		case 0:
			strcpy(&timebuffer[0], "Sun, ");
			break;
		case 1:
			strcpy(&timebuffer[0], "Mon, ");
			break;
		case 2:
			strcpy(&timebuffer[0], "Tue, ");
			break;
		case 3:
			strcpy(&timebuffer[0], "Wed, ");
			break;
		case 4:
			strcpy(&timebuffer[0], "Thu, ");
			break;
		case 5:
			strcpy(&timebuffer[0], "Fri, ");
			break;
		default:
			strcpy(&timebuffer[0], "Sat, ");
			break;
		}

		// Don't need snprintf here since buffer size is 64 - 5 (for weekday) = 59
		// and the most this can print out is 13 bytes
		sprintf(&timebuffer[5], "%02d ", ltime->tm_mday);
		switch (ltime->tm_mon)
		{
		case 0:
			strcat(timebuffer, "Jan ");
			break;
		case 1:
			strcat(timebuffer, "Feb ");
			break;
		case 2:
			strcat(timebuffer, "Mar ");
			break;
		case 3:
			strcat(timebuffer, "Apr ");
			break;
		case 4:
			strcat(timebuffer, "May ");
			break;
		case 5:
			strcat(timebuffer, "Jun ");
			break;
		case 6:
			strcat(timebuffer, "Jul ");
			break;
		case 7:
			strcat(timebuffer, "Aug ");
			break;
		case 8:
			strcat(timebuffer, "Sep ");
			break;
		case 9:
			strcat(timebuffer, "Oct ");
			break;
		case 10:
			strcat(timebuffer, "Nov ");
			break;
		case 11:
			strcat(timebuffer, "Dec ");
			break;
		}

		// Absolute maximum for this is 5 + 13 + 4 = 22, leaving 41 bytes for strftime
		// and strftime will use a maximum of 19 bytes (if timezone starts with +/-)
		const size_t buffer_length = strlen(timebuffer);

		strftime(
			&timebuffer[buffer_length],
			sizeof(timebuffer) / sizeof(timebuffer[0]) - 13,
			"%Y %H:%M:%S %z",
			ltime
		);

		append(&builder, timebuffer);
		append(&builder, "</pubDate>\n");
	}

	if (conf->webmaster != NULL)
	{
		append(&builder, "\t<webMaster>");
		append(&builder, conf->webmaster);
		append(&builder, "</webMaster>\n");
	}

	append(&builder, "\t<generator>housecat</generator>\n\t<docs>https://cyber.harvard.edu/rss/rss.html</docs>\n");

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
				append(&builder, section->subs[i]->rss);
		}

	for (int i=0; i < section->numposts; ++i)
	{
		if (section->posts[i]->rss != NULL)
			append(&builder, section->posts[i]->rss);
	}

	for (int i=0; i < section->numdrafts; ++i)
	{
		if (section->drafts[i]->rss != NULL)
			append(&builder, section->drafts[i]->rss);
	}

	append(&builder, "</channel>\n");
	section->rss = builder.str;
	return NULL;
}

h_err* h_rss_init_item(h_post* post, const h_conf* conf)
{
	strbuf builder = {NULL, 0};

	append(&builder, "\t<item>\n\t\t<title>");
	append(&builder, post->title);
	append(&builder, "</title>\n\t\t<link>");
	if (conf->url != NULL)
	{
		char* urlpath = h_util_path_join(conf->url, post->path);
		append(&builder, urlpath);
		free(urlpath);
	}
	else
	{
		append(&builder, post->path);
	}
	append(&builder, "</link>\n\t\t<description>");

	// decide whether or not to use full content
	if (conf->rss_fullcontent)
	{
		append(&builder, "<![CDATA[\n");
		append_truncated_html(&builder, post->html);
		append(&builder, "\n]]>");
	}
	else if (post->rss_metadata->description)
	{
		append(&builder, post->rss_metadata->description);
	}
	append(&builder, "</description>\n");

	if (post->rss_metadata->author)
	{
		append(&builder, "\t\t<author>");
		append(&builder, post->rss_metadata->author);
		append(&builder, "</author>\n");
	}

	if (conf->use_guid)
	{
		if (conf->url != NULL)
		{
			char* urlpath = h_util_path_join(conf->url, post->path);
			append(&builder, "\t\t<guid isPermaLink=\"true\">");
			append(&builder, urlpath);
			free(urlpath);
		}
		else
		{
			append(&builder, "\t\t<guid isPermaLink=\"false\">");
			append(&builder, post->path);
		}
		append(&builder, "</guid>\n");
	}

	if (post->rss_metadata->date)
	{
		append(&builder, "\t\t<pubDate>");
		append(&builder, post->rss_metadata->date);
		append(&builder, "</pubDate>\n");
	}

	for (size_t i = 0; post->rss_metadata->category[i] != NULL; ++i)
	{
		append(&builder, "\t\t<category>");
		append(&builder, post->rss_metadata->category[i]);
		append(&builder, "</category>\n");
	}

	append(&builder, "\t</item>\n");
	post->rss = builder.str;
	return NULL;
}

h_rss_post* h_rss_post_create()
{
	h_rss_post* rss = malloc(sizeof(*rss));
	if (rss == NULL)
		return NULL;

	rss->author = NULL;
	rss->description = NULL;
	rss->date = NULL;
	rss->category = malloc(sizeof(*rss->category));
	if (rss->category == NULL)
	{
		free(rss);
		return NULL;
	}

	rss->category[0] = NULL;
	return rss;
}

void h_rss_post_free(h_rss_post* rss)
{
	if (rss == NULL)
		return;

	free(rss->author);
	free(rss->description);
	free(rss->date);
	for (size_t i=0; rss->category[i] != NULL; ++i)
		free(rss->category[i]);
	free(rss->category);
	free(rss);
}

h_rss_section* h_rss_section_create()
{
	h_rss_section* rss = malloc(sizeof(*rss));
	if (rss == NULL)
		return NULL;

	rss->title = NULL;
	rss->description = NULL;
	rss->language = NULL;
	rss->editor = NULL;
	rss->copyright = NULL;
	rss->img = NULL;
	rss->ttl = NULL;
	rss->category = malloc(sizeof(*rss->category));
	if (rss->category == NULL)
	{
		free(rss);
		return NULL;
	}

	rss->category[0] = NULL;
	return rss;
}

void h_rss_section_free(h_rss_section* rss)
{
	if (rss == NULL)
		return;

	free(rss->title);
	free(rss->description);
	free(rss->language);
	free(rss->editor);
	free(rss->copyright);
	free(rss->img);
	free(rss->ttl);
	for (size_t i=0; rss->category[i]  != NULL; ++i)
		free(rss->category[i]);
	free(rss->category);
	free(rss);
}
