/*
 * Grand digital clock for curses compatible terminals
 * Usage: gdc [-s] [n]   -- run for n seconds (default infinity)
 * Flags: -s: scroll
 *
 * modified 10-18-89 for curses (jrl)
 * 10-18-89 added signal handling
 */
#ifdef __HELIOS
#include <helios.h>
#include <syslib.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <curses.h>
#include <signal.h>
#include <string.h>

/* it won't be */
#ifdef __STDC__
time_t now; /* yeah! */
#else
long now; /* yeah! */
#endif
struct tm *tm;

short disp[11] = {
	075557, 011111, 071747, 071717, 055711,
	074717, 074757, 071111, 075757, 075717, 002020
};
long old[6], next[6], New[6], mask;
char scrol;

void set(int t, int n);
void clr(void);
void standt(int on);
void movto(int line, int col);

int sigtermed=0;

#ifndef __HELIOS

void sighndl(signo)
int signo;
{
	signal(signo, sighndl);
	sigtermed=signo;
}

#else

void sighndl(int signo)
{
	sigtermed = TRUE;
	signo = signo;	/* happy compiler */
}

#endif

int main(argc, argv)
	int argc;
	char **argv;
{
	register long t, a;
	register i, j, s, n, k;
	signal(SIGHUP,sighndl);
	signal(SIGINT,sighndl);
	signal(SIGQUIT,sighndl);
	signal(SIGTERM,sighndl);

	initscr();
#ifdef QWRERT__HELIOS
	nonl();
	cbreak();
#endif
	clr();
	refresh();

	n = scrol = 0;
	while(--argc > 0) {
		if(**++argv == '-') 
			scrol = 1;
		else
			n = atoi(*argv);
	}
	do {
		mask = 0;
		time(&now);
		tm = localtime(&now);
		set(tm->tm_sec%10, 0);
		set(tm->tm_sec/10, 4);
		set(tm->tm_min%10, 10);
		set(tm->tm_min/10, 14);
		set(tm->tm_hour%10, 20);
		set(tm->tm_hour/10, 24);
		set(10, 7);
		set(10, 17);
		for(k=0; k<6; k++) {
			if(scrol) {
				for(i=0; i<5; i++)
					New[i] = New[i]&~mask | New[i+1]&mask;
				New[5] = New[5]&~mask | next[k]&mask;
			} else
				New[k] = New[k]&~mask | next[k]&mask;
			next[k] = 0;
			for(s=1; s>=0; s--)
			{
				standt(s);
				for(i=0; i<6; i++)
				{
					if ((a = (New[i]^old[i])&(s ? New : old)[i]) != 0)
					{
						for(j=0,t=1<<26; t; t>>=1,j++)
						{
							if(a&t)
							{
								if(!(a&(t<<1)))
								{
									movto(i, 2*j);
								}
								addstr("  ");
							}
						}
					}
					if(!s)
					{
						old[i] = New[i];
					}
				}
				if(!s)
				{
					refresh();
				}
			}
		}
		movto(6, 0);
		refresh();
#ifdef __HELIOS
		Delay(OneSec/2);
#else
		sleep(1);
#endif
		if (sigtermed)
		{
			standend();
			clear();
			refresh();
			endwin();
#ifndef __HELIOS
			fprintf(stderr, "gdc terminated by signal %d\n", sigtermed);
#else
			fprintf(stderr, "gdc terminated by signal\n");
#endif
			exit(1);
		}
	} while(--n);
	standend();
	clear();
	refresh();
	endwin();
	return(0);
}

void set(t, n)
	int t;
	register n;
{
	register i, m;

	m = 7<<n;
	for(i=0; i<5; i++) {
		next[i] |= (long) ((disp[t]>>(4-i)*3) & 07) << n;
		mask |= (next[i]^old[i])&m;
	}
	if(mask&m)
		mask |= m;
}

/* terminal-dependent routines */
void clr(void)
{
	clear();
	refresh();
}

void standt(on)
int on;
{
	if (on)
	{
		standout();
	}
	else
	{
		standend();
	}
}

void movto(int line, int col)
{
	move(line, col);
}
