#include <stdio.h>
#include <curses.h>
#include "sokoban.h"
#include <pwd.h>

#ifdef __MSDOS__
char * getlogin()		/* interactive login name */
{	static char loginname[MAXUSERNAME];
	fputs("\nName? ", stdout);
	fgets(loginname, MAXUSERNAME, stdin);
	loginname[strlen(loginname)-1] = 0;
	return(loginname);
}
#endif

extern char  *strrchr(), *getlogin(), *getpass();
extern short readscreen(), play(), outputscore(), getuserlevel(),
	     makenewscore(), restoregame(), score();

short	curses = 1;
short scoring = 1;
short level, packets, savepack, moves, pushes, rows, cols;
short scorelevel, scoremoves, scorepushes;
char  map[MAXROW+1][MAXCOL+1];
POS   ppos;
char  *username, *prgname;

static short optshowscore = 0, 
	     optmakescore = 0, 
             optrestore = 0, 
	     optlevel = 0; 
static short superuser = 0;

static short userlevel;

main( argc, argv) 
short argc; 
char *argv[];
{
   short ret, ret2;
	struct passwd * ppasswd;
	

   scorelevel = 0;
   moves = pushes = packets = savepack = 0;
   if( (prgname = strrchr( argv[0], '/')) == NULL)
      prgname = argv[0];
   else prgname++;
   if( (username = getlogin()) == NULL && (ppasswd = getpwuid( getuid() )) == NULL)
   {
      ret = E_NOUSER;
   }
   else {
   	if (username == NULL)
   		username = ppasswd->pw_name;
   		
      superuser = (strcmp( username, SUPERUSER) == 0);
      if( (ret = checkcmdline( argc, argv)) == 0) {
         if( optshowscore)
	    ret = outputscore();
         else if( optmakescore) {
	    if( superuser) {
	       if( (ret = getpassword()) == 0)
	          ret = makenewscore();
	    }
	    else ret = E_NOSUPER;
	 }
	 else if( optrestore) {
	    ret = restoregame();
	 }
         else if( (ret = getuserlevel( &userlevel)) == 0) {
            if( optlevel > 0) {
	       if( superuser) {
	          level = optlevel;
		  scoring = 0;
	       }
	       else if( userlevel < optlevel)
	          ret = E_LEVELTOOHIGH;
	       else level = optlevel;
	    }
	    else level = userlevel;
         }
      }
   }
   if( ret == 0)
      ret = gameloop();
   errmess( ret);
   if( scorelevel && scoring) {
      ret2 = score();
      errmess( ret2);
   }
   exit( ret);
}

checkcmdline( argc, argv) 
short argc; 
char *argv[];
{
   short ret = 0;

   if( argc > 1)
      if( (argc == 2) && (argv[1][0] == '-')) {
	 if( (argv[1][1] == 's') && (argv[1][2] == '\0'))
	    optshowscore = 1;
	 else if( (argv[1][1] == 'c') && (argv[1][2] == '\0'))
	    optmakescore = 1;
	 else if( (argv[1][1] == 'r') && (argv[1][2] == '\0'))
	    optrestore = 1;
	 else if( (optlevel = atoi( &(argv[1][1]))) == 0)
	    ret = E_USAGE;
      }
      else ret = E_USAGE;
   return( ret);
}

gameloop() {

   short ret = 0;

   initscr(); noecho(); crmode();
   if( ! optrestore) ret = readscreen();
   while( ret == 0) {
      if( (ret = play()) == 0) {
         level++;
         moves = pushes = packets = savepack = 0;
         ret = readscreen();
      }
   }
   clear(); refresh(); 
   nocrmode(); echo(); endwin();
   return( ret);
}

getpassword() {

   return( (strcmp(getpass("Password: "), PASSWORD) == 0) ? 0 : E_ILLPASSWORD);
}

char *message[] = {
   "illegal error number",
   "cannot open screen file",
   "more than one player position in screen file",
   "illegal char in screen file",
   "no player position in screenfile",
   "too much rows in screen file",
   "too much columns in screenfile",
   "quit the game",
   NULL,			/* errmessage deleted */
   "cannot get your username",
   "cannot open savefile",
   "error writing to savefile",
   "cannot stat savefile",
   "error reading savefile",
   "cannot restore, your savefile has been altered",
   "game saved",
   "too much users in score table",
   "cannot open score file",
   "error reading scorefile",
   "error writing scorefile",
   "illegal command line syntax",
   "illegal password",
   "level number too big in command line",
   "only superuser is allowed to make a new score table",
   "cannot find file to restore"
};

errmess( ret) 
register short ret;
{
   if( ret != E_ENDGAME) {
      fprintf( stderr, "%s: ", prgname);
      switch( ret) {
         case E_FOPENSCREEN: case E_PLAYPOS1:   case E_ILLCHAR: 
	 case E_PLAYPOS2:    case E_TOMUCHROWS: case E_TOMUCHCOLS: 
	 case E_ENDGAME:     case E_NOUSER:      
	 case E_FOPENSAVE:   case E_WRITESAVE:  case E_STATSAVE:    
	 case E_READSAVE:    case E_ALTERSAVE:  case E_SAVED:       
	 case E_TOMUCHSE:    case E_FOPENSCORE: case E_READSCORE: 
	 case E_WRITESCORE:  case E_USAGE:	case E_ILLPASSWORD:
	 case E_LEVELTOOHIGH: case E_NOSUPER:	case E_NOSAVEFILE:
			     fprintf( stderr, "%s\n", message[ret]);
                             break;
         default:            fprintf( stderr, "%s\n", message[0]);
                             break;
      }
      if( ret == E_USAGE) usage();
   }
}

static char *usagestr[] = {
   "           -c:    create new score table (superuser only)\n",
   "           -r:    restore saved game\n",
   "           -s:    show score table\n",
   "           -<nn>: play this level (<nn> must be greater 0)\n",
   NULL
};

usage() {

   register short i;

   fprintf( stderr, "Usage: %s [-{s|c|r|<nn>}]\n\n", prgname);
   for( i = 0; usagestr[i] != NULL; i++)
      fprintf( stderr, "%s", usagestr[i]);
}
