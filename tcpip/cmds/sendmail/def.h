/*
 * Copyright (c) 1991 Regents of the University of California and
 * the University of Illinois Board of Trustees.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Contributed by Paul Pomes, University of Illinois Computing Services Office.
 *
 *	@(#)def.h	1.1 (Berkeley) 2/17/91
 */

/*
static char *rcsid = "$Header: /hsrc/tcpip/cmds/sendmail/RCS/def.h,v 1.1 1992/01/20 14:46:28 craig Exp $";
*/

/*
 * ANSI C function declarators
 */

#ifndef __HELIOS
#ifndef lint
# ifdef _DEFINE
static char def_h_sccsid[] = "@(#)$Version$";
# endif /* _DEFINE */
#endif /* !lint */
#endif

#ifdef __STDC__

/* List of machine types that don't do type promotion in prototypes */
# if defined(apollo)
#  define	CC_WONT_PROMOTE
# endif /* apollo */

#ifndef __HELIOS
/* alias.c */
void alias(ADDRESS *, ADDRESS **);
char * aliaslookup(char *);
void initaliases(bool);
void forward(ADDRESS *, ADDRESS **);
#endif

/* arpadate.c */
#ifndef __HELIOS
char * arpadate(char *);
#else
char * arpadate(void);
#endif

#ifndef __HELIOS
/* clock.c */
EVENT * setevent(time_t, void (*f)(), int);
void clrevent(EVENT *);
void Xsleep(unsigned int);
#endif

/* collect.c */
void collect(bool);

#ifndef __HELIOS
/* conf.c */
void setdefaults();
void setdefuser();
int getruid();
int getrgid();
char * username();
char * ttypath();
bool checkcompat(ADDRESS *);
void holdsigs();
void rlsesigs();
int getla();
bool shouldqueue(long);
# ifdef VSPRINTF
/* void setproctitle(const char *, va_alist);	fails with gcc */
void setproctitle();
# else /* !VSPRINTF */
void setproctitle();
# endif /* VSPRINTF */
void reapchild();

/* convtime.c */
time_t convtime(const char *);
char * pintvl(time_t, bool);
#endif

/* daemon.c */
#ifndef __HELIOS
void getrequests();
void clrdaemon();
# ifdef	CC_WONT_PROMOTE
int makeconnection(const char *, u_short, FILE **, FILE **);
# else	/* !CC_WONT_PROMOTE */
int makeconnection(const char *, int, FILE **, FILE **); /* u_short -> int */
# endif	/* CC_WONT_PROMOTE */
char ** myhostname(char [], int);
bool maphostname(char *, bool);
# ifdef	CC_WONT_PROMOTE
bool mapinit(char);
char * mapkey(char, char *, int, const char *);
# else	/* !CC_WONT_PROMOTE */
bool mapinit(int);				/* char -> int */
char * mapkey(int, char *, int, const char *);	/* char -> int */
# endif	/* CC_WONT_PROMOTE */
#else
int getrequests(void) ;
int makeconnection(const char *, u_short, FILE **, FILE **) ;
char **myhostname(char [], int) ;
#endif

/* deliver.c */
#ifndef __HELIOS
int deliver(ENVELOPE *, ADDRESS *);;
int dofork();
int endmailer(int, const char *);
int openmailer(MAILER *, char **, ADDRESS *, bool, FILE **, FILE **);
void giveresponse(int, MAILER *, ENVELOPE *);
void logdelivery(const char *);
void putfromline(FILE *, MAILER *);
void putbody(FILE *, MAILER *, ENVELOPE *);
# ifdef	CC_WONT_PROMOTE
void sendall(ENVELOPE *, char);
# else	/* !CC_WONT_PROMOTE */
void sendall(ENVELOPE *, int);			/* char -> int */
# endif	/* CC_WONT_PROMOTE */
#else
int deliver(ENVELOPE *) ;
int endmailer(int, const char *);
int openmailer(MAILER *, char **, bool, FILE **, FILE **);
void putbody(FILE *, MAILER *, ENVELOPE *);
void giveresponse(int, ENVELOPE *) ;
#endif

#ifndef __HELIOS
/* domain.c */
int getmxrr(const char *, char **, const char *, int *);
bool getcanonname(char *, int);

/* envelope.c */
ENVELOPE * newenvelope(ENVELOPE *);
void dropenvelope(ENVELOPE *);
void clearenvelope(ENVELOPE *, bool);
void initsys();
void settime();
void openxscript(ENVELOPE *);
void setsender(char *);
#endif

/* err.c */
#ifndef __HELIOS
# ifdef VSPRINTF
void syserr();
void usrerr();
void message();
void nmessage();
# else /* !VSPRINTF */
/*
 * The following don't work as gcc transforms va_alist for some reason.
 *
 * void syserr(const char *, va_alist);
 * void usrerr(const char *, va_alist);
 * void message(const char *, const char *, va_alist);
 * void nmessage(const char *, const char *, va_alist);
 */
