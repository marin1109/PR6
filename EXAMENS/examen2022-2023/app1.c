#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

int main() {
    // Création de la socket UDP
    int sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Activer l'option SO_REUSEADDR
    int ok = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &ok, sizeof(ok)) < 0) {
        perror("setsockopt SO_REUSEADDR");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Activer l'option SO_BROADCAST
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &ok, sizeof(ok)) < 0) {
        perror("setsockopt SO_BROADCAST");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Configuration de l'adresse de diffusion
    struct sockaddr_in adr;
    memset(&adr, 0, sizeof(adr));
    adr.sin_family = AF_INET;
    adr.sin_port = htons(1234);
    if (inet_pton(AF_INET, "255.255.255.255", &adr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Génération et envoi des nombres aléatoires
    srand(time(NULL));
    for (int i = 0; i < 21; i++) {
        int nb = htonl(rand() % 50 - 25);
        printf("Envoi de %d\n", ntohl(nb));
        if (sendto(sock, &nb, sizeof(nb), 0, (struct sockaddr *)&adr, sizeof(adr)) < 0) {
            perror("sendto");
            close(sock);
            exit(EXIT_FAILURE);
        }
    }

    // Fermeture de la socket
    close(sock);
    return 0;
}
