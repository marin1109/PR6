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
#define RECV_PORT 12121
#define SEND_PORT 10201
#define IP_MULTICAST "225.225.225.225"
#define IF_NAME "wlo1"

int main(int argc, char *argv[]) {
    if (argc != 1) {
        printf("Usage: %s\n", argv[0]);
        return 1;
    }
    
    char buffer[BUFFER_SIZE];

    // Création du socket de réception
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }

    // Autoriser plusieurs sockets à utiliser le même numéro de port
    int yes = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
        perror("setsockopt");
        close(sockfd);
        return EXIT_FAILURE;
    }

    // Initialisation de l'adresse de réception
    struct sockaddr_in gradr;
    memset(&gradr, 0, sizeof(gradr));
    gradr.sin_family = AF_INET;
    gradr.sin_port = htons(RECV_PORT);
    gradr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Liaison du socket à l'adresse et au port
    if (bind(sockfd, (struct sockaddr*)&gradr, sizeof(gradr)) < 0) {
        perror("bind");
        close(sockfd);
        return EXIT_FAILURE;
    }

    // Configuration du groupe de multicast
    struct ip_mreq group;
    memset(&group, 0, sizeof(group));
    if (inet_aton(IP_MULTICAST, &group.imr_multiaddr) == 0) {
        fprintf(stderr, "Invalid multicast address: %s\n", IP_MULTICAST);
        close(sockfd);
        return EXIT_FAILURE;
    }
    group.imr_interface.s_addr = htonl(INADDR_ANY); // Utiliser n'importe quelle interface

    // Rejoindre le groupe de multicast
    if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &group, sizeof(group)) < 0) {
        perror("setsockopt");
        close(sockfd);
        return EXIT_FAILURE;
    }

    // Boucle pour recevoir et envoyer des paquets
    while (1) {
        struct sockaddr_in gradr_recv;
        socklen_t addr_len = sizeof(gradr_recv);

        // Réception d'un message
        ssize_t recv_len = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&gradr_recv, &addr_len);
        if (recv_len < 0) {
            perror("recvfrom");
            close(sockfd);
            return EXIT_FAILURE;
        }
        buffer[recv_len] = '\0';  // S'assurer que la chaîne de caractères est terminée correctement

        // Obtention de l'heure actuelle
        time_t now = time(NULL);
        struct tm *tm_struct = localtime(&now);
        if (tm_struct == NULL) {
            perror("localtime");
            close(sockfd);
            return EXIT_FAILURE;
        }
        char time_buf[9];
        strftime(time_buf, sizeof(time_buf), "%H:%M:%S", tm_struct);

        // Obtention de l'adresse de l'expéditeur
        char adr_buf[INET_ADDRSTRLEN];
        if (inet_ntop(AF_INET, &gradr_recv.sin_addr, adr_buf, sizeof(adr_buf)) == NULL) {
            perror("inet_ntop");
            close(sockfd);
            return EXIT_FAILURE;
        }

        // Création du socket d'envoi
        int sockfd_send = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd_send < 0) {
            perror("socket");
            close(sockfd);
            return EXIT_FAILURE;
        }

        // Initialisation de l'adresse d'envoi
        struct sockaddr_in gradr_send;
        memset(&gradr_send, 0, sizeof(gradr_send));
        gradr_send.sin_family = AF_INET;
        if (inet_pton(AF_INET, IP_MULTICAST, &gradr_send.sin_addr) != 1) {
            perror("inet_pton");
            close(sockfd_send);
            close(sockfd);
            return EXIT_FAILURE;
        }
        gradr_send.sin_port = htons(SEND_PORT);

        // Formater le message à envoyer
        char send_buf[BUFFER_SIZE + INET_ADDRSTRLEN + sizeof(time_buf) + 2];
        snprintf(send_buf, sizeof(send_buf), "%s\t%s\t%s", adr_buf, time_buf, buffer);

        // Envoyer le message
        if (sendto(sockfd_send, send_buf, strlen(send_buf), 0, (struct sockaddr*)&gradr_send, sizeof(gradr_send)) < 0) {
            perror("sendto");
            close(sockfd_send);
            close(sockfd);
            return EXIT_FAILURE;
        }

        close(sockfd_send);  // Fermer le socket d'envoi après utilisation
    }

    close(sockfd);  // Fermer le socket de réception (inatteignable dans ce code)

    return EXIT_SUCCESS;
}
