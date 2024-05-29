#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 12345
#define MAX_CLIENTS 10

void handle_client(int client_socket){
    int n = rand() % 65536;
    int attemps = 20;
    uint16_t guess;
    uint8_t response[2];
    printf("Le nombre à deviner est %d\n", n);

    while(attemps > 0){
        if(recv(client_socket, &guess, sizeof(guess), 0) < 0){
            perror("recv_serveur");
            exit(EXIT_FAILURE);
        }

        guess = ntohs(guess);
        
        if(n == guess){
            printf("Le client a réussi la devinette\n");
            response[0] = 0;
            response[1] = 1;
            if (send(client_socket, response, sizeof(response), 0) < 0) {
                perror("send_server");
            }
            break;
        }
        else if(guess < n){
            printf("Le client a proposé %d, c'est trop petit\n", guess);
            response[0] = --attemps;
            response[1] = 1;
            if (send(client_socket, response, sizeof(response), 0) < 0) {
                perror("send_server");
            }
        }
        else if(guess > n){
            printf("Le client a proposé %d, c'est trop grand\n", guess);
            response[0] = --attemps;
            response[1] = 0;
            if (send(client_socket, response, sizeof(response), 0) < 0) {
                perror("send_server");
            }
        }

        if(attemps == 0){
            printf("Le client a épuisé ses tentatives\n");
            response[0] = 0;
            response[1] = 0;
            if (send(client_socket, response, sizeof(response), 0) < 0) {
                perror("send_server");
            }
        }

    }
    close(client_socket);
}

int main(){
    srand(time(NULL));
    int socket_serveur, socket_client;
    struct sockaddr_in connect_serveur;

    socket_serveur = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_serveur < 0){
        perror("socket_serveur");
        return EXIT_FAILURE;
    }

    memset(&connect_serveur, 0, sizeof(connect_serveur));
    connect_serveur.sin_family = AF_INET;
    connect_serveur.sin_addr.s_addr = INADDR_ANY;
    connect_serveur.sin_port = htons(PORT);

    if(bind(socket_serveur, (struct sockaddr*)&connect_serveur, sizeof(connect_serveur)) < 0) {
        perror("bind");
        close(socket_serveur);
        return EXIT_FAILURE;
    }

    if(listen(socket_serveur, MAX_CLIENTS) < 0) {
        perror("listen");
        close(socket_serveur);
        return EXIT_FAILURE;
    }

    while(1){
        socket_client = accept(socket_serveur, NULL, NULL);
        if(socket_client < 0){
            perror("accept");
            close(socket_client);
            continue;
        }

        handle_client(socket_client);
    }

    close(socket_serveur);

    return EXIT_SUCCESS;
}