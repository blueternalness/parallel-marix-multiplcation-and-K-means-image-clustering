CC = gcc

all: p1a p1b p2a p2b

p1a: p1a.c
	$(CC) -o p1a p1a.c -pthread

p1b: p1b.c
	$(CC) -o p1b p1b.c -pthread

p2a: p2a.c
	$(CC) -o p2a p2a.c -pthread

p2b: p2b.c
	$(CC) -o p2b p2b.c -pthread

clean:
	rm -f p1a p1b p2a p2b