#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h> // Pour close()
#include <time.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 37
#define SECONDS_1900_TO_1970 2208988800UL

int main(){
    // Création de la socket
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }

    // Configuration de l'adresse du serveur
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_IP, &addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(s); // Fermer la socket avant de quitter
        return EXIT_FAILURE;
    }

    // Connexion au serveur
    int r = connect(s, (struct sockaddr *)&addr, sizeof(addr));
    if (r == -1) {
        perror("connect");
        close(s); // Fermer la socket avant de quitter
        return EXIT_FAILURE;
    }

    // Réception du temps depuis le serveur
    uint32_t server_time;
    ssize_t n = recv(s, &server_time, sizeof(server_time), 0);
    if (n < 0) {
        perror("recv");
    } else if (n != sizeof(server_time)) {
        fprintf(stderr, "Erreur: nombre d'octets reçus inattendu\n");
    } else {
        // Conversion du network order au host order
        server_time = ntohl(server_time);
        
        // Conversion en temps Unix
        time_t unix_time = server_time - SECONDS_1900_TO_1970;
        
        // Conversion en chaîne de caractères pour affichage
        char *time_str = ctime(&unix_time);
        if (time_str != NULL) {
            printf("Heure reçue du serveur: %s", time_str);
        } else {
            perror("ctime");
        }
    }

    // Fermeture de la connexion
    close(s);
    return EXIT_SUCCESS;
}
