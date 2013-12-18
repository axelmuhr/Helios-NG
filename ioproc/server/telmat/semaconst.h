/*
  Fichier semaconst.h
  
  Definition des constantes utilisees par les fonctions decritres dans
  le fichier semactrl.c et les servers.
  Ce fichier doit etre identique pour tous les servers devant partager
  les memes semaphores.
*/  

#define TRANS_SEMA_KEY	99990	/* A ne reutiliser par aucun logiciel autre		*/
				/* que les servers accedant a la carte transputer	*/
				/* reserve dans [99990 .. 99997]			*/ 
#define NB_SEMA_SET	1	/* Nombre d'operation simultanee par server		*/
#define NB_SEMA		9	/* Nombre de semaphores utilises par tableau		*/
#define SEMA_INIT	1	/* Valeur initiale des semaphores			*/
#define ACQUIRE		-1
#define RELEASE		1
#define TRANS_FREE	-2
#define SEMA_ERROR	-1
#define TRANS_UNKNOW	-3
