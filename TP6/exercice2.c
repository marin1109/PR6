#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <hostname>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *hostname = argv[1];
    struct addrinfo hints, *res, *p;
    int status;
    char *buf = (char *)calloc(INET6_ADDRSTRLEN, sizeof(char));

    // Préparation des structures hints
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // IPv4 ou IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP

    // Résolution du nom de l'hôte en adresse
    if ((status = getaddrinfo(hostname, NULL, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 1;
    }

    p = res;

    // Boucle à travers les résultats et se connecter à la première possible
    while(p != NULL) {
        if(p -> ai_family == AF_INET6) {
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p -> ai_addr;
            printf("Adresse IPv6: %s\n", inet_ntop(p -> ai_family, &ipv6 -> sin6_addr, buf, INET6_ADDRSTRLEN));
        }
        else if(p -> ai_family == AF_INET) {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p -> ai_addr;
            printf("Adresse IPv4: %s\n", inet_ntop(p -> ai_family, &ipv4 -> sin_addr, buf, INET6_ADDRSTRLEN));
        }
        else {
            p = p -> ai_next;
            continue;
        }

        p = p -> ai_next;
    }

    freeaddrinfo(res); // Libérer la mémoire allouée par getaddrinfo

    return EXIT_SUCCESS;
}
