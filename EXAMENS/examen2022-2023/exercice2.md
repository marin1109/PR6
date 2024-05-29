#### On suppose que les appels de fonctions s’exécutent sans erreur

```c
1       // application 1
2   int main() {
3       int sock = socket(PF_INET, SOCK_STREAM, 0);
4
5       int ok = 1;
6       setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &ok, sizeof(ok));
7
8       struct sockaddr_in adr;
9       memset(&adr, 0, sizeof(adr));
10      adr.sin_family = AF_INET;
11      adr.sin_port = htons(1234);
12      inet_pton(AF_INET, "255.255.255.255", &adr.sin_addr);
13
14      int i, nb;
15      srand(time(NULL));
16      for (i = 0; i <= 20; i++) {
17          nb = htonl(rand() % 50 - 25);
18          sendto(sock, &nb, sizeof(nb), 0, (struct sockaddr *)&adr, sizeof(struct sockaddr_in));
19      }
20
21      close(sock);
22      return 0;
23  }
```

```c
1   // application 2
2   void *fa(void *arg) {
3       int *v = (int *)arg;
4       double a = *v;
5       for (int i = 0; i < 3; i++)
6           a = a * (log(a) + 1);
7       int fd = open("fic.txt", O_WRONLY | O_CREAT | O_APPEND, 0600);
8       write(fd, &a, sizeof(double));
9       close(fd);
10
11      return NULL;
12  }
13
14  void *fb(void *arg) {
15      int *v = (int *)arg;
16      double b = *v;
17      for (int i = 0; i < 3; i++)
18          b = b + (exp(b) + 1);
19      int fd = open("fic.txt", O_WRONLY | O_CREAT | O_APPEND, 0600);
20      write(fd, v, sizeof(double));
21      close(fd);
22
23      return NULL;
24  }
25
26  int main() {
27      int sock = socket(PF_INET, SOCK_DGRAM, 0);
28
29      struct sockaddr_in adrsock;
30      memset(&adrsock, 0, sizeof(adr));
31      adrsock.sin_family = AF_INET;
32      adrsock.sin_port = htons(1234);
33      adrsock.sin_addr.s_addr = htonl(INADDR_ANY);
34
35      bind(sock, (struct sockaddr *)&adrsock, sizeof(struct sockaddr_in));
36
37      int n;
38      pthread_t thread;
39      while (1) {
40          recvfrom(sock, &n, sizeof(n), 0, NULL, NULL);
41          n = ntohl(n);
42          if (n > 0)
43              pthread_create(&thread, NULL, fa, &n);
44          else
45              pthread_create(&thread, NULL, fb, &n);
46      }
47
48      return 0;
49  }
```

1. Quel type de communication est utilisée ici ?
    - Il s'agit d'une communication broadcast de type UDP utilisant le port 1234.

2. Connaissant les valeurs de `nb` envoyées par l’application 1 à l’application 2, pour quelles raisons ne peut-on savoir, sans consulter le contenu du fichier `fic.txt`, ce qu’il contient ?
    - Nous ne pouvons pas savoir ce que contient le fichier, car l'application 2 utilise des threads pour écrire, sans se soucier de la concurrence d'accès.

3. Écrire le code qui permet de corriger le problème souligné à la question précédente en précisant où il s’insère dans le code donné ci-dessus.
    - Nous devons ajouter un mutex pour protéger l'accès au fichier `fic.txt` dans les fonctions `fa` et `fb`.
    
