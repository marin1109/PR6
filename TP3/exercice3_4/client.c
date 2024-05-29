#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12345
#define IP_ADDRESS "127.0.0.1"

void play_game(int socket_client){
    int min = 0, max = 65535;
    uint16_t guess;
    uint8_t response[2];

    while(1){
        guess = htons((min + max) / 2);
        printf("Proposition : %d\n", ntohs(guess));
        if(send(socket_client, &guess, sizeof(guess), 0) < 0) {
            perror("send_client");
            close(socket_client);
            exit(EXIT_FAILURE);
        }
        
        if(recv(socket_client, response, sizeof(response), 0) < 0) {
            perror("recv_client");
            close(socket_client);
            exit(EXIT_FAILURE);
        }

        if (response[1] == 1 && response[0] == 0) {
            printf("GAGNE\n");
            break;
        }
        else if (response[1] == 0 && response[0] == 0) {
            printf("PERDU\n");
            break;
        }
        else if (response[1] == 1) {
            min = ntohs(guess) + 1;
        }
        else if (response[1] == 0) {
            max = ntohs(guess) - 1;
        }
    }

    close(socket_client);
}

int main(){
    int socket_client;
    struct sockaddr_in connect_client;

    socket_client = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_client < 0) {
        perror("socket_client");
        return EXIT_FAILURE;
    }

    memset(&connect_client, 0, sizeof(connect_client));
    connect_client.sin_family = AF_INET;
    connect_client.sin_addr.s_addr = inet_addr(IP_ADDRESS);
    connect_client.sin_port = htons(PORT);

    if(connect(socket_client, (struct sockaddr*)&connect_client, sizeof(connect_client)) < 0) {
        perror("connect_client");
        close(socket_client);
        return EXIT_FAILURE;
    }

    play_game(socket_client);

    return EXIT_SUCCESS;
}