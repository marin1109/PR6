#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#define BUF_SIZE 100

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <hostname> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *hostname = argv[1];
    const char *port = argv[2];
    int sockfd;
    struct addrinfo hints, *res, *p;
    int status;
    char buf[BUF_SIZE];
    ssize_t numbytes;

    // Préparation des structures hints
    memset(&hints, 0, sizeof hints);
    hints.ai_family = 
    hints.ai_socktype = SOCK_STREAM; // TCP

    // Résolution du nom de l'hôte en adresse
    if ((status = getaddrinfo(hostname, port, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 1;
    }

    p = res;

    // Boucle à travers les résultats et se connecter à la première possible
    while(p != NULL) {
        sockfd = socket(p -> ai_family, p -> ai_socktype, p -> ai_protocol);
        if(sockfd > 0) {
            if(connect(sockfd, p -> ai_addr, p -> ai_addrlen) == 0) {
                break;
            }
            close(sockfd);
        }
        
        p = p -> ai_next;
    }

    if (p == NULL) {
        fprintf(stderr, "failed to connect\n");
        return 2;
    }

    freeaddrinfo(res); // Libérer la mémoire allouée par getaddrinfo

    // Envoi et réception de données
    printf("Enter message: ");
    fgets(buf, BUF_SIZE, stdin);

    if (send(sockfd, buf, strlen(buf), 0) < 0) {
        perror("send");
        close(sockfd);
        return EXIT_FAILURE;
    }

    if ((numbytes = recv(sockfd, buf, BUF_SIZE-1, 0)) < 0) {
        perror("recv");
        close(sockfd);
        return EXIT_FAILURE;
    }

    buf[numbytes] = '\0';
    printf("Received: %s\n", buf);

    close(sockfd);
    return EXIT_SUCCESS;
}
