CC = gcc
CFLAGS = -Wall -pthread -o

all: unique multi tmp

unique:
	$(CC) $(CFLAGS) jeu_devinette_unique.o jeu_devinette_unique.c

multi:
	$(CC) $(CFLAGS) jeu_devinette_multi.o jeu_devinette_multi.c

tmp:
	$(CC) $(CFLAGS) tmp.o tmp.c

clean:
	rm -f *.o