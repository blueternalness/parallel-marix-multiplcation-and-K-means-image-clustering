CC = gcc

all: p1a p1b p2a p2b

p1a: p1a.c
	$(CC) -o p1a p1a.c

p1b: p1b.c
	$(CC) -o p1b p1b.c

p2a: p2a.c
	$(CC) -o p2a p2a.c

p2b: p2b.c
	$(CC) -o p2b p2b.c

clean:
	rm -f p1a p1b p2a p2b