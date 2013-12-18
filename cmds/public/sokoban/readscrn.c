#include <stdio.h>
#include "sokoban.h"

extern char *malloc();
extern FILE *fopen();

extern short level, packets, savepack, rows, cols;
extern char  map[MAXROW+1][MAXCOL+1];
extern POS   ppos;

short readscreen() {

   FILE *screen;
   char *fnam;
   short j, c, ret = 0;

   fnam = malloc( strlen( SCREENPATH) + 12);
   sprintf( fnam, "%s/screen.%d", SCREENPATH, level);
   if( (screen = fopen( fnam, "r")) == NULL) 
      ret = E_FOPENSCREEN;
   else {
      packets = savepack = rows = j = cols  = 0;
      ppos.x = -1; ppos.y = -1;
      while( (ret == 0) && ((c = getc( screen)) != EOF)) {
         if( c == '\n') {
	    map[rows++][j] = '\0';
	    if( rows > MAXROW) 
	       ret = E_TOMUCHROWS;
	    else {
	       if( j > cols) cols = j;
	       j = 0;
	    }
	 }
	 else if( (c == player.obj_intern) || (c == playerstore.obj_intern)) {
	    if( ppos.x != -1) 
	       ret = E_PLAYPOS1;
	    else { 
	       ppos.x = rows; ppos.y = j;
	       map[rows][j++] = c;
	       if( j > MAXCOL) ret = E_TOMUCHCOLS;
	    }
	 }
	 else if( (c == save.obj_intern) || (c == packet.obj_intern) ||
		  (c == wall.obj_intern) || (c == store.obj_intern) ||
		  (c == ground.obj_intern)) {
	    if( c == save.obj_intern)   { savepack++; packets++; }
	    if( c == packet.obj_intern) packets++;
	    map[rows][j++] = c;
	    if( j > MAXCOL) ret = E_TOMUCHCOLS;
	 }
	 else ret = E_ILLCHAR;
      }
      fclose( screen);
      if( (ret == 0) && (ppos.x == -1)) ret = E_PLAYPOS2;
   }
   return( ret);
}
