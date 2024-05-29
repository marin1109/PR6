#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

#define IP_ADRESS "10.51.63.255" // Adresse de broadcast

int main()
{
    // Création d'un socket UDP
    int sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        perror("socket");
        exit(1);
    }

    // Autoriser l'envoi de messages en broadcast
    int ok = 1;
    int r = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &ok, sizeof(ok));
    if (r == -1)
    {
        perror("setsockopt SO_BROADCAST");
        close(sock);
        exit(1);
    }

    // Configuration de l'adresse de broadcast
    struct sockaddr_in adrdiff;
    memset(&adrdiff, 0, sizeof(adrdiff));
    adrdiff.sin_family = AF_INET;
    adrdiff.sin_port = htons(8888); // Port 8888
    r = inet_pton(AF_INET, IP_ADRESS, &adrdiff.sin_addr);
    if (r <= 0)
    {
        perror("pb adresse");
        close(sock);
        exit(1);
    }

    char buf[100]; // Buffer pour les messages à envoyer
    // Envoi de 10 messages en broadcast
    for (int i = 0; i <= 10; i++)
    {
        sprintf(buf, "bonjour %d", i);
        r = sendto(sock, buf, strlen(buf), 0, (struct sockaddr *)&adrdiff, (socklen_t)sizeof(struct sockaddr_in));
        if (r < 0)
        {
            perror("sendto");
            close(sock);
            exit(1);
        }
    }

    // Fermeture du socket
    close(sock);
    return 0;
}
