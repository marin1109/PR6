## On considère les codes des applications 1 et 2 présentés à la fin de l'exercice.
### On rappelle qu'on ne se préoccupe pas ici des erreurs des appels systèmes. Vous considérerez de plus que les exécutions ne rencontrent pas de problème réseau.

1. Les lignes 26 et 32 de l'application 1 peuvent poser problème. Pourquoi ? Corriger la ligne 26 sans changer le reste du code.
    - Un appel unique à `recv` de la ligne 26 ne garantit pas que l'ensemble des données envoyées par l'application 2 sera reçu. Il est possible que les données soient reçues en plusieurs morceaux. Il faut donc appeler `recv` dans une boucle jusqu'à ce que toutes les données soient reçues. 
    - De même pour `send` de la ligne 32, il faut appeler `send` dans une boucle jusqu'à ce que toutes les données soient envoyées.
    La ligne 26 doit être corrigée comme suit :
```c
int lu = 0, n;
while ((n = recv(sock3, buf + lu, BUF_SIZE - lu, 0)) > 0) {
    lu += n;
}
```

2. Décrire textuellement, étape par étape, le protocole entre les deux applications 1 et 2.

    - Étapes de l'Application 1 :
        - L'application 1 crée une socket UDP et prépare un message contenant l'adresse du site "nivose.informatique.univ-paris-diderot.fr" et un numéro de port (passé en argument lors de l'exécution).
        - Elle envoie ce message à une adresse multicast de type IPv6 et sur le port 1212.
        - Ensuite, elle crée une socket TCP et se lie au port spécifié (passé en argument lors de l'exécution).
        - Elle met cette socket en mode écoute pour attendre les connexions entrantes.
        - Lorsqu'une connexion TCP est acceptée, elle reçoit un message (le calcul 37*45+12) de l'application 2.
        - L'utilisateur traite ce calcul et envoie le résultat de retour à l'application 2.
    - Étapes de l'Application 2 :
        - L'application 2 crée une socket UDP et se prépare à recevoir des messages multicast sur le port 1212.
        - Elle reçoit un message multicast contenant l'adresse du site et le port TCP de l'application 1.
        - Elle utilise ces informations pour créer une nouvelle socket TCP et se connecte à l'application 1 en utilisant l'adresse et le port reçus.
        - Une fois connectée, elle prépare le calcul 37*45+12 et l'envoie à l'application 1 via la connexion TCP.
        - Elle attend de recevoir le résultat du calcul de l'application 1.
        - Après réception, elle affiche le résultat.

3. Écrire le code de la fonction voir prepa_recept(int sock). (~6 lignes)

```c
1   void prepa_recept(int sock) {
2       struct sockaddr_in6 adr;
3       memset(&adr, 0, sizeof(adr));
4       adr.sin6_family = AF_INET6;
5       adr.sin6_port = htons(1212);
6       adr.sin6_addr = in6addr_any;
7       bind(sock, (struct sockaddr *) &adr, sizeof(adr));
8       struct ipv6_mreq mreq;
9       inet_pton(AF_INET6, "ff12::ae2:b", &mreq.ipv6mr_multiaddr);
10      setsockopt(sock, IPPROTO_IPV6, IPV6_JOIN_GROUP, &mreq, sizeof(mreq));
8   }
```

4. Écrire le code de la fonction int prepa_env_recept(char *h, char *p). (~15 lignes)

```c
1   int prepa_env_recept(char *h, char *p){
2       struct addrinfo hints, *res, *rp;
3       int sock;
4       memset(&hints, 0, sizeof(hints));
5       hints.ai_family = AF_INET6;
6       hints.ai_socktype = SOCK_STREAM;
7       getaddrinfo(h, p, &hints, &res);
8       rp = res;
9       while (rp != NULL) {
10          if((sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_addrlen)) != -1){
11              if(connect(sock, rp->ai_addr, rp->ai_addrlen) != -1){
12                  return sock;
13              }
14              close(sock);
15          }
16      rp = rp->ai_next;
17      }
18      if(rp==NULL) return 2;
19      freeaddrinfo(res);
20  }
```

5. Modifier le code de l'application 2 afin que celle-ci puisse communiquer avec deux entités de l'application 1 en parallèle et déterminer un gagnant (le premier qui donne une réponse correcte) s'il y en a un. L'application 2 attend d'être en communication avec deux entités de l'application 1 avant d'envoyer en parallèle à chacune le calcul. Elle renvoie à chaque entité son statut, `g` pour gagnant, `p` pour perdant et `n` lorsqu'il n'y a pas de gagnant. 

Pour avoir tous les points à cette question, vous devez respecter les contraintes suivantes : utiliser des threads et ne pas utiliser de variable globale. (~35 lignes sans compter les lignes réutilisées)

```c
1   typedef struct {
2       int *win;
3       int *sock;
4       char *calcul;
5       int res;
6       pthread_mutex_t mutex;
7   } infos;
8
9   int main(int argc, char *argv[]) {
    // code avant la ligne 16
16      pthread_t t[2];
17      int win = calloc(1, sizeof(int));
18      pthread_mutex_init(&mutex, NULL);
19      info *i = calloc(2, sizeof(infos));
20      for(int j = 0; j < 2; j++) {
21          int sock = prepa_env_recept(buf, memchr(buf, '\0', lu) + 1);
22          if(sock != -1) {
23              int sock_num = calloc(1, sizeof(int));
24              *sock_num = sock;
25              i[j].win = win;
26              i[j].sock = sock_num;
27              i[j].mutex = mutex;
28              i[j].calcul = argv[1];
29              i[j].res = res_du_calcul(argv[1]);
30              free(sock_num);
31          }
32          else {
33              j--;
34          }
35      }
36      for(int j = 0; j < 2; j++)
37          pthread_create(&t[j], NULL, serve, &i[j]);
37      for(int j = 0; j < 2; j++) {
38          pthread_join(t[j], NULL);
39      }
40      pthread_mutex_destroy(&mutex);
41      close(i[0].sock);
42      close(i[1].sock);
43      free(win);
44      free(i);
45      return 0;
46  }
47
48  void *serve(void *arg) {
49      infos *i = (infos *) arg;
50      for(int j = 0; j < 5; j++) { // on limite à 5 essais
51          pthread_mutex_lock(&i->mutex);
52          if(*i->win == 0) {
53              char buf[32] = {0};
54              recv(*i->sock, &buf, sizeof(buf), 0);
55              if(*i.win == 0 && itoa(buf) == i->res) {
56                  *i->win = 1;
57                  send(*i->sock, "g", 1, 0);
58                  pthread_mutex_unlock(&i->mutex);
59                  return NULL;
60              }
61          } else {
62              send(*i->sock, "p", 1, 0);
63              pthread_mutex_unlock(&i->mutex);
64              return NULL;
65          }
66          pthread_mutex_unlock(&i->mutex);
67      }
68      send(*i->sock, "n", 1, 0);
69      return NULL;
70  }
```

6. Décrire textuellement les étapes du côté de l'application 2 pour sécuriser les échanges des lignes 20 et 21 avec OpenSSL.
    - Pour sécuriser les échanges des lignes 20 et 21 avec OpenSSL, il faut d'abord initialiser la librairie OpenSSL avec `SSL_library_init()`. Ensuite, il faut créer un contexte `ctx` SSL en négociant le meilleur algorithme de chiffrement partagé avec le serveur. Ensuite, l'objet `ctx` utilisera le certificat signé par nous-même. Ensuite, il faut créer un objet `ssl` à partir du contexte `ctx` et l'associer à la socket `sock2`. Enfin, il faut utiliser `SSL_read` et `SSL_write` pour lire et écrire les données de manière sécurisée.

```c
1   // Application 1
2
3   #define BUF_SIZE 1024
4
5   int main(int argc, char *args[]) {
6       int sock1 = socket(AF_INET6, SOCK_DGRAM, 0);
7
8       struct sockaddr_in6 adr;
9       memset(&adr, 0, sizeof(adr));
10      adr.sin6_family = AF_INET6;
11      inet_pton(AF_INET6, "ff12::ae2:b", &adr.sin6_addr);
12      adr.sin6_port = htons(1212);
13
14      char buf[BUF_SIZE];
15      int l;
16      l = sprintf(buf, "%s\0%s\0", "nivose.informatique.univ-paris-diderot.fr", args[1]);
17      sendto(sock1, buf, l, 0, (struct sockaddr *) &adr, sizeof(adr));
18      close(sock1);
19
20      int sock2 = socket(AF_INET6, SOCK_STREAM, 0);
21      struct sockaddr_in6 adr2 = {AF_INET6, htons(args[1]), 0, IN6ADDR_ANY_INIT, 0};
22      bind(sock2, (struct sockaddr *) &adr2, sizeof(adr2));
23      listen(sock2, 0);
24
25      int sock3 = accept(sock2, NULL, NULL);
26      int lu = recv(sock3, buf, BUF_SIZE, 0);
27      buf[lu] = '\0';
28      printf("%s\n", buf);
29      int result;
30      sscanf("%d\n", &result);
31      result = htonl(result);
32      send(sock3, &result, sizeof(result), 0);
33
34      close(sock2);   close(sock3);
35      return 0;
36  }
```

```c
1   // Application 2
2
3   #define BUF_SIZE 1024
4
5   int main(int argc, char *argv[]) {
6       int sock1 = socket(AF_INET6, SOCK_DGRAM, 0);
7
8       // prepare sock1 pour la reception du 1er message de l'application 1
9       prepa_recept(sock1);
10
11      char buf[BUF_SIZE];
12      memset(buf, 0, sizeof(buf));
13      int lu = read(sock1, buf, BUF_SIZE);
14
15      //retourne une socket preparee pour l'envoie et la reception des messages suivant
16      int sock2 = prepa_env_recept(buf, memchr(buf, '\0', lu) + 1);
17
18      char calcul[] = "37*45+12";
19      int res;
20      send(sock2, calcul, strlen(calcul), 0);
21      recv(sock2, &res, sizeof(res), 0);
22      printf("%d\n", ntohl(res));
23
24      close(sock1);   close(sock2);
25      return 0;
26  }
```
