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

typedef struct t {
    char buf[2*BUFFER_SIZE+1];
    int cur; // position de la ligne suivante dans le tampon
    int size; // taille courante du tampon
} buf_t;

int find_newline(buf_t* buf);
int is_last_line(buf_t* buf);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <hostname> <word>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *hostname = argv[1];
    const char *word = argv[2];
    
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

    char ipstr[INET6_ADDRSTRLEN];
    void *addr;
    if (p->ai_family == AF_INET) {
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
        addr = &(ipv4->sin_addr);
    } else {
        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
        addr = &(ipv6->sin6_addr);
    }

    inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
    printf("Connected to %s\n", ipstr);

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
        fprintf(stderr, "Error: ce n'est pas un serveur dict\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Received: %s\n", buffer);

    char send_msg[BUFFER_SIZE];
    snprintf(send_msg, sizeof(send_msg), "DEFINE * %s\r\n", word);

    if (send(sockfd, send_msg, strlen(send_msg), 0) == -1) {
        perror("send");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Sent: %s", send_msg);

    buf_t buf = {.cur = 0, .size = 0};
    while ((bytes_received = recv(sockfd, buf.buf + buf.size, BUFFER_SIZE, 0)) > 0) {
        buf.size += bytes_received;
        buf.buf[buf.size] = '\0';
        
        int newline_pos;
        while ((newline_pos = find_newline(&buf)) != -1) {
            buf.buf[newline_pos] = '\0';
            printf("%s\n", buf.buf + buf.cur);
            
            if (is_last_line(&buf)) {
                close(sockfd);
                return 0;
            }
            
            buf.cur = newline_pos + 2;
        }

        // Shift remaining data to the start of the buffer
        if (buf.cur > 0) {
            memmove(buf.buf, buf.buf + buf.cur, buf.size - buf.cur);
            buf.size -= buf.cur;
            buf.cur = 0;
        }
    }

    if (bytes_received == -1) {
        perror("recv");
    }

    close(sockfd);
    return 0;
}

int find_newline(buf_t* buf) {
    for (int i = buf->cur; i < buf->size - 1; i++) {
        if (buf->buf[i] == '\r' && buf->buf[i+1] == '\n') {
            return i;
        }
    }
    return -1;
}

int is_last_line(buf_t* buf) {
    if (strncmp(buf->buf + buf->cur, "250", 3) == 0 || strncmp(buf->buf + buf->cur, "552", 3) == 0) {
        return 1;
    }
    if (strcmp(buf->buf + buf->cur, ".") == 0) {
        printf("\n");
        return 0;
    }
    return 0;
}
