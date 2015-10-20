#ifndef H_CONF_H
#define H_CONF_H

typedef struct h_conf
{
	char* title;
	int posts_per_page;
	int logo;
	char* root;
} h_conf;

h_conf* h_conf_parse(char* str, int len);

#endif
