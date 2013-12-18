#include <ctype.h>
#include <curses.h>
#include "sokoban.h"

/* defining the types of move */
#define MOVE 		1
#define PUSH 		2
#define SAVE 		3
#define UNSAVE 		4
#define STOREMOVE 	5
#define STOREPUSH 	6

/* defines for control characters */
#define CNTL_L		'\014'
#define CNTL_K		'\013'
#define CNTL_H		'\010'
#define CNTL_J		'\012'
#define CNTL_R		'\022'
#define CNTL_U		'\025'

extern char  map[MAXROW+1][MAXCOL+1];
extern short rows, cols, level, moves, pushes, savepack, packets;
extern short scorelevel, scoremoves, scorepushes;
extern POS   ppos;

static POS   tpos1,		   /* testpos1: 1 pos. over/under/left/right */
             tpos2,		   /* testpos2: 2 pos.  "                    */
             lastppos,		   /* the last player position (for undo)    */
             lasttpos1, lasttpos2; /* last test positions (for undo)         */
static char lppc, ltp1c, ltp2c;    /* the char for the above pos. (for undo) */
static short action, lastaction;

/** For the temporary save **/
static char  tmp_map[MAXROW+1][MAXCOL+1];
static short tmp_pushes, tmp_moves, tmp_savepack;
static POS   tmp_ppos;

short play() {

   short c;
   short ret;
   short undolock = 1;		/* locked for undo */

   showscreen();
   tmpsave();
   ret = 0;
   while( ret == 0) {
      switch( (c = getch())) {
	 case 'S':    /* show Highscores				*/
                      clear();
                      refresh();
                      showscore();
                      getch();
                      clear();
                      showscreen();
	              break;
	 case 'Q':    /* quit the game 					*/
	              ret = E_ENDGAME; 
	              break;
         case 'r':    restoregame();
                      showscreen();
                      break;
         case 's':    savegame();
                      showscreen();
                      break;
	 case 'q':    /* save the games					*/
		      if( (ret = savegame()) == 0)
			 ret = E_SAVED;
		      break;
	 case '?':    /* show the help file				*/
		      showhelp();
		      showscreen();
		      break;
	 case CNTL_R: /* refresh the screen 				*/
		      clear();
		      showscreen();
		      break;
	 case 'c':    /* temporary save					*/
		      tmpsave();
		      break;
	 case CNTL_U: /* reset to temporary save 			*/
		      tmpreset();
		      undolock = 1;
		      showscreen();
		      break;
	 case 'U':    /* undo this level 				*/
		      moves = pushes = 0;
		      if( (ret = readscreen()) == 0) {
		         showscreen();
			 undolock = 1;
		      }
		      break;
	 case 'u':    /* undo last move 				*/
		      if( ! undolock) {
		         undomove();
		         undolock = 1;
		      }
		      break;
	 case 'k':    /* up 						*/
	 case 'K':    /* run up 					*/
	 case CNTL_K: /* run up, stop before object 			*/
	 case 'j':    /* down 						*/
	 case 'J':    /* run down 					*/
	 case CNTL_J: /* run down, stop before object 			*/
	 case 'l':    /* right 						*/
	 case 'L':    /* run right 					*/
	 case CNTL_L: /* run right, stop before object 			*/
	 case 'h':    /* left 						*/
	 case 'H':    /* run left 					*/
	 case CNTL_H: /* run left, stop before object 			*/
		      do {
		         if( (action = testmove( c)) != 0) {
			    lastaction = action;
		            lastppos.x = ppos.x; lastppos.y = ppos.y;
		            lppc = map[ppos.x][ppos.y];
		            lasttpos1.x = tpos1.x; lasttpos1.y = tpos1.y; 
		            ltp1c = map[tpos1.x][tpos1.y];
		            lasttpos2.x = tpos2.x; lasttpos2.y = tpos2.y; 
		            ltp2c = map[tpos2.x][tpos2.y];
		            domove( lastaction); 
		            undolock = 0;
		         }
		      } while( (action != 0) && (! islower( c))
			      && (packets != savepack));
		      break;
	 default:     helpmessage(); break;
      }
      if( (ret == 0) && (packets == savepack)) {
	 scorelevel = level;
	 scoremoves = moves;
	 scorepushes = pushes;
	 break;
      }
   }
   return( ret);
}