void syserr();
void usrerr();
void message();
void nmessage();
# endif /* VSPRINTF */
#else
void syserr(const char *, ...);
void usrerr(const char *, ...);
void message(const char *, const char *, ...);
void nmessage(const char *, const char *, ...);
#endif
char * errstring(int);

#ifndef __HELIOS
/* getloadavg.c */
int getloadavg(caddr_t);
#endif

/* headers.c */
#ifndef __HELIOS
int chompheader(char *, bool);
void addheader(char *, const char *, ENVELOPE *);
char * hvalue(const char *);
bool isheader(const char *);
void eatheader(ENVELOPE *, FILE *);
char * crackaddr(char *);
void putheader(FILE *, MAILER *, ENVELOPE *);
void commaize(HDR *, char *, FILE *, bool, MAILER *);
#else
#ifdef XXX_HEADER
void putheader(FILE *, MAILER *, ENVELOPE *, bool);
#endif
#endif

#ifndef __HELIOS
/* macro.c */
void expand(const char *, char *, const char *, ENVELOPE *);
# ifdef	CC_WONT_PROMOTE
void define(char, char *, ENVELOPE *);
char * macvalue(char, ENVELOPE *);
# else	/* !CC_WONT_PROMOTE */
void define(int, char *, ENVELOPE *);		/* char -> int */
char * macvalue(int, ENVELOPE *);		/* char -> int */
# endif	/* CC_WONT_PROMOTE */
#endif

/* main.c */
#ifndef __HELIOS
void finis(void);
void disconnect(bool);
#else
void disconnect(void);
#endif

#ifndef __HELIOS
/* parseaddr.c */
# ifdef	CC_WONT_PROMOTE
ADDRESS * parseaddr(const char *, ADDRESS *, int, char);
char ** prescan(char *, char, char []);
# else	/* !CC_WONT_PROMOTE */
ADDRESS * parseaddr(char *, ADDRESS *, int, int);	/* char -> int */
char ** prescan(char *, int, char []);		/* char -> int */
# endif	/* CC_WONT_PROMOTE */
void loweraddr(ADDRESS *);
bool invalidaddr(const char *);
void rewrite(char **, int);
void cataddr(char **, char *, int);
bool sameaddr(ADDRESS *, ADDRESS *);
void printaddr(ADDRESS *, bool);
char * remotename(char *, MAILER *, bool, bool, bool);
#endif

/* queue.c */
#ifndef __HELIOS
FILE * queueup(ENVELOPE *, bool, bool);
void runqueue(int);
void printqueue();
# ifdef	CC_WONT_PROMOTE
char * queuename(ENVELOPE *, char);
# else	/* !CC_WONT_PROMOTE */
char * queuename(ENVELOPE *, int);		/* char -> int */
# endif	/* CC_WONT_PROMOTE */
void unlockqueue(ENVELOPE *);
void setctladdr(ADDRESS *);
#else
char * queuename(ENVELOPE *, char);
#endif

#ifndef __HELIOS
/* readcf.c */
void readcf(char *);
void printrules();
# ifdef	CC_WONT_PROMOTE
void setoption(char, const char *, bool, bool);
# else	/* !CC_WONT_PROMOTE */
void setoption(int, const char *, bool, bool);	/* char -> int */
# endif	/* CC_WONT_PROMOTE */
void setclass(int, const char *);

/* recipient.c */
void sendtolist(const char *, ADDRESS *, ADDRESS **);
ADDRESS * recipient(ADDRESS *, ADDRESS **);
void include(const char *, const char *, ADDRESS *, ADDRESS **);
void sendtoargv(char **);
ADDRESS * getctladdr(ADDRESS *);

/* savemail.c */
void savemail(ENVELOPE *);
int returntosender(const char *, ADDRESS *, bool);

/* srvrsmtp.c */
void smtp(bool);

/* stab.c */
STAB * stab(const char *, int, int);

/* stats.c */
void markstats(ENVELOPE *, ADDRESS *);
void poststats(char *);
#endif

/* sysexits.c */
char * statstring(int);

#ifndef __HELIOS
/* trace.c */
void tTsetup(u_char *, int, const char *);
void tTflag(const char *);
#endif

/* usersmtp.c */
#ifndef __HELIOS
# ifdef MAIL11V3
int smtpinit(MAILER *, char **, ENVELOPE *);
# else /* ! MAIL11V3 */
int smtpinit(MAILER *, char **);
# endif /* MAIL11V3 */
int smtprcpt(ADDRESS *, MAILER *);
int smtpdata(MAILER *, ENVELOPE *);
void smtpquit(MAILER *);
int smtpstat(MAILER *);
#else
int smtpinit  (MAILER *, char **) ;
int smtprcpt  (MAILER *) ;
int smtpdata  (MAILER *) ;
void smtpquit (MAILER *) ;
#endif

