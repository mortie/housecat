#include "h_err.h"
#include "h_section.h"
#include "h_post.h"

#include <stdlib.h>
#include <stdio.h>

void print_section_s(h_section* section)
{
	printf("section '%s' ('%s'):\n", section->title, section->slug);
}

void print_section_p(h_post* post)
{
	printf("\tpost '%s' ('%s')\n", post->title, post->slug);
}

void print_section(h_section* section)
{
	print_section_s(section);

	int i;
	for (i = 0; i < section->numposts; ++i)
	{
		h_post* post = section->posts[i];
		print_section_p(post);
	}

	for (i = 0; i < section->numsubs; ++i)
	{
		h_section* sub = section->subs[i];
		print_section(sub);
	}
	puts("----------");
}

int main(int argc, char** argv)
{
	if (argc < 0)
		return 1;

	h_section* section = malloc(sizeof(h_section));
	h_err* err = h_section_init_from_dir(section, argv[1]);
	if (err)
	{
		h_err_print(err);
		exit(1);
	}

	print_section(section);

	return 0;
}
