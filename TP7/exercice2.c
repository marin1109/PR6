#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

#define PORT "2628"
#define BUFFER_SIZE 1024

void handle_response(int sockfd);

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <hostname> <command> <word/expression>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *hostname = argv[1];
    const char *command = argv[2];
    const char *word = argv[3];
    
    struct addrinfo hints, *res, *p;
    int status, sockfd;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;        // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;    // TCP stream sockets
    hints.ai_flags = AI_V4MAPPED | AI_ADDRCONFIG; // For IPv6 with fallback to IPv4

    if ((status = getaddrinfo(hostname, PORT, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    // Loop through all the results and connect to the first we can
    for (p = res; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "failed to connect\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res); // free the linked list

    char buffer[BUFFER_SIZE];
    ssize_t bytes_received = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received == -1) {
        perror("recv");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    buffer[bytes_received] = '\0';
    
    if (strncmp(buffer, "220 ", 4) != 0) {
        fprintf(stderr, "Error: not a DICT server\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Received: %s\n", buffer);

    char send_msg[BUFFER_SIZE];
    if (strcmp(command, "SHOW") == 0 || strcmp(command, "DEFINE") == 0) {
        snprintf(send_msg, sizeof(send_msg), "%s * %s\r\n", command, word);
    } else if (strcmp(command, "MATCH") == 0) {
        snprintf(send_msg, sizeof(send_msg), "MATCH * . %s\r\n", word);
    } else {
        fprintf(stderr, "Unsupported command: %s\n", command);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (send(sockfd, send_msg, strlen(send_msg), 0) == -1) {
        perror("send");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Sent: %s", send_msg);

    handle_response(sockfd);

    close(sockfd);
    return 0;
}

void handle_response(int sockfd) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    while ((bytes_received = recv(sockfd, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        printf("%s", buffer);

        if (strstr(buffer, "250 ") != NULL || strstr(buffer, "552 ") != NULL) {
            break;
        }
    }

    if (bytes_received == -1) {
        perror("recv");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
}
