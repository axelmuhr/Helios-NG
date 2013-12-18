
#include <sys/types.h>
#include <syslib.h>
#include <message.h>
#include <codes.h>
#include <gsp.h>
#include <servlib.h>

#include <rpc/rpc.h>
#include "nfs_prot.h"

typedef struct MMsgBuf {
	MCB		mcb;			/* message control block*/
	word		control[IOCMsgMax];	/* control vector	*/
	byte		data[IOCDataMax]; 	/* data vector		*/
} MMsgBuf;


typedef struct nfshandle
{
	Node		Node;		/* link in cache	*/
	uword		Hash;		/* hash value		*/
	word		Error;		/* last NFS error	*/
	bool		InUse;		/* being used?		*/
	fattr		Attr;		/* attributes from NFS	*/
	nfs_fh		File;		/* NFS file handle	*/
	char		*Path;		/* file path name	*/
} nfshandle;

#define MAXHANDLES 64			/* max no.of handles	*/

extern Semaphore nfslock;		/* lock for serializing NFS routines */

extern CLIENT	*nfsclnt;		/* RPC client handle	*/

extern int nfs_errno(nfsstat stat);

extern int Mount(char *host, char *root);
extern void DisMount(char *root);

extern void dir_server(MCB *mcb, nfshandle *h);
extern void file_server(MCB *mcb, nfshandle *h);

#define DEBUG	(MyTask->Flags & 1024)
