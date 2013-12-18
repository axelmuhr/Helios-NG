
/* # line 26 "ftpcmd.y" */

#ifdef lint
static char sccsid[] = "@(#)ftpcmd.y	5.20.1.1 (Berkeley) 3/2/89";
#endif /* not lint */

#include <sys/param.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <arpa/ftp.h>

#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include <pwd.h>
#include <setjmp.h>
#include <syslog.h>
#include <sys/stat.h>
#include <time.h>

#ifdef __HELIOS
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <posix.h>
#endif

#include "ftpd.h"
  
extern	struct sockaddr_in data_dest;
extern	int logged_in;
extern	struct passwd *pw;
extern	int guest;
extern	int logging;
extern	int type;
extern	int form;
extern	int debug;
extern	int timeout;
extern	int maxtimeout;
extern  int pdata;
extern	char hostname[], remotehost[];
extern	char proctitle[];
extern	char *globerr;
extern	int usedefault;
extern  int transflag;
extern  char tmpline[];

static	int cmd_type;
static	int cmd_form;
static	int cmd_bytesz;
char	cbuf[512];
char	*fromname;

/*char	*index();*/
# define A 257
# define B 258
# define C 259
# define E 260
# define F 261
# define I 262
# define L 263
# define N 264
# define P 265
# define R 266
# define S 267
# define T 268
# define SP 269
# define CRLF 270
# define COMMA 271
#ifdef STRING
#undef STRING
#endif
# define STRING 272
# define NUMBER 273
# define USER 274
# define PASS 275
# define ACCT 276
# define REIN 277
# define QUIT 278
# define PORT 279
# define PASV 280
# define TYPE 281
# define STRU 282
# define MODE 283
# define RETR 284
# define STOR 285
# define APPE 286
# define MLFL 287
# define MAIL 288
# define MSND 289
# define MSOM 290
# define MSAM 291
# define MRSQ 292
# define MRCP 293
# define ALLO 294
# define REST 295
# define RNFR 296
# define RNTO 297
# define ABOR 298
# define DELE 299
# define CWD 300
# define LIST 301
# define NLST 302
# define SITE 303
# define STAT 304
# define HELP 305
# define NOOP 306
# define MKD 307
# define RMD 308
# define PWD 309
# define CDUP 310
# define STOU 311
# define SMNT 312
# define SYST 313
# define SIZE 314
# define MDTM 315
# define UMASK 316
# define IDLE 317
# define CHMOD 318
# define LEXERR 319
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern short yyerrflag;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
#ifndef YYSTYPE
#define YYSTYPE int
#endif
YYSTYPE yylval, yyval;
# define YYERRCODE 256

/* # line 657 "ftpcmd.y" */


extern jmp_buf errcatch;

#define	CMD	0	/* beginning of command */
#define	ARGS	1	/* expect miscellaneous arguments */
#define	STR1	2	/* expect SP followed by STRING */
#define	STR2	3	/* expect STRING */
#define	OSTR	4	/* optional SP then STRING */
#define	ZSTR1	5	/* SP then optional STRING */
#define	ZSTR2	6	/* optional STRING after SP */
#define	SITECMD	7	/* SITE command */
#define	NSTR	8	/* Number followed by a string */

struct tab {
	char	*name;
	short	token;
	short	state;
	short	implemented;	/* 1 if command is implemented */
	char	*help;
};

struct tab cmdtab[] = {		/* In order defined in RFC 765 */
	{ "USER", USER, STR1, 1,	"<sp> username" },
	{ "PASS", PASS, ZSTR1, 1,	"<sp> password" },
	{ "ACCT", ACCT, STR1, 0,	"(specify account)" },
	{ "SMNT", SMNT, ARGS, 0,	"(structure mount)" },
	{ "REIN", REIN, ARGS, 0,	"(reinitialize server state)" },
	{ "QUIT", QUIT, ARGS, 1,	"(terminate service)", },
	{ "PORT", PORT, ARGS, 1,	"<sp> b0, b1, b2, b3, b4" },
	{ "PASV", PASV, ARGS, 1,	"(set server in passive mode)" },
	{ "TYPE", TYPE, ARGS, 1,	"<sp> [ A | E | I | L ]" },
	{ "STRU", STRU, ARGS, 1,	"(specify file structure)" },
	{ "MODE", MODE, ARGS, 1,	"(specify transfer mode)" },
	{ "RETR", RETR, STR1, 1,	"<sp> file-name" },
	{ "STOR", STOR, STR1, 1,	"<sp> file-name" },
	{ "APPE", APPE, STR1, 1,	"<sp> file-name" },
	{ "MLFL", MLFL, OSTR, 0,	"(mail file)" },
	{ "MAIL", MAIL, OSTR, 0,	"(mail to user)" },
	{ "MSND", MSND, OSTR, 0,	"(mail send to terminal)" },
	{ "MSOM", MSOM, OSTR, 0,	"(mail send to terminal or mailbox)" },
	{ "MSAM", MSAM, OSTR, 0,	"(mail send to terminal and mailbox)" },
	{ "MRSQ", MRSQ, OSTR, 0,	"(mail recipient scheme question)" },
	{ "MRCP", MRCP, STR1, 0,	"(mail recipient)" },
	{ "ALLO", ALLO, ARGS, 1,	"allocate storage (vacuously)" },
	{ "REST", REST, ARGS, 0,	"(restart command)" },
	{ "RNFR", RNFR, STR1, 1,	"<sp> file-name" },
	{ "RNTO", RNTO, STR1, 1,	"<sp> file-name" },
	{ "ABOR", ABOR, ARGS, 1,	"(abort operation)" },
	{ "DELE", DELE, STR1, 1,	"<sp> file-name" },
	{ "CWD",  CWD,  OSTR, 1,	"[ <sp> directory-name ]" },
	{ "XCWD", CWD,	OSTR, 1,	"[ <sp> directory-name ]" },
	{ "LIST", LIST, OSTR, 1,	"[ <sp> path-name ]" },
	{ "NLST", NLST, OSTR, 1,	"[ <sp> path-name ]" },
	{ "SITE", SITE, SITECMD, 1,	"site-cmd [ <sp> arguments ]" },
	{ "SYST", SYST, ARGS, 1,	"(get type of operating system)" },
	{ "STAT", STAT, OSTR, 1,	"[ <sp> path-name ]" },
	{ "HELP", HELP, OSTR, 1,	"[ <sp> <string> ]" },
	{ "NOOP", NOOP, ARGS, 1,	"" },
	{ "MKD",  MKD,  STR1, 1,	"<sp> path-name" },
	{ "XMKD", MKD,  STR1, 1,	"<sp> path-name" },
	{ "RMD",  RMD,  STR1, 1,	"<sp> path-name" },
	{ "XRMD", RMD,  STR1, 1,	"<sp> path-name" },
	{ "PWD",  PWD,  ARGS, 1,	"(return current directory)" },
	{ "XPWD", PWD,  ARGS, 1,	"(return current directory)" },
	{ "CDUP", CDUP, ARGS, 1,	"(change to parent directory)" },
	{ "XCUP", CDUP, ARGS, 1,	"(change to parent directory)" },
	{ "STOU", STOU, STR1, 1,	"<sp> file-name" },
	{ "SIZE", SIZE, OSTR, 1,	"<sp> path-name" },
	{ "MDTM", MDTM, OSTR, 1,	"<sp> path-name" },
	{ NULL,   0,    0,    0,	0 }
};

