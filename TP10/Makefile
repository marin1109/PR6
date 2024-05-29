CC = gcc
CFLAGS = -Wall -o

all: fichiersC serveur_select serveur_poll jeu

fichiersC:
	$(CC) $(CFLAGS) 'Fichiers C - Cours'/poll_recv.o 'Fichiers C - Cours'/poll_recv.c

serveur_select:
	$(CC) $(CFLAGS) serveur_select.o serveur_select.c

serveur_poll:
	$(CC) $(CFLAGS) serveur_poll.o serveur_poll.c

jeu:
	$(CC) $(CFLAGS) jeu.o jeu.c

clean:
	rm -f *.o 'Fichiers C - Cours'/*.o