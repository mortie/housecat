build:
	gcc -O3 -Wall -Wextra -o housecat src/build/*.c src/*.c -std=c99 -D_GNU_SOURCE=1

debug:
	gcc -g -Wall -Wextra -o housecat src/build/*.c src/*.c -std=c99 -DDEBUG=1 -D_GNU_SOURCE=1

install:
	mv housecat /usr/bin/housecat

clean:
	rm housecat
