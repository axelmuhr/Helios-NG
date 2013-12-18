
/* for source compatability */

#include "uio.h"
#include <setjmp.h>

struct user
{
	int		u_uid;
	int		u_error;
	struct proc	*u_procp;
	struct	
	{
		int	ru_msgsnd;
		int	ru_msgrcv;
	} u_ru;
	struct
	{
		int	r_val1;
		int	r_val2;
	} u_r;
	char		u_ofile[4];
	void		*u_ap;
	jmp_buf		u_qsave;
	int		u_timeout;
};

extern struct user u;
