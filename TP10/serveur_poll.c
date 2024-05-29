#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>

#define BUFFER_SIZE 512
#define FD_SIZE 10

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char buffer[BUFFER_SIZE];
    int activity;
    int sock_client;

    int sock_serv = socket(PF_INET, SOCK_STREAM, 0);
    if (sock_serv < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }

    struct sockaddr_in address_sock;
    memset(&address_sock, 0, sizeof(address_sock));
    address_sock.sin_family = AF_INET;
    address_sock.sin_port = htons(atoi(argv[1]));
    address_sock.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock_serv, (struct sockaddr *)&address_sock, sizeof(address_sock)) < 0) {
        perror("bind");
        close(sock_serv);
        return EXIT_FAILURE;
    }

    if (listen(sock_serv, SOMAXCONN) < 0) { // Utilisation de SOMAXCONN pour la file d'attente
        perror("listen");
        close(sock_serv);
        return EXIT_FAILURE;
    }

    // Allocation initiale pour le tableau de struct pollfd
    struct pollfd *a_surveiller = malloc(sizeof(*a_surveiller) * FD_SIZE);
    if (!a_surveiller) {
        perror("malloc");
        close(sock_serv);
        return EXIT_FAILURE;
    }
    a_surveiller[0].fd = sock_serv;
    a_surveiller[0].events = POLLIN;
    int client_count = 1; // Compteur de clients
    int max_size = FD_SIZE;

    while (1) {
        activity = poll(a_surveiller, client_count, -1);

        if (activity < 0 && errno != EINTR) {
            perror("poll");
            break;
        }

        if (a_surveiller[0].revents & POLLIN) { // Nouveau client
            sock_client = accept(sock_serv, NULL, NULL);
            if (sock_client < 0) {
                perror("accept");
                continue;
            }

            fcntl(sock_client, F_SETFL, O_NONBLOCK); // Mettre le socket client en mode non-bloquant

            if (client_count == max_size) { // Si le tableau est plein, doubler sa taille
                max_size *= 2;
                a_surveiller = realloc(a_surveiller, sizeof(*a_surveiller) * max_size);
                if (!a_surveiller) {
                    perror("realloc");
                    close(sock_serv);
                    return EXIT_FAILURE;
                }
            }

            a_surveiller[client_count].fd = sock_client;
            a_surveiller[client_count].events = POLLIN;
            client_count++; // Incrémenter le compteur de clients

            printf("Nouveau client connecté, socket fd %d\n", sock_client);
        }

        for (int i = 1; i < client_count; i++) { // Parcourir les clients
            if (a_surveiller[i].revents & POLLIN) {
                int r = recv(a_surveiller[i].fd, buffer, BUFFER_SIZE, 0);
                if (r < 0) {
                    perror("recv");
                    continue;
                }

                if (r == 0) { // Client déconnecté
                    close(a_surveiller[i].fd);
                    printf("Client déconnecté, socket fd %d\n", a_surveiller[i].fd);
                    a_surveiller[i] = a_surveiller[client_count - 1]; // Remplacer par le dernier élément
                    client_count--; // Réduire le compteur de clients
                    i--; // Ré-évaluer l'index actuel
                } else {
                    buffer[r] = '\0';
                    send(a_surveiller[i].fd, buffer, r, 0); // Réponse en écho
                }
            }
        }
    }

    close(sock_serv);
    free(a_surveiller); // Libérer la mémoire allouée

    return EXIT_SUCCESS;
}
