/******************************************************************************
**
**
**	FILE		:	X_Open/sys/ipc.h
**
**
**	DESCRIPTION	:	X-Open header file : <sys/ipc.h>
**
**
******************************************************************************/


/* Mode bits */
#define IPC_CREAT  0001000  /* creation si la cle n'existe pas */
#define IPC_EXCL   0002000  /* echec si la cle existe */
#define IPC_NOWAIT 0004000  /* erreur en cas d'attente */

/* Key */
#define IPC_PRIVATE (key_t)0 /* not implemented */

/* Control Commands */
#define IPC_RMID  0  /* suppression d'un identificateur */
#define IPC_SET   1  /* options "set" */
#define IPC_STAT  2  /* options "get" */

/* ipc_perm structure */
struct ipc_perm {
	uid_t  uid;   /* owner's user ID */
	gid_t  gid;   /* owner's group ID */
	uid_t  cuid;  /* creator's user ID */
	gid_t  cgid;  /* creator's group ID */
	mode_t mode;  /* read/write permission */
};

/* Error codes, returned by semaphore operations, not defined in 
	Helios header file <errno.h> */
#define	ENOSYS		1000;  /* functionnality not implemented */
#define	EIDRM		1001;  /* identifier removed */


/* keys used to communicate with the IPCserver */
#define		SS_IPCserver		0x19000000
#define		FG_MASK_TYPE		0x0000f000
#define		FG_MASK_OP		0x0000fff0
#define		FG_SEMAPHORE		0x0000c000
#define		FG_SEMGET		0x0000c010
#define		FG_SEMCTL		0x0000c020
#define		FG_SEMOP		0x0000c040
