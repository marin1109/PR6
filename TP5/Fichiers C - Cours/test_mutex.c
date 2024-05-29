#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

// Variable partagée
int var = 0;
// Mutex pour protéger l'accès à la variable partagée
pthread_mutex_t verrou = PTHREAD_MUTEX_INITIALIZER;

// Fonction exécutée par chaque thread
void *serve(void *arg) {
    sleep(1);  // Simuler une opération longue
    
    // Verrouiller l'accès à la variable partagée
    pthread_mutex_lock(&verrou);
    
    // Modifier la variable partagée
    var += 1;
    
    // Afficher la nouvelle valeur de la variable
    printf("var = %d\n", var);
    
    // Déverrouiller l'accès à la variable partagée
    pthread_mutex_unlock(&verrou);
    
    return NULL;
}

int main(int argc, char *argv[]) {
    // Vérifier que le nombre correct d'arguments a été passé
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <number_of_threads>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Récupérer le nombre de threads à créer
    int len = atoi(argv[1]);
    pthread_t *tpthread = malloc(len * sizeof(pthread_t));
    if (!tpthread) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    int compt = 0;

    // Créer les threads
    while (compt < len) {
        if (pthread_create(&tpthread[compt], NULL, serve, NULL)) {
            perror("pthread_create");
            continue;
        }  
        compt++;
    }

    // Attendre la fin de chaque thread
    for (int i = 0; i < len; i++) {
        pthread_join(tpthread[i], NULL);
    }

    // Libérer les ressources
    free(tpthread);
    pthread_mutex_destroy(&verrou);

    return 0;
}
