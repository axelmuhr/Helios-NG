#include <stdio.h>
#include <signal.h>
#include "sokoban.h"

extern FILE *fopen();
extern char  *username;
extern short scorelevel, scoremoves, scorepushes;

static short scoreentries;
static struct {
   char user[MAXUSERNAME];
   short lv, mv, ps;
} scoretable[MAXSCOREENTRIES];

static FILE *scorefile;
static int sfdbn;

short outputscore() {

   short ret;

   while( creat( LOCKFILE, 0666) < 0);	/* lock the score file */
   if( (ret = readscore()) == 0)
      showscore();
   unlink( LOCKFILE);
   return( (ret == 0) ? E_ENDGAME : ret);
}

short makenewscore() {

   short ret = 0;

   while( creat( LOCKFILE, 0666) < 0) ;
   scoreentries = 0;
#ifdef __MSDOS__
   if( (scorefile = fopen( SCOREFILE, "wb")) == NULL)
      ret = E_FOPENSCORE;
   else {
	if (fwrite (&scoreentries, 1, sizeof( scoreentries), scorefile) != sizeof(scoreentries))
	    ret = E_WRITESCORE;
#else
   if( (scorefile = fopen( SCOREFILE, "w")) == NULL)
      ret = E_FOPENSCORE;
   else {
      sfdbn = fileno( scorefile);
      if( write( sfdbn, &scoreentries, sizeof(scoreentries)) != sizeof(scoreentries)) ret = E_WRITESCORE;
#endif
      fclose( scorefile);
   }
   unlink( LOCKFILE);
   return( (ret == 0) ? E_ENDGAME : ret);
}

short getuserlevel( lv)
short *lv;
{
   short ret = 0, pos;

   while( creat( LOCKFILE, 0666) < 0);
#ifdef __MSDOS__
   if( (scorefile = fopen( SCOREFILE, "rb")) == NULL)
#else
   if( (scorefile = fopen( SCOREFILE, "r")) == NULL)
#endif
      ret = E_FOPENSCORE;
   else {
      if( (ret = readscore()) == 0)
	 *lv = ( (pos = finduser()) > -1) ? scoretable[pos].lv+1 : 1;
   }
   unlink( LOCKFILE);
   return( ret);
}

short score() {
   
   short ret;

   while( creat( LOCKFILE, 0666) < 0);	/* lock the score file */
   if( (ret = readscore()) == 0)
      if( (ret = makescore()) == 0)
	 if( (ret = writescore()) == 0)
	    showscore();
   unlink( LOCKFILE);
   return( (ret == 0) ? E_ENDGAME : ret);
}

readscore() {

   short ret = 0;
   long tmp;

#ifdef __MSDOS__
   if( (scorefile = fopen( SCOREFILE, "rb")) == NULL)
      ret = E_FOPENSCORE;
   else {
      tmp = sizeof( scoreentries);
      if( fread( &scoreentries, 1, tmp, scorefile) != tmp) ret = E_READSCORE;
      else {
	 tmp = scoreentries * sizeof( scoretable[0]);
	 if( fread( scoretable, 1, tmp, scorefile) != tmp) ret = E_READSCORE;
      }
#else
   if( (scorefile = fopen( SCOREFILE, "r")) == NULL)
   {
   	fprintf( stderr, "failed to open score file\n" );
      ret = E_FOPENSCORE;
   }
   else {
      sfdbn = fileno( scorefile);
      if( read( sfdbn, &scoreentries, sizeof(scoreentries)) != sizeof(scoreentries))
      {
      	fprintf( stderr, "failed to read in scoreentries structure\n" );
      	ret = E_READSCORE;
      }
      else {
	 tmp = scoreentries * sizeof( scoretable[0]);
	 if( read( sfdbn, &(scoretable[0]), tmp) != tmp)
	 {
	 	fprintf( stderr, "failed to read in %d score table entries\n", scoreentries );
	 	ret = E_READSCORE;
	 }
      }
#endif
      fclose( scorefile);
   }
   return( ret);
}

makescore() {

   short ret = 0, pos, i, build = 1, insert;

   if( (pos = finduser()) > -1) {	/* user already in score file */
      insert =    (scorelevel > scoretable[pos].lv)
	       || ( (scorelevel == scoretable[pos].lv) &&
                    (scoremoves < scoretable[pos].mv)
		  )
	       || ( (scorelevel == scoretable[pos].lv) &&
		    (scoremoves == scoretable[pos].mv) &&
		    (scorepushes < scoretable[pos].ps)
		  );
      if( insert) { 			/* delete existing entry */
	 for( i = pos; i < scoreentries-1; i++)
	    cp_entry( i, i+1);
	 scoreentries--;
      }
      else build = 0;
   }
   else if( scoreentries == MAXSCOREENTRIES)
      ret = E_TOMUCHSE;
   if( (ret == 0) && build) {
      pos = findpos();			/* find the new score position */
      if( pos > -1) {			/* score table not empty */
	 for( i = scoreentries; i > pos; i--)
	    cp_entry( i, i-1);
      }
      else pos = scoreentries;

      strcpy( scoretable[pos].user, username);
      scoretable[pos].lv = scorelevel;
      scoretable[pos].mv = scoremoves;
      scoretable[pos].ps = scorepushes;
      scoreentries++;
   }
   return( ret);
}

finduser() {

   short i, found = 0;

   for( i = 0; (i < scoreentries) && (! found); i++)
      found = (strcmp( scoretable[i].user, username) == 0);
   return( (found) ? i-1 : -1);
}

findpos() {
 
   short i, found = 0;

   for( i = 0; (i < scoreentries) && (! found); i++)
      found =    (scorelevel > scoretable[i].lv)
	      || ( (scorelevel == scoretable[i].lv) &&
                   (scoremoves < scoretable[i].mv)
		 )
	      || ( (scorelevel == scoretable[i].lv) &&
		   (scoremoves == scoretable[i].mv) &&
		   (scorepushes < scoretable[i].ps)
		 );
   return( (found) ? i-1 : -1);
}

writescore() {

   short ret = 0;
   long tmp;

#ifdef __MSDOS__
   if( (scorefile = fopen( SCOREFILE, "wb")) == NULL)
      ret = E_FOPENSCORE;
   else {
      tmp = sizeof( scoreentries);
      if( fwrite( &scoreentries, 1, tmp, scorefile) != tmp) ret = E_WRITESCORE;
      else {
	 tmp = scoreentries * sizeof( scoretable[0]);
	 if( fwrite( scoretable, 1, tmp, scorefile) != tmp) ret = E_WRITESCORE;
      }
#else
   if( (scorefile = fopen( SCOREFILE, "w")) == NULL)
      ret = E_FOPENSCORE;
   else {
      sfdbn = fileno( scorefile);
      if( write( sfdbn, &scoreentries, sizeof(scoreentries)) != sizeof(scoreentries)) ret = E_WRITESCORE;
      else {
	 tmp = scoreentries * sizeof( scoretable[0]);
	 if( write( sfdbn, &(scoretable[0]), tmp) != tmp) ret = E_WRITESCORE;
      }
#endif
      fclose( scorefile);
   }
   return( ret);
}

showscore() {

   register short lastlv = 0, lastmv = 0, lastps = 0, i;

   fprintf( stdout, "Rank        User     Level     Moves    Pushes\n");
   fprintf( stdout, "==============================================\n");
   for( i = 0; i < scoreentries; i++) {
      if( (scoretable[i].lv == lastlv)&& 
	  (scoretable[i].mv == lastmv) && 
	  (scoretable[i].ps == lastps))
	 fprintf( stdout, "      ");
      else {
         lastlv = scoretable[i].lv;
         lastmv = scoretable[i].mv;
         lastps = scoretable[i].ps;
         fprintf( stdout, "%4d  ", i+1);
      }
      fprintf( stdout, "%10s  %8d  %8d  %8d\n", scoretable[i].user, 
		scoretable[i].lv, scoretable[i].mv, scoretable[i].ps);
   }
}

cp_entry( i1, i2)
register short i1, i2;
{
   strcpy( scoretable[i1].user, scoretable[i2].user);
   scoretable[i1].lv = scoretable[i2].lv;
   scoretable[i1].mv = scoretable[i2].mv;
   scoretable[i1].ps = scoretable[i2].ps;
}
