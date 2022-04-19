# Housecat

Housecat is a command-line static site generator, written in C. It should support any POSIX environment.

## Installation

Installing Housecat is simple. A make && sudo make install should do the trick.

	git clone https://github.com/mortie/housecat
	cd housecat
	make && sudo make install

## Usage

Housecat's interface is simple; `housecat <directory>`. It expects the target directory to look somewhat like this:

	|-- conf - A configuration file
	|
	|-- input/ - The input files and folders, where page structure and posts are stored.
	|   |-- rss.conf - Optional configuration file for an rss feed
	|   |-- 00000-home/
	|   |-- |-- rss.conf - The rss configuration can be in any directory to apply only to that directory
	|       |-- 00000-helloworld.html
	|       |-- 00001-hello2.html
	|
	|-- theme/ - The directory containing the theme files.
	|
	|-- plugins/ - The directory containing plugins, if any

See the [example directory](https://github.com/mortie/housecat/tree/master/example) for an example.

Housecat will create a directory, called `public`, in the target directory. This directory will contain the compiled HTML files, and should be served by a web server.

## Markdown

Housecat will assume all files in `input/` is valid HTML, except for files named `rss.conf`,
and just include them blindly into the generated output. However, most people probably don't want to write directly in HTML; you probably want to write in something like markdown. Housecat doesn't do anything with markdown or similar formats by design, but a simple script will fix that. I'm personally using this script for my own blog, which takes markdown files in `raw/`, turns them into HTML, and put them in `input/` for Housecat to process:

``` bash
#!/bin/bash

build()
{
	for F in $(ls "$1"); do
		if [ -d "$1/$F" ]; then
			mkdir -p "$2/$F"
			build "$1/$F" "$2/$F"
		else
			if [[ "$F" =~ .*\.md$ ]]; then
				name=$(sed 's/\.md$/.html/' <<< "$F")
				sed -ne '/^#/,$' "$1/$F" | cmark | cat <(sed -e '/^#/Q' "$1/$F") - > "$2/$name"
			else # copy non-markdow files directly (rss.conf)
				cp "$1/$F" "$2/$F"
			fi
		fi
	done
}

build "raw" "input"

housecat .
```

The `MakeHTML` file is a sample Makefile that converts markdown in `raw/` to html
in `input/`.

[cmark](https://github.com/jgm/cmark/tree/master/man) is the reference implementation of CommonMark, a standardized flavour of markdown. Using any other implementation of markdown, or any other markup language, is as simple as slightly modifying that script.

## Configuration options

Housecat expects a `conf` file in the directory given
as the command-line argument. The format for the configuration
options are
```
key: value
```
with one pair per line. A key contains only alphanumeric characters and underscores,
and the value is the rest of the line after the `:`, excluding whitespace directly
after the `:` (the value starts at the first nonwhitespace character).
If there are multiples of the same key in the configuration file,
the final setting takes effect.

The options that are required in `conf` are:
* `title` A string, the main title of the site
* `logo` A boolean (`true` or `false`), whether or not the site has a logo
* `posts_per_page` An integer, the number of posts per page on the index pages
* `root` a string, the root path of the site in the url. For example, if the
	site is located at `http://example.com/housecat-site`, `root` would be
	`/housecat-site`. Use `/` if the site is directly on your url and not in a subdirectory.

The optional options in `conf` are:
* `rss` A boolean (`true` or `false`), defaults to `false`. If this option is
	true, housecat will generate rss feeds.
* `rss_drafts` A boolean (`true` or `false`), defaults to `false`.
	If this option is true, posts that are drafts (posts whose titles
	start with `DRAFT:`) will be included in the rss feed.
* `rss_fullcontent` A boolean (`true` or `false`), defaults to `true`.
	If this option is true, the RSS `<item>` tags will use the full
	content of a post in their `<description>`, otherwise the RSS `<item>` tags
	will use the short descrption found in the RSS configuration section
	of the post input file.
* `url` A string, defaults to an empty string. Use this option
	to specify the site url, including the schema (e.g. `https://`)
	so the rss feed can contain real links to the content. Without
	a url, the links in the rss feeds will just be relative paths
	from the `public/` directory.
* `rss_level` should be unset, `global`, `section`, or`subsection`.
	(defaults to `global`). This option dictates how fine-grained the rss
	feed channels should be.

	* With `global`, all of the site's posts
		will go into a single channel.

	* With `section`, the posts
		directly in `input/` will go in their own channel, and every
		directory that is in `input/` will get its own channel.
		Posts in the subdirectories of the subdirectories
		(e.g. `input/section/subsection1/` and `input/section/subsection2`)
		will go into the rss channel of the first section they are in
		(both of the previous example subdirectories would be
		aggregated into the `input/section` channel).

	* With `subsection`, there will be a channel for every directory.
		For example, posts in `input/` will be in a channel, posts
		from `input/section/` will be in a different channel, and
		posts from `input/section/subsection` will be in yet another
		channel.

	Every post is in exactly one channel.

* `use_guid` A boolean (`true` or `false`), defaults to `true`.
	If this option is true, Housecat will generate a `guid`
	tag for every item with the `guid` being a link to the post
	(if `url` is set) or the relative path of the post html file.

* `use_pubdate` A boolean (`true` or `false`), defaults to
	`false`. If this option is true Housecat will use the
	current time and current locale settings to generate a
	`pubDate` tag for the channels.

* `webmaster` a string. If set, Housecat will create a `webMaster`
	tag for all rss channels. This tag is meant to contain the email
	of the site's webmaster.

## Channel RSS Options

Optionally, Housecat will read `rss.conf` files from `input/`
to generate metadata for the rss channels. The options cascade,
so having an `rss.conf` file in a directory will also set the
channel options on all subdirectories and subsubdirectories and so
on, unless the subdirectory in question has its own
`rss.conf` that takes precedence.

The `rss.conf` files share the same syntax as `conf`, with the
possible options being:

* `title` a string, the title of the rss channel

* `description` a string, a description of the rss channel

* `language` a string, specifies the language of the rss channel
	(e.g. `en-us`)

* `editor` a string, specifies the email address of the editor
	responsible for the rss channel

* `copyright` a string, specifies copyright information for the
	channel

* `img` a string, specifies a url to an icon image for the rss
	channel (note: Housecat does not prepend the `url` variable for
	you here)

* `ttl` a string, specifies the time-to-live for the rss channel
	(this tells readers how often to update)

Furthermore, there is a pseudo-option `category`. Setting
`category` adds a category to the rss channel. Unlike
other options, every specified category will be included since
channels can have multiple categories.

Finally, Housecat will not look deeper than it needs to.
If the `rss_level` is `global`, then only `input/rss.conf`
matters. If the level is `section`, only the confs in the
top level sections matter.

## Post RSS options

Housecat allows individual posts to specify rss information.
Housecat expects this post information to be contained at the
top of the input file, above the title `<h1>` tags, in a HTML
comment. A post with rss information would look like:

```html
<!---author: me!
description: an example post
-->
<h1>Example title</h1>

<p>Hello world</p>
```

The available options for posts are:

* `author` a string, the author of the post

* `description` a string, a description of the post

* `date` a string, the date the post was made. To be completely
	conformant with rss standards, dates should follow
	[RFC-822](https://www.ietf.org/rfc/rfc822.txt).

Furthermore, posts can have an unlimited number of
`category` settings. Every specified category will be
applied to the post.
