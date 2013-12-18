#include <syslib.h>
#include <nonansi.h>
#include <attrib.h>

#include <stdio.h>
#include "sys/time.h"
#include "sys/ioctl.h"
#include <fcntl.h>
#include <signal.h>
#include <sgtty.h>
/* system and terminal dependent code for mz */

extern int contact;
extern int ctrlc;
extern unsigned int basetime;
extern unsigned int curtime;		/*converted time from get_time */
extern char *msg_ver;
extern int playing_at_skill_level;
extern int n_dots_eaten;

/* Set to one if you want to try line graphics, doesn't always work well */
/* with some curses packages. */

static int lgraph = 0; 

#if 0
/* name of log file */
static char *log_file = "/helios/local/games/lib/mzlog";
#endif

/*
* Structure returned by gettimeofday(2) system call,
* and used in other calls.
*
* struct timeval {
*	long	tv_sec;		 seconds 
*	long	tv_usec;	 and microseconds 
*}
*/

struct timeval current_time;
/* Control C comes here */

request_quit()
{
	contact = 1;
	ctrlc = 1;
}
/* 
 * Termcap initialization 
 */


static	char	buf[1024];	/* termcap entry read here */
static	char	cap[256];	/* capability strings go in here */
static int	Rows;
static int Columns;

static char	*T_EL;		/* erase the entire current line */
static char	*T_IL;		/* insert one line */
static char	*T_DL;		/* delete one line */
static char	*T_SO;		/* begin standout mode */
static char	*T_SE;		/* end standout mode */
static char	*T_SC;		/* save the cursor position */
static char	*T_ED;		/* erase display (may optionally home cursor) */
static char	*T_RC;		/* restore the cursor position */
static char	*T_CI;		/* invisible cursor (very optional) */
static char	*T_CV;		/* visible cursor (very optional) */

static char	*T_CM;		/* cursor motion string */

extern	int	tgetent();
extern  int 	tgetnum();
extern	char	*tgetstr();
extern	char	*getenv();
extern	char	*tgoto();

int
t_init()
{
	char	*term;
	int	n;
	char	*cp = cap;

	if ((term = getenv("TERM")) == NULL)
		return 0;

	if (tgetent(buf, term) != 1)
		return 0;

	if ((n = tgetnum("li")) == -1)
		return 0;
	else
		Rows = n;

	if ((n = tgetnum("co")) == -1)
		return 0;
	else
		Columns = n;

	/*
	 * Get mandatory capability strings.
	 */
	if ((T_CM = tgetstr("cm", &cp)) == NULL)
		return 0;

	if ((T_EL = tgetstr("ce", &cp)) == NULL)
		return 0;

	if ((T_ED = tgetstr("cl", &cp)) == NULL)
		return 0;

	/*
	 * Optional capabilities.
	 */
	if ((T_IL = tgetstr("al", &cp)) == NULL)
		T_IL = "";

	if ((T_DL = tgetstr("dl", &cp)) == NULL)
		T_DL = "";

	if ((T_SO = tgetstr("so", &cp)) == NULL)
		T_SO = "";

	if ((T_SE = tgetstr("se", &cp)) == NULL)
		T_SE = "";

	if ((T_SC = tgetstr("sc", &cp)) == NULL)
		T_SC = "";

	if ((T_RC = tgetstr("rc", &cp)) == NULL)
		T_RC = "";

	if ((T_CI = tgetstr("vs", &cp)) == NULL)
		T_CI = "";

	if ((T_CV = tgetstr("ve", &cp)) == NULL)
		T_CV = "";

	return 1;
}
static struct sgttyb oldttys;
static struct sgttyb newttys;
init_io()
{
#ifndef __HELIOS
	signal (SIGINT, request_quit);
#endif
	t_init();
#if 0
	gtty(0,&oldttys);
	gtty(0,&newttys);
	newttys.sg_flags |= CBREAK;
	newttys.sg_flags |= RAW;
	newttys.sg_flags &= ~ECHO;
	stty(0,&newttys);
#else
 { Attributes attr;
   GetAttributes(Heliosno(stdin), &attr);
   RemoveAttribute(&attr, ConsoleEcho);
   AddAttribute(&attr, ConsoleRawInput);
   SetAttributes(Heliosno(stdin), &attr);
   setvbuf(stdin, NULL, _IONBF, 0);
 }
#endif
}