struct tab sitetab[] = {
	{ "UMASK", UMASK, ARGS, 1,	"[ <sp> umask ]" },
	{ "IDLE", IDLE, ARGS, 1,	"[ <sp> maximum-idle-time ]" },
	{ "CHMOD", CHMOD, NSTR, 1,	"<sp> mode <sp> file-name" },
	{ "HELP", HELP, OSTR, 1,	"[ <sp> <string> ]" },
	{ NULL,   0,    0,    0,	0 }
};

struct tab *
lookup(p, cmd)
	register struct tab *p;
	char *cmd;
{

	for (; p->name != NULL; p++)
		if (strcmp(cmd, p->name) == 0)
			return (p);
	return (0);
}

#include <arpa/telnet.h>

/*
 * getline - a hacked up version of fgets to ignore TELNET escape codes.
 */
char *
getline(
	char *s,
	int n,
	register FILE *iop )
{
	register c;
	register char *cs;

	cs = s;
/* tmpline may contain saved command from urgent mode interruption */
	for (c = 0; tmpline[c] != '\0' && --n > 0; ++c) {
		*cs++ = tmpline[c];
		if (tmpline[c] == '\n') {
			*cs++ = '\0';
			if (debug)
				syslog(LOG_DEBUG, "command: %s", s);
			tmpline[0] = '\0';
			return(s);
		}
		if (c == 0)
			tmpline[0] = '\0';
	}
	while ((c = getc(iop)) != EOF) {
		c &= 0377;
		if (c == IAC) {
		    if ((c = getc(iop)) != EOF) {
			c &= 0377;
			switch (c) {
			case WILL:
			case WONT:
				c = getc(iop);
				printf("%c%c%c", IAC, DONT, 0377&c);
				(void) fflush(stdout);
				continue;
			case DO:
			case DONT:
				c = getc(iop);
				printf("%c%c%c", IAC, WONT, 0377&c);
				(void) fflush(stdout);
				continue;
			case IAC:
				break;
			default:
				continue;	/* ignore command */
			}
		    }
		}
		*cs++ = c;
		if (--n <= 0 || c == '\n')
			break;
	}
	if (c == EOF && cs == s)
		return (NULL);
	*cs++ = '\0';
	if (debug)
		syslog(LOG_DEBUG, "command: %s", s);
	return (s);
}

static void
toolong( int a )
{
	time_t now;

	reply(421,
	  "Timeout (%d seconds): closing control connection.", timeout);
	(void) time(&now);
	if (logging) {
		syslog(LOG_INFO,
			"User %s timed out after %d seconds at %s",
			(pw ? pw -> pw_name : "unknown"), timeout, ctime(&now));
	}
	dologout(1);
	return;
	a=a;
}

void
upper(s)
	register char *s;
{
	while (*s != '\0') {
		if (islower(*s))
			*s = toupper(*s);
		s++;
	}
}

char *
copy(s)
	char *s;
{
	char *p;

	p = (char *) malloc((unsigned) strlen(s) + 1);
	if (p == NULL)
		fatal("Ran out of memory.");
	(void) strcpy(p, s);
	return (p);
}

