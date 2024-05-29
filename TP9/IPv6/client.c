#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <time.h>

#define BUFFER_SIZE 1024
#define RECV_PORT 10201
#define IP_MULTICAST "FF12::1"
#define IF_NAME "wlo1"

int main(int argc, char *argv[]) {
    if (argc != 1) {
        printf("Usage: %s\n", argv[0]);
        return 1;
    }

    int sock;
    if ((sock = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        return 1;
    }

    int ok = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &ok, sizeof(ok)) < 0) {
        perror("setsockopt");
        close(sock);
        return 1;
    }

    struct sockaddr_in6 adr;
    memset(&adr, 0, sizeof(adr));
    adr.sin6_family = AF_INET6;
    adr.sin6_addr = in6addr_any;
    adr.sin6_port = htons(RECV_PORT);

    if (bind(sock, (struct sockaddr*)&adr, sizeof(adr)) < 0) {
        perror("bind");
        close(sock);
        return 1;
    }

    // Correction: Vérification de if_nametoindex avec un message d'erreur personnalisé
    int ifindex = if_nametoindex(IF_NAME);
    if (ifindex == 0) {
        fprintf(stderr, "Failed to get interface index for %s\n", IF_NAME);
        close(sock);
        return 1;
    }

    struct ipv6_mreq group;
    // Correction: Utilisation correcte de inet_pton et gestion des erreurs
    if (inet_pton(AF_INET6, IP_MULTICAST, &group.ipv6mr_multiaddr) != 1) {
        perror("inet_pton");
        close(sock);
        return 1;
    }
    group.ipv6mr_interface = ifindex;

    if (setsockopt(sock, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, &group, sizeof(group)) < 0) {
        perror("setsockopt");
        close(sock);
        return 1;
    }

    char buffer[BUFFER_SIZE];
    struct sockaddr_in6 sender;
    socklen_t sender_len = sizeof(sender);

    while (1) {
        // Correction: Initialisation de sender_len à chaque appel pour éviter des comportements imprévisibles
        sender_len = sizeof(sender);
        ssize_t n = recvfrom(sock, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&sender, &sender_len);
        if (n < 0) {
            perror("recvfrom");
            close(sock);
            return 1;
        }
        buffer[n] = '\0'; // Ajout de la terminaison de chaîne pour éviter des problèmes d'affichage
        printf("%s\n", buffer);
    }
    
    close(sock);

    return EXIT_SUCCESS;
}
