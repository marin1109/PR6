## Le but est d'écrire une application sur IPV6 qui permet la communication en anneau avec ses pairs.
    - Cela signifie, par exemple, que si 3 instances de l’application a1, a2 et a3 communiquent :
        - a1 envoie ses messages à a2 et reçoit ceux de a3
        - a2 envoie ses messages à a3 et reçoit ceux de a1
        - a3 envoie ses messages à a1 et reçoit ceux de a2



### Insertion dans l'anneau

##### Au lancement d'une instance *a* de l'application, celle-ci commence par s'insérer dans l'anneau de pairs. Elle doit donc demander à s'insérer et attendre qu'un pair accepte qu'elle s'insère entre lui et le pair suivant. Le protocole est le suivant :
1. au lancement, l’instance *a* envoie un message « HELLO » sur le port 4444 à toutes les machines du réseau local ayant une application écoutant sur ce port.

2. Elle attend en retour une réponse et s’occupe de la première réponse reçue. Si la réponse provient de l’instance *a1* et que *a1* est suivie de *a2* dans l’anneau, alors le message contient l’adresse IPv6 de l’hôte de *a2* sous sa forme chaînes de caractères. L’instance *a* stocke cette adresse.

3. Dès qu’elle reçoit une réponse d’une instance a1, elle lui renvoit l’argument du main qui correspond à son adresse IPv6 sous sa forme chaînes de caractères. Elle ne se préoccupe pas des autres réponses éventuelles.
    
##### L’instance *a* peut alors s’insérer dans l’anneau pour recevoir les messages de *a1* et envoyer ses messages à *a2*

1. À l’étape 1, quel type de socket IPV6 et quel type de communication doit-on utiliser ? Pourquoi ?
    - On doit utiliser un socket de type `SOCK_DGRAM` pour une communication en mode multicast. Cela permet d'envoyer les messages aux différentes instances connectées sur le réseau local.

2. Il manque la valeur d’une donnée réseau dans l’énoncé. Donnez une valeur possible à cette donnée et expliquez en quoi cette valeur est pertinente.
    - La valeur manquante est le l'adresse IP sur laquelle les instances de l'application écoutent. Utiliser un adresse de multicast permet de s'assurer que le message "HELLO" est reçu par toutes les instances de l'application sur le réseau local.

3. À l’étape 2, quel type de communication différente de celle de l’étape 1 peut-on utiliser ? Pourquoi ?
    - On peut utiliser une communication de type `SOCK_STREAM` pour établir une connexion entre les pairs. Cela permet de s'assurer que les messages sont bien reçus par les pairs.

4. Le protocole ci-dessus est incomplet. Quel problème n’est pas évoqué ? Expliquez quelles sont les raisons possibles pour que ce problème advienne. Proposez une solution à ce problème sans écrire le code mais en expliquant ce qu’il faudrait faire (scénario).
    - Le protocole ne prend pas en compte le cas où l'instance *a* ne reçoit pas de réponse à son message "HELLO". Cela peut se produire si le message est perdu en cours de route. Pour résoudre ce problème, on pourrait ajouter un mécanisme de timeout à l'attente de réponse. Si aucune réponse n'est reçue dans un délai donné, l'instance *a* pourrait renvoyer son message "HELLO".

5. Écrire le code de l’application correspondant aux trois étapes. (∼20 lignes)

```c
1   int main() {
2       int sock;
3       struct sockaddr_in6 addr;
4       char buffer[256];
5       int n;
6       sock = socket(AF_INET6, SOCK_DGRAM, 0);
7       struct sockaddr_in6 addr;
8       memset(&addr, 0, sizeof(addr));
9       addr.sin6_family = AF_INET6;
10      addr.sin6_port = htons(4444);
11      inet_pton(AF_INET6, "ff02::1", &addr.sin6_addr);
12      addr.sin6_scope_id = if_nametoindex("ifname");
13      sendto(sock, "HELLO", 5, 0, (struct sockaddr*)&addr, sizeof(addr));
14      char *rec_buf = (char *)calloc(INET6_ADDRSTRLEN, sizeof(char));
15      struct sockaddr_in6 rec_addr;
16      socklen_t rec_addr_len = sizeof(rec_addr);
17      n = recvfrom(sock, rec_buf, INET6_ADDRSTRLEN, 0, (struct sockaddr*)&rec_addr, &rec_addr_len);
18     sendto(sock, argv[1], strlen(argv[1]), 0, (struct sockaddr*)&rec_addr, rec_addr_len);
19      close(sock);
20      return EXIT_SUCCESS;
21  }
```

### Partage des tâches

##### Les messages circulant dans l’anneau correspondent à des tâches. Chaque instance de l’anneau peut recevoir une tâche via son entrée standard ou via un message reçu du pair précédent dans l’anneau.
##### Si le message provient de l’entrée standard, alors elle l’envoit au pair suivant dans l’annneau.
##### Si le message provient du pair précédent, elle attend pendant maximum 20 secondes que l’utilisateur signale qu’il se saisit de la tâche, sinon la tâche est envoyée au pair suivant. Les applications doivent donc faire plusieurs actions simultanément :
1. écouter sur le port 5555 les messages UDP de tâches provenant du pair précédent et réagir,
2. lire sur l’entrée standard une nouvelle tâche et réagir,
3. écouter sur le port 4444 les demandes d’insertion dans l’anneau et réagir.

