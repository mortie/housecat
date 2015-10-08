build:
	gcc -O3 -o housecat src/*.c

debug:
	gcc -DDBUG -o housecat src/*.c

clean:
	rm housecat
