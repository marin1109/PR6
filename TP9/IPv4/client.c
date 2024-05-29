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
#define IP_MULTICAST "225.225.225.225"
#define IF_NAME "wlo1"

int main(int argc, char *argv[]) {
    if (argc != 1) {
        printf("Usage: %s\n", argv[0]);
        return 1;
    }

    // Création du socket
    int sock;
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        return 1;
    }

    // Réutilisation de l'adresse
    int ok = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &ok, sizeof(ok)) < 0) {
        perror("setsockopt");
        close(sock);
        return 1;
    }

    // Initialisation de l'adresse du socket
    struct sockaddr_in adr;
    memset(&adr, 0, sizeof(adr));
    adr.sin_family = AF_INET;
    adr.sin_addr.s_addr = htonl(INADDR_ANY);
    adr.sin_port = htons(RECV_PORT);

    // Liaison du socket
    if (bind(sock, (struct sockaddr*)&adr, sizeof(adr)) < 0) {
        perror("bind");
        close(sock);
        return 1;
    }

    // Obtenir l'index de l'interface
    int ifindex = if_nametoindex(IF_NAME);
    if (ifindex == 0) {
        fprintf(stderr, "Failed to get interface index for %s\n", IF_NAME);
        close(sock);
        return 1;
    }

    // Rejoindre le groupe multicast
    struct ip_mreqn group;
    memset(&group, 0, sizeof(group));
    if (inet_aton(IP_MULTICAST, &group.imr_multiaddr) == 0) {
        fprintf(stderr, "Invalid multicast address: %s\n", IP_MULTICAST);
        close(sock);
        return 1;
    }
    group.imr_ifindex = ifindex;

    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &group, sizeof(group)) < 0) {
        perror("setsockopt");
        close(sock);
        return 1;
    }

    char buffer[BUFFER_SIZE];
    struct sockaddr_in sender;
    socklen_t sender_len = sizeof(sender);

    while (1) {
        sender_len = sizeof(sender);
        ssize_t n = recvfrom(sock, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&sender, &sender_len);
        if (n < 0) {
            perror("recvfrom");
            close(sock);
            return 1;
        }
        buffer[n] = '\0'; // Terminaison de chaîne
        printf("%s\n", buffer);
    }
    
    close(sock);
    return EXIT_SUCCESS;
}
