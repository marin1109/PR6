#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 7879                                      // numero de port du serveur
#define ADDR "fdc7:9dd5:2c66:be86:4849:43ff:fe49:79bf" // adresse du serveur
#define SIZE_MESS 100
#define NOM "les L3"

int main()
{

    //*** creation de la socket ***
    int fdsock = socket(PF_INET6, SOCK_STREAM, 0);
    if (fdsock == -1)
    {
        perror("creation socket");
        exit(1);
    }

    //*** creation de l'adresse du destinataire (serveur) ***
    struct sockaddr_in6 address_sock;
    memset(&address_sock, 0, sizeof(address_sock));
    address_sock.sin6_family = AF_INET6;
    address_sock.sin6_port = htons(PORT);
    inet_pton(AF_INET6, ADDR, &address_sock.sin6_addr);

    //*** demande de connexion au serveur ***
    int r = connect(fdsock, (struct sockaddr *)&address_sock, sizeof(address_sock));
    if (r == -1)
    {
        perror("echec de la connexion");
        close(fdsock);
        exit(2);
    }

    //*** envoie d'un message ***
    char bufsend[SIZE_MESS];
    memset(bufsend, 0, SIZE_MESS);

    sprintf(bufsend, "Hello %s", NOM);
    int ecrit = send(fdsock, bufsend, strlen(bufsend), 0);
    if (ecrit <= 0)
    {
        perror("erreur ecriture");
        close(fdsock);
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
        close(fdsock);
        exit(4);
    }
    if (recu == 0)
    {
        printf("serveur off\n");
        close(fdsock);
        exit(0);
    }

    // bufrecv[recu] = '\0';
    printf("%s\n", bufrecv);

    //*** fermeture de la socket ***
    close(fdsock);

    return 0;
}