Un message de tâche ne contient pas plus de 255 caractères.


1. Écrire le code C de l’application qui initialise les différentes sockets et adresses nécessaires aux trois actions. (∼15 lignes)

```c
1   int main() {
2       int sock_5555, sock_4444;
3       struct sockaddr_in6 addr_5555, addr_4444;
4       char buffer[256];
5       int n;
6       sock_5555 = socket(AF_INET6, SOCK_DGRAM, 0);
7       if (sock_5555 < 0) 
8           exit(1);
9       memset(addr_5555, 0, sizeof(addr_5555));
10      addr_5555.sin6_family = AF_INET6;
11      addr_5555.sin6_addr = in6addr_any;
12      addr_5555.sin6_port = htons(5555);
13      if (bind(sock_5555, (struct sockaddr*)&addr_5555, sizeof(addr_5555)) < 0)
14          exit(1);
15      sock_4444 = socket(AF_INET6, SOCK_DGRAM, 0);
16      if (sock_4444 < 0)
17          exit(1);
18      addr_4444.sin6_family = AF_INET6;
19      addr_4444.sin6_addr = in6addr_any;
20      addr_4444.sin6_port = htons(4444);
21      memset(addr_4444, 0, sizeof(addr_4444));
22      if (bind(sock_4444, (struct sockaddr*)&addr_4444, sizeof(addr_4444)) < 0)
23          exit(1);
```

2. Que faudrait-il ajouter dans votre code précédent pour pouvoir faire tourner plusieurs instances d’un anneau sur la même machine ?

    - Pour faire tourner plusieurs instances d'un anneau sur la même machine, il faudrait ajouter l'option `SO_REUSEPORT` lors de la création des sockets. Cela permet de réutiliser le même port pour applications différentes.

3. Écrire le code C de l’application permettant la simultanéité des actions sans créer de nouveau processus système ou léger et sans écrire, pour le moment, le code correspondant aux réactions des actions. Vous mettrez, pour le moment, à la place du code correspondant aux réactions des trois actions les commentaires respectifs //reaction1, //reaction2 ou //reaction3. La prise en charge de la réaction 1 sera traité à la question suivante. Vous ne devez pas vous en préoccuper dans cette question. (∼15 lignes)

```c
1   int main() {
    // code de la question précédente
25      struct pollfd fds[3];
26      fds[0].fd = sock_5555;
27      fds[0].events = POLLIN;
28      fds[1].fd = STDIN_FILENO;
29      fds[1].events = POLLIN;
30      fds[2].fd = sock_4444;
31      fds[2].events = POLLIN;
32      while (1) {
33          if (fds[0].revents & POLLIN) {
34              //reaction1
35          }
36          if (fds[1].revents & POLLIN) {
37              //reaction2
38          }
39          if (fds[2].revents & POLLIN) {
40              //reaction3
41          }
42      }
43      return EXIT_SUCCESS;
44  }
```

4. La réaction de l’action 1 consiste à attendre pendant au plus 20 secondes que l’utilisateur entre n’importe quelle chaîne de caractères sur l’entrée standard. Dans ce cas, cela signifie que la tâche reçue va être traitée par l’utilisateur et elle ne doit pas être renvoyée dans l’anneau. L’action 1 est donc terminée. Sinon (20 secondes se sont écoulées sans message sur l’entrée standard), il faut envoyer au pair suivant dans l’anneau la tâche reçue. 
Par souci de simplification, on interdit à l’utilisateur d’initier une action 2 pendant ces 20 secondes. Il doit également laisser un temps minimal de 20 secondes entre deux actions 2. 
Écrire le code de la réaction à l’action 1. Attention, vous ne devez toujours pas créer de nouveau processus système ou léger, mais la simultanéité des tâches doit être conservée. Cela implique qu’outre l’écriture du code correspondant au commentaire reaction1, vous devez modifier le code de la question précédente. Ne réécrivez pas tout, mais juste les morceaux modifiés ou ajoutés en expliquant où ils s’insèrent dans votre code de la question précédente. (∼10 lignes)

```c
35             if (fds[0].revents & POLLIN) {
36                 struct sockaddr_in6 addr;
37                 socklen_t addr_len = sizeof(addr);
38                 n = recvfrom(sock_5555, buffer, 255, 0, (struct sockaddr*)&addr, &addr_len);
39                 buffer[n] = '\0';
40
41                 struct pollfd fd = { .fd = STDIN_FILENO, .events = POLLIN };
42                 int val = poll(&fd, 1, 20000);
43                 if (val > 0) {
44                     if (fd.revents & POLLIN) {
45                         fgets(buffer, 255, stdin);
46                         sendto(sock_5555, buffer, strlen(buffer), 0, (struct sockaddr*)&next_peer, next_peer_len);
46                     }
47                 } else if (val == 0) {
48                     sendto(sock_5555, buffer, n, 0, (struct sockaddr*)&next_peer, sizeof(next_peer));
49                 }
50             }
51             if (fds[1].revents & POLLIN) {
52                 //reaction2
53             }
54             if (fds[2].revents & POLLIN) {
55                 fgets(buffer, 255, stdin);
56                 //reaction3
57             }
```