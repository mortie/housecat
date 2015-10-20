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
	|   |-- home/
	|       |-- 00000-helloworld.html
	|       |-- 00001-hello2.html
	|
	|-- theme/ - The directory containing the theme files.

See the [example directory](http://git.mort.coffee/mort/housecat/src/master/example) for an example.

Housecat will create a directory, called `public`, in the target directory. This directory will contain the compiled HTML files, and should be served by a web server.
