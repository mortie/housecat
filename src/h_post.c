#include "h_post.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

enum parsemode
{
	PARSE_BEFORE,
	PARSE_IN_OPEN_START,
	PARSE_IN_OPEN_END,
	PARSE_IN_MID,
	PARSE_IN_CLOSE,
	PARSE_AFTER,
	PARSE_DONE
};

void h_post_init(h_post* post, char* title, char* html, char* slug)
{
	post->title = title;
	post->html = html;
	post->slug = slug;
}

h_err* h_post_init_from_file(h_post* post, char* path)
{
	//Get the file pointer
	FILE* f = fopen(path, "r");
	if (f == NULL)
		return h_err_from_errno(errno, path);

	//Get the length
	fseek(f, 0L, SEEK_END);
	int length = ftell(f);
	rewind(f);

	//Create string to hold the file
	char* fstr = malloc((length + 1) * sizeof(char));
	if (fstr == NULL)
		return h_err_create(H_ERR_ALLOC, NULL);

	//Parse mode to keep track of the parsing
	enum parsemode mode = PARSE_BEFORE;

	//Scan through the file
	char c, prev;
	int i = 0;
	while ((c = getc(f)) != EOF)
	{
		if (mode == PARSE_BEFORE && c == '<')
		{
			mode = PARSE_IN_OPEN_START;
		}
		else if (mode == PARSE_IN_OPEN_START && prev == 'h' && c == '1')
		{
			mode = PARSE_IN_OPEN_END;
		}
		else if (mode == PARSE_IN_OPEN_END && c == '>')
		{
			mode = PARSE_IN_MID;
			post->title = fstr + i + 1;
		}
		else if (mode == PARSE_IN_MID && c == '<')
		{
			mode = PARSE_IN_CLOSE;
			fstr[i++] = '\0';
		}
		else if (mode == PARSE_IN_CLOSE && c == '>')
		{
			mode = PARSE_AFTER;
		}
		else if (mode == PARSE_AFTER && !isspace(c))
		{
			mode = PARSE_DONE;
			post->html = fstr + i;
		}

		prev = c;

		//Build string holding the file
		fstr[i++] = c;
	}

	post->_fstr = fstr;

	//Get post's slug from the path

	int lastslash = 0;
	int lastdot = 0;
	length = strlen(path);
	for (i = 0; i < length; ++i)
	{
		if (path[i] == '/')
			lastslash = i;
		else if (path[i] == '.')
			lastdot = i;
	}
	if (lastslash > lastdot)
		lastdot = length;

	int chars = lastdot - lastslash - 6 - 1;
	post->slug = malloc(chars * sizeof(char));
	if (post->slug == NULL)
		return h_err_create(H_ERR_ALLOC, NULL);

	memcpy(post->slug, path + lastslash + 1 + 6, chars * sizeof(char));

	return NULL;
}
