#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h> // Pour close()

int main() {
    // Création du socket
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }

    // Initialisation de la structure sockaddr_in
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(13);

    // Conversion de l'adresse IP de texte en binaire
    if (inet_pton(AF_INET, "192.168.70.237", &addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(s); // Fermer le socket avant de quitter
        return EXIT_FAILURE;
    }

    // Connexion au serveur
    int r = connect(s, (struct sockaddr *)&addr, sizeof(addr));
    if (r == -1) {
        perror("connect");
        close(s); // Fermer le socket avant de quitter
        return EXIT_FAILURE;
    }

    // Lecture de la réponse du serveur
    char buffer[256];
    ssize_t n = recv(s, buffer, sizeof(buffer) - 1, 0);
    if (n < 0) {
        perror("recv");
    } else {
        buffer[n] = '\0'; // Assurer la terminaison de la chaîne
        printf("Heure reçue du serveur: %s", buffer);
    }

    // Fermeture de la connexion
    close(s);

    return EXIT_SUCCESS;
}