```c
1   // application 2
2   pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
3
4   void *fa(void *arg) {
5       int *v = (int *)arg;
6       double a = *v;
7       for (int i = 0; i < 3; i++)
8           a = a * (log(a) + 1);
9       pthread_mutex_lock(&mutex);
10      int fd = open("fic.txt", O_WRONLY | O_CREAT | O_APPEND, 0600);
11      write(fd, &a, sizeof(double));
12      close(fd);
13      pthread_mutex_unlock(&mutex);
14
15      return NULL;
16  }
17
18  void *fb(void *arg) {
19      int *v = (int *)arg;
20      double b = *v;
21      for (int i = 0; i < 3; i++)
22          b = b + (exp(b) + 1);
23      pthread_mutex_lock(&mutex);
24      int fd = open("fic.txt", O_WRONLY | O_CREAT | O_APPEND, 0600);
25      write(fd, v, sizeof(double));
26      close(fd);
27      pthread_mutex_unlock(&mutex);
28
29      return NULL;
30  }
31 
32  int main() {
33      int sock = socket(PF_INET, SOCK_DGRAM, 0);
34
35      struct sockaddr_in adrsock;
36      memset(&adrsock, 0, sizeof(adr));
37      adrsock.sin_family = AF_INET;
38      adrsock.sin_port = htons(1234);
39      adrsock.sin_addr.s_addr = htonl(INADDR_ANY);
40
41      bind(sock, (struct sockaddr *)&adrsock, sizeof(struct sockaddr_in));
42
43      int n;
44      pthread_t thread;
45      while (1) {
46          recvfrom(sock, &n, sizeof(n), 0, NULL, NULL);
47          n = ntohl(n);
48          if (n > 0)
49              pthread_create(&thread, NULL, fa, &n);
50          else
51              pthread_create(&thread, NULL, fb, &n);
52          pthread_join(thread, NULL);
53      }
54
55      return 0;
56  }
```


4. Décrire, étape par étape, le protocole entre les deux applications 1 et 2 en prenant en compte vos corrections.
    - Application 1 :
        - Crée une socket UDP.
        - Configure la socket pour réutiliser l'adresse.
        - Prépare une adresse de diffusion (broadcast) pour l'envoi de messages.
        - Génére aléatoirement 20 nombres et les envoie à l'application 2 en utilisant la socket. 
        - Ferme la socket.
    - Application 2 :
        - Crée une socket UDP.
        - Configure et associe la socket à une adresse locale et un port.
        - Attend la réception des messages UDP.
        - Pour chaque message reçu :
            - Si le nombre reçu est positif, crée un thread pour exécuter la fonction fa.
            - Si le nombre reçu est négatif, crée un thread pour exécuter la fonction fb.
            - Les threads créés sont joints pour attendre leur terminaison.
        - Les fonctions fa et fb :
            - Effectuent des calculs spécifiques.
            - Verrouillent un mutex pour protéger l'accès concurrent au fichier.
            - Écrivent les résultats dans le fichier fic.txt.
            - Déverrouillent le mutex après l'écriture.

5. On efface les lignes 39 et 46 du code de l’application 2. Corrigez ce code (précisez où il s’insère ou les lignes modifiées), afin que l’application 2 s’exécute toujours normalement.
    - Nous devons ajouter créer un thread qui va gérer cette tâche de réception des messages UDP et créer une nouvelle fonction pour cette tâche.

```c
32  void *receive(void *arg);
33  main() {
34      int sock = socket(PF_INET, SOCK_DGRAM, 0);
35
36      struct sockaddr_in adrsock;
37      memset(&adrsock, 0, sizeof(adrsock));
38      adrsock.sin_family = AF_INET;
39      adrsock.sin_port = htons(1234);
40      adrsock.sin_addr.s_addr = htonl(INADDR_ANY);
41
42      bind(sock, (struct sockaddr *)&adrsock, sizeof(struct sockaddr_in));
43
44      pthread_t thread;
45      pthread_create(&thread, NULL, receive, &sock);
46      pthread_join(thread, NULL);
47
48      return 0;
49  }
50
51  void *receive(void *arg) {
52      int sock = *(int *)arg;
53      int n;
54      pthread_t thread;
55      while (1) {
56          recvfrom(sock, &n, sizeof(n), 0, NULL, NULL);
57          n = ntohl(n);
58          if (n > 0)
59              pthread_create(&thread, NULL, fa, &n);
60          else
61              pthread_create(&thread, NULL, fb, &n);
62          pthread_join(thread, NULL);
63      }
64
65      return NULL;
66  }
```
