#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void handle_client(int client_socket) {
    srand(time(NULL));
    int n = rand() % 65536;
    char buffer[BUFFER_SIZE];
    int bytes_received;
    int tentatives = 0;

    while (tentatives < 20) {
        bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received < 0) {
            perror("Erreur lors de la réception");
            close(client_socket);
            return;
        }

        buffer[bytes_received] = '\0';
        printf("Client : %s", buffer);

        char *endptr;
        long guess = strtol(buffer, &endptr, 10);

        if (guess == n) {
            send(client_socket, "GAGNE\n", 6, 0);
            close(client_socket);
            return;
        } else if (guess < n) {
            snprintf(buffer, BUFFER_SIZE, "PLUS %d\n", 19 - tentatives);
            send(client_socket, buffer, strlen(buffer), 0);
        } else {
            snprintf(buffer, BUFFER_SIZE, "MOINS %d\n", 19 - tentatives);
            send(client_socket, buffer, strlen(buffer), 0);
        }
        tentatives++;
    }
    snprintf(buffer, BUFFER_SIZE, "PERDU. Le nombre était %d\n", n);
    send(client_socket, buffer, strlen(buffer), 0);
    printf("Perdu, le nombre était %d\n", n);
    close(client_socket);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Erreur lors de la création de la socket");
        return EXIT_FAILURE;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur lors de la liaison de la socket");
        close(server_socket);
        return EXIT_FAILURE;
    }

    if (listen(server_socket, 1) < 0) {
        perror("Erreur lors de la mise en écoute");
        close(server_socket);
        return EXIT_FAILURE;
    }

    printf("Serveur en attente de connexion...\n");

    while (1) {
        client_socket = accept(server_socket, NULL, NULL);
        if (client_socket < 0) {
            perror("Erreur lors de l'acceptation de la connexion");
            close(server_socket);
            return EXIT_FAILURE;
        }

        handle_client(client_socket);
    }

    close(server_socket);
    return EXIT_SUCCESS;
}