int
yylex()
{
	static int cpos, state;
	register char *cp, *cp2;
	register struct tab *p;
	int n;
	char c;

	for (;;) {
		switch (state) {

		case CMD:
			(void) signal(SIGALRM, toolong);
			(void) alarm((unsigned) timeout);
			if (getline(cbuf, sizeof(cbuf)-1, stdin) == NULL) {
				reply(221, "You could at least say goodbye.");
				dologout(0);
			}
			(void) alarm(0);
#ifdef SETPROCTITLE
			if (strncasecmp(cbuf, "PASS", 4) != NULL)
				setproctitle("%s: %s", proctitle, cbuf);
#endif /* SETPROCTITLE */
			if ((cp = index(cbuf, '\r')) != NULL) {
				*cp++ = '\n';
				*cp = '\0';
			}
			if ((cp = strpbrk(cbuf, " \n")) != NULL)
				cpos = cp - cbuf;
			if (cpos == 0)
				cpos = 4;
			c = cbuf[cpos];
			cbuf[cpos] = '\0';
			upper(cbuf);
			p = lookup(cmdtab, cbuf);
			cbuf[cpos] = c;
			if (p != 0) {
				if (p->implemented == 0) {
					nack(p->name);
					longjmp(errcatch,0);
					/* NOTREACHED */
				}
				state = p->state;
				*(char **)&yylval = p->name;
				return (p->token);
			}
			break;

		case SITECMD:
			if (cbuf[cpos] == ' ') {
				cpos++;
				return (SP);
			}
			cp = &cbuf[cpos];
			if ((cp2 = strpbrk(cp, " \n")) != NULL)
				cpos = cp2 - cbuf;
			c = cbuf[cpos];
			cbuf[cpos] = '\0';
			upper(cp);
			p = lookup(sitetab, cp);
			cbuf[cpos] = c;
			if (p != 0) {
				if (p->implemented == 0) {
					state = CMD;
					nack(p->name);
					longjmp(errcatch,0);
					/* NOTREACHED */
				}
				state = p->state;
				*(char **)&yylval = p->name;
				return (p->token);
			}
			state = CMD;
			break;

		case OSTR:
			if (cbuf[cpos] == '\n') {
				state = CMD;
				return (CRLF);
			}
			/* FALLTHROUGH */

		case STR1:
		case ZSTR1:
		dostr1:
			if (cbuf[cpos] == ' ') {
				cpos++;
				state = (state == OSTR ? STR2 : state + 1);
				return (SP);
			}
			break;

		case ZSTR2:
			if (cbuf[cpos] == '\n') {
				state = CMD;
				return (CRLF);
			}
			/* FALLTHROUGH */

		case STR2:
			cp = &cbuf[cpos];
			n = strlen(cp);
			cpos += n - 1;
			/*
			 * Make sure the string is nonempty and \n terminated.
			 */
			if (n > 1 && cbuf[cpos] == '\n') {
				cbuf[cpos] = '\0';
				*(char **)&yylval = copy(cp);
				cbuf[cpos] = '\n';
				state = ARGS;
				return (STRING);
			}
			break;

		case NSTR:
			if (cbuf[cpos] == ' ') {
				cpos++;
				return (SP);
			}
			if (isdigit(cbuf[cpos])) {
				cp = &cbuf[cpos];
				while (isdigit(cbuf[++cpos]))
					;
				c = cbuf[cpos];
				cbuf[cpos] = '\0';
				yylval = atoi(cp);
				cbuf[cpos] = c;
				state = STR1;
				return (NUMBER);
			}
			state = STR1;
			goto dostr1;

		case ARGS:
			if (isdigit(cbuf[cpos])) {
				cp = &cbuf[cpos];
				while (isdigit(cbuf[++cpos]))
					;
				c = cbuf[cpos];
				cbuf[cpos] = '\0';
				yylval = atoi(cp);
				cbuf[cpos] = c;
				return (NUMBER);
			}
			switch (cbuf[cpos++]) {

			case '\n':
				state = CMD;
				return (CRLF);

			case ' ':
				return (SP);

			case ',':
				return (COMMA);

			case 'A':
			case 'a':
				return (A);

			case 'B':
			case 'b':
				return (B);

			case 'C':
			case 'c':
				return (C);

			case 'E':
			case 'e':
				return (E);

			case 'F':
			case 'f':
				return (F);

			case 'I':
			case 'i':
				return (I);

			case 'L':
			case 'l':
				return (L);

			case 'N':
			case 'n':
				return (N);

			case 'P':
			case 'p':
				return (P);

			case 'R':
			case 'r':
				return (R);

			case 'S':
			case 's':
				return (S);

			case 'T':
			case 't':
				return (T);

			}
			break;

		default:
			fatal("Unknown state in scanner.");
		}
		yyerror((char *) 0);
		state = CMD;
		longjmp(errcatch,0);
	}
}

void
help(ctab, s)
	struct tab *ctab;
	char *s;
{
	register struct tab *c;
	register int width, NCMDS;
	char *type;

	if (ctab == sitetab)
		type = "SITE ";
	else
		type = "";
	width = 0, NCMDS = 0;
	for (c = ctab; c->name != NULL; c++) {
		int len = strlen(c->name);

		if (len > width)
			width = len;
		NCMDS++;
	}
	width = (width + 8) &~ 7;
	if (s == 0) {
		register int i, j, w;
		int columns, lines;

		lreply(214, "The following %scommands are recognized %s.",
		    type, "(* =>'s unimplemented)");
		columns = 76 / width;
		if (columns == 0)
			columns = 1;
		lines = (NCMDS + columns - 1) / columns;
		for (i = 0; i < lines; i++) {
			printf("   ");
			for (j = 0; j < columns; j++) {
				c = ctab + j * lines + i;
				printf("%s%c", c->name,
					c->implemented ? ' ' : '*');
				if (c + lines >= &ctab[NCMDS])
					break;
				w = strlen(c->name) + 1;
				while (w < width) {
					putchar(' ');
					w++;
				}
			}
			printf("\r\n");
		}
		(void) fflush(stdout);
		reply(214, "Direct comments to ftp-bugs@%s.", hostname);
		return;
	}
	upper(s);
	c = lookup(ctab, s);
	if (c == (struct tab *)0) {
		reply(502, "Unknown command %s.", s);
		return;
	}
	if (c->implemented)
		reply(214, "Syntax: %s%s %s", type, c->name, c->help);
	else
		reply(214, "%s%-*s\t%s; unimplemented.", type, width,
		    c->name, c->help);
}

