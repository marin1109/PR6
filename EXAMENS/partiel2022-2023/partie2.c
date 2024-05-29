#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char* argv[]) {
    // Utilisation par défaut de www.google.com et port 443
    char* hostname = "www.google.com";
    char* port = "443";  // Port HTTPS

    int sock;
    socklen_t addr_len;

    struct addrinfo hints, *res, *p;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // Permet IPv4 ou IPv6
    hints.ai_socktype = SOCK_STREAM;

    int status;
    if ((status = getaddrinfo(hostname, port, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return EXIT_FAILURE;
    }

    int connected = 0;

    for (p = res; p != NULL; p = p->ai_next) {
        sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sock == -1) {
            perror("socket");
            continue;
        }

        if (connect(sock, p->ai_addr, p->ai_addrlen) == -1) {
            perror("connect");
            close(sock);
            continue;
        }

        connected = 1;
        break; // Connexion réussie
    }

    if (!connected) {
        fprintf(stderr, "Failed to connect to %s:%s\n", hostname, port);
        freeaddrinfo(res);
        return EXIT_FAILURE;
    }

    char addr_str[INET6_ADDRSTRLEN];
    void *addr;
    if (p->ai_family == AF_INET) {
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
        addr = &(ipv4->sin_addr);
    } else { // AF_INET6
        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
        addr = &(ipv6->sin6_addr);
    }

    inet_ntop(p->ai_family, addr, addr_str, sizeof(addr_str));
    printf("Connected to %s:%s (IP: %s)\n", hostname, port, addr_str);

    freeaddrinfo(res);
    close(sock);

    return EXIT_SUCCESS;
}