close_io()
{
#if 1
	stty(0,&oldttys);
	graph_off();
#endif
}

get_time()
{
	(void) gettimeofday ( &current_time, NULL );
	curtime = (((current_time.tv_sec * 100) + (current_time.tv_usec/10000))
	- basetime);
}

cursor_off()
{
#if 0
	printf("%1cXs1A",0x1b);
#else
	printf(T_CI);
#endif
}

cursor_on()
{
#if 0
	printf("%1cXs1E",0x1b);
#else
	printf(T_CV);
#endif
}

static int graph_on_flag = 0;

/* turn on graphics to display boundries and dots */
graph_on()
{
	if(lgraph) /* terminal dependent code  for mbf 4313*/
	{
		printf("%c%c",0x1b,0x4e);/* 7270 mode off */
		printf("%c",0xe); /* so to select g1 character set */
		graph_on_flag = 1; 
	}
}

graph_off()
{
	if(lgraph)
	{
		printf("%c",0xf); /* si to select g0 character set */
		graph_on_flag = 0; 
	}
}

display_dot()
{
	int setgrph;
	setgrph = 0;
	if(lgraph && (graph_on_flag == 0))
	{
		graph_on();
		setgrph++;
	}

	if(lgraph) /* terminal dependent code  for mbf 4313*/
		printf("w");
	else
		printf(".");
	if(setgrph)
		graph_off();
}

display_horiz_line()
{
	int setgrph;
	setgrph = 0;
	if(lgraph && (graph_on_flag == 0))
	{
		graph_on();
		setgrph++;
	}

	if(lgraph)  /* if line graphics enabled */
		printf("e");
	else
		printf("-"); 

	if(setgrph)
		graph_off();
}

display_vert_line()
{
	int setgrph;
	setgrph = 0;
	if(lgraph && (graph_on_flag == 0))
	{
		graph_on();
		setgrph++;
	}

	if(lgraph)  /* if line graphics enabled */
		printf("d");
	else
		printf("|");

	if(setgrph)
		graph_off();
}

display_upper_left_angle()
{
	int setgrph;
	setgrph = 0;
	if(lgraph && (graph_on_flag == 0))
	{
		graph_on();
		setgrph++;
	}

		if(lgraph)
			printf("c");
		else
			printf("*"); 

	if(setgrph)
		graph_off();
}

display_upper_right_angle()
{
	int setgrph;
	setgrph = 0;
	if(lgraph && (graph_on_flag == 0))
	{
		graph_on();
		setgrph++;
	}

		if(lgraph)  /* if line graphics enabled */
			printf("a");
		else
			printf("*");

	if(setgrph)
		graph_off();
}

display_lower_right_angle()
{
	int setgrph;
	setgrph = 0;
	if(lgraph && (graph_on_flag == 0))
	{
		graph_on();
		setgrph++;
	}

		if(lgraph)  /* if line graphics enabled */
			printf("q");
		else
			printf("*");

	if(setgrph)
		graph_off();
}

display_lower_left_angle()
{
	int setgrph;
	setgrph = 0;
	if(lgraph && (graph_on_flag == 0))
	{
		graph_on();
		setgrph++;
	}

	if(lgraph)  /* if line graphics enabled */
		printf("s");
	else
		printf("*");

	if(setgrph)
		graph_off();
}

display_vert_and_left_line()
{
	int setgrph;
	setgrph = 0;
	if(lgraph && (graph_on_flag == 0))
	{
		graph_on();
		setgrph++;
	}

		if(lgraph)  /* if line graphics enabled */
			printf("v");
		else
			printf("*");

	if(setgrph)
		graph_off();
}

display_hor_and_upper_vert_line()
{
	int setgrph;
	setgrph = 0;
	if(lgraph && (graph_on_flag == 0))
	{
		graph_on();
		setgrph++;
	}

		if(lgraph)  /* if line graphics enabled */
			printf("b");
		else
			printf("*");

	if(setgrph)
		graph_off();
}

display_vert_and_right_line()
{
	int setgrph;
	setgrph = 0;
	if(lgraph && (graph_on_flag == 0))
	{
		graph_on();
		setgrph++;
	}

		if(lgraph)  /* if line graphics enabled */
			printf("t");
		else
			printf("*");

	if(setgrph)
		graph_off();
}

