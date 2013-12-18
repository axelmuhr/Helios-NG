
/*
  Fichier semactrl.c
  
  Description des fonctions de gestion de semaphores.
  Ce meme fichier doit etre compile et linke avec tous les servers
  accedant a une carte transputer et donc partageant les memes semaphores.
  
  Une seule fonction est visible par les servers :

    int trans_sema(trans_num,sem_op)
    char *trans_num;
    int sem_op;
    {
    }

  trans_num est le `nom' du transputer dont l'acces est demande :
  typique : "/dev/link`x'[a|b|c]_mt`y'
    
  sem_op est le type de l'operation a effectuer pour ce transputer :
     { ACQUIRE, RELEASE }    (defini dans semaconst.h)
     
  le retour de cette fonction peut etre : (defini dans semaconst.h)

    TRANS_FREE : si sem_op = ACQUIRE , le transputer etait libre et est
	                               maintenant verrouille.
	         si sem_op = RELEASE , le transputer est maintenant libre. 

    SEMA_ERROR : erreur systeme (??? a priori `impossible'). 

    TRANS_UNKNOW : trans_num n'a pu etre etabli.

    Autre entier non defini dans semaconst.h : 
      Le transputer est utilise, le retour de trans_sema est le pid du 
      processus utilisant actuellement le transputer demande.


*/





#include <sys/errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include "semaconst.h"





extern void perror();
extern int errno;



/* 
  Operation sur semaphore.
  SEM_UNDO : la derniere operation sur sem_num est remise a jour a la fin
             du processus qui l'a effectuee.
  (nettoyage automatique des semaphores en cas de plantage de servers)
  IPC_NOWAIT : le processus demandeur ne reste pas bloque en attente de
               liberation du semaphore.
*/  


static int set_sema(sem_id,sem_num,sem_op)
int 	sem_id,
	sem_num,
	sem_op;
{
	struct sembuf	sb;

	sb.sem_num=sem_num;
	sb.sem_op=sem_op;
	sb.sem_flg=SEM_UNDO|IPC_NOWAIT;
	return(semop(sem_id,&sb,NB_SEMA_SET));
}








/*
    Retourne un identificateur (entier positif) unique pour le
    tableau de semaphore de cle key. Cet entier est cree, et le
    tableau est initialise (utilisation en semaphores binaires)
    lors du premier appel de semget avec key apres un boot du sun,
    ou apres l'utilisation de la commande UNIX ipcrm. 

    La cle key est choisi arbitrairement par le programmeur.
    ici : #define TRANS_SEMA_KEY 99999.
    Pour garantir le role des semaphores, aucun logiciel faisant
    un usage autre de semaphores ne doit reutiliser la meme cle !
    (99999 : commune et reserve pour les servers accedant a la carte
     transputer)
*/

static int get_semid(key)
int	key;
{
	int	sem_id,
		i;
	ushort	array[NB_SEMA];

	if((sem_id=semget(key,NB_SEMA,0)) == -1)
	{
 		if(errno != ENOENT)
 		{
			perror("Recuperation de semaphores ");
			return(SEMA_ERROR);
		}
		if((sem_id=semget(key,NB_SEMA,00666|IPC_CREAT)) == -1)
  		{
			perror("Creation de semaphores ");
			return(SEMA_ERROR);
		}
		for(i=0;i<NB_SEMA;i++)
			array[i]=SEMA_INIT; 
		if(semctl(sem_id,0,SETALL,array) == -1)
 		{
			perror("Initialisation de semaphores ");
			return(SEMA_ERROR);
		}
	}
	return(sem_id);
}





/*
  Retourne le pid du dernier processus ayant effectue une operation
  sur sem_num.
*/  


static int sema_process(sem_id,sem_num)
int	sem_id,
	sem_num;
{
	int pid;

	if((pid=semctl(sem_id,sem_num,GETPID,0)) == -1)
	{
		perror("Controle sur semaphore ");
		return(SEMA_ERROR);
	}
	return(pid);
}





static int ident_sema(trans_num,sem_num,key)
char	*trans_num;
int	*sem_num,
	*key;
{
	int	i=0,
		s=0;

	while(trans_num[i] != '\0')
	{
		if(trans_num[i] == '/')
			s=i+1;
		i++;
	}
	i=s;
	s= -1;
	while(trans_num[i] != '\0')
	{
		if((trans_num[i] >= '0') && (trans_num[i] <= '9') && (s == -1))
			s=i;
		i++;
	}
	if(s == -1)
		return(-1);
	switch(trans_num[s])
	{
		case '8' :	*sem_num=0;
				s++;
				break;
		case '2' :	if(trans_num[s+1] == '4')
				{
					*sem_num=3;
					s+=2;
				}
				else
					return(-1);
				break;
		case '3' :	if(trans_num[s+1] == '2')
				{
					*sem_num=6;
					s+=2;
				}
				else
					return(-1);
				break;
		default :	return(-1);
	}
	switch(trans_num[s])
	{
		case 'a' :	s++;
				break;
		case 'b' :	s++;
				*sem_num+=1;
				break;
		case 'c' :	s++;
				*sem_num+=2;
				break;
	}
	i=s;
	s= -1;
 	while(trans_num[i] != '\0')
	{
		if((trans_num[i] >= '0') && (trans_num[i]<='9') && (s == -1))
			s=i;
		i++;
	}
	if(s == -1)
		return(-1);
	*key=trans_num[s]-'0';
	return(1);
} 







/*
  Seule fonction visible par les servers.

  Operation possible :
   ACQUIRE : demande de semaphore 
   RELEASE : liberation de semaphore
 
  appel de type : trans_sema("1",ACQUIRE)
    demande d'acquisition du semaphore associe au transputer 1

  retour possible :  (defini dans semaconst.h)
   TRANS_FREE : si sem_op = ACQUIRE , le transputer etait libre et est
                maintenant verrouille.
                si sem_op = RELEASE , le transputer est maintenant libre. 

   SEMA_ERROR : erreur systeme (???).

   TRANS_UNKNOW : trans_num non etabli.

   autre : pid du processus utilisant la semaphore demandee.
*/


int trans_sema(trans_num,sem_op)
char	*trans_num;
int	sem_op;
{
	int	sem_num,
		sem_id,
		key,	
		cr;

/*	printf("In trans_sema %s %d\n\r", trans_num, sem_op);
*/
	if(ident_sema(trans_num,&sem_num,&key) == -1)
		return(TRANS_UNKNOW);
	sem_id=get_semid(key+TRANS_SEMA_KEY);
	switch(sem_op)
	{
		case ACQUIRE :	if(set_sema(sem_id,sem_num,sem_op) == -1)
				{
					if(errno == EAGAIN)
						return(sema_process(sem_id,sem_num));  
					perror("Operation sur semaphore ");
					return(SEMA_ERROR);
				}  
				return(TRANS_FREE);
		case RELEASE :	if((cr=semctl(sem_id,sem_num,GETVAL,0)) == 1)
					return(TRANS_FREE);
				if(cr == -1)
				{
					perror("Controle sur semaphore ");
					return(SEMA_ERROR);
				} 
				if(set_sema(sem_id,sem_num,sem_op) == -1)
				{
					perror("Operation sur semaphore ");
					return(SEMA_ERROR);
				}
				return(TRANS_FREE);
	}
}
