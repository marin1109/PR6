CC = gcc
CFLAGS = -Wall -o

all: FichiersC exercice1 exercice2 exercice3 exercice4 exercice5 

FichiersC:
	$(CC) $(CFLAGS) ./'Fichiers C - Cours'/client_ipv4.o ./'Fichiers C - Cours'/client_ipv4.c
	$(CC) $(CFLAGS) ./'Fichiers C - Cours'/client_ipv6.o ./'Fichiers C - Cours'/client_ipv6.c

exercice1: exercice1.c
	$(CC) $(CFLAGS) exercice1.o exercice1.c

exercice2: exercice2.c
	$(CC) $(CFLAGS) exercice2.o exercice2.c

exercice3: exercice3.c
	$(CC) $(CFLAGS) exercice3.o exercice3.c

exercice4: exercice4.c
	$(CC) $(CFLAGS) exercice4.o exercice4.c

exercice5: exercice5.c
	$(CC) $(CFLAGS) exercice5.o exercice5.c

clean:
	rm -f *.o ./'Fichiers C - Cours'/*.o  

.PHONY: all clean
