#define	JB_REGS		3		/* registers */
#define	JB_RA		(JB_REGS+31)	/* return address */

/*
 * WARNING: a jmp_buf must be as large as a sigcontext since
 * longjmp uses one to perform a sigreturn
 */
#define	SIGCONTEXT_PAD	48
#define	NJBREGS		(JB_RA+1+SIGCONTEXT_PAD)
#define _JBLEN	NJBREGS


#ifndef _JMP_BUF
#define _JMP_BUF  10
#endif

typedef int jmp_buf[_JBLEN];

extern int setjmp(jmp_buf env);

extern void longjmp(jmp_buf env, int val);
