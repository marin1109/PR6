#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <bits/waitflags.h>
#include <sys/wait.h>

#define PORT 4242
#define IP6_ADDRESS "::1"

// Prototypes des fonctions
int server_1p();
int server_1p_fork();
int server_1p_thread();
int server_np(int n, int k);
void game_1p(int sock);
void game_np(int njoueurs, int k, int* socks);
void* game_1p_thread_func(void* arg);
void* game_np_thread_func(void* arg);

int main(int argc, char **argv) {
    if (argc != 2 && argc != 4) {
        fprintf(stderr, "Usage: %s <(1p|1p-fork|1p-thread)>\nUsage: %s <-np> <nombre_de_joueurs> <nombre_essais>\n", argv[0], argv[0]);
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "1p") == 0) {
        return server_1p();
    } else if (strcmp(argv[1], "1p-fork") == 0) {
        return server_1p_fork();
    } else if (strcmp(argv[1], "1p-thread") == 0) {
        return server_1p_thread();
    } else if (strcmp(argv[1], "-np") == 0 && argc == 4) {
        int n = atoi(argv[2]);
        int k = atoi(argv[3]);
        server_np(n, k);
    } else {
        fprintf(stderr, "Argument invalide : %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void game_1p(int sock) {
    srand(time(NULL) + sock);

    // Valeur aléatoire mystère entre 0 et 65535
    unsigned short int n = rand() % (1 << 16);
    printf("Nb mystère pour partie socket %d = %d\n", sock, n);

    unsigned short int guess;
    int tentatives = 20;
    int gagne = 0;
    char buff_in[100];

    while (recv(sock, buff_in, sizeof(buff_in), 0) > 0) {
        sscanf(buff_in, "%hu", &guess);
        printf("Joueur courant a envoyé : %d\n", guess);

        char reponse[20];
        if (n != guess) {
            tentatives--;
        }

        if (tentatives == 0) {
            sprintf(reponse, "PERDU\n");
        } else if (n < guess) {
            sprintf(reponse, "MOINS %d\n", tentatives);
        } else if (n > guess) {
            sprintf(reponse, "PLUS %d\n", tentatives);
        } else {
            sprintf(reponse, "GAGNE\n");
            gagne = 1;
        }

        send(sock, reponse, strlen(reponse), 0);

        if (gagne || tentatives == 0) {
            break;
        }
    }

    printf("Fin de partie\n");
    close(sock);
}

void game_np(int njoueurs, int k, int* socks) {
    srand(time(NULL));
    unsigned short int n = rand() % (1 << 16);
    printf("Nombre mystère pour la partie = %d\n", n);

    unsigned short int guess;
    int taille = 0;
    int tentatives[njoueurs];
    int gagne[njoueurs];
    int joueurs_gagne = 0;
    int joueurs_tentatives_epuise = 0;
    char buff_in[100];
    char reponse[20];

    for (int i = 0; i < njoueurs; i++) {
        tentatives[i] = k;
        gagne[i] = 0;
    }

    while (joueurs_gagne + joueurs_tentatives_epuise < njoueurs) {
        for (int i = 0; i < njoueurs; i++) {
            if (tentatives[i] > 0 && (taille = recv(socks[i], buff_in, 100, 0)) > 0) {
                sscanf(buff_in, "%hu", &guess);
                printf("Joueur %d a envoyé : %d\n", i + 1, guess);

                if (n == guess) {
                    sprintf(reponse, "GAGNE %d\n", joueurs_gagne + 1);
                    joueurs_gagne++;
                    gagne[i] = 1;
                } else if (tentatives[i] == 1) {
                    sprintf(reponse, "PERDU\n");
                    joueurs_tentatives_epuise++;
                } else {
                    tentatives[i]--;
                    if (n < guess) {
                        sprintf(reponse, "MOINS %d\n", tentatives[i]);
                    } else {
                        sprintf(reponse, "PLUS %d\n", tentatives[i]);
                    }
                }

                send(socks[i], reponse, strlen(reponse), 0);

                if (gagne[i] || tentatives[i] == 0) {
                    close(socks[i]);
                }
            }
        }
    }

    for (int i = 0; i < njoueurs; i++) {
        close(socks[i]);
    }
    printf("Fin de partie\n");
}

int server_1p() {
    int serv_sock = socket(PF_INET6, SOCK_STREAM, 0);
    if (serv_sock < 0) {
        perror("Erreur de création du socket");
        exit(EXIT_FAILURE);
    }

    int no = 0;
    if (setsockopt(serv_sock, IPPROTO_IPV6, IPV6_V6ONLY, &no, sizeof(no)) < 0) {
        perror("Erreur de setsockopt");
        close(serv_sock);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in6 serv_addr;

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin6_family = AF_INET6;
    serv_addr.sin6_port = htons(PORT);
    inet_pton(AF_INET6, IP6_ADDRESS, &serv_addr.sin6_addr);

    if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Erreur de bind");
        close(serv_sock);
        exit(EXIT_FAILURE);
    }

    if (listen(serv_sock, 1) < 0) {
        perror("Échec de listen");
        close(serv_sock);
        exit(EXIT_FAILURE);
    }

    printf("Serveur prêt, en attente de connexions...\n");

    while (1) {
        int client_sock = accept(serv_sock, NULL, NULL);
        if (client_sock < 0) {
            perror("Échec de accept");
            close(serv_sock);
            exit(EXIT_FAILURE);
        }

        printf("Connexion acceptée, nouvelle partie lancée.\n");
        game_1p(client_sock);
    }

    close(serv_sock);
    return 0;
}

int server_1p_fork() {
    int serv_sock = socket(PF_INET6, SOCK_STREAM, 0);
    if (serv_sock < 0) {
        perror("Erreur de création du socket");
        exit(EXIT_FAILURE);
    }

    int no = 0;
    if (setsockopt(serv_sock, IPPROTO_IPV6, IPV6_V6ONLY, &no, sizeof(no)) < 0) {
        perror("Erreur de setsockopt");
        close(serv_sock);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in6 serv_addr;

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin6_family = AF_INET6;
    serv_addr.sin6_port = htons(PORT);
    inet_pton(AF_INET6, IP6_ADDRESS, &serv_addr.sin6_addr);

    if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Erreur de bind");
        close(serv_sock);
        exit(EXIT_FAILURE);
    }

    if (listen(serv_sock, SOMAXCONN) < 0) {
        perror("Échec de listen");
        close(serv_sock);
        exit(EXIT_FAILURE);
    }

    printf("Serveur prêt, en attente de connexions...\n");

    while (1) {
        int client_sock = accept(serv_sock, NULL, NULL);
        if (client_sock < 0) {
            perror("Échec de accept");
            close(serv_sock);
            exit(EXIT_FAILURE);
        }

        printf("Connexion acceptée, nouvelle partie lancée.\n");
        pid_t pid = fork();
        if (pid < 0) {
            perror("Erreur de fork");
            close(serv_sock);
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            close(serv_sock);
            game_1p(client_sock);
            exit(EXIT_SUCCESS);
        } else {
            close(client_sock);
            waitpid(-1, NULL, WNOHANG);
        }
    }

    close(serv_sock);
    return 0;
}

int server_1p_thread() {
    int sock_serv = socket(PF_INET6, SOCK_STREAM, 0);
    if (sock_serv < 0) {
        perror("Erreur de création du socket");
        exit(EXIT_FAILURE);
    }

    int no = 0;
    if (setsockopt(sock_serv, IPPROTO_IPV6, IPV6_V6ONLY, &no, sizeof(no)) < 0) {
        perror("Erreur de setsockopt");
        close(sock_serv);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in6 serv_addr;

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin6_family = AF_INET6;
    serv_addr.sin6_port = htons(PORT);
    inet_pton(AF_INET6, IP6_ADDRESS, &serv_addr.sin6_addr);
    
    if (bind(sock_serv, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Erreur de bind");
        close(sock_serv);
        exit(EXIT_FAILURE);
    }

    if (listen(sock_serv, SOMAXCONN) < 0) {
        perror("Erreur de listen");
        close(sock_serv);
        exit(EXIT_FAILURE);
    }

    printf("Serveur prêt, en attente de connexions...\n");

    while(1) {
        int* sock_client = malloc(sizeof(int));
        if (!sock_client) {
            perror("malloc");
            continue;
        }
        *sock_client = accept(sock_serv, NULL, NULL);
        if (*sock_client < 0) {
            perror("accept");
            free(sock_client);
            continue;
        }

        pthread_t thread;
        if (pthread_create(&thread, NULL, game_1p_thread_func, sock_client) != 0) {
            perror("pthread_create");
            free(sock_client);
            continue;
        }
        pthread_detach(thread);
    }

    close(sock_serv);

    return EXIT_SUCCESS;  
}

int server_np(int n, int k) {
    int sock_serv = socket(PF_INET6, SOCK_STREAM, 0);
    if (sock_serv < 0) {
        perror("Erreur de création du socket");
        exit(EXIT_FAILURE);
    }

    int no = 0;
    if (setsockopt(sock_serv, IPPROTO_IPV6, IPV6_V6ONLY, &no, sizeof(no)) < 0) {
        perror("Erreur de setsockopt");
        close(sock_serv);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in6 serv_addr;

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin6_family = AF_INET6;
    serv_addr.sin6_port = htons(PORT);
    serv_addr.sin6_addr = in6addr_any;

    if (bind(sock_serv, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Erreur de bind");
        close(sock_serv);
        exit(EXIT_FAILURE);
    }

    if (listen(sock_serv, SOMAXCONN) < 0) {
        perror("Échec de listen");
        close(sock_serv);
        exit(EXIT_FAILURE);
    }

    printf("Serveur prêt, en attente de connexions...\n");

    while (1) {
        int* socks = malloc(sizeof(int) * (n + 2));
        if (!socks) {
            perror("malloc");
            continue;
        }
        socks[0] = n;
        socks[1] = k;
        for (int i = 0; i < n; i++) {
            socks[i + 2] = accept(sock_serv, NULL, NULL);
            if (socks[i + 2] < 0) {
                perror("accept");
                for (int j = 0; j < i; j++) {
                    close(socks[j + 2]);
                }
                free(socks);
                continue;
            }
        }

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, game_np_thread_func, socks) != 0) {
            perror("pthread_create");
            for (int i = 0; i < n; i++) {
                close(socks[i + 2]);
            }
            free(socks);
            continue;
        }

        pthread_detach(thread_id);
    }

    close(sock_serv);
    return EXIT_SUCCESS;
}

void* game_1p_thread_func(void* arg) {
    int* sock = (int*)arg;
    game_1p(*sock);
    free(sock);
    return NULL;
}

void* game_np_thread_func(void* arg) {
    int* socks = (int*)arg;
    game_np(socks[0], socks[1], &socks[2]);
    free(socks);
    return NULL; 
}
