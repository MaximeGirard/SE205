/*
   Ce programme envoie (en boucle !) des datagrammes vers le serveur
   dont on donne l'adresse IP et numero de port sur la ligne de commande.
   Utilisation: Client_UDP machine_cible num_port
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
  static char MESSAGE[] = "Message emis par                              ";
  struct sockaddr_in s_serveur; /* description du socket du serveur */

  int sock;                /* descripteur du socket local*/
  struct hostent *m_cible; /* description de la machine cible */

  char Nom_Local[256];
  extern struct hostent *gethostbyname();

  if (argc != 3)
  {
    printf("Utilisation: %s machine_cible num_port\n", argv[0]);
    exit(1);
  }

  /* creation de la socket en mode datagramme */
  if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
  {
    perror("Ouverture du socket client");
    exit(1);
  }

  /* obtenir l adresse IP de la machine cible */
  if ((m_cible = gethostbyname(argv[1])) == (struct hostent *)0)
  {
    printf("%s: machine cible inconnue\n", argv[1]);
    exit(2);
  }
  /* designation du serveur cible */
  memcpy(&s_serveur.sin_addr, m_cible->h_addr_list[0], m_cible->h_length);
  s_serveur.sin_family = AF_INET;
  s_serveur.sin_port = htons(atoi(argv[2]));

  if (gethostname(Nom_Local, sizeof Nom_Local))
  {
    perror("gethostname");
    exit(1);
  }

  sprintf(MESSAGE + 20, "pid %d sur %s", getpid(), Nom_Local);

  while (1)
  {
    if (sendto(sock, MESSAGE, strlen(MESSAGE), 0, &s_serveur, sizeof(s_serveur)) < 0)
    {
      perror("Ecriture sur socket serveur");
    }
    printf("Envoi de datagramme (%s) vers %s\n", MESSAGE, argv[1]);
    sleep(5);
  }
  close(sock);
}
