#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* --------------------------------------------------------

Exemple d utilisation de select :
ce serveur scrute simultanement le clavier et des sockets dont le numero
du premier et le nombre ont ete passes sur la ligne de commande.

Fonctionnement :
apres avoir cree tous les sockets, le processus se bloque sur le select
en indiquant qu'il attend un evenement sur stdin ou n'importe lequel des
file descriptors correspondamt aux sockets crees.
Lors de l'occurrence de l'evenement, on verifie d'ou il vient et on prend
les mesures adequates.

----------------------------------------------------------- */

int Sock_Comm[10];
struct sockaddr_in Decrit_sock[10];

void RAZ_Sock(int socket);
void Lire(int socket);
void Creer_Sock(int Num_Sock, int Port);

int main(int argc, char *argv[])
{

  int i, Nb_Ports, Prem_Port, Ret_sel;
  char Nom_hote[256];
  struct hostent Struct_hote, *Ptr_hote;
  struct timeval Delai;
  fd_set Masque_R, Masque_W, Masque_E;

  if (argc != 3)
  {
    printf("Utilisation : %s Prem_port Nb_ports !\n", argv[0]);
    exit(1);
  }
  /* Prendre le nom de la machine locale 	 */
  if (gethostname(Nom_hote, sizeof Nom_hote))
  {
    perror("gethostname");
    exit(1);
  }

  printf("Nom hote : %s\n", Nom_hote);

  /* Pointeur vers la description du noeud local 	 */
  if ((Ptr_hote = gethostbyname(Nom_hote)) == NULL)
  {
    perror("gethostbyname");
    exit(1);
  }

  /* sauvegarde */
  Struct_hote = *Ptr_hote;

  /* Ouverture des sockets demandes */
  Nb_Ports = atoi(argv[2]);
  Prem_Port = atoi(argv[1]);
  for (i = 0; i < Nb_Ports; i++)
  {
    Creer_Sock(i, Prem_Port);
    Prem_Port++;
  }
  /* Mise en place du select */

  while (1)
  {
    printf("Le serveur (pid %d) attendre sur select\n", getpid());

    FD_ZERO(&Masque_R);
    FD_ZERO(&Masque_E);
    FD_ZERO(&Masque_W);
    FD_SET(0, &Masque_R);

    for (i = 0; i < Nb_Ports; i++)
      FD_SET(Sock_Comm[i], &Masque_R);

    Delai.tv_sec = 300;
    Delai.tv_usec = 0;

    Ret_sel = select(33, &Masque_R, &Masque_W, &Masque_E, &Delai);

    switch (Ret_sel)
    {
    case -1:
    {
      perror("select");
      for (i = 0; i < Nb_Ports; i++)
        RAZ_Sock(Sock_Comm[i]);
      exit(1);
    }
    case 0:
    {
      printf("Select rend 0 apres le time out\n");
      printf("Ralbol je ferme tout!\n");
      for (i = 0; i < Nb_Ports; i++)
        RAZ_Sock(Sock_Comm[i]);
      exit(1);
    }
    }

    /* On  a recu quelque chose :
       verifier si cet evenement vient du clavier...  */
    if (FD_ISSET(STDIN_FILENO, &Masque_R))
    {
      char tmp[80];
      scanf("%s", tmp);
      printf("\nlu sur le clavier : %s\n", tmp);
    }
    /*  ... ou des sockets :  */
    for (i = 0; i < Nb_Ports; i++)
      if (FD_ISSET(Sock_Comm[i], &Masque_R))
        Lire(Sock_Comm[i]);
  } /* fin while */
}

/**********************Fermer socket ****************************/
void RAZ_Sock(int socket)
{
  printf("Fermeture du socket %d\n", socket);
  if (shutdown(socket, 2) == -1)
    perror("shutdown");
  if (close(socket))
    perror("close");
}

/***********************Creer socket ****************************/
void Creer_Sock(int Num_Sock, int Port)
{
  int Taille;
  if ((Sock_Comm[Num_Sock] = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
  {
    perror("socket");
    exit(1);
  }
  Decrit_sock[Num_Sock].sin_family = PF_INET;
  Decrit_sock[Num_Sock].sin_port = htons(Port);
  Decrit_sock[Num_Sock].sin_addr.s_addr = INADDR_ANY;

  printf("Creer : Sock_Comm : %d, htons(Port) : %d, Port : %d\n",
         Sock_Comm[Num_Sock], Decrit_sock[Num_Sock].sin_port, Port);
  Taille = sizeof Decrit_sock[Num_Sock];

  if (bind(Sock_Comm[Num_Sock],
           (struct sockaddr *)&(Decrit_sock[Num_Sock]), Taille))
  {
    perror("bind");
    RAZ_Sock(Sock_Comm[Num_Sock]);
  }
}

/******************* Lire socket *********************************/
void Lire(int socket)
{
  int Long_Tab, Long_dist, Drapeaux, Ret_Recv;
  char Tab[256];
  struct sockaddr_in Decrit_dist;
  Long_Tab = sizeof(Tab);
  Long_dist = sizeof(Decrit_dist);
  Drapeaux = 0; /* Drapeaux peut inclure: MSG_OOB, MSG_PEEK, ... */
  Ret_Recv = recvfrom(socket , Tab, Long_Tab, 0,
                      (struct sockaddr *)&Decrit_dist, &Long_dist);
  if (Ret_Recv == -1)
  {
    perror("recvfrom");
    exit(1);
  }
  else
  {
    Tab[Ret_Recv] = '\0';
    printf("(%d) %s\n", Ret_Recv, Tab);
  }
}
