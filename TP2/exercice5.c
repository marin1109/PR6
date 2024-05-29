#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 4242
#define SERVER_ADDR "::1" 

int main() {
    int s = socket(AF_INET6, SOCK_STREAM, 0);
    if (s < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }

    struct sockaddr_in6 addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(PORT);

    if (inet_pton(AF_INET6, SERVER_ADDR, &addr.sin6_addr) <= 0) {
        perror("inet_pton");
        close(s); // Fermer le socket avant de quitter
        return EXIT_FAILURE;
    }

    int r = connect(s, (struct sockaddr *)&addr, sizeof(addr));
    if (r < 0) {
        perror("connect");
        close(s);
        return EXIT_FAILURE;
    }

    char buffer[1024];
    for(int i = 0; i <= 10; i++){
        snprintf(buffer, sizeof(buffer), "Hello%d", i);

        // Send the message
        if (send(s, buffer, strlen(buffer), 0) < 0) {
            perror("send");
            close(s);
            return EXIT_FAILURE;
        }

        // Clear the buffer
        memset(buffer, 0, sizeof(buffer));

        // Receive the response
        ssize_t n = recv(s, buffer, sizeof(buffer) - 1, 0);
        if (n < 0) {
            perror("recv");
            close(s);
            return EXIT_FAILURE;
        }

        // Null-terminate the received data
        buffer[n] = '\0';

        // Print the response
        printf("Received: %s\n", buffer);
    } 

    close(s);
    
    return EXIT_SUCCESS;
}
