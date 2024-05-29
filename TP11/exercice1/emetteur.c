#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

#define IP_ADDRESS "10.51.63.255"
#define BUFFER_SIZE 1024
#define PORT 8080

int main() {
    int sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int broadcast = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0) {
        perror("setsockopt SO_BROADCAST");
        close(sock);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in broadcast_addr;
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, IP_ADDRESS, &broadcast_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sock);
        exit(EXIT_FAILURE);
    }
    
    const char *message = "HELLO";
    int r = sendto(sock, message, strlen(message), 0, (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr));
    if (r < 0) {
        perror("sendto");
        close(sock);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server;
    socklen_t taille = sizeof(server);
    char buf[BUFFER_SIZE];

    int rec = recvfrom(sock, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&server, &taille);
    if (rec < 0) {
        perror("recvfrom");
        close(sock);
        exit(EXIT_FAILURE);
    }

    buf[rec] = '\0';
    if (strcmp(buf, "ACK") == 0) {
        printf("Le serveur tourne sur l'adresse : %s\n", inet_ntoa(server.sin_addr));
    } else {
        printf("Message inattendu reçu : %s\n", buf);
    }

    close(sock);
    return EXIT_SUCCESS;
}
