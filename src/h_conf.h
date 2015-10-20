#ifndef H_CONF_H
#define H_CONF_H

typedef struct h_conf
{
	char* title;
	int logo;
} h_conf;

h_conf* h_conf_parse(char* str, int len);

#endif