void
sizecmd(filename)
char *filename;
{
	switch (type) {
	case TYPE_L:
	case TYPE_I: {
		struct stat stbuf;
		if (stat(filename, &stbuf) < 0 ||
		    (stbuf.st_mode&S_IFMT) != S_IFREG)
			reply(550, "%s: not a plain file.", filename);
		else
			reply(213, "%lu", stbuf.st_size);
		break;}
	case TYPE_A: {
		FILE *fin;
		register int c, count;
		struct stat stbuf;
		fin = fopen(filename, "r");
		if (fin == NULL) {
			perror_reply(550, filename);
			return;
		}
		if (fstat(fileno(fin), &stbuf) < 0 ||
		    (stbuf.st_mode&S_IFMT) != S_IFREG) {
			reply(550, "%s: not a plain file.", filename);
			(void) fclose(fin);
			return;
		}

		count = 0;
		while((c=getc(fin)) != EOF) {
			if (c == '\n')	/* will get expanded to \r\n */
				count++;
			count++;
		}
		(void) fclose(fin);

		reply(213, "%ld", count);
		break;}
	default:
		reply(504, "SIZE not implemented for Type %c.", "?AEIL"[type]);
	}
}

short yyexca[] ={
-1, 1,
	0, -1,
	-2, 0,
	};
# define YYNPROD 74
# define YYLAST 326
short yyact[]={

  34, 109, 124, 199, 197, 194, 126, 188, 187, 184,
 175, 126, 110, 112, 111, 153,  89, 101,   4,   5,
  75, 171,  33,   6,   7,   8,   9,  10,  12,  13,
  14, 134, 104,  73,  71, 198, 196, 191,  11, 180,
  35,  19,  20,  18,  21,  16,  15,  28,  17,  22,
  23,  24,  25,  26,  27,  29, 120,  30,  31,  32,
 173, 172, 148, 147, 144, 143, 130, 129, 103, 102,
  96,  95,  94,  93,  56,  55, 195, 192, 189, 186,
 182, 179, 178, 177, 176, 170, 169, 168, 167, 166,
 165, 164, 163, 162, 161, 157, 140, 138, 128, 127,
 155, 121, 119, 118, 156, 117,  99, 108, 107,  68,
  67,  64,  57,  53,  50,  39, 190, 181, 174, 123,
 122, 116, 115, 114, 113, 106, 105,  98,  97,  92,
  91,  90,  87,  88,  62,  52,  43,  42,  41,  40,
  38,  86,  37,  82,  36, 160,  77,  84,  83,  78,
 183,  79,  80,  44, 125, 100, 154,  85,  81,  76,
  74,  72,  70,   3,   2,   1,   0,  45,  46,  47,
  48,  49,  51,   0,   0,  54,   0,   0,  58,  59,
  60,  61,   0,  63,   0,  65,  66,   0,   0,  69,
   0,   0,   0,   0,   0,   0,   0, 131, 132, 133,
   0,   0,   0, 135, 136, 137,   0,   0,   0,   0,
 139,   0, 141, 142,   0,   0,   0,   0,   0,   0,
 149, 150, 151, 152,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0, 145, 146,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0, 159,
 158,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0, 193,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0, 185 };
short yypact[]={

-1000,-256,-1000,-1000,-125,-127,-129,-155,-130,-131,
-132,-133,-1000,-1000,-1000,-1000,-1000,-156,-1000,-134,
-157,-1000,-195,-158,-1000,-1000,-1000,-1000,-135,-1000,
-159,-1000,-1000,-160,-161,-1000,-238,-239,-253,-1000,
-111,-118,-126,-257,-138,-139,-140,-197,-199,-141,
-1000,-142,-255,-1000,-201,-1000,-240,-1000,-143,-144,
-162,-163,-304,-145,-1000,-146,-147,-1000,-1000,-148,
-165,-1000,-167,-1000,-168,-215,-169,-149,-150,-1000,
-267,-171,-1000,-1000,-1000,-172,-1000,-1000,-1000,-203,
-255,-255,-255,-1000,-241,-1000,-255,-255,-255,-173,
-1000,-1000,-1000,-255,-174,-255,-255,-1000,-1000,-205,
-1000,-1000,-207,-255,-255,-255,-255,-1000,-1000,-1000,
-258,-1000,-164,-164,-262,-1000,-1000,-1000,-1000,-1000,
-121,-176,-177,-178,-179,-180,-181,-182,-1000,-183,
-1000,-184,-185,-1000,-251,-209,-151,-1000,-263,-186,
-187,-188,-189,-232,-1000,-1000,-1000,-1000,-1000,-1000,
-152,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,
-1000,-190,-1000,-264,-264,-191,-1000,-1000,-1000,-1000,
-265,-266,-1000,-192,-1000,-153,-1000,-234,-193,-1000,
-255,-268,-1000,-194,-235,-1000,-269,-236,-270,-1000 };
short yypgo[]={

   0, 165, 164, 163, 162, 161, 160, 159, 158, 157,
 153, 106, 150, 154, 156, 155 };
short yyr1[]={

   0,   1,   1,   1,   2,   2,   2,   2,   2,   2,
   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
   2,   2,   2,   2,   2,   2,   2,   3,   4,   5,
   5,  13,   6,  14,  14,  14,   7,   7,   7,   7,
   7,   7,   7,   7,   8,   8,   8,   9,   9,   9,
  11,  15,  12,  10 };
short yyr2[]={

   0,   0,   2,   2,   4,   4,   4,   2,   4,   4,
   4,   4,   8,   5,   5,   5,   3,   5,   3,   5,
   5,   2,   5,   4,   2,   3,   5,   2,   4,   2,
   5,   5,   3,   3,   4,   6,   5,   7,   9,   4,
   6,   5,   2,   5,   5,   2,   2,   5,   1,   0,
   1,   1,  11,   1,   1,   1,   1,   3,   1,   3,
   1,   1,   3,   2,   1,   1,   1,   1,   1,   1,
   1,   1,   1,   0 };
