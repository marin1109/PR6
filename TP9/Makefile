CC = gcc
CFLAGS = -Wall -o

all: fichiersC IPv6 IPv4

fichiersC: 
	$(CC) $(CFLAGS) 'Fichiers C - Cours'/abonne4.o 'Fichiers C - Cours'/abonne_multicast_ipv4.c
	$(CC) $(CFLAGS) 'Fichiers C - Cours'/abonne6.o 'Fichiers C - Cours'/abonne_multicast_ipv6.c
	$(CC) $(CFLAGS) 'Fichiers C - Cours'/diffuseur4.o 'Fichiers C - Cours'/diffuseur_multicast_ipv4.c
	$(CC) $(CFLAGS) 'Fichiers C - Cours'/diffuseur6.o 'Fichiers C - Cours'/diffuseur_multicast_ipv6.c

IPv6: serveur6 client6

IPv4: serveur4 client4

serveur4: 
	$(CC) $(CFLAGS) IPv4/serveur.o IPv4/serveur.c

client4:
	$(CC) $(CFLAGS) IPv4/client.o IPv4/client.c

serveur6: 
	$(CC) $(CFLAGS) IPv6/serveur.o IPv6/serveur.c

client6: 
	$(CC) $(CFLAGS) IPv6/client.o IPv6/client.c

clean:
	rm -f 'Fichiers C - Cours'/*.o IPv4/*.o IPv6/*.o 