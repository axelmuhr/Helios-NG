
/* for source compatability */

extern int selwait;

struct proc
{
	int		p_pid;		/* fake pid */
	caddr_t		p_wchan;
	caddr_t		p_sock;
};
