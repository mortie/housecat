#include "post.h"
#include "util.h"
#include "rss.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

static int startswith(const char* str, const char* substr) {
	return strncmp(str, substr, strlen(substr)) == 0;
}

static void put_pair(void* i, const char* key, const char* val)
{
	h_rss_post* item = i;
	if (h_util_streq(key, "author"))
	{
		item->author = realloc(item->author, strlen(val) + 1);
		strcpy(item->author, val);
	}
	else if (h_util_streq(key, "description"))
	{
		item->description = realloc(item->description, strlen(val) + 1);
		strcpy(item->description, val);
	}
	else if (h_util_streq(key, "date"))
	{
		item->date = realloc(item->date, strlen(val) + 1);
		strcpy(item->date, val);
	}
	else if (h_util_streq(key, "category"))
	{
		size_t category_length = 0;
		while (item->category[category_length] != NULL)
			category_length += 1;
		// category_length is the index of the NULL
		// so the list has caterogy_length + 1 entries including NULL
		// the + 2 adds another

		item->category = realloc(item->category, (category_length+2)*sizeof(*item->category));
		item->category[category_length] = malloc(strlen(val) + 1); // will alwayy be NULL due to how the LL works, no realloc
		strcpy(item->category[category_length], val);
		item->category[category_length + 1] = NULL;
	}
}

enum parsemode
{
	PARSE_BEFORE, // Starting parser state
	PARSE_COMMENT_START, // State for the -s in <!--
	PARSE_COMMENT, // Inside of a comment
	PARSE_COMMENT_END, // First - at the end of a comment
	PARSE_IN_OPEN_START, // The <h1> tag for post title
	PARSE_IN_OPEN_END, // The <h1> tag for post title
	PARSE_IN_MID, // The pot title
	PARSE_IN_CLOSE, // The </h1> after post title
	PARSE_AFTER,// The </h1> after post title
	PARSE_DONE, // Rest of the post
	PARSE_TAG_OPEN, // Start of a tag, on <
	PARSE_TAG_CLOSE, // End of a tag, on >
	PARSE_NORMAL,
	PARSE_KEY,
	PARSE_VALUE
};

static void metadata_parse(h_post* post, char* data) {
	if (data == NULL)
		return;

	h_conf_parse(data, strlen(data) + 1, post->rss_metadata, put_pair);
}