/* util.c */
#ifndef __HELIOS
void stripquotes(char *, bool);
int qstrlen(const char *);
char * capitalize(const char *);
char * xalloc(int);
char ** copyplist(char **, bool);
void printav(char **);
# ifdef	CC_WONT_PROMOTE
char lower(char);
# else	/* !CC_WONT_PROMOTE */
char lower(int);				/* char -> int */
# endif	/* CC_WONT_PROMOTE */
void xputs(const char *);
void makelower(char *);
void buildfname(const char *, const char *, char *);
bool safefile(char *, int, int);
void fixcrlf(char *, bool);
FILE * dfopen(const char *, const char *);
void putline(char *, FILE *, MAILER *);
void xunlink(char *);
char * sfgets(char *, int, FILE *);
char * fgetfolded(char *, int, FILE *);
time_t curtime();
bool atobool(const char *);
int atooct(const char *);
int waitfor(int);
bool bitintersect(BITMAP, BITMAP);
bool bitzerop(BITMAP);
void printcav(char **);
void WritePid();
#else
void makelower(char *);
void fixcrlf(char *, bool);
void putline(char *, FILE *, MAILER *);
void unlink_temps (ENVELOPE *) ;
char *sfgets(char *, int, FILE *);
int waitfor(int);
void WritePid(char *);
#endif

/* version.c */
#ifdef __HELIOS
char *Version_ID (void) ;
#endif

#ifdef __HELIOS
/* extras.c */
void debugf (char *, ...) ;
void Init_Env (ENVELOPE *) ;
void Init_Mailer (MAILER *) ;
bool local_name (char *) ;
void create_xf (ENVELOPE *) ;
void intsig(void) ;
void finis(void);
char *itoa (register int) ;
bool valid_name (char *) ;
#endif

#else /* !__STDC__ */

/* This keeps non-ANSI compilers happy */
# define	const

/* alias.c */
void alias();
char * aliaslookup();
void initaliases();
void forward();

/* arpadate.c */
char * arpadate();

/* clock.c */
EVENT * setevent();
void clrevent();
void Xsleep();

/* collect.c */
void collect();

/* conf.c */
void setdefaults();
void setdefuser();
int getruid();
int getrgid();
char * username();
char * ttypath();
bool checkcompat();
void holdsigs();
void rlsesigs();
int getla();
bool shouldqueue();
# ifdef VSPRINTF
void setproctitle();
# else /* !VSPRINTF */
void setproctitle();
# endif /* VSPRINTF */
void reapchild();

/* convtime.c */
time_t convtime();
char * pintvl();

/* daemon.c */
void getrequests();
void clrdaemon();
int makeconnection();
char ** myhostname();
bool maphostname();
bool mapinit();
char * mapkey();

/* deliver.c */
int deliver();;
int dofork();
int endmailer();
int openmailer();
void giveresponse();
void logdelivery();
void putfromline();
void putbody();
void sendall();

/* domain.c */
int getmxrr();
bool getcanonname();

/* envelope.c */
ENVELOPE * newenvelope();
void dropenvelope();
void clearenvelope();
void initsys();
void settime();
void openxscript();
void setsender();

/* err.c */
void syserr();
void usrerr();
void message();
void nmessage();
char * errstring();

/* getloadavg.c */
int getloadavg();

/* headers.c */
int chompheader();
void addheader();
char * hvalue();
bool isheader();
void eatheader();
char * crackaddr();
void putheader();
void commaize();

/* macro.c */
void expand();
void define();
char * macvalue();

/* main.c */
void finis();
void disconnect();

/* parseaddr.c */
ADDRESS * parseaddr();
void loweraddr();
bool invalidaddr();
char ** prescan();
void rewrite();
void cataddr();
bool sameaddr();
void printaddr();
char * remotename();

/* queue.c */
FILE * queueup();
void runqueue();
void printqueue();
char * queuename();
void unlockqueue();
void setctladdr();

/* readcf.c */
void readcf();
void printrules();
void setoption();
void setclass();

/* recipient.c */
void sendtolist();
ADDRESS * recipient();
void include();
void sendtoargv();
ADDRESS * getctladdr();

/* savemail.c */
void savemail();
int returntosender();

/* srvrsmtp.c */
void smtp();

/* stab.c */
STAB * stab();

/* stats.c */
void markstats();
void poststats();

/* sysexits.c */
char * statstring();

/* trace.c */
void tTsetup();
void tTflag();

/* usersmtp.c */
# ifdef MAIL11V3
int smtpinit();
# else /* ! MAIL11V3 */
int smtpinit();
# endif /* MAIL11V3 */
int smtprcpt();
int smtpdata();
void smtpquit();
int smtpstat();

/* util.c */
void stripquotes();
int qstrlen();
char * capitalize();
char * xalloc();
char ** copyplist();
void printav();
char lower();
void xputs();
void makelower();
void buildfname();
bool safefile();
void fixcrlf();
FILE * dfopen();
void putline();
void xunlink();
char * sfgets();
char * fgetfolded();
time_t curtime();
bool atobool();
int atooct();
int waitfor();
bool bitintersect();
bool bitzerop();
void printcav();
void WritePid();

/* version.c */

/* random ones */
char *getenv();
char *malloc();

#endif /* __STDC__ */
