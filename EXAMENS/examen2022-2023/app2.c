#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>
#include <math.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *fa(void *arg) {
    pthread_mutex_lock(&mutex);
    int *v = (int *)arg;
    double a = *v;
    for (int i = 0; i < 3; i++){
        a = a + 1;
    }
    int fd = open("fic.txt", O_WRONLY | O_CREAT | O_APPEND, 0600);
    char buffer[100];
    sprintf(buffer, "%f\n", a);
    write(fd, buffer, strlen(buffer));
    close(fd);
    printf("a = %f\n", a);
    pthread_mutex_unlock(&mutex);
    return NULL;
}

void *fb(void *arg) {
    pthread_mutex_lock(&mutex);
    int *v = (int *)arg;
    double b = *v;
    for (int i = 0; i < 3; i++){
        b = b + 1;
    }
    int fd = open("fic.txt", O_WRONLY | O_CREAT | O_APPEND, 0600);
    char buffer[100];
    sprintf(buffer, "%f\n", b);
    write(fd, buffer, strlen(buffer));
    close(fd);
    printf("b = %f\n", b);
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main() {
    int sock = socket(PF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in adrsock;
    memset(&adrsock, 0, sizeof(adrsock));
    adrsock.sin_family = AF_INET;
    adrsock.sin_port = htons(1234);
    adrsock.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(sock, (struct sockaddr *)&adrsock, sizeof(struct sockaddr_in));

    int n;
    pthread_t thread;
    while (1) {
        recvfrom(sock, &n, sizeof(n), 0, NULL, NULL);
        n = ntohl(n);
        if (n > 0)
            pthread_create(&thread, NULL, fa, &n);
        else
            pthread_create(&thread, NULL, fb, &n);
        pthread_join(thread, NULL);
    }

    return 0;
}