#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8888
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    int server_socket, client_socket;
    struct sockaddr_in6 server_addr, client_addr;
    socklen_t client_len;
    char buffer[BUFFER_SIZE];
    int bytes_received;

    // Création de la socket serveur
    server_socket = socket(AF_INET6, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Erreur lors de la création de la socket");
        return EXIT_FAILURE;
    }

    // Initialisation de l'adresse du serveur
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_addr = in6addr_any;
    server_addr.sin6_port = htons(PORT);

    // Liaison de la socket serveur à l'adresse
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur lors de la liaison de la socket");
        close(server_socket);
        return EXIT_FAILURE;
    }

    // Mise en écoute des connexions entrantes
    if (listen(server_socket, 5) < 0) {
        perror("Erreur lors de la mise en écoute");
        close(server_socket);
        return EXIT_FAILURE;
    }

    printf("Serveur en attente de connexion...\n");

    while (1) {
        // Acceptation d'une connexion entrante
        client_len = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("Erreur lors de l'acceptation de la connexion");
            close(server_socket);
            return EXIT_FAILURE;
        }

        char client_ip[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, &client_addr.sin6_addr, client_ip, sizeof(client_ip));
        printf("Client connecté : %s\n", client_ip);

        // Réception et renvoi des messages du client
        while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
            send(client_socket, buffer, bytes_received, 0);
        }

        if (bytes_received < 0) {
            perror("Erreur lors de la réception");
        }

        printf("Client déconnecté\n");

        // Fermeture de la socket client
        close(client_socket);
    }

    // Fermeture de la socket serveur
    close(server_socket);

    return EXIT_SUCCESS;
}