testmove( action)
register short action;
{
   register short ret;
   register char  tc;
   register short stop_at_object;

   if( (stop_at_object = iscntrl( action))) action = action + 'A' - 1;
   action = (isupper( action)) ? tolower( action) : action;
   if( (action == 'k') || (action == 'j')) {
      tpos1.x = (action == 'k') ? ppos.x-1 : ppos.x+1;
      tpos2.x = (action == 'k') ? ppos.x-2 : ppos.x+2;
      tpos1.y = tpos2.y = ppos.y;
   }
   else {
      tpos1.y = (action == 'h') ? ppos.y-1 : ppos.y+1;
      tpos2.y = (action == 'h') ? ppos.y-2 : ppos.y+2;
      tpos1.x = tpos2.x = ppos.x;
   }
   tc = map[tpos1.x][tpos1.y];
   if( (tc == packet.obj_intern) || (tc == save.obj_intern)) {
      if( ! stop_at_object) {
         if( map[tpos2.x][tpos2.y] == ground.obj_intern)
            ret = (tc == save.obj_intern) ? UNSAVE : PUSH;
         else if( map[tpos2.x][tpos2.y] == store.obj_intern)
            ret = (tc == save.obj_intern) ? STOREPUSH : SAVE;
         else ret = 0;
      }
      else ret = 0;
   }
   else if( tc == ground.obj_intern)
      ret = MOVE;
   else if( tc == store.obj_intern)
      ret = STOREMOVE;
   else ret = 0;
   return( ret);
}

domove( moveaction) 
register short moveaction;
{
   map[ppos.x][ppos.y] = (map[ppos.x][ppos.y] == player.obj_intern) 
			       ? ground.obj_intern 
			       : store.obj_intern;
   switch( moveaction) {
      case MOVE:      map[tpos1.x][tpos1.y] = player.obj_intern; 	break;
      case STOREMOVE: map[tpos1.x][tpos1.y] = playerstore.obj_intern; 	break;
      case PUSH:      map[tpos2.x][tpos2.y] = map[tpos1.x][tpos1.y];
		      map[tpos1.x][tpos1.y] = player.obj_intern;	
		      pushes++;						break;
      case UNSAVE:    map[tpos2.x][tpos2.y] = packet.obj_intern;
		      map[tpos1.x][tpos1.y] = playerstore.obj_intern;		
		      pushes++; savepack--;			 	break;
      case SAVE:      map[tpos2.x][tpos2.y] = save.obj_intern;
		      map[tpos1.x][tpos1.y] = player.obj_intern;
                      addch(7);	
                      refresh();
		      savepack++; pushes++;				break;
      case STOREPUSH: map[tpos2.x][tpos2.y] = save.obj_intern;
		      map[tpos1.x][tpos1.y] = playerstore.obj_intern;		
		      pushes++;						break;
   }
   moves++;
   dispmoves(); disppushes(); dispsave();
   mapchar( map[ppos.x][ppos.y], ppos.x, ppos.y);
   mapchar( map[tpos1.x][tpos1.y], tpos1.x, tpos1.y);
   mapchar( map[tpos2.x][tpos2.y], tpos2.x, tpos2.y);
   move( MAXROW+1, 0);
   refresh();
   ppos.x = tpos1.x; ppos.y = tpos1.y;
}

undomove() {

   map[lastppos.x][lastppos.y] = lppc;
   map[lasttpos1.x][lasttpos1.y] = ltp1c;
   map[lasttpos2.x][lasttpos2.y] = ltp2c;
   ppos.x = lastppos.x; ppos.y = lastppos.y;
   switch( lastaction) {
      case MOVE:      moves--;				break;
      case STOREMOVE: moves--;				break;
      case PUSH:      moves--; pushes--;		break;
      case UNSAVE:    moves--; pushes--; savepack++;	break;
      case SAVE:      moves--; pushes--; savepack--;	break;
      case STOREPUSH: moves--; pushes--;		break;
   }
   dispmoves(); disppushes(); dispsave();
   mapchar( map[ppos.x][ppos.y], ppos.x, ppos.y);
   mapchar( map[lasttpos1.x][lasttpos1.y], lasttpos1.x, lasttpos1.y);
   mapchar( map[lasttpos2.x][lasttpos2.y], lasttpos2.x, lasttpos2.y);
   move( MAXROW+1, 0);
   refresh();
}

tmpsave() {

   register short i, j;

   for( i = 0; i < rows; i++) for( j = 0; j < cols; j++)
      tmp_map[i][j] = map[i][j];
   tmp_pushes = pushes;
   tmp_moves = moves;
   tmp_savepack = savepack;
   tmp_ppos.x = ppos.x; tmp_ppos.y = ppos.y;
}

tmpreset() {

   register short i, j;

   for( i = 0; i < rows; i++) for( j = 0; j < cols; j++)
      map[i][j] = tmp_map[i][j];
   pushes = tmp_pushes;
   moves = tmp_moves;
   savepack = tmp_savepack;
   ppos.x = tmp_ppos.x; ppos.y = tmp_ppos.y;
}
