#ifndef H_STRS_H
#define H_STRS_H

#define H_STRS_JS_START \
	"(function(){\n" \
		"var conf = %s;\n" \
		"conf.root = '%s';\n"

#define H_STRS_JS_END \
	"})();\n"

#define H_STRS_JS_LIB \
	"(function(){\n" \
		"var q = document.querySelector.bind(document);\n" \
		"var qa = document.querySelectorAll.bind(document);\n" \
		"window.lib = {};\n" \
		"lib.elems = {};\n" \
		"lib.isListPage = !!q('.posts');\n" \
		"lib.isArticle = !lib.isListPage;\n" \
		"if (lib.isListPage) {\n" \
			"lib.elems.posts = [].slice.call(qa('.post'));\n" \
			"lib.elems.postsContainer = q('.posts');\n" \
		"} else {\n" \
			"lib.elems.post = q('.post');\n" \
		"}\n" \
	"})();\n"

#define H_STRS_PHP_START \
	"<?php\n" \
		"$conf = <<<EOT\n" \
		"%s\n" \
		"EOT;\n" \
		"$conf = json_decode($conf);\n" \
		"$conf->root = '%s';\n" \
	"?>\n"

#endif
