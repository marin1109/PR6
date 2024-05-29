#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 5555
#define BUFFER_SIZE 4096
#define MAX_CLIENTS 10
#define START_MSG "DEBUT\n"
#define ACK_MSG "BIEN RECU\n"

int main(){
    int socket_server, socket_client;
    struct sockaddr_in server;
    char buffer[BUFFER_SIZE] = {0};
    FILE *file;

    // Créer le socket
    socket_server = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_server < 0) {
        perror("socket_server");
        return EXIT_FAILURE;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    // Attacher le socket au port
    if(bind(socket_server, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("bind_server");
        close(socket_server);
        return EXIT_FAILURE;
    }

    // Écouter les connexions entrantes
    if(listen(socket_server, MAX_CLIENTS) < 0) {
        perror("listen_server");
        close(socket_server);
        return EXIT_FAILURE;
    }

    printf("Serveur en attente de connexion...\n");

    // Accepter une connexion
    if((socket_client = accept(socket_server, NULL, NULL)) < 0) {
        perror("accept_server");
        close(socket_server);
        return EXIT_FAILURE;
    }

    // Envoyer "DEBUT\n" au client
    send(socket_client, START_MSG, strlen(START_MSG), 0);

    // Recevoir la taille du fichier
    recv(socket_client, buffer, BUFFER_SIZE, 0);
    int file_size = atoi(buffer);
    printf("Taille du fichier : %d\n", file_size);

    // Ouvrir le fichier en écriture
    file = fopen("data.txt", "wb");
    if (file == NULL) {
        perror("fopen");
        close(socket_client);
        close(socket_server);
        return EXIT_FAILURE;
    }

    // Lire le contenu du fichier envoyé par le client
    int total_bytes_received = 0;
    while (total_bytes_received < file_size) {
        int bytes_read = recv(socket_client, buffer, BUFFER_SIZE, 0);
        fwrite(buffer, 1, bytes_read, file);
        total_bytes_received += bytes_read;
    }

    fclose(file);

    // Envoyer "BIEN RECU\n" au client
    send(socket_client, ACK_MSG, strlen(ACK_MSG), 0);

    // Fermer la connexion
    close(socket_client);
    close(socket_server);

    return EXIT_SUCCESS;
}
