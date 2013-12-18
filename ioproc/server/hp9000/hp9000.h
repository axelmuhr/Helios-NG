/*
** Localisation header file for the HP9000
*/

	/* HP9000 Unix has a logname routine */
#define logname my_logname

	/* These ioctl calls have the Posix names */
#define TCGETS   TCGETATTR
#define TCSETS   TCSETATTR
#define TCSETSW  TCSETATTRD

#define statfs(a, b, c, d) statfs(a, b)

#define seteuid(a) setresuid(-1, a, -1)

#define getdtablesize() 60

