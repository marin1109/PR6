CC = gcc
CFLAGS = -Wall -o

.PHONY: all clean

all: FichiersCCours exercice1 exercice2

FichiersCCours: client_ssl envoibroadcast mon_wakeonlan_broadcast recoitbroadcast serveur_ssl

client_ssl: 
	$(CC) $(CFLAGS) 'Fichiers C - Cours'/client_ssl.o 'Fichiers C - Cours'/client_ssl.c -lssl -lcrypto

envoibroadcast: 
	$(CC) $(CFLAGS) 'Fichiers C - Cours'/envoibroadcast.o 'Fichiers C - Cours'/envoibroadcast.c

mon_wakeonlan_broadcast:
	$(CC) $(CFLAGS) 'Fichiers C - Cours'/mon_wakeonlan_broadcast.o 'Fichiers C - Cours'/mon_wakeonlan_broadcast.c

recoitbroadcast:
	$(CC) $(CFLAGS) 'Fichiers C - Cours'/recoitbroadcast.o 'Fichiers C - Cours'/recoitbroadcast.c

serveur_ssl:
	$(CC) $(CFLAGS) 'Fichiers C - Cours'/serveur_ssl.o 'Fichiers C - Cours'/serveur_ssl.c -lssl -lcrypto

exercice1: exercice1_emetteur exercice1_recepteur

exercice1_emetteur: 
	$(CC) $(CFLAGS) exercice1/emetteur.o exercice1/emetteur.c

exercice1_recepteur:
	$(CC) $(CFLAGS) exercice1/recepteur.o exercice1/recepteur.c

exercice2: exercice2_client_https

exercice2_client_https:
	$(CC) $(CFLAGS) exercice2/client_https.o exercice2/client_https.c -lssl -lcrypto

clean:
	rm -f *.o 'Fichiers C - Cours'/*.o 'exercice1'/*.o 'exercice2'/*.o