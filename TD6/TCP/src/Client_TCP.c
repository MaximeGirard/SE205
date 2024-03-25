/*********************************************************
            -Ce client attend des entrees sur stdin et les
             envoie au serveur auquel il est connecte.
            - UTILISER la commande netstat pour voir
             les connexions entrantes et sortantes
             sur la machine.
*********************************************************/

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
  int Sock_Des, pid;
  struct sockaddr_in Le_Serveur;
  struct sockaddr_in Mon_Sockaddr;
  struct hostent *ph;
  static char message[] = "Message envoye par client            \n";
  char Buf[BUFSIZ];

  int Taille = sizeof(Mon_Sockaddr);

  if (argc != 3)
  {
    printf("Utilisation: %s Nom-du-serveur Port-du-serveur\n", argv[0]);
    exit(1);
  }

  pid = getpid();
  sprintf(message + 27, "%d", pid);

  /* creation du socket du client */
  if ((Sock_Des = socket(PF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("Erreur d'ouverture du socket");
    exit(1);
  }

  /* Construction de l'adresse du serveur qui comprend plusieurs champs :
   *  - adresse IP de la machine ou se trouve le serveur
   *    (recuperee par gethostbyname)
   *  - numero de port du serveur
   *    (passe sur la ligne de commande)
   */
  Le_Serveur.sin_family = AF_INET;
  if ((ph = gethostbyname(argv[1])) == (struct hostent *)0)
  {
    fprintf(stderr, "%s: Hote inconnu\n", argv[1]);
    exit(2);
  }
  memcpy((char *)&Le_Serveur.sin_addr, (char *)ph->h_addr_list[0], ph->h_length);
  Le_Serveur.sin_port = htons(atoi(argv[2]));

  /* connexion vers le serveur */
  if (connect(Sock_Des, (struct sockaddr *)&Le_Serveur, sizeof(Le_Serveur)) < 0)
  {
    perror("Erreur de connexion sur le socket serveur");
    exit(2);
  }

  /* Verifications sur les ports :
   * getsockname donne le port local
   * getpeername donne le port distant
   */
  if (getsockname(Sock_Des, (struct sockaddr *)&Mon_Sockaddr, &Taille) < 0)
  {
    perror("Client: erreur sur getsocketname");
    exit(1);
  }
  printf("Numero de mon port: %d\n", ntohs(Mon_Sockaddr.sin_port));

  if (getpeername(Sock_Des, (struct sockaddr *)&Mon_Sockaddr, &Taille) < 0)
  {
    perror("Client : erreur sur getsocketname");
    exit(1);
  }
  printf("Numero du port distant: %d\n", ntohs(Mon_Sockaddr.sin_port));
  printf(" **************************************\n");

  /* attendre message de debut de comm. depuis serveur */
  memset(Buf, 0, sizeof(Buf));

  if (read(Sock_Des, Buf, sizeof(Buf)) < 0)
    perror("Erreur de lecture sur le socket de comm.");

  printf("Ici client pid %d, je lis:\n %s\n", pid, Buf);

  if (write(Sock_Des, message, strlen(message)) < 0)
    perror("Erreur d'ecriture vers le serveur");

  /*
   * Entrer les messages a transmettre via le clavier (stdin)
   * la fin des entrees est indiquee par la chaine fin
   */
  while (fgets(Buf, sizeof(Buf), stdin))
  {
    if (!strncmp(Buf, "fin", 3))
      break;
    if (write(Sock_Des, Buf, strlen(Buf)) < 0)
      perror("Erreur d'ecriture vers le serveur");
  }

  shutdown(Sock_Des, 1);

  /* lire l'eventuelle reponse residuelle du serveur */
  if (read(Sock_Des, Buf, sizeof(Buf)) < 0)
    perror("Erreur de lecture sur le socket de comm.");

  printf("Ici client pid %d, je lis :\n %s\n", pid, Buf);

  close(Sock_Des);
  exit(0);
}
