#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

int main()
{
    // Création d'un socket UDP
    int sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        perror("socket");
        exit(1);
    }

    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Configuration de l'adresse du socket
    struct sockaddr_in address_sock;
    address_sock.sin_family = AF_INET;
    address_sock.sin_port = htons(8888);              // Port 8888
    address_sock.sin_addr.s_addr = htonl(INADDR_ANY); // Toutes les interfaces locales

    // Liaison du socket à l'adresse spécifiée
    int r = bind(sock, (struct sockaddr *)&address_sock, sizeof(struct sockaddr_in));
    if (r)
    {
        perror("bind");
        close(sock);
        exit(1);
    }

    struct sockaddr_in emet; // Structure pour stocker l'adresse de l'émetteur
    socklen_t taille = sizeof(emet);
    char buf[100]; // Buffer pour recevoir les messages
    char adr[100]; // Buffer pour stocker l'adresse de l'émetteur

    // Boucle pour recevoir des messages
    while (1)
    {
        int rec = recvfrom(sock, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&emet, &taille);
        if (rec < 0)
        {
            perror("recvfrom");
            close(sock);
            exit(1);
        }

        buf[rec] = '\0'; // Terminer la chaîne reçue
        printf("Message reçu : %s\n", buf);
        printf("Port de l'émetteur: %d\n", ntohs(emet.sin_port));

        // Conversion de l'adresse IP de l'émetteur en chaîne de caractères
        inet_ntop(AF_INET, &(emet.sin_addr), adr, sizeof(adr));
        printf("Adresse de l'émetteur: %s\n\n", adr);
    }

    close(sock);
    return 0;
}
