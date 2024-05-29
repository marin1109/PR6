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
#include <time.h>

#define BUFFER_SIZE 512

void send_countdown(int sock, int seconds) {
    for (int i = seconds; i > 0; i--) {
        char buffer[BUFFER_SIZE];
        snprintf(buffer, BUFFER_SIZE, "%d\n", i);
        send(sock, buffer, strlen(buffer), 0);
        sleep(1);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }
    srand(time(NULL));

    char buffer[BUFFER_SIZE];
    int activity;
    int sock_client;
    int x, y;
    int score = 0;

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

    if (setsockopt(sock_serv, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) {
        perror("setsockopt");
        close(sock_serv);
        return EXIT_FAILURE;
    }

    if (bind(sock_serv, (struct sockaddr *)&address_sock, sizeof(address_sock)) < 0) {
        perror("bind");
        close(sock_serv);
        return EXIT_FAILURE;
    }

    if (listen(sock_serv, 1) < 0) { // Utilisation de SOMAXCONN pour la file d'attente
        perror("listen");
        close(sock_serv);
        return EXIT_FAILURE;
    }
    
    // Allocation initiale pour le tableau de struct pollfd
    struct pollfd a_surveiller[2];
    a_surveiller[0].fd = sock_serv;
    a_surveiller[0].events = POLLIN;

    printf("En attente d'une connexion...\n");

    while (1) {
        activity = poll(a_surveiller, 1, -1); // Attendre une connexion

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

            a_surveiller[1].fd = sock_client;
            a_surveiller[1].events = POLLIN;

            printf("Client connecté !\n");

            for (int i = 0; i < 5; i++) {
                x = rand() % 20 + 1;
                y = rand() % 20 + 1;

                snprintf(buffer, BUFFER_SIZE, "Combien font %d * %d ?\n", x, y);
                send(sock_client, buffer, strlen(buffer), 0);

                send_countdown(sock_client, 10);

                activity = poll(&a_surveiller[1], 1, 0);

                if (activity < 0 && errno != EINTR) {
                    perror("poll");
                    break;
                }

                if (a_surveiller[1].revents & POLLIN) {
                    memset(buffer, 0, BUFFER_SIZE);
                    recv(sock_client, buffer, BUFFER_SIZE, 0);

                    if (atoi(buffer) == x * y) {
                        score++;
                        char *res = "Bonne réponse!\n";
                        send(sock_client, res, strlen(res), 0);
                    } else {
                        char *res = "Mauvaise réponse!\n";
                        send(sock_client, res, strlen(res), 0);
                    }
                } else {
                    char *res = "TROP LENT!!!\n";
                    send(sock_client, res, strlen(res), 0);
                }
            }

            snprintf(buffer, BUFFER_SIZE, "Multiplications correctes : %d\n", score);
            send(sock_client, buffer, strlen(buffer), 0);

            // Fermer la socket client une fois le jeu terminé
            close(sock_client);
            break;
        }
    }

    // Fermer la socket du serveur avant de quitter
    close(sock_serv);
    return EXIT_SUCCESS;
}
