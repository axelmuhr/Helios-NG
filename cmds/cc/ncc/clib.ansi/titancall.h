/* STARDENT SYSCALLs */

#define SYS_syscall	0
#define SYS_exit	1
#define SYS_fork	2
#define SYS_read	3
#define SYS_write	4
#define SYS_open	5
#define SYS_close	6
#define SYS_wait	7
#define SYS_creat	8
#define SYS_link	9
#define SYS_unlink	10
#define SYS_execv	11
#define SYS_chdir	12
#define SYS_time	13
#define SYS_mknod	14
#define SYS_chmod	15
#define SYS_chown	16
#define SYS_brk		17
#define SYS_stat	18
#define SYS_lseek	19
#define SYS_getpid	20
#define SYS_mount	21
#define SYS_umount	22
#define SYS_setuid	23
#define SYS_getuid	24
#define SYS_stime	25
#define SYS_ptrace	26
#define SYS_alarm	27
#define SYS_fstat	28
#define SYS_pause	29
#define SYS_utime	30
#define SYS_stty	31
#define SYS_gtty	32
#define SYS_access	33
#define SYS_nice	34
#define SYS_statfs	35
#define SYS_sync	36
#define SYS_kill	37
#define SYS_fstatfs	38
#define SYS_setpgrp	39
#define SYS_dup		41
#define SYS_pipe	42
#define SYS_times	43
#define SYS_profil	44
#define SYS_plock	45
#define SYS_setgid	46
#define SYS_getgid	47
#define SYS_signal	48
#define SYS_msgsys	49
#define SYS_sysmips	50
#define SYS_acct	51
#define SYS_shmsys	52
#define SYS_semsys	53
#define SYS_ioctl	54
#define SYS_uadmin	55
#define SYS_utssys	57
#define SYS_setppri	58
#define SYS_execve	59
#define SYS_umask	60
#define SYS_chroot	61
#define SYS_fcntl	62
#define SYS_ulimit	63
#define SYS_advfs	70
#define SYS_unadvfs	71
#define SYS_rmount	72
#define SYS_rumount	73
#define SYS_rfstart	74
#define SYS_rdebug	76
#define SYS_rfstop	77
#define SYS_rfsys	78
#define SYS_rmdir	79
#define SYS_mkdir	80
#define SYS_getdents	81
#define SYS_sysfs	84
#define SYS_getmsg	85
#define SYS_putmsg	86
#define SYS_poll	87
/* This system call is internal for signal handling code */
#define SYS_sigreturn	88
/* fine SYS_await	89				 */
/* fine SYS_astat	90				 */
#define SYS__sigset42_	91 
#define SYS_sigvec	92 
#define SYS_sigblock	93 
#define SYS_sigsetmask	94 
#define SYS_sigpause	95 
#define SYS_sigstack	96 
#define SYS_siginterrupt	97 
#define SYS_wait3	98 
#define SYS_fchmod	99 
#define SYS_fchown	100 
#define SYS_getpgrp42	101 
#define SYS_setpgrp42	102 
#define SYS_setreuid	103 
#define SYS_setregid	104 
#define SYS_fsync	105 
#define SYS_killpg	106 
#define SYS_getrusage	107 
#define SYS_phys	108
#define SYS_thread	109
#define	SYS_threadreserve 110
#define	SYS_spawn	111
#define SYS_gethostname	112
#define SYS_sethostname	113
#define	SYS_truncate	114
#define	SYS_ftruncate	115
#define	SYS_symlink	116
#define	SYS_readlink	117
#define	SYS_lstat	118
#define	SYS_setitimer	119
#define	SYS_getitimer	120
#define	SYS_gtmofd	121
#define	SYS_stmofd	122
#define	SYS_gethostid	123
#define	SYS_sethostid	124
#define	SYS_madvise	125
#define	SYS_getpagesize	126
#define	SYS_mips_stat	127
#define	SYS_mips_fstat	128
#define	SYS_mips_lstat	129
#define	SYS_mips_wait3	130
