build:
	gcc -O3 -Wall -Wextra -o housecat src/*.c -std=c99 -D_GNU_SOURCE=1

debug:
	gcc -Wall -Wextra -o housecat src/*.c -std=c99 -DDEBUG=1 -D_GNU_SOURCE=1

clean:
	rm housecat
