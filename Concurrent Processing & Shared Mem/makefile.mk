all: fish pellet swimmill
fish : fish.o
	gcc -o fish fish.
fish.o : fish.c
	gcc -c fish.c -o fish.o
pellet : pellet.o
	gcc -o pellet pellet.o
pellet.o : pellet.c
	gcc -c pellet.c -o pellet.o
swimmill : swimmill.o
	gcc -o swimmill swimmill.c
swimmill.o : swimmill.c
	gcc -c swimmill.c -o swimmill.o
