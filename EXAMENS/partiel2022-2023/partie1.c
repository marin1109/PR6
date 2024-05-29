#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *serve(void *arg) {
    pthread_mutex_lock(&mutex);
    int *var = (int *)arg;
  	*var *= 2;
  	int entier = *var;
  	entier++;
  	printf("valeur : %d\n", entier);
  	entier--;
  	int f = open("fic.txt", O_WRONLY | O_APPEND | O_CREAT, 0666);
  	if (f < 0) exit(1);
    char buf[12]; // enough to hold a 32-bit integer
    snprintf(buf, sizeof(buf) + 1, "%d\n", entier);
    write(f, buf, strlen(buf));
    close(f);
    pthread_mutex_unlock(&mutex);
 	return NULL;
}

int main(int argc, char *argv[]) {
 	int tour = 0;
 	int *a = malloc(sizeof(int));
 	*a = 1;

	while (tour < 10) {
 		pthread_t thread;

 		if (pthread_create(&thread, NULL, serve, a))
 			continue;
        pthread_detach(thread);
 		tour++;
 	}
 	sleep(10);

 	return 0;
}