short yychk[]={

-1000,  -1,  -2,  -3, 274, 275, 279, 280, 281, 282,
 283, 294, 284, 285, 286, 302, 301, 304, 299, 297,
 298, 300, 305, 306, 307, 308, 309, 310, 303, 311,
 313, 314, 315, 278, 256, 296, 269, 269, 269, 270,
 269, 269, 269, 269, -10, -10, -10, -10, -10, -10,
 270, -10, 269, 270, -10, 270, 269, 270, -10, -10,
 -10, -10, 269, -10, 270, -10, -10, 270, 270, -10,
  -4, 272,  -5, 272,  -6, 273,  -7, 257, 260, 262,
 263,  -8, 261, 266, 265,  -9, 267, 258, 259, 273,
 269, 269, 269, 270, 269, 270, 269, 269, 269, -11,
 -15, 272, 270, 269, 272, 269, 269, 270, 270, 305,
 316, 318, 317, 269, 269, 269, 269, 270, 270, 270,
 271, 270, 269, 269, 269, -13, 273, 270, 270, 270,
 269, -11, -11, -11, 272, -11, -11, -11, 270, -11,
 270, -11, -11, 270, 269, -10, -10, 270, 269, -11,
 -11, -11, -11, 273, -14, 264, 268, 259, -14, -13,
 266, 270, 270, 270, 270, 270, 270, 270, 270, 270,
 270, 272, 270, 269, 269, 273, 270, 270, 270, 270,
 271, 269, 270, -12, 273, -12, 270, 273, 273, 270,
 269, 271, 270, -11, 273, 270, 271, 273, 271, 273 };
short yydef[]={

   1,  -2,   2,   3,   0,   0,   0,   0,   0,   0,
   0,   0,  73,  73,  73,  73,  73,  73,  73,   0,
   0,  73,   0,   0,  73,  73,  73,  73,   0,  73,
   0,  73,  73,   0,   0,  73,   0,  49,   0,   7,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  21,   0,   0,  24,   0,  27,   0,  29,   0,   0,
   0,   0,   0,   0,  42,   0,   0,  45,  46,   0,
   0,  48,   0,  50,   0,   0,   0,  56,  58,  60,
  61,   0,  64,  65,  66,   0,  67,  68,  69,   0,
   0,   0,   0,  16,   0,  18,   0,   0,   0,   0,
  70,  71,  25,   0,   0,   0,   0,  32,  33,   0,
  73,  73,   0,   0,   0,   0,   0,   4,   5,   6,
   0,   8,   0,   0,   0,  63,  51,   9,  10,  11,
   0,   0,   0,   0,   0,   0,   0,   0,  23,   0,
  28,   0,   0,  34,   0,   0,   0,  39,   0,   0,
   0,   0,   0,   0,  57,  53,  54,  55,  59,  62,
   0,  13,  14,  15,  17,  19,  20,  22,  26,  30,
  31,   0,  36,   0,   0,   0,  41,  43,  44,  47,
   0,   0,  35,   0,  72,   0,  40,   0,   0,  37,
   0,   0,  12,   0,   0,  38,   0,   0,   0,  52 };
#ifdef lint
static char yaccpar_sccsid[] = "@(#)yaccpar	4.1	(Berkeley)	2/11/83";
#endif /* not lint */

#
# define YYFLAG -1000
# define YYERROR goto yyerrlab
# define YYACCEPT return(0)
# define YYABORT return(1)

/*	parser for yacc output	*/

#ifdef YYDEBUG
int yydebug = 0; /* 1 for debugging */
#endif
YYSTYPE yyv[YYMAXDEPTH]; /* where the values are stored */
int yychar = -1; /* current input token number */
int yynerrs = 0;  /* number of errors */
short yyerrflag = 0;  /* error recovery flag */

