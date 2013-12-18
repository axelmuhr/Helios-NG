/******************************************************************************
**
**
**	FILE		:	X_Open/sys/msg.h
**
**
**	DESCRIPTION	:	X-Open header file : <sys/msg.h>
**
**
******************************************************************************/


/* permission definitions */

#define MSG_OWN_A 0200 /* autorisation de modification pour le proprietaire */
#define MSG_OWN_R 0400 /* autorisation de consultation pour le proprietaire */

#define MSG_GRP_A 0020 /* autorisation de modification pour le groupe */
#define MSG_GRP_R 0040 /* autorisation de consultation pour le groupe */

#define MSG_OTH_A 0002 /* autorisation de modification pour le reste */
#define MSG_OTH_R 0004 /* autorisation de consultation pour le reste */


/* message operation flags */

#define MSG_NOERROR	 010000 /* pas d'erreur si message trop long */


/* messages constantes definitions */

#define MSGMAX 12	/* taille maximum d'un message en octets */
#define MSGMNB 30	/* nombre maximum d'octet par file */
#define MSGMNI 20	/* nombre maximal d'identificateur  */

/*
** structures definitions
*/


/* message structure for msgsnd() and msgrcv() */

struct message {
	long	mtype;
	char	*mtext;
};


/* msqid_ds structure */

struct msqid_ds {
        struct ipc_perm msg_perm; 
	unsigned short  msg_qnum;	/* number of messages currently on queue */
	unsigned short  msg_qbytes;	/* max number of bytes allowed on queue */
        pid_t           msg_lspid;	/* pid of last msgsnd */
        pid_t           msg_lrpid;	/* pid of last msgrcv */
	time_t          msg_stime;	/* time of last msgsnd */
	time_t          msg_rtime;	/* time of last msgrcv */
	time_t          msg_ctime;	/* time of last change */
};