display_hor_and_bottom_vert_line()
{
	int setgrph;
	setgrph = 0;
	if(lgraph && (graph_on_flag == 0))
	{
		graph_on();
		setgrph++;
	}

		if(lgraph)  /* if line graphics enabled */
			printf("r");
		else
			printf("*");

	if(setgrph)
		graph_off();
}

display_hor_and_vert_line()
{
	int setgrph;
	setgrph = 0;
	if(lgraph && (graph_on_flag == 0))
	{
		graph_on();
		setgrph++;
	}

		if(lgraph)  /* if line graphics enabled */
			printf("u");
		else
			printf("+");

	if(setgrph)
		graph_off();
}


int chk_for_key()
{
	static int ln = 0;
	static int index = 0;
	int x;
	char c[80];
/* one method of checking for keystrokes */
	if ( (x=key_pressed()) != -1) 
	{
	  return(x);
	}
/* another method, both work on the authors system, chose what works on yours */
#if 0
	if(ioctl(0, FIONREAD, &ln) == -1) ln = 0;
	if(ln != 0)
	{
	read(0,&c[0],1);
	return c[0];
	}
#endif
	return 0;
}

/*
 *	This function will return -1 if no key is available, or the key
 *	that was pressed by the user.  It is checking stdin, without blocking.
 *
 */
int key_pressed()
{
#if 0
	int mask=1;
	static char keypressed;
	struct timeval waittime;
	int num_chars_read;

	waittime.tv_sec=0;
	waittime.tv_usec=0;
	if (select(1,&mask,0,0,&waittime)) {
		num_chars_read=read(0,&keypressed,1);
		if (num_chars_read == 1)
			return((int)keypressed);
	}
	return(-1);
#else
  byte buf[20];
  if (Read(Heliosno(stdin), buf, 1, OneSec / 20) < 1)
   return(-1);
  else
   return(buf[0]);
#endif

}

curpos(row,col)
int row;
int col;
{
	printf("%s",tgoto(T_CM, col, row));
#if 0
	move(row,col);
	row++;
	col++;
	printf("%1cX=%02d%03d",0x1b,row,col);
#endif
}

/* enter with oldrow, oldcol, newrow, newcol, oldchar, newchar */
mov_chr(orow,ocol,nrow,ncol,oc,nc)
register int orow,ocol;
register int nrow,ncol;
char oc,nc;
{
#if 0
	orow++;
	ocol++;
	nrow++;
	ncol++;
	printf("%1cX=%02d%03d%c%1cX=%02d%03d%c",0x1b,orow,
		ocol,oc,0x1b,nrow,ncol,nc);
#endif
	curpos(orow,ocol);
	printf("%c",oc);
	curpos(nrow,ncol);
	printf("%c",nc);
}

erase_screen()
{
	printf(T_ED);
}



#if 0
FILE *fp_out;

/*
 *  open the output file
 */
open_log_file(log_file_name)
char *log_file_name;
{
	int i;
	for(i=0;i < 5;i++)
	{

	if((fp_out = fopen(log_file_name, "a")) == NULL ) 
	{
	/*
		printf("Cannot open output file: %s\n", log_file_name);
		exit(1);
	*/
	}
	else
	break;
	}

	return (int)fp_out;
}

log_entry(log_file_name,usr_string)
char *log_file_name;
char *usr_string;
{
	char *index_out;
	char *usr_name;
	char *usr_time;
	struct timeval current_time;

	index_out = log_file_name;
	if(open_log_file(index_out)== NULL) return 0;
	(void) gettimeofday ( &current_time, NULL );
	usr_time = (char *)ctime(&current_time);
	usr_name = (char *)getlogin();
	fprintf(fp_out,"%s %s %s %s",msg_ver,usr_name,usr_string,usr_time);
	fclose(fp_out);
}

log_in()
{
	log_entry(log_file ,"logged in ");
}

log_out()
{
	char msglogout[80];
	sprintf(msglogout,"pellets %3d, skill %2d, logged out",
		n_dots_eaten,playing_at_skill_level);
	log_entry(log_file ,msglogout);
}

#else
void log_in()
{
}

void log_out()
{
}
#endif
