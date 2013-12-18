/******************************************************************************
**
**
**	FILE		:	X_Open/sys/sem.h
**
**
**	DESCRIPTION	:	X-Open header file : <sys/sem.h>
**
**
******************************************************************************/


/* permission definitions */
#define SEM_OWN_A 0200 /* autorisation de modification pour le proprietaire */
#define SEM_OWN_R 0400 /* autorisation de consultation pour le proprietaire */
#define SEM_GRP_A 0020 /* autorisation de modification pour le groupe */
#define SEM_GRP_R 0040 /* autorisation de consultation pour le groupe */
#define SEM_OTH_A 0002 /* autorisation de modification pour le reste */
#define SEM_OTH_R 0004 /* autorisation de consultation pour le reste */

/* semaphore operation flags */
#define SEM_UNDO 010000 /* not implemented */

/* semaphores constantes definitions */
#define SEMMAP 10    /*  */
#define SEMMNI 10    /* max number identifiers */
#define SEMMNS 60    /* max number system */
#define SEMMSL 25    /* max number of semaphore per set */
#define SEMOPM 10    /* max number of operation per semop call */
#define SEMVMX 32767 /* valeur max d'un semaphore */

/* semctl command definitions */
#define GETNCNT 3  /* extraction de semnctn */
#define GETPID  4  /* extraction de sempid */
#define GETVAL  5  /* extraction de la valeur d'un semaphore de l'ensemble */
#define GETALL  6  /* extraction de l'ensemble des valeurs des semaphores */
#define GETZCNT 7  /* extraction de semzctn */
#define SETVAL  8  /* initialisation d'un semaphore de l'ensemble */
#define SETALL  9  /* initialisation de plusieurs semaphores de l'ensemble */

/* structures definitions */
/* semaphore structure */
struct sem {
	unsigned short semval;
	pid_t          sempid;
	unsigned short semncnt;
	unsigned short semzcnt;
};

/* semid_ds structure */
struct semid_ds {
        struct ipc_perm sem_perm; 
	unsigned short  sem_nsems;
	time_t          sem_otime;
	time_t          sem_ctime;
};

/* sembuf structure */
struct sembuf {
	unsigned short sem_num ;
	short          sem_op ;
	short          sem_flg ;
};

extern int semget(key_t key, int nsems, int semflg);
extern int semctl(int semid, int semnum, int cmd, 
			union semun{
				int		val;
				struct semid_ds	*buf;
				unsigned short	*array;
			} arg);
extern int semop (int semid, struct sembuf *ops, unsigned nsops);
