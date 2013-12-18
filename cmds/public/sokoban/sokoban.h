/***********************************************************************
   You may wish to alter the following directory paths
***********************************************************************/
/**/
/* SCREENPATH: the name of the directioy where the screen file are held */
/**/
#define SCREENPATH 	"/var1/gnumisc/games/sokoban-2.0/screens"

/**/
/* SAVEPATH: the name of the path where save files are held */
/*           Attention: Be sure that there are no other files with */
/*                      the name <username>.sav                    */
/**/
#define SAVEPATH	"/var1/gnumisc/games/sokoban-2.0/savegames"

/**/
/* LOCKPATH: temporary file which is created to ensure that no users */
/*           work with the scorefile at the same time                */
/**/
#define LOCKFILE	"/tmp/sok.lock"

/**/
/* SCOREFILE: the full pathname of the score file */
/**/
#define SCOREFILE	"/var1/gnumisc/games/sokoban-2.0/sok.score"

/**/
/* MAXUSERNAME: defines the maximum length of a system's user name */
/**/
#define MAXUSERNAME	10

/**/
/* MAXSCOREENTRIES: defines the maximum numner of entries in the scoretable */
/**/
#define MAXSCOREENTRIES	50

/**/
/* SUPERUSER: defines the name of the game superuser */
/**/
#define SUPERUSER "martyn"

/**/
/* PASSWORD: defines the password necessary for creating a new score file */
/**/
#define PASSWORD "bla"

/**/
/* OBJECT: this typedef is used for internal and external representation */
/*         of objects                                                    */
/**/
typedef struct {
   char obj_intern;	/* internal representation of the object */
   char obj_display;	/* display char for the object		 */
   short invers;	/* if set to 1 the object will be shown invers */
} OBJECT;

/**/
/* You can now alter the definitions below.
/* Attention: Do not alter `obj_intern'. This would cause an error */
/*            when reading the screenfiles                         */
/**/
static OBJECT 
   player = 	 { '@', '&', 0 },
   playerstore = { '+', '&', 1 },
   store = 	 { '.', 'O', 0 },
   packet = 	 { '$', '$', 0 },
   save = 	 { '*', '$', 1 },
   ground = 	 { ' ', ' ', 0 },
   wall = 	 { '#', '#', 1 };

/*************************************************************************
********************** DO NOT CHANGE BELOW THIS LINE *********************
*************************************************************************/
#define MAXROW		20
#define MAXCOL		40

typedef struct {
   short x, y;
} POS;

#define E_FOPENSCREEN	1
#define E_PLAYPOS1	2
#define E_ILLCHAR	3
#define E_PLAYPOS2	4
#define E_TOMUCHROWS	5
#define E_TOMUCHCOLS	6
#define E_ENDGAME	7
#define E_NOUSER	9
#define E_FOPENSAVE	10
#define E_WRITESAVE	11
#define E_STATSAVE	12
#define E_READSAVE	13
#define E_ALTERSAVE	14
#define E_SAVED		15
#define E_TOMUCHSE	16
#define E_FOPENSCORE	17
#define E_READSCORE	18
#define E_WRITESCORE	19
#define E_USAGE		20
#define E_ILLPASSWORD	21
#define E_LEVELTOOHIGH	22
#define E_NOSUPER	23
#define E_NOSAVEFILE	24
