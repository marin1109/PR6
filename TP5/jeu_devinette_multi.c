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
#define BUFFER_SIZE 256

typedef struct {
    int* socks;
    pthread_mutex_t winner_mutex;
    pthread_cond_t winner_cond;
    int winner;
} game_data_t;

void *monitor_winner(void *arg);
void game(int* socks, game_data_t* game_data);
void *game_np(void *arg);
void *start_game(void *arg);

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
        pthread_t game_thread;
        int* args = malloc(2 * sizeof(int));
        if (args == NULL) {
            perror("malloc");
            close(sock_serv);
            exit(EXIT_FAILURE);
        }

        args[0] = sock_serv;
        args[1] = n;

        if (pthread_create(&game_thread, NULL, start_game, (void*)args) != 0) {
            perror("pthread_create");
            free(args);
            close(sock_serv);
            exit(EXIT_FAILURE);
        }

        pthread_detach(game_thread);
    }

    close(sock_serv);
    return EXIT_SUCCESS;
}

void *start_game(void *arg) {
    int* args = (int*)arg;
    int sock_serv = args[0];
    int n = args[1];
    free(args);

    unsigned short int mystery_number = rand() % (1 << 16);
    printf("Nombre mystère pour la partie = %d\n", mystery_number);

    int* socks = malloc((n + 2) * sizeof(int));
    if (socks == NULL) {
        perror("malloc");
        close(sock_serv);
        pthread_exit(NULL);
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
            pthread_exit(NULL);
        }
    }

    game_data_t game_data;
    game_data.socks = socks;
    pthread_mutex_init(&game_data.winner_mutex, NULL);
    pthread_cond_init(&game_data.winner_cond, NULL);
    game_data.winner = -1;

    pthread_t monitor_thread;
    if (pthread_create(&monitor_thread, NULL, monitor_winner, &game_data) != 0) {
        perror("pthread_create");
        for (int i = 0; i < n; i++) {
            close(socks[i + 2]);
        }
        free(socks);
        close(sock_serv);
        pthread_exit(NULL);
    }

    pthread_t* threads = malloc(n * sizeof(pthread_t));
    if (threads == NULL) {
        perror("malloc");
        for (int i = 0; i < n; i++) {
            close(socks[i + 2]);
        }
        free(socks);
        close(sock_serv);
        pthread_exit(NULL);
    }

    for (int i = 0; i < n; i++) {
        if (pthread_create(&threads[i], NULL, game_np, &game_data) != 0) {
            perror("pthread_create");
            for (int j = 0; j < n; j++) {
                close(socks[j + 2]);
            }
            free(socks);
            free(threads);
            close(sock_serv);
            pthread_exit(NULL);
        }
    }

    for (int i = 0; i < n; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_join(monitor_thread, NULL);

    free(socks);
    free(threads);
    pthread_mutex_destroy(&game_data.winner_mutex);
    pthread_cond_destroy(&game_data.winner_cond);

    pthread_exit(NULL);
}

void game(int* socks, game_data_t* game_data) {
    unsigned int guess;
    char msg[BUFFER_SIZE];
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

            pthread_mutex_lock(&game_data->winner_mutex);
            if (game_data->winner != -1) {
                pthread_mutex_unlock(&game_data->winner_mutex);
                return;
            }
            if (guess == n) {
                game_data->winner = i;
                sprintf(msg, "GAGNE\n");
                send(socks[i + 2], msg, strlen(msg), 0);
                pthread_cond_signal(&game_data->winner_cond);
                pthread_mutex_unlock(&game_data->winner_mutex);
                return;
            } else if (guess < n) {
                sprintf(msg, "PLUS\n");
            } else {
                sprintf(msg, "MOINS\n");
            }
            pthread_mutex_unlock(&game_data->winner_mutex);

            if (send(socks[i + 2], msg, strlen(msg), 0) < 0) {
                perror("send");
                return;
            }
        }
    }
}

void* monitor_winner(void* arg) {
    game_data_t* game_data = (game_data_t*)arg;
    int* socks = game_data->socks;
    int nb_joueurs = socks[0];

    pthread_mutex_lock(&game_data->winner_mutex);
    while (game_data->winner == -1) {
        pthread_cond_wait(&game_data->winner_cond, &game_data->winner_mutex);
    }
    pthread_mutex_unlock(&game_data->winner_mutex);

    for (int i = 0; i < nb_joueurs; i++) {
        if (i != game_data->winner) {
            if (send(socks[i + 2], "PERDU\n", 6, 0) < 0) {
                perror("send");
            }
            close(socks[i + 2]);
        }
    }
    close(socks[game_data->winner + 2]);

    return NULL;
}

void *game_np(void *arg) {
    game_data_t* game_data = (game_data_t*)arg;
    game(game_data->socks, game_data);
    return NULL;
}
