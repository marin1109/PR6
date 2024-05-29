#### Soit le code suivant tel que serve soit une fonction de prototype void *serve(void *) envoyant un message sur la socket passée en argument :

```c
1 int tour = 0;
2
3 while (tour < 10) {
4 	int sock2 = accept(sock, NULL, NULL);
5 	pthread_t thread;
6 	if (pthread_create(&thread, NULL, serve, &sock2))
7 		continue;
8 	tour++;
9 }
```

1. *Quels problèmes peut-on rencontrer lors de l’exécution de ce code ?*
	- Non-jointure des threads créés : les threads créés ne sont pas joints, ce qui peut entraîner des fuites de ressources.
	- Utilisation de la même variable pour plusieurs threads : la variable `sock2` est utilisée pour plusieurs threads, ce qui ferait que plusieurs threads utilisent la même connexion.

2. *Et comment corriger cela ?*
	- Pour la non-jointure des threads, il suffit de rajouter un appel à `pthread_join` après la création de chaque thread, de cette manière il faudra créer un tableau de threads pour pouvoir les joindre tous à la fin de la boucle `while`. Ou bien, on peut faire appel à `pthread_detach` pour détacher le thread crée dans la boucle `while`.
	- Pour l'utilisation de la même variable pour plusieurs threads, de allouer dynamiquement la variable `sock2` pour chaque thread.

#### Soit le code suivant :

```c
1  void *serve(void *arg) {
2  	int *var = (int *)arg;
3  	*var *= 2;
4  	int entier = *var;
5  	entier++;
6  	printf(”valeur : %d\n”, entier);
7  	entier--;
8  	int f = open(”fic.txt”, O_WRONLY | O_APPEND | O_CREAT, 0666);
9  	if (f < 0) exit(1);
10 	write(f, &entier, sizeof(entier));
11 	close(f);
12 	return NULL;
13 }
14
15 int main(int argc, char *argv[]) {
16 	int tour = 0;
17 	int *a = malloc(sizeof(int));
18 	*a = 1;
19
20 	while (tour < 10) {
21 		pthread_t thread;
22
23 		if (pthread_create(&thread, NULL, serve, a))
24 			continue;
25 		tour++;
26 	}
27 	sleep(10);
28
29 	return 0;
30 }
```

3. *Lors de l'éxécution de ce code, que se passe-t-il si l'appel système à open en ligne échoue?*
	- Le programme termine brutalement, y compris tous les threads. Cela est dû à l'appel à `exit(1)` en cas d'échec de l'ouverture du fichier.

4. *Pourquoi a-t-on mis l'instruction sleep(10) en ligne 27?*
	- Pour attendre que tous les threads aient fini d'exécuter la fonction `serve` avant de terminer le programme.

5. *Modifier le programme afin qu'il permette d'obtenir un résultat équivalent sans faire appel à l'instruction sleep(10).*
	- Il suffit de faire appel à `pthread_detach` pour détacher les threads après leur création.

```c
15 int main(int argc, char *argv[]) {
16 	int tour = 0;
17 	int *a = malloc(sizeof(int));
18 	*a = 1;
19
20 	while (tour < 10) {
21 		pthread_t thread;
22
23 		if (pthread_create(&thread, NULL, serve, a))
24 			continue;
25 		pthread_detach(thread); // Détacher le thread
26 		tour++;
27 	}
28
29 	return 0;
30 }
```

6. *Peut-on connaître le contenu du fichier fic.txt après exécution du code ? Expliquez.*
	- Non, car plusieurs threads écrivent dans le fichier `fic.txt` en même temps, ce qui peut entraîner des problèmes de concurrence.

7. *Déterminez les sections critiques de ce code.*
	- Les sections critiques sont :
		- L'écriture dans le fichier `fic.txt` : plusieurs threads écrivent dans le fichier en même temps.
		- L'écriture dans la variable `entier` : plusieurs threads écrivent dans la variable `entier` en même temps.

8. *Corrigez le programme, afin qu'après exécution, le fichier fic.txt contienne toutes les valeurs prises par la variable a, à l'exception de la valeur 1.*

```c
1  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // Initialisation du mutex
2  void *serve(void *arg) {
3 	pthread_mutex_lock(&mutex); // Verrouiller le mutex
4 	int *var = (int *)arg;
5 	*var *= 2;
6 	int entier = *var;
7 	entier++;
8 	printf(”valeur : %d\n”, entier);
9 	entier--;
10 	int f = open(”fic.txt”, O_WRONLY | O_APPEND | O_CREAT, 0666);
11  	if (f < 0) exit(1);
12 	write(f, &entier, sizeof(entier));
13 	close(f);
14 	pthread_mutex_unlock(&mutex); // Déverrouiller le mutex
15 	return NULL;
16 }
```

9. *Donnez alors toutes les valeurs prises par la variable a.*
	- Les valeurs prises par la variable `a` sont : 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024.

10. *Modifiez le programme afin qu'il prenne comme unique argument de la ligne de commande le nom du fichier et qu'il passe ensuite la variable a et ce nom en paramètre de la fonction serve afin qu'elle l'utilise dans l'appel à open.*

```c
1 struct serve_args { // Structure pour passer les arguments à la fonction serve
2 	int *var;
3 	char *filename;
4 };
5
6 void *serve(void *arg) {
7 	struct serve_args *args = (struct serve_args *)arg;
8 	pthread_mutex_lock(&mutex);
9 	*(args->var) *= 2;
10 	int entier = *(args->var);
11 	entier++;
12 	printf(”valeur : %d\n”, entier);
13 	entier--;
14 	int f = open(args->filename, O_WRONLY | O_APPEND | O_CREAT, 0666);
15 	if (f < 0) exit(1);
16 	write(f, &entier, sizeof(entier));
17 	close(f);
18 	pthread_mutex_unlock(&mutex);
19 	return NULL;
20 }
21
22 main(int argc, char *argv[]) {
23 	if (argc != 2) {
24 		fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
25 		exit(1);
26 	}
27
28 	int tour = 0;
29 	int *a = malloc(sizeof(int));
30 	*a = 1;
31
32 	struct serve_args args = {a, argv[1]}; // Arguments à passer à la fonction serve
33
34 	while (tour < 10) {
35 		pthread_t thread;
36
37 		if (pthread_create(&thread, NULL, serve, &args))
38 			continue;
39 		pthread_detach(thread);
40 		tour++;
41 	}
42
43 	return 0;
44	}
```

