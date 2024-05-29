#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <limits.h>

#define PORT 8080

pthread_mutex_t winner_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t winner_cond = PTHREAD_COND_INITIALIZER;
int winner = -1;


void *monitor_winner(void *arg);
void game(int* socks);
void *game_np(void *arg);

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <number of users>\n", argv[0]);
        return EXIT_FAILURE;
    }

    srand(time(NULL));
    unsigned short int mystery_number;

    char *endptr;
    long val = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0' || val <= 0 || val > INT_MAX) {
        fprintf(stderr, "Invalid number of users\n");
        return EXIT_FAILURE;
    }
    int n = (int)val;

    int sock_serv = socket(PF_INET6, SOCK_STREAM, 0);
    if (sock_serv < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    int yes = 1;
    if (setsockopt(sock_serv, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
        perror("setsockopt");
        close(sock_serv);
        exit(EXIT_FAILURE);
    }

    int no = 0;
    if (setsockopt(sock_serv, IPPROTO_IPV6, IPV6_V6ONLY, &no, sizeof(no)) < 0) {
        perror("setsockopt");
        close(sock_serv);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in6 serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin6_family = AF_INET6;
    serv_addr.sin6_port = htons(PORT);
    serv_addr.sin6_addr = in6addr_any;

    if (bind(sock_serv, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Bind failed");
        close(sock_serv);
        exit(EXIT_FAILURE);
    }

    if (listen(sock_serv, SOMAXCONN) < 0) {
        perror("Listen failed");
        close(sock_serv);
        exit(EXIT_FAILURE);
    }

    printf("Serveur en attente de connexions sur le port %d\n", PORT);

    while (1) {
        mystery_number = rand() % (1 << 16);
        printf("Nombre mystère pour la partie = %d\n", mystery_number);

        int* socks = malloc((n + 2) * sizeof(int));
        if (socks == NULL) {
            perror("malloc");
            close(sock_serv);
            exit(EXIT_FAILURE);
        }

        socks[0] = n;
        socks[1] = mystery_number;

        for (int i = 0; i < n; i++) {
            socks[i + 2] = accept(sock_serv, NULL, NULL);
            if (socks[i + 2] < 0) {
                perror("Accept failed");
                for (int j = 0; j < i; j++) {
                    close(socks[j + 2]);
                }
                free(socks);
                close(sock_serv);
                exit(EXIT_FAILURE);
            }
        }

        pthread_t monitor_thread;
        if (pthread_create(&monitor_thread, NULL, monitor_winner, (void*)socks) != 0) {
            perror("pthread_create");
            for (int i = 0; i < n; i++) {
                close(socks[i + 2]);
            }
            free(socks);
            close(sock_serv);
            exit(EXIT_FAILURE);
        }

        pthread_t* threads = malloc(n * sizeof(pthread_t));
        if (threads == NULL) {
            perror("malloc");
            for (int i = 0; i < n; i++) {
                close(socks[i + 2]);
            }
            free(socks);
            close(sock_serv);
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < n; i++) {
            if (pthread_create(&threads[i], NULL, game_np, (void*)socks) != 0) {
                perror("pthread_create");
                for (int j = 0; j < n; j++) {
                    close(socks[j + 2]);
                }
                free(socks);
                free(threads);
                close(sock_serv);
                exit(EXIT_FAILURE);
            }
        }

        for (int i = 0; i < n; i++) {
            pthread_join(threads[i], NULL);
        }

        pthread_join(monitor_thread, NULL);

        free(socks);
        free(threads);
        winner = -1;
    }

    close(sock_serv);
    pthread_mutex_destroy(&winner_mutex);
    pthread_cond_destroy(&winner_cond);

    return EXIT_SUCCESS;
}


void game(int* socks) {
    unsigned int guess;
    char msg[256];
    int nb_joueurs = socks[0];
    int n = socks[1];

    while (1) {
        for (int i = 0; i < nb_joueurs; i++) {
            memset(msg, 0, sizeof(msg));
            int recv_size = recv(socks[i + 2], &msg, sizeof(msg) - 1, 0);
            if (recv_size <= 0) {
                perror("recv");
                return;
            }

            msg[recv_size] = '\0';
            guess = atoi(msg);
            printf("Joueur %d a proposé %d\n", i + 1, guess);

            pthread_mutex_lock(&winner_mutex);
            if (winner != -1) {
                pthread_mutex_unlock(&winner_mutex);
                return;
            }
            if (guess == n) {
                winner = i;
                sprintf(msg, "GAGNE\n");
                send(socks[i + 2], msg, strlen(msg), 0);
                pthread_cond_signal(&winner_cond);
                pthread_mutex_unlock(&winner_mutex);
                return;
            } else if (guess < n) {
                sprintf(msg, "PLUS\n");
            } else {
                sprintf(msg, "MOINS\n");
            }
            pthread_mutex_unlock(&winner_mutex);

            if (send(socks[i + 2], msg, strlen(msg), 0) < 0) {
                perror("send");
                return;
            }
        }
    }
}

void* monitor_winner(void* arg) {
    int* socks = (int*)arg;
    int nb_joueurs = socks[0];

    pthread_mutex_lock(&winner_mutex);
    while (winner == -1) {
        pthread_cond_wait(&winner_cond, &winner_mutex);
    }
    pthread_mutex_unlock(&winner_mutex);


    for (int i = 0; i < nb_joueurs; i++) {
        if (i != winner) {
            if (send(socks[i + 2], "PERDU\n", 6, 0) < 0) {
                perror("send");
            }
            close(socks[i + 2]);
        }
    }
    close(socks[winner + 2]);

    return NULL;
}


void *game_np(void *arg) {
    int *socks = (int*)arg;
    game(socks);
    return NULL;
}
