# Housecat

Housecat is a command-line static site generator, written in C. It should support any POSIX environment.

## Installation

Installing Housecat is simple. A make && sudo make install should do the trick.

	git clone http://git.mort.coffee/mort/housecat.git
	cd housecat
	make && sudo make install

## Usage

Housecat's interface is simple; `housecat <directory>`. It expects the target directory to look somewhat like this:

	|-- conf - A configuration file
	|
	|-- input/ - The input files and folders, where page structure and posts are stored.
	|   |-- 00000-home/
	|       |-- 00000-helloworld.html
	|       |-- 00001-hello2.html
	|
	|-- theme/ - The directory containing the theme files.
	|
	|-- plugins/ - The directory containing plugins, if any

See the [example directory](https://github.com/mortie/housecat/tree/master/example) for an example.

Housecat will create a directory, called `public`, in the target directory. This directory will contain the compiled HTML files, and should be served by a web server.

## Markdown

Housecat will assume all files in `input/` is valid HTML, and just include them blindly into the generated output. However, most people probably don't want to write directly in HTML; you probably want to write in something like markdown. Housecat doesn't do anything with markdown or similar formats by design, but a simple script will fix that. I'm personally using this script for my own blog, which takes markdown files in `raw/`, turns them into HTML, and put them in `input/` for Housecat to process:

``` bash
#!/bin/bash

build()
{
	for F in $(ls "$1"); do
		if [ -d "$1/$F" ]; then
			mkdir -p "$2/$F"
			build "$1/$F" "$2/$F"
		else
			name=$(sed 's/\.md$/.html/' <<< "$F")
			cmark < "$1/$F" > "$2/$name"
		fi
	done
}

build "raw" "input"

housecat .
```

[cmark](https://github.com/jgm/cmark/tree/master/man) is the reference implementation of CommonMark, a standardized flavour of markdown. Using any other implementation of markdown, or any other markup language, is as simple as slightly modifying that script.
