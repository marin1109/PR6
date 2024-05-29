#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 5555
#define IP_ADDRESS "127.0.0.1"
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]){
    if(argc != 2){
        printf("Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *file = fopen(argv[1], "rb");
    if (file == NULL) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    int socket_client;
    struct sockaddr_in connect_client;
    char buffer[BUFFER_SIZE] = {0};

    socket_client = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_client < 0) {
        perror("socket_client");
        fclose(file);
        return EXIT_FAILURE;
    }

    memset(&connect_client, 0, sizeof(connect_client));
    connect_client.sin_family = AF_INET;
    connect_client.sin_port = htons(PORT);
    connect_client.sin_addr.s_addr = inet_addr(IP_ADDRESS);

    if(connect(socket_client, (struct sockaddr *)&connect_client, sizeof(connect_client)) < 0) {
        perror("connect_client");
        close(socket_client);
        fclose(file);
        return EXIT_FAILURE;
    }

    recv(socket_client, buffer, BUFFER_SIZE, 0);
    printf("Message du serveur : %s\n", buffer);

    // Calculer la taille du fichier
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Envoyer la taille du fichier
    sprintf(buffer, "%ld\n", file_size);
    send(socket_client, buffer, strlen(buffer), 0);

    // Envoyer le contenu du fichier
    int bytes_read;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        send(socket_client, buffer, bytes_read, 0);
    }

    fclose(file);

    // Recevoir "BIEN RECU\n"
    recv(socket_client, buffer, BUFFER_SIZE, 0);
    printf("Message du serveur : %s\n", buffer);

    // Fermer la connexion
    close(socket_client);

    return EXIT_SUCCESS;
}