int
yyparse() {

	short yys[YYMAXDEPTH];
	short yyj, yym;
	register YYSTYPE *yypvt;
	register short yystate, *yyps, yyn;
	register YYSTYPE *yypv;
	register short *yyxi;

	yystate = 0;
	yychar = -1;
	yynerrs = 0;
	yyerrflag = 0;
	yyps = yys; yyps--;
	yypv = yyv; yypv--;

 yystack:    /* put a state and value onto the stack */

#ifdef YYDEBUG
	if( yydebug  ) printf( "state %d, char 0%o\n", yystate, yychar );
#endif
		if( ++yyps> &yys[YYMAXDEPTH] ) { yyerror( "yacc stack overflow" ); return(1); }
		*yyps = yystate;
		++yypv;
		*yypv = yyval;

 yynewstate:

	yyn = yypact[yystate];

	if( yyn<= YYFLAG ) goto yydefault; /* simple state */

	if( yychar<0 ) if( (yychar=yylex())<0 ) yychar=0;
	if( (yyn += yychar)<0 || yyn >= YYLAST ) goto yydefault;

	if( yychk[ yyn=yyact[ yyn ] ] == yychar ){ /* valid shift */
		yychar = -1;
		yyval = yylval;
		yystate = yyn;
		if( yyerrflag > 0 ) --yyerrflag;
		goto yystack;
		}

 yydefault:
	/* default state action */

	if( (yyn=yydef[yystate]) == -2 ) {
		if( yychar<0 ) if( (yychar=yylex())<0 ) yychar = 0;
		/* look through exception table */

		for( yyxi=yyexca; (*yyxi!= (-1)) || (yyxi[1]!=yystate) ; yyxi += 2 ) ; /* VOID */

		while( *(yyxi+=2) >= 0 ){
			if( *yyxi == yychar ) break;
			}
		if( (yyn = yyxi[1]) < 0 ) return(0);   /* accept */
		}

	if( yyn == 0 ){ /* error */
		/* error ... attempt to resume parsing */

		switch( yyerrflag ){

		case 0:   /* brand new error */

			yyerror( "syntax error" );
/*		yyerrlab: */
			++yynerrs;

		case 1:
		case 2: /* incompletely recovered error ... try again */

			yyerrflag = 3;

			/* find a state where "error" is a legal shift action */

			while ( yyps >= yys ) {
			   yyn = yypact[*yyps] + YYERRCODE;
			   if( yyn>= 0 && yyn < YYLAST && yychk[yyact[yyn]] == YYERRCODE ){
			      yystate = yyact[yyn];  /* simulate a shift of "error" */
			      goto yystack;
			      }
			   yyn = yypact[*yyps];

			   /* the current yyps has no shift onn "error", pop stack */

#ifdef YYDEBUG
			   if( yydebug ) printf( "error recovery pops state %d, uncovers %d\n", *yyps, yyps[-1] );
#endif
			   --yyps;
			   --yypv;
			   }

			/* there is no state on the stack with an error shift ... abort */

	yyabort:
			return(1);


		case 3:  /* no shift yet; clobber input char */

#ifdef YYDEBUG
			if( yydebug ) printf( "error recovery discards char %d\n", yychar );
#endif

			if( yychar == 0 ) goto yyabort; /* don't discard EOF, quit */
			yychar = -1;
			goto yynewstate;   /* try again in the same state */

			}

		}

	/* reduction by production yyn */

#ifdef YYDEBUG
		if( yydebug ) printf("reduce %d\n",yyn);
#endif
		yyps -= yyr2[yyn];
		yypvt = yypv;
		yypv -= yyr2[yyn];
		yyval = yypv[1];
		yym=yyn;
			/* consult goto table to find next state */
		yyn = yyr1[yyn];
		yyj = yypgo[yyn] + *yyps + 1;
		if( yyj>=YYLAST || yychk[ yystate = yyact[yyj] ] != -yyn ) yystate = yyact[yypgo[yyn]];
		switch(yym){
			
case 2:
/* # line 99 "ftpcmd.y" */
 {
			fromname = (char *) 0;
		} break;
case 4:
/* # line 106 "ftpcmd.y" */
 {
			user((char *) yypvt[-1]);
			free((char *) yypvt[-1]);
		} break;
case 5:
/* # line 111 "ftpcmd.y" */
 {
			pass((char *) yypvt[-1]);
			free((char *) yypvt[-1]);
		} break;
case 6:
/* # line 116 "ftpcmd.y" */
 {
			usedefault = 0;
			if (pdata >= 0) {
				(void) close(pdata);
				pdata = -1;
			}
			reply(200, "PORT command successful.");
		} break;
case 7:
/* # line 125 "ftpcmd.y" */
 {
			passive();
		} break;
case 8:
/* # line 129 "ftpcmd.y" */
 {
			switch (cmd_type) {

			case TYPE_A:
				if (cmd_form == FORM_N) {
					reply(200, "Type set to A.");
					type = cmd_type;
					form = cmd_form;
				} else
					reply(504, "Form must be N.");
				break;

			case TYPE_E:
				reply(504, "Type E not implemented.");
				break;

			case TYPE_I:
				reply(200, "Type set to I.");
				type = cmd_type;
				break;

			case TYPE_L:
#if NBBY == 8
				if (cmd_bytesz == 8) {
					reply(200,
					    "Type set to L (byte size 8).");
					type = cmd_type;
				} else
					reply(504, "Byte size must be 8.");
#else /* NBBY == 8 */
				UNIMPLEMENTED for NBBY != 8
#endif /* NBBY == 8 */
			}
		} break;
case 9:
/* # line 164 "ftpcmd.y" */
 {
			switch (yypvt[-1]) {

			case STRU_F:
				reply(200, "STRU F ok.");
				break;

			default:
				reply(504, "Unimplemented STRU type.");
			}
		} break;
case 10:
/* # line 176 "ftpcmd.y" */
 {
			switch (yypvt[-1]) {

			case MODE_S:
				reply(200, "MODE S ok.");
				break;

			default:
				reply(502, "Unimplemented MODE type.");
			}
		} break;
case 11:
/* # line 188 "ftpcmd.y" */
 {
			reply(202, "ALLO command ignored.");
		} break;
case 12:
/* # line 192 "ftpcmd.y" */
 {
			reply(202, "ALLO command ignored.");
		} break;
case 13:
/* # line 196 "ftpcmd.y" */
 {
			if (yypvt[-3] && yypvt[-1] != NULL)
				retrieve((char *) 0, (char *) yypvt[-1]);
			if (yypvt[-1] != NULL)
				free((char *) yypvt[-1]);
		} break;
case 14:
/* # line 203 "ftpcmd.y" */
 {
			if (yypvt[-3] && yypvt[-1] != NULL)
				store((char *) yypvt[-1], "w", 0);
			if (yypvt[-1] != NULL)
				free((char *) yypvt[-1]);
		} break;
case 15:
/* # line 210 "ftpcmd.y" */
 {
			if (yypvt[-3] && yypvt[-1] != NULL)
				store((char *) yypvt[-1], "a", 0);
			if (yypvt[-1] != NULL)
				free((char *) yypvt[-1]);
		} break;
case 16:
/* # line 217 "ftpcmd.y" */
 {
			if (yypvt[-1])
				send_file_list(".");
		} break;
case 17:
/* # line 222 "ftpcmd.y" */
 {
			if (yypvt[-3] && yypvt[-1] != NULL) 
				send_file_list((char *) yypvt[-1]);
			if (yypvt[-1] != NULL)
				free((char *) yypvt[-1]);
		} break;
case 18:
/* # line 229 "ftpcmd.y" */
 {
			if (yypvt[-1])
				retrieve("/helios/bin/ls -l", "");
		} break;
case 19:
/* # line 234 "ftpcmd.y" */
 {
			if (yypvt[-3] && yypvt[-1] != NULL)
				retrieve("/helios/bin/ls -l %s", (char *) yypvt[-1]);
			if (yypvt[-1] != NULL)
				free((char *) yypvt[-1]);
		} break;
case 20:
/* # line 241 "ftpcmd.y" */
 {
			if (yypvt[-3] && yypvt[-1] != NULL)
				statfilecmd((char *) yypvt[-1]);
			if (yypvt[-1] != NULL)
				free((char *) yypvt[-1]);
		} break;
case 21:
/* # line 248 "ftpcmd.y" */
 {
			statcmd();
		} break;
case 22:
/* # line 252 "ftpcmd.y" */
 {
			if (yypvt[-3] && yypvt[-1] != NULL)
				mydelete((char *) yypvt[-1]);
			if (yypvt[-1] != NULL)
				free((char *) yypvt[-1]);
		} break;
case 23:
/* # line 259 "ftpcmd.y" */
 {
			if (fromname) {
				renamecmd(fromname, (char *) yypvt[-1]);
				free(fromname);
				fromname = (char *) 0;
			} else {
				reply(503, "Bad sequence of commands.");
			}
			free((char *) yypvt[-1]);
		} break;
case 24:
/* # line 270 "ftpcmd.y" */
 {
			reply(225, "ABOR command successful.");
		} break;
case 25:
/* # line 274 "ftpcmd.y" */
 {
			if (yypvt[-1])
				cwd(pw->pw_dir);
		} break;
case 26:
/* # line 279 "ftpcmd.y" */
 {
			if (yypvt[-3] && yypvt[-1] != NULL)
				cwd((char *) yypvt[-1]);
			if (yypvt[-1] != NULL)
				free((char *) yypvt[-1]);
		} break;
case 27:
/* # line 286 "ftpcmd.y" */
 {
			help(cmdtab, (char *) 0);
		} break;
case 28:
/* # line 290 "ftpcmd.y" */
 {
			register char *cp = (char *)yypvt[-1];

			if (strncasecmp(cp, "SITE", 4) == 0) {
				cp = (char *)yypvt[-1] + 4;
				if (*cp == ' ')
					cp++;
				if (*cp)
					help(sitetab, cp);
				else
					help(sitetab, (char *) 0);
			} else
				help(cmdtab, (char *) yypvt[-1]);
		} break;
case 29:
/* # line 305 "ftpcmd.y" */
 {
			reply(200, "NOOP command successful.");
		} break;
case 30:
/* # line 309 "ftpcmd.y" */
 {
			if (yypvt[-3] && yypvt[-1] != NULL)
				makedir((char *) yypvt[-1]);
			if (yypvt[-1] != NULL)
				free((char *) yypvt[-1]);
		} break;
case 31:
/* # line 316 "ftpcmd.y" */
 {
			if (yypvt[-3] && yypvt[-1] != NULL)
				removedir((char *) yypvt[-1]);
			if (yypvt[-1] != NULL)
				free((char *) yypvt[-1]);
		} break;
case 32:
/* # line 323 "ftpcmd.y" */
 {
			if (yypvt[-1])
				pwd();
		} break;
case 33:
/* # line 328 "ftpcmd.y" */
 {
			if (yypvt[-1])
				cwd("..");
		} break;
case 34:
/* # line 333 "ftpcmd.y" */
 {
			help(sitetab, (char *) 0);
		} break;
case 35:
/* # line 337 "ftpcmd.y" */
 {
			help(sitetab, (char *) yypvt[-1]);
		} break;
case 36:
/* # line 341 "ftpcmd.y" */
 {
			int oldmask;

			if (yypvt[-1]) {
				oldmask = umask(0);
				(void) umask(oldmask);
				reply(200, "Current UMASK is %03o", oldmask);
			}
		} break;
case 37:
/* # line 351 "ftpcmd.y" */
 {
			int oldmask;

			if (yypvt[-3]) {
				if ((yypvt[-1] == -1) || (yypvt[-1] > 0777)) {
					reply(501, "Bad UMASK value");
				} else {
					oldmask = umask(yypvt[-1]);
					reply(200,
					    "UMASK set to %03o (was %03o)",
					    yypvt[-1], oldmask);
				}
			}
		} break;
case 38:
/* # line 366 "ftpcmd.y" */
 {
			if (yypvt[-5] && (yypvt[-1] != NULL)) {
				if (yypvt[-3] > 0777)
					reply(501,
				"CHMOD: Mode value must be between 0 and 0777");
				else if (chmod((char *) yypvt[-1], yypvt[-3]) < 0)
					perror_reply(550, (char *) yypvt[-1]);
				else
					reply(200, "CHMOD command successful.");
			}
			if (yypvt[-1] != NULL)
				free((char *) yypvt[-1]);
		} break;
case 39:
/* # line 380 "ftpcmd.y" */
 {
			reply(200,
			    "Current IDLE time limit is %d seconds; max %d",
				timeout, maxtimeout);
		} break;
case 40:
/* # line 386 "ftpcmd.y" */
 {
			if (yypvt[-1] < 30 || yypvt[-1] > maxtimeout) {
				reply(501,
			"Maximum IDLE time must be between 30 and %d seconds",
				    maxtimeout);
			} else {
				timeout = yypvt[-1];
				(void) alarm((unsigned) timeout);
				reply(200,
				    "Maximum IDLE time set to %d seconds",
				    timeout);
			}
		} break;
case 41:
/* # line 400 "ftpcmd.y" */
 {
			if (yypvt[-3] && yypvt[-1] != NULL)
				store((char *) yypvt[-1], "w", 1);
			if (yypvt[-1] != NULL)
				free((char *) yypvt[-1]);
		} break;
case 42:
/* # line 407 "ftpcmd.y" */
 {
#ifdef unix
#ifdef BSD
			reply(215, "UNIX Type: L%d Version: BSD-%d",
				NBBY, BSD);
#else /* BSD */
			reply(215, "UNIX Type: L%d", NBBY);
#endif /* BSD */
#else /* unix */
			reply(215, "UNKNOWN Type: L%d", NBBY);
#endif /* unix */
		} break;
case 43:
/* # line 428 "ftpcmd.y" */
 {
			if (yypvt[-3] && yypvt[-1] != NULL)
				sizecmd((char *) yypvt[-1]);
			if (yypvt[-1] != NULL)
				free((char *) yypvt[-1]);
		} break;
case 44:
/* # line 445 "ftpcmd.y" */
 {
			if (yypvt[-3] && yypvt[-1] != NULL) {
				struct stat stbuf;
				if (stat((char *) yypvt[-1], &stbuf) < 0)
					perror_reply(550, (char *) yypvt[-1]);
				else if ((stbuf.st_mode&S_IFMT) != S_IFREG) {
					reply(550, "%s: not a plain file.",
						(char *) yypvt[-1]);
				} else {
					register struct tm *t;

					t = gmtime(&stbuf.st_mtime);
					reply(213,
					    "19%02d%02d%02d%02d%02d%02d",
					    t->tm_year, t->tm_mon+1, t->tm_mday,
					    t->tm_hour, t->tm_min, t->tm_sec);
				}
			}
			if (yypvt[-1] != NULL)
				free((char *) yypvt[-1]);
		} break;
case 45:
/* # line 467 "ftpcmd.y" */
 {
			reply(221, "Goodbye.");
			dologout(0);
		} break;
case 46:
/* # line 472 "ftpcmd.y" */
 {
			yyerrok;
		} break;
case 47:
/* # line 477 "ftpcmd.y" */
 {
			if (yypvt[-3] && yypvt[-1]) {
				fromname = renamefrom((char *) yypvt[-1]);
				if (fromname == (char *) 0 && yypvt[-1]) {
					free((char *) yypvt[-1]);
				}
			}
		} break;
case 49:
/* # line 493 "ftpcmd.y" */
 {
			*(char **)&(yyval) = "";
		} break;
case 52:
/* # line 504 "ftpcmd.y" */
 {
			register char *a, *p;

			a = (char *)&data_dest.sin_addr;
			a[0] = yypvt[-10]; a[1] = yypvt[-8]; a[2] = yypvt[-6]; a[3] = yypvt[-4];
			p = (char *)&data_dest.sin_port;
			p[0] = yypvt[-2]; p[1] = yypvt[-0];
			data_dest.sin_family = AF_INET;
		} break;
case 53:
/* # line 516 "ftpcmd.y" */
 {
		yyval = FORM_N;
	} break;
case 54:
/* # line 520 "ftpcmd.y" */
 {
		yyval = FORM_T;
	} break;
case 55:
/* # line 524 "ftpcmd.y" */
 {
		yyval = FORM_C;
	} break;
case 56:
/* # line 530 "ftpcmd.y" */
 {
		cmd_type = TYPE_A;
		cmd_form = FORM_N;
	} break;
case 57:
/* # line 535 "ftpcmd.y" */
 {
		cmd_type = TYPE_A;
		cmd_form = yypvt[-0];
	} break;
case 58:
/* # line 540 "ftpcmd.y" */
 {
		cmd_type = TYPE_E;
		cmd_form = FORM_N;
	} break;
case 59:
/* # line 545 "ftpcmd.y" */
 {
		cmd_type = TYPE_E;
		cmd_form = yypvt[-0];
	} break;
case 60:
/* # line 550 "ftpcmd.y" */
 {
		cmd_type = TYPE_I;
	} break;
case 61:
/* # line 554 "ftpcmd.y" */
 {
		cmd_type = TYPE_L;
		cmd_bytesz = NBBY;
	} break;
case 62:
/* # line 559 "ftpcmd.y" */
 {
		cmd_type = TYPE_L;
		cmd_bytesz = yypvt[-0];
	} break;
case 63:
/* # line 565 "ftpcmd.y" */
 {
		cmd_type = TYPE_L;
		cmd_bytesz = yypvt[-0];
	} break;
case 64:
/* # line 572 "ftpcmd.y" */
 {
		yyval = STRU_F;
	} break;
case 65:
/* # line 576 "ftpcmd.y" */
 {
		yyval = STRU_R;
	} break;
case 66:
/* # line 580 "ftpcmd.y" */
 {
		yyval = STRU_P;
	} break;
case 67:
/* # line 586 "ftpcmd.y" */
 {
		yyval = MODE_S;
	} break;
case 68:
/* # line 590 "ftpcmd.y" */
 {
		yyval = MODE_B;
	} break;
case 69:
/* # line 594 "ftpcmd.y" */
 {
		yyval = MODE_C;
	} break;
case 70:
/* # line 600 "ftpcmd.y" */
 {
		/*
		 * Problem: this production is used for all pathname
		 * processing, but only gives a 550 error reply.
		 * This is a valid reply in some cases but not in others.
		 */
		if (logged_in && yypvt[-0] && strncmp((char *) yypvt[-0], "~", 1) == 0) {
			*(char **)&(yyval) = *glob((char *) yypvt[-0]);
			if (globerr != NULL) {
				reply(550, globerr);
				yyval = NULL;
			}
			free((char *) yypvt[-0]);
		} else
			yyval = yypvt[-0];
	} break;
case 72:
/* # line 622 "ftpcmd.y" */
 {
		register int ret, dec, multby, digit;

		/*
		 * Convert a number that was read as decimal number
		 * to what it would be if it had been read as octal.
		 */
		dec = yypvt[-0];
		multby = 1;
		ret = 0;
		while (dec) {
			digit = dec%10;
			if (digit > 7) {
				ret = -1;
				break;
			}
			ret += digit * multby;
			multby *= 8;
			dec /= 10;
		}
		yyval = ret;
	} break;
case 73:
/* # line 647 "ftpcmd.y" */
 {
		if (logged_in)
			yyval = 1;
		else {
			reply(530, "Please login with USER and PASS.");
			yyval = 0;
		}
	} break;
		}
		goto yystack;  /* stack new state and value */

	}
