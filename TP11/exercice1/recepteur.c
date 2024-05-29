#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

#define PORT 8080
#define BUFFER_SIZE 100

int main() {
    int sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in address_sock;
    memset(&address_sock, 0, sizeof(address_sock));
    address_sock.sin_family = AF_INET;
    address_sock.sin_port = htons(PORT);
    address_sock.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (struct sockaddr *)&address_sock, sizeof(address_sock)) < 0) {
        perror("bind");
        close(sock);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in emet;
    socklen_t taille = sizeof(emet);
    char buf[BUFFER_SIZE];

    int rec = recvfrom(sock, buf, BUFFER_SIZE - 1, 0, (struct sockaddr *)&emet, &taille);
    if (rec < 0) {
        perror("recvfrom");
        close(sock);
        exit(EXIT_FAILURE);
    }

    buf[rec] = '\0';
    if (strcmp("HELLO", buf) == 0) {
        printf("HELLO reçu de %s:%d\n", inet_ntoa(emet.sin_addr), ntohs(emet.sin_port));
        sendto(sock, "ACK", strlen("ACK"), 0, (struct sockaddr *)&emet, taille);
    }

    close(sock);
    return EXIT_SUCCESS;
}