static h_err* post_parse(h_post* post, FILE* f, int length, const char* path) {
	//Create string to hold the file
	// The +3 is for the 3 added NUL bytes at the end
	// of the rss metadata, title, and html data.
	char* fstr = malloc((length + 3) * sizeof(char));
	if (fstr == NULL)
		return h_err_create(H_ERR_ALLOC, NULL);

	//Parse mode to keep track of the parsing
	enum parsemode mode = PARSE_BEFORE;

	// Pointer to the start of the rss metadata
	char* rssdata = NULL;

	//Scan through the file
	// Housecat expects a comment with the rss info (optional),
	// followed by a <h1> tag with the title.
	char c;
	char prev = 0;
	int i = 0;
	while ((c = getc(f)) != EOF)
	{
		switch (mode)
		{
		case PARSE_BEFORE:
			if (c == '<')
				mode = PARSE_IN_OPEN_START;
			break;
		case PARSE_IN_OPEN_START:
			if (prev == 'h' && c == '1')
				mode = PARSE_IN_OPEN_END;
			else if (prev == '<' && c == '!')
				mode = PARSE_COMMENT_START;
			break;
		case PARSE_COMMENT_START:
			if (c == '-' && prev == '-')
			{
				rssdata = fstr + i + 1;
				mode = PARSE_COMMENT;
			}
			break;
		case PARSE_COMMENT:
			if (c == '-' && prev == '-')
				mode = PARSE_TAG_CLOSE;
			break;
		case PARSE_TAG_CLOSE:
			if (c == '>')
			{
				fstr[i - 2] = '\0';
				fstr[i - 1] = '-';
				fstr[i++] = '-';
				mode = PARSE_BEFORE;
			}
			else
				mode = PARSE_COMMENT;
			break;
		case PARSE_IN_OPEN_END:
			if (c == '>')
			{
				mode = PARSE_IN_MID;
				post->title = fstr + i + 1;
			}
			break;
		case PARSE_IN_MID:
			if (c == '<')
			{
				mode = PARSE_IN_CLOSE;
				fstr[i++] = '\0';
			}
			break;
		case PARSE_IN_CLOSE:
			if (c == '>')
				mode = PARSE_AFTER;
			break;
		case PARSE_AFTER:
			if (!isspace(c))
			{
				mode = PARSE_DONE;
				post->html = fstr + i;
			}
			break;
		default: // should never get here
			prev = c;
		}

		prev = c;

		//Build string holding the file
		fstr[i++] = c;
	}

	if (post->title == NULL)
	{
		free(fstr);
		return h_err_create(H_ERR_FORMAT_NOTITLE, path);
	}

	char* title_new = malloc(strlen(post->title) + 1);
	if (title_new == NULL)
	{
		free(fstr);
		post->title = NULL;
		post->html = NULL;
		return h_err_create(H_ERR_ALLOC, path);
	}
	strcpy(title_new, post->title);
	post->title = title_new;

	metadata_parse(post, rssdata);

	fstr[i - 1] = '\0';

	char* html_new = malloc(strlen(post->html) + 1);
	if (html_new == NULL)
	{
		free(fstr);
		free(post->title);
		post->title = NULL;
		return h_err_create(H_ERR_ALLOC, path);
	}
	strcpy(html_new, post->html);
	post->html = html_new;

	free(fstr);

	return NULL;
}

h_err* h_post_init_from_file(h_post* post, const char* path, const char* spath, int depth)
{
	post->title = NULL;
	post->slug = NULL;

	//Get the file pointer
	FILE* f = fopen(path, "r");
	if (f == NULL)
		return h_err_from_errno(errno, path);

	//Get the length
	fseek(f, 0L, SEEK_END);
	int length = ftell(f);
	rewind(f);

	h_err* err = post_parse(post, f, length, path);
	fclose(f);
	if (err)
		return err;

	//Get post's slug from the path
	int lastslash = 0;
	int lastdot = 0;
	length = strlen(path);
	for (int i = 0; i < length; ++i)
	{
		if (path[i] == '/')
			lastslash = i;
		else if (path[i] == '.')
			lastdot = i;
	}
	if (lastslash > lastdot)
		lastdot = length;

	int chars = lastdot - lastslash - 6;
	post->slug = malloc(chars * sizeof(char));
	if (post->slug == NULL)
		return h_err_create(H_ERR_ALLOC, NULL);

	memcpy(post->slug, path + lastslash + 1 + 6, chars * sizeof(char));

	post->slug[chars - 1] = '\0';

	post->path = h_util_path_join(spath, post->slug);
	post->depth = depth;

	if (startswith(post->title, "DRAFT:"))
		post->isdraft = 1;
	else
		post->isdraft = 0;

	return NULL;
}

h_post* h_post_create()
{
	h_post* post = malloc(sizeof(*post));
	if (post == NULL)
		return NULL;

	post->title = NULL;
	post->slug = NULL;
	post->html = NULL;
	post->path = NULL;
	post->rss = NULL;

	post->rss_metadata = h_rss_post_create();
	if (post->rss_metadata == NULL)
	{
		free(post);
		return NULL;
	}

	return post;
}

void h_post_free(h_post* post)
{
	if (post == NULL)
		return;

	free(post->title);
	free(post->slug);
	free(post->html);
	free(post->path);
	free(post->rss);
	h_rss_post_free(post->rss_metadata);
	free(post);
}
