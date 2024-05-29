#### Soit le code suivant :

```c
1 int sock = socket(PF_INET6, SOCK_STREAM, 0);
2 if (sock == -1)
3 	exit(1);
4
5 struct sockaddr_in6 addrsock;
6 addrsock.sin6_family = AF_INET6;
7 addrsock.sin6_port = htons(1212);
8 if (inet_pton(AF_INET6, ADDR, &addrsock.sin6_addr) <= 0)
9 	exit(1);
10
11 int r = connect(sock, (struct sockaddr*)&addrsock, sizeof(addrsock));
```

1. *Quel type de connexion doit accepter le serveur afin qu’une application exécutant ce code puisse s’y connecter ?*
    - Le serveur doit accepter une connexion IPv6 sur le port 1212 de l'adresse `ADDR` et doit aussi utiliser le protocole TCP.

2. *Quel problème de fiabilité pose ce code ? Pourquoi et comment le corriger ?*
    - `addrsock` n'est pas initialisé, il faut initialiser `addrsock` avec `memset(&addrsock, 0, sizeof(addrsock));` avant de l'utiliser.
    - Il faut aussi vérifier que `connect` ne retourne pas `-1` pour s'assurer que la connexion a bien été établie.

#### On suppose que deux applications communiquent en mode TCP et que l’application 1 exécute le 1er bout de code en parallèle de l’application 2 qui exécute le 2e bout de code ci-dessous. En supposant que les appels systèmes ne fassent pas d’erreur, l’application 2 affiche-t-elle quelque chose ? Quand ? Quoi ? Comment corriger ce problème ?

```c
1 // application 1
2 char bufsend[SIZE_MESS];
3
4 sprintf(bufsend, ”Bonjour le cours de programmation reseaux\n”);
5 int ecrit = send(sock1, bufsend, strlen(bufsend), 0);
6 sprintf(bufsend, ”Au revoir\n”);
7 ecrit = send(sock1, bufsend, strlen(bufsend), 0);
```

```c
1 // application 2
2 char bufrecv[SIZE_MESS + 1];
3 memset(bufrecv, 0, SIZE_MESS + 1);
4
5 int recu = recv(sock2, bufrecv, SIZE_MESS, 0);
6 printf(”%s\n”, bufrecv);
7 recu = recv(sock2, bufrecv, SIZE_MESS, 0);
8 printf(”%s\n”, bufrecv);
```

- L'application 2 affiche quelque chose après que l'application 1 ait envoyé le message ```Bonjour le cours de programmation reseaux\n``` mais elle n'affiche rien après que l'application 1 ait envoyé le message "Au revoir\n". 
- L'application 2 affiche quelque chose après chaque `recv`, mais les résultats peuvent être incorrects ou mélangés.
- Pour corriger ce problème : 
    - Il faut réinitialiser `bufrecv` avant chaque appel à `recv` pour éviter de lire des données résiduelles.
    - Il faut aussi vérifier que `recv` ne retourne pas `-1` pour s'assurer que la lecture s'est bien passée. 

#### Soit le code suivant :
    
```c
1 struct addrinfo hints, *res, *p;
2
3 memset(&hints, 0, sizeof(hints));
4 hints.ai_family = AF_INET6;
5 hints.ai_socktype = SOCK_STREAM;
6
7 if ((getaddrinfo(hostname, port, &hints, &res)) != 0 || res == NULL)
8 	exit(1);
9
10 *addr_len = sizeof(struct sockaddr_in6);
11 p = res;
12 while (p != NULL) {
13 	*sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
14  if (*sock > 0) {
15 	    if (connect(*sock, p->ai_addr, *addr_len) == 0)
16 		    break;
17
18 	    close(*sock);
19 	}
20 	p = p->ai_next;
21 }
22 printf(”%d\n”, ntohs(p->ai_addr->sin6_port));
```

1. *En supposant que les appels systèmes s’exécutent sans erreur, expliquez ce que fait ce code en précisant le type et les valeurs possibles des variables hostname et port.*
    - Ce code crée une socket TCP IPv6 et se connecte à un serveur en utilisant les informations obtenues par `getaddrinfo`.
    - `hostname` est le nom du serveur à contacter et `port` est le port sur lequel le serveur écoute.
    - Si la connexion est établie, le code affiche le port du serveur.

2. *Expliquez pourquoi la ligne 16 est nécessaire.*
    - La ligne 16 est nécessaire pour sortir de la boucle `while` si la connexion a été établie avec succès, sinon on continue à essayer de se connecter avec les autres adresses obtenues par `getaddrinfo` et en fermant la socket on risque de ne plus pouvoir se connecter au serveur, car il n'y a plus d'adresse à essayer.

3. *Que faut-il changer dans ce code pour qu’il permette des connexions IPv4 ou IPv6 à un serveur ?*
    - Il faut changer la ligne 4 pour que `hints.ai_family` soit initialisé à `AF_UNSPEC` pour permettre des connexions IPv4 ou IPv6.

#### Soit le code suivant :

```c
1 while (1) {
2 	int sock2 = accept(sock, NULL, NULL);
3 	if (sock2 == -1) {
4 		exit(1);
5 	}
6 }
```

1. *Que représentent les variables sock et sock2 ?*
    - `sock` est le descripteur de la socket du serveur (la socket d'écoute) et `sock2` est le descripteur de la socket du client (la socket acceptée).

2. *Décrivez les étapes nécessaires à faire avant l’exécution de ce code en précisant les noms des fonctions appelées. Il n’est pas demandé d’écrire du code.*
    - Il faut créer une socket avec `socket` et la lier à une adresse avec `bind` pour que le serveur puisse écouter les connexions entrantes, puis il faut appeler `listen` pour mettre la socket en mode écoute.

3. *Complétez ce code pour qu’il accepte plusieurs clients en parallèle en utilisant des threads puis qu’il envoie à chacun le message ≪OK ≫.*
    
```c
1 void* client_handler(void* arg) {
2 	int sock = *(int*)arg;
3 	char buf[SIZE_MESS];
4 	memset(buf, 0, SIZE_MESS);
5 	sprintf(buf, "OK\n");
6 	send(sock, buf, strlen(buf), 0);
7 	close(sock);
8   free(arg);
9 	return NULL;
10 }
11
12 int main() {
13 int sock = socket(PF_INET6, SOCK_STREAM, 0);
14 if (sock == -1)
15 	exit(1);
16
17 struct sockaddr_in6 addrsock;
18 addrsock.sin6_family = AF_INET6;
19 addrsock.sin6_port = htons(1212);
20 if (inet_pton(AF_INET6, ADDR, &addrsock.sin6_addr) <= 0)
21 	exit(1);
22 memset(&addrsock.sin6_addr, 0, sizeof(addrsock.sin6_addr));
23
24 if (bind(sock, (struct sockaddr*)&addrsock, sizeof(addrsock)) == -1)
25 	exit(1);
26
27 if (listen(sock, SOMAXCONN) == -1)
28 	exit(1);
29 
30 while (1) {
31  int sock2 = malloc(sizeof(int));
32 	*sock2 = accept(sock, NULL, NULL);
33 	if (*sock2 == -1) {
34 		free(sock2);
35 		exit(1);
36 	}
37 	pthread_t thread;
38 	pthread_create(&thread, NULL, client_handler, sock2);
39  pthread_detach(thread);
40 }
41
42 close(sock);
44 return 0;
45 }
```    

#### Donnez les différences entre processus et processus légers
    - Les processus sont des programmes en cours d'exécution qui ont leur propre espace mémoire, tandis que les processus légers (threads) partagent le même espace mémoire.