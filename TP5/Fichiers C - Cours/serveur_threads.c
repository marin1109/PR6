#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define SIZE_BUF 256

// Fonction de service pour gérer les connexions clients
void *serve(void *arg) {
    int sock = *((int *) arg);
    char buf[SIZE_BUF];
    memset(buf, 0, sizeof(buf));
    
    // Réception des données du client
    int recu = recv(sock, buf, SIZE_BUF, 0);
    if (recu < 0) {
        perror("recv");
        close(sock);
        free(arg);
        int *ret = malloc(sizeof(int));
        *ret = 1;
        pthread_exit(ret);
    }
    if (recu == 0) {
        fprintf(stderr, "send du client nul\n");
        close(sock);
        free(arg);
        return NULL;
    }

    // Affichage des données reçues
    printf("reçu : %s\n", buf);

    // Envoi d'une réponse au client
    char c = 'o';
    int ecrit = send(sock, &c, 1, 0);
    if (ecrit <= 0) {
        perror("send");
    }

    // Fermeture de la connexion
    close(sock);
    free(arg);
    return NULL;
}

int main(int argc, char *argv[]) {
    // Vérification du nombre d'arguments
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Création de l'adresse du serveur
    struct sockaddr_in6 address_sock;
    memset(&address_sock, 0, sizeof(address_sock));
    address_sock.sin6_family = AF_INET6;
    address_sock.sin6_port = htons(atoi(argv[1]));
    address_sock.sin6_addr = in6addr_any;

    // Création de la socket
    int sock = socket(PF_INET6, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("creation socket");
        exit(EXIT_FAILURE);
    }

    // Réutilisation de l'adresse
    int opt = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Liaison de la socket à l'adresse et au port
    if (bind(sock, (struct sockaddr *) &address_sock, sizeof(address_sock)) < 0) {
        perror("erreur bind");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Le serveur écoute les connexions entrantes
    if (listen(sock, SOMAXCONN) < 0) {
        perror("erreur listen");
        close(sock);
        exit(EXIT_FAILURE);
    }

    while (1) {
        struct sockaddr_in6 addrclient;
        socklen_t size = sizeof(addrclient);

        // Allocation de mémoire pour la socket client
        int *sock_client = malloc(sizeof(int));
        if (!sock_client) {
            perror("malloc");
            continue;
        }

        // Acceptation d'une nouvelle connexion
        *sock_client = accept(sock, (struct sockaddr *) &addrclient, &size);
        if (*sock_client < 0) {
            perror("accept");
            free(sock_client);
            continue;
        }

        // Création d'un thread pour gérer la connexion
        pthread_t thread;
        if (pthread_create(&thread, NULL, serve, sock_client)) {
            perror("pthread_create");
            close(*sock_client);
            free(sock_client);
            continue;
        }

        // Détachement du thread pour éviter les fuites de mémoire
        pthread_detach(thread);

        // Affichage de l'adresse du client
        char nom_dst[INET6_ADDRSTRLEN];
        printf("client connecté : %s %d\n", inet_ntop(AF_INET6, &addrclient.sin6_addr, nom_dst, sizeof(nom_dst)), ntohs(addrclient.sin6_port));
    }

    // Fermeture de la socket serveur
    close(sock);
    return 0;
}
