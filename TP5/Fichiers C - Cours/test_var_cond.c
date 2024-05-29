#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

int var = 0;  // Variable partagée entre les threads
pthread_mutex_t verrou = PTHREAD_MUTEX_INITIALIZER;  // Mutex pour protéger l'accès à la variable partagée
pthread_cond_t vcond = PTHREAD_COND_INITIALIZER;  // Condition variable pour synchroniser les threads

// Fonction exécutée par chaque thread
void *serve(void *arg) {
    pthread_mutex_lock(&verrou);
    
    // Attendre la condition si 'var' est égale à 0
    if (var == 0) {
        pthread_cond_wait(&vcond, &verrou);
    }

    // Incrémenter la variable partagée
    var += 1;
    printf("var = %d\n", var);
    
    pthread_mutex_unlock(&verrou);
    
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <number_of_threads>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int len = atoi(argv[1]);
    pthread_t *tpthread = malloc(len * sizeof(pthread_t));
    if (!tpthread) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    // Créer les threads
    for (int i = 0; i < len; i++) {
        if (pthread_create(&tpthread[i], NULL, serve, NULL)) {
            perror("pthread_create");
            continue;
        }
    }

    // Générer une valeur initiale non nulle et impaire pour 'var'
    srand(getpid());
    while (var % 2 == 0) {
        var = rand();
    }
    printf("thread principal : var = %d\n", var);

    // Signaler tous les threads en attente
    pthread_mutex_lock(&verrou);
    pthread_cond_broadcast(&vcond);
    pthread_mutex_unlock(&verrou);

    // Attendre la fin de chaque thread
    for (int i = 0; i < len; i++) {
        pthread_join(tpthread[i], NULL);
    }

    // Libérer les ressources
    free(tpthread);
    pthread_mutex_destroy(&verrou);
    pthread_cond_destroy(&vcond);

    return 0;
}
