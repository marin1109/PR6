#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

void play_game(int sock) {
    char buffer[1024] = {0};
    int low = 0, high = 65535, guess;
    int attempts_left;

    for (int i = 0; i < 20; i++) {
        guess = (low + high) / 2;
        sprintf(buffer, "%d\n", guess);
        send(sock, buffer, strlen(buffer), 0);
        read(sock, buffer, 1024);

        if (strncmp(buffer, "GAGNE", 5) == 0) {
            printf("GAGNE\n");
            break;
        } else if (strncmp(buffer, "PLUS", 4) == 0) {
            sscanf(buffer, "PLUS %d", &attempts_left);
            low = guess + 1;
        } else if (strncmp(buffer, "MOINS", 5) == 0) {
            sscanf(buffer, "MOINS %d", &attempts_left);
            high = guess - 1;
        }

        if (attempts_left == 0) {
            printf("PERDU\n");
            break;
        }
    }
}

int main(int argc, char const *argv[]) {
    struct sockaddr_in serv_addr;
    int sock = 0;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    play_game(sock);

    close(sock);
    return 0;
}
