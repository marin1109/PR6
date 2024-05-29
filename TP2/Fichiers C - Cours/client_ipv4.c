// script donnannt les machines allumees : /usr/local/sbin/clients_on

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 7879        // numero de port du serveur
#define ADDR "127.0.0.1" // adresse du serveur
#define SIZE_MESS 100
#define NOM "les L3"

int main()
{

    //*** creation de la socket ***
    int fdsock = socket(PF_INET, SOCK_STREAM, 0);
    if (fdsock == -1)
    {
        perror("creation socket");
        exit(1);
    }

    //*** creation de l'adresse du destinataire (serveur) ***
    struct sockaddr_in address_sock;
    memset(&address_sock, 0, sizeof(address_sock));
    address_sock.sin_family = AF_INET;
    address_sock.sin_port = htons(PORT);
    inet_pton(AF_INET, ADDR, &address_sock.sin_addr);

    //*** demande de connexion au serveur ***
    int r = connect(fdsock, (struct sockaddr *)&address_sock, sizeof(address_sock));
    if (r == -1)
    {
        perror("echec de la connexion");
        exit(2);
    }

    //*** envoie d'un message ***
    char bufsend[SIZE_MESS];
    memset(bufsend, 0, SIZE_MESS);

    sprintf(bufsend, "Hello %s\n", NOM);
    int ecrit = send(fdsock, bufsend, strlen(bufsend), 0);
    if (ecrit <= 0)
    {
        perror("erreur ecriture");
        exit(3);
    }
    if (ecrit < strlen(bufsend))
        printf("les %ld derniers octet du message sont perdus\n", strlen(bufsend) - ecrit);

    //*** reception d'un message ***
    char bufrecv[SIZE_MESS + 1];
    memset(bufrecv, 0, SIZE_MESS + 1);

    int recu = recv(fdsock, bufrecv, SIZE_MESS * sizeof(char), 0);
    if (recu < 0)
    {
        perror("erreur lecture");
        exit(4);
    }
    if (recu == 0)
    {
        printf("serveur off\n");
        exit(0);
    }

    // bufrecv[recu] = '\0';
    printf("%s\n", bufrecv);

    //*** fermeture de la socket ***
    close(fdsock);

    return 0;
}
