/* Compile via: cc gnuchess.c -o gnuchess. For display version,
   add -DDISPLAY flag and -lcurses -ltermcap libraries to
   command-line. For SUN Chesstool version, add -DCHESSTOOL
   but don't include display options in last sentence.
   For faster version, add -O flag to any of these flags.
*/
/* This file contains code for CHESS.
   Copyright (C) 1986, 1987 Free Software Foundation, Inc.

This file is part of CHESS.

CHESS is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY.  No author or distributor
accepts responsibility to anyone for the consequences of using it
or for whether it serves any particular purpose or works at all,
unless he says so in writing.  Refer to the CHESS General Public
License for full details.

Everyone is granted permission to copy, modify and redistribute
CHESS, but only under the conditions described in the
CHESS General Public License.   A copy of this license is
supposed to have been given to you along with CHESS so you
can know your rights and responsibilities.  It should be in a
file named COPYING.  Among other things, the copyright notice
and this notice must be preserved on all copies.  */

/* Modified 12/18/87 by Bill Randle (billr@tekred.tek.com) to
   remove a redundant #include and #ifdef section.
*/

#include <stdio.h>
#include <ctype.h>
#include <signal.h>

#ifdef MSDOS
#include <dos.h>
#include <stdlib.h>
#include <time.h>
#include <alloc.h>
#define printz printf
#define scanz scanf
#define refresh();   
#define initscr();  
#define crmode();   
#define nocrmode();   
#define endwin();   
#define _stklen 8000
#define ttblsz 4096
#else
#include <sys/param.h>
#include <sys/times.h>
#define ttblsz 16384
#ifdef DISPLAY
#include <curses.h>
#define printz printw
#define scanz fflush(stdout), scanw
#else
#define printz printf
#define scanz fflush(stdout), scanf
#define refresh();   
#define initscr();  
#define crmode();   
#define nocrmode();   
#define endwin();   
#endif DISPLAY
#endif MSDOS
#ifndef HZ
#define HZ 60
#endif

#define neutral 0
#define white 1
#define black 2 
#define no_piece 0
#define pawn 1
#define knight 2
#define bishop 3
#define rook 4
#define queen 5
#define king 6
#define valueP 100
#define valueN 330
#define valueB 330
#define valueR 520
#define valueQ 980
#define valueK 999
#define ctlP 0x4000
#define ctlN 0x2800
#define ctlB 0x1800
#define ctlR 0x0400
#define ctlQ 0x0200
#define ctlK 0x0100
#define ctlBQ 0x1200
#define ctlRQ 0x0600
#define ctlNN 0x2000
#define px " PNBRQK"
#define qx " pnbrqk"
#define rx "12345678"
#define cx "abcdefgh"
#define check 0x0001
#define capture 0x0002
#define draw 0x0004
#define promote 0x0008
#define cstlmask 0x0010
#define epmask 0x0020
#define exact 0x0040
#define pwnthrt 0x0080
#define truescore 0x0001
#define lowerbound 0x0002
#define upperbound 0x0004
#define maxdepth 30
#define true 1
#define false 0
#define absv(x) ((x) < 0 ? -(x) : (x))
#define taxicab(a,b) (absv(col[a]-col[b]) + absv(row[a]-row[b]))

struct leaf
  {
    short f,t,score,reply;
    unsigned short flags;
  };
struct GameRec
  {
    unsigned short gmove;
    short score,depth,time,piece,color;
    long nodes;
  };
struct TimeControlRec
  {
    short moves[3];
    long clock[3];
  };
struct hashval
  {
    unsigned long bd;
    unsigned short key;
  };
struct hashentry
  {
    unsigned long hashbd;
    unsigned short reply,flags;
    short score,depth;
  };

char mvstr1[5],mvstr2[5];
struct leaf Tree[2000],*root;
short TrPnt[maxdepth],board[64],color[64];
short row[64],col[64],locn[8][8],Pindex[64],svalue[64];
short PieceList[3][16],PieceCnt[3],atak[3][64],PawnCnt[3][8];
short castld[3],kingmoved[3],mtl[3],pmtl[3],emtl[3],hung[3];
short mate,post,opponent,computer,Sdepth,Awindow,Bwindow,randflag;
long ResponseTime,ExtraTime,Level,et,et0,time0,cputimer,ft;
long NodeCnt,evrate,ETnodes,EvalNodes,HashCnt;
long OperatorTime;
short quit,reverse,bothsides,hashflag,InChk,player,force,easy,beep;
short wking,bking,TOsquare,timeout,Zscore,zwndw,cptrval,prune,slk;
short HasPawn[3],HasKnight[3],HasBishop[3],HasRook[3],HasQueen[3];
short ChkFlag[maxdepth],CptrFlag[maxdepth],PawnThreat[maxdepth];
short Pscore[maxdepth],Tscore[maxdepth],Threat[maxdepth];
struct GameRec GameList[240];
short GameCnt,Game50,epsquare,lpost;
short BookSize,BookDepth,MaxSearchDepth;
struct TimeControlRec TimeControl;
short TCflag,TCmoves,mycnt1,mycnt2;
#ifdef MSDOS
unsigned short Book[80][24];
#else
unsigned short Book[250][50];
struct tms tmbuf1,tmbuf2;
#endif
short otherside[3]={0,2,1};
short map[64]=
   {26,27,28,29,30,31,32,33,38,39,40,41,42,43,44,45,
    50,51,52,53,54,55,56,57,62,63,64,65,66,67,68,69,
    74,75,76,77,78,79,80,81,86,87,88,89,90,91,92,93,
    98,99,100,101,102,103,104,105,110,111,112,113,114,115,116,117};
short unmap[144]=
   {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,0,1,2,3,4,5,6,7,-1,-1,-1,-1,8,9,10,11,12,13,14,15,-1,-1,
    -1,-1,16,17,18,19,20,21,22,23,-1,-1,-1,-1,24,25,26,27,28,29,30,31,-1,-1,
    -1,-1,32,33,34,35,36,37,38,39,-1,-1,-1,-1,40,41,42,43,44,45,46,47,-1,-1,
    -1,-1,48,49,50,51,52,53,54,55,-1,-1,-1,-1,56,57,58,59,60,61,62,63,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
short Stboard[64]=
   {rook,knight,bishop,queen,king,bishop,knight,rook,
    pawn,pawn,pawn,pawn,pawn,pawn,pawn,pawn,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    pawn,pawn,pawn,pawn,pawn,pawn,pawn,pawn,
    rook,knight,bishop,queen,king,bishop,knight,rook};
short Stcolor[64]=
   {white,white,white,white,white,white,white,white,
    white,white,white,white,white,white,white,white,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    black,black,black,black,black,black,black,black,
    black,black,black,black,black,black,black,black};
short sweep[7]= {false,false,false,true,true,true,false};
short sweep1[7]= {false,false,false,false,true,true,false};
short sweep2[7]= {false,false,false,true,false,true,false};
short Dpwn[3]={0,4,6};
short Dstart[7]={6,4,8,4,0,0,0};
short Dstop[7]={7,5,15,7,3,7,7};
short Dir[16]={1,12,-1,-12,11,13,-11,-13,10,-10,14,-14,23,-23,25,-25};
short rank7[3] = {0,6,1};
unsigned short killr0[maxdepth],killr1[maxdepth],killr2[maxdepth];
unsigned short killr3[maxdepth],Qkillr[maxdepth],PrVar[maxdepth];
unsigned short PV,hint,Swag0,Swag1,Swag2,Swag3,Swag4,Swag5;
unsigned short hashkey;
unsigned long hashbd;
struct hashval hashcode[3][7][64];
#ifdef MSDOS
struct hashentry far *ttable,*ptbl;
#else
struct hashentry *ttable,*ptbl;
int TerminateSearch(),Die();
#endif MSDOS
unsigned char history[2][64][64];

short Mwpawn[64],Mbpawn[64],Mknight[3][64],Mbishop[3][64],Mking[3][64];
short WeakSq[3][64],Kfield[3][64],contempt[3];
short value[7]={0,valueP,valueN,valueB,valueR,valueQ,valueK};
short control[7]={0,ctlP,ctlN,ctlB,ctlR,ctlQ,ctlK};
short passed_pawn1[8]={0,15,30,60,100,180,280,800};
short passed_pawn2[8]={0,15,25,35,50,90,140,800};
short passed_pawn3[8]={0,5,10,15,20,30,120,800};
short ISOLANI[8] = {-12,-14,-16,-20,-20,-16,-14,-12};
short DOUBLED[8] = {-12,-12,-12,-10,-10,-12,-12,-12};
short BMBLTY[30] = {-4,-2,0,1,2,3,4,5,6,7,8,9,10,11,12,13,13,14,14,
                    15,15,16,16,17,17,18,18,18,18,18};
short RMBLTY[30] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,14,15,15,
                    16,16,17,17,18,18,19,19,20,20,20,20};
short Kthreat[16] = {0,-6,-16,-32,-48,-64,-64,-64,-64,-64,-64,-64,
                     -64,-64,-64,-64};
short KNIGHTPOST,KNIGHTSTRONG,BISHOPSTRONG,KATAK;
short PEDRNK2B,PBKWRD,PWEAKA,PWEAKH,PADVNCM,PAWNSHIELD;
short RHOPN,RHOPNX,KHOPN,KHOPNX;
short ATAKD,HUNGP,HUNGX,KCASTLD,KMOVD,XRAY;
short stage,Zwmtl,Zbmtl,c1,c2,KSFTY,Developed[3];
short KingOpening[64]=
   {  0,  0, -4, -8, -8, -4,  0,  0,
     -4, -4, -8,-12,-12, -8, -4, -4,
    -12,-16,-20,-20,-20,-20,-16,-12,
    -16,-20,-24,-24,-24,-24,-20,-16,
    -16,-20,-24,-24,-24,-24,-20,-16,
    -12,-16,-20,-20,-20,-20,-16,-12,
     -4, -4, -8,-12,-12, -8, -4, -4,
      0,  0, -4, -8, -8, -4,  0,  0};
short KingEnding[64]=
   { 0, 6,12,18,18,12, 6, 0,
     6,12,18,24,24,18,12, 6,
    12,18,24,30,30,24,18,12,
    18,24,30,36,36,30,24,18,
    18,24,30,36,36,30,24,18,
    12,18,24,30,30,24,18,12,
     6,12,18,24,24,18,12, 6,
     0, 6,12,18,18,12, 6, 0};
short DyingKing[64]=
   { 0, 8,16,24,24,16, 8, 0,
     8,32,40,48,48,40,32, 8,
    16,40,56,64,64,56,40,16,
    24,48,64,72,72,64,48,24,
    24,48,64,72,72,64,48,24,
    16,40,56,64,64,56,40,16,
     8,32,40,48,48,40,32, 8,
     0, 8,16,24,24,16, 8, 0};
short pknight[64]=
   { 0, 4, 8,10,10, 8, 4, 0,
     4, 8,16,20,20,16, 8, 4,
     8,16,24,28,28,24,16, 8,
    10,20,28,32,32,28,20,10,
    10,20,28,32,32,28,20,10,
     8,16,24,28,28,24,16, 8,
     4, 8,16,20,20,16, 8, 4,
     0, 4, 8,10,10, 8, 4, 0};
short pbishop[64]=
   {14,14,14,14,14,14,14,14,
    14,22,18,18,18,18,22,14,
    14,18,22,22,22,22,18,14,
    14,18,22,22,22,22,18,14,
    14,18,22,22,22,22,18,14,
    14,18,22,22,22,22,18,14,
    14,22,18,18,18,18,22,14,
    14,14,14,14,14,14,14,14};
short PawnAdvance[64]=
   { 0, 0, 0, 0, 0, 0, 0, 0,
     0, 2, 4, 0, 0, 4, 2, 0,
     6, 8,10,12,12,10, 8, 6,
     8,12,20,32,32,20,12, 8,
    12,20,28,40,40,28,20,12,
    16,28,36,48,48,36,28,16,
    16,28,36,48,48,36,28,16,
     0, 0, 0, 0, 0, 0, 0, 0};


main(argc,argv)
int argc; char *argv[];
{
#ifdef MSDOS
  ttable = (struct hashentry far *)farmalloc(ttblsz *
           (unsigned long)sizeof(struct hashentry));
  printf("%lu bytes free\n",farcoreleft());
#else
  ttable = (struct hashentry *)malloc(ttblsz *
           (unsigned long)sizeof(struct hashentry));
  signal(SIGINT,Die); signal(SIGQUIT,Die);
#endif MSDOS
#ifdef CHESSTOOL
  setlinebuf(stdout);
  if (argc > 1) Level = atoi(argv[1]);
  else Level = 30; /* Default to 30 seconds */
#endif CHESSTOOL
  initscr();
  crmode();
  NewGame();
  while (!(quit))
    {
      if (bothsides && !mate) SelectMove(opponent,1); else input_command();
      if (!(quit || mate || force)) SelectMove(computer,1);
    }
  nocrmode();
  endwin();
}


#ifndef MSDOS
Die()
{
char s[80];
  signal(SIGINT,SIG_IGN);
  signal(SIGQUIT,SIG_IGN);
  gotoXY(50,24);
  printz("Abort? ");
  scanz("%s",s);
  if (strcmp(s,"yes") == 0)
    {
#ifdef DISPLAY
      gotoXY(1,24);
      nocrmode();
      endwin();
#endif DISPLAY
      exit(0);
    }
  signal(SIGINT,Die); signal(SIGQUIT,Die);
}

TerminateSearch()
{
  signal(SIGINT,SIG_IGN);
  signal(SIGQUIT,SIG_IGN);
  timeout = true;
  bothsides = false;
  signal(SIGINT,Die); signal(SIGQUIT,Die);
}
#endif


OpeningBook()
{
short i,j,r0,pnt;
unsigned m,r;
  srand((unsigned)time0);
  r0 = m = 0;
  for (i = 0; i < BookSize; i++)
    {
      for (j = 0; j <= GameCnt; j++)
        if (GameList[j].gmove != Book[i][j]) break;
      if (j > GameCnt)
        if ((r=rand()) > r0)
          {
            r0 = r; m = Book[i][GameCnt+1];
            hint = Book[i][GameCnt+2];
          }
    }
  for (pnt = TrPnt[1]; pnt < TrPnt[2]; pnt++)
    if ((Tree[pnt].f<<8) + Tree[pnt].t == m) Tree[pnt].score = 0;
  sort(TrPnt[1],TrPnt[2]-1);
  if (Tree[TrPnt[1]].score < 0) BookDepth = -1;
}


ShowDepth(ch)
char ch;
{
#ifdef DISPLAY
  gotoXY(75,1); printz("%2d%c",Sdepth,ch); ClrEoln();
#endif DISPLAY
}

ShowResults(ch)
char ch;
{
#ifndef DISPLAY
#ifndef CHESSTOOL
register int i;
  printz("%2d%c  %5d %4ld %7ld   ",Sdepth,ch,root->score,et,NodeCnt);
  for (i = 1; PrVar[i] > 0; i++)
    {
      algbr((short)(PrVar[i] >> 8),(short)(PrVar[i] & 0xFF),false);
      if (i == 9 || i == 17) printz("\n                          ");
      printz("%5s ",mvstr1);
    }
  printz("\n");
  fflush(stdout);
#endif CHESSTOOL
#endif DISPLAY
}


int SelectMove(side,iop)
short side,iop;

/*
    Select a move by calling function search() at progressively deeper 
    ply until time is up or a mate or draw is reached. An alpha-beta 
    window of -9000 to +90 points is set around the score returned from the 
    previous iteration. 
*/

{
static short i,j,alpha,beta,score,tempb,tempc,temps,xside,rpt;

#ifndef MSDOS
  (void) times(&tmbuf1);
  signal(SIGINT,TerminateSearch); signal(SIGQUIT,TerminateSearch);
#endif MSDOS
  timeout = false;
  xside = otherside[side];
  if (iop != 2) player = side;
  if (TCflag)
    {
      ResponseTime = (TimeControl.clock[side]) /
                     (TimeControl.moves[side] + 3) -
                     OperatorTime;
      ResponseTime += (ResponseTime*TimeControl.moves[side])/(2*TCmoves+1);
    }
  else ResponseTime = Level;
  if (iop == 2) ResponseTime = 999;
  if (Sdepth > 0 && root->score > Zscore-zwndw) ResponseTime -= ft;
  else if (ResponseTime < 1) ResponseTime = 1;
  ExtraTime = 0;
  
  if (Sdepth == 0)
  {
    ExaminePosition();
    ptbl = ttable;
    for (i = 0; i < ttblsz; i++,ptbl++) ptbl->depth = 0;
    for (i = 0; i < 64; i++)
      for (j = 0; j < 64; j++)
        history[0][i][j] = history[1][i][j] = 0;
    TOsquare = -1;
    PV = 0;
    if (iop != 2) hint = 0;
    for (i = 0; i < maxdepth; i++)
     PrVar[i] = killr0[i] = killr1[i] = killr2[i] = killr3[i] = Qkillr[i] = 0;
#ifdef DISPLAY
    for (i = 1; i < 17; i++)
      {
        gotoXY(50,i); ClrEoln();
      }
#else
    post = false;
#ifndef CHESSTOOL
   printz("\nMove# %d    Target= %ld    Clock: %ld\n",TimeControl.moves[side],
           ResponseTime,TimeControl.clock[side]);
#endif CHESSTOOL
#endif DISPLAY  
    alpha = -9000; beta = 9000; rpt = 0;
    TrPnt[1] = 0; root = &Tree[0];
    MoveList(side,1);
    if (GameCnt < BookDepth) OpeningBook(); else BookDepth = -1;
    if (BookDepth > 0) timeout = true;
    NodeCnt = ETnodes = EvalNodes = HashCnt = 0;
    Zscore = 0; zwndw = 25;
  }
  
  while (!timeout && Sdepth < MaxSearchDepth)
    {
      Sdepth++;
      ShowDepth(' ');
      score = search(side,1,Sdepth,alpha,beta,PrVar,&rpt);
      for (i = 1; i <= Sdepth; i++) killr0[i] = PrVar[i];
      if (score < alpha)
        {
          ShowDepth('-');
          ExtraTime = 5*ResponseTime;
          score = search(side,1,Sdepth,-9000,beta,PrVar,&rpt);
        }
      if (score > beta && !(root->flags & exact))
        {
          ShowDepth('+');
          ExtraTime = 0;
          score = search(side,1,Sdepth,alpha,9000,PrVar,&rpt);
        }
      score = root->score;
      ShowResults('.');
      beta = score + Bwindow;
      if (Awindow != 9000) alpha = score - Awindow;
      for (i = 1; i <= Sdepth; i++) killr0[i] = PrVar[i];
      if (!timeout) sort(0,TrPnt[2]-1);
      if (score > Zscore-zwndw && score > Tree[1].score+250) ExtraTime = 0;
      else if (score > Zscore-3*zwndw) ExtraTime = ResponseTime;
      else ExtraTime = 3*ResponseTime;
      if (root->flags & exact) timeout = true;
      if (Tree[1].score < -9000) timeout = true;
      if (4*et > 2*ResponseTime + ExtraTime) timeout = true;
      if (!timeout)
        {
          Tscore[0] = score;
          if (Zscore == 0) Zscore = score;
          else Zscore = (Zscore+score)/2;
        }
      zwndw = 25+absv(Zscore/12);
    }

  score = root->score;
  if (rpt >= 2 || score < -12000) root->flags |= draw;
  if (iop == 2) return(0);
  if (BookDepth < 0) hint = PrVar[2];
  ElapsedTime(1);
#ifdef DISPLAY
  gotoXY(50,22); printz("Nodes=   %6ld",NodeCnt); ClrEoln();
  gotoXY(50,23); printz("Nodes/Sec= %4ld",evrate); ClrEoln();
#else
#ifndef CHESSTOOL
printz("Nodes= %ld  Eval= %ld  Hash= %ld  Rate= %ld  ",
        NodeCnt,EvalNodes,HashCnt,evrate);
printz("CPU= %.2ld:%.2ld.%.2ld\n",
        cputimer/6000,(cputimer % 6000)/100,cputimer % 100);
#endif CHESSTOOL
#endif DISPLAY

  if (score > -9999 && rpt <= 2)
    {
      MakeMove(side,root,&tempb,&tempc,&temps);
      algbr(root->f,root->t,root->flags & cstlmask);
    }
  else algbr(PieceList[side][0],PieceList[side][0],false);
  if (root->flags & epmask) UpdateDisplay(0,0,1,0);
  else UpdateDisplay(root->f,root->t,0,root->flags & cstlmask);
#ifdef CHESSTOOL
  gotoXY(50,17); printz("%d. ... %s",++mycnt1,mvstr1); ClrEoln();
  if (root->flags & draw) printz("Draw\n");
  if (score == -9999)
    if (opponent == white) printz("White\n"); else printz("Black\n");
  if (score == 9998)
    if (computer == white) printz("White\n"); else printz("Black\n");
#else
  gotoXY(50,17); printz("My move is: %s",mvstr1);
  if (beep) printz("%c",7);
  ClrEoln();
  gotoXY(50,24);
  if (root->flags & draw) printz("Draw game!");
  if (score == -9999) printz("opponent mates!");
  else if (score == 9998) printz("computer mates!");
  else if (score < -9000) printz("opponent will soon mate!");
  else if (score > 9000)  printz("computer will soon mate!");
  ClrEoln();
#endif CHESSTOOL
#ifndef DISPLAY
  printz("\n");
#endif DISPLAY
  if (score == -9999 || score == 9998) mate = true;
  if (mate) hint = 0;
  if (post) post_move(root);
  if (root->flags & cstlmask) Game50 = GameCnt;
  else if (board[root->t] == pawn || (root->flags & capture)) 
    Game50 = GameCnt;
  GameList[GameCnt].score = score;
  GameList[GameCnt].nodes = NodeCnt;
  GameList[GameCnt].time = (short)et;
  GameList[GameCnt].depth = Sdepth;
  if (TCflag)
    {
      TimeControl.clock[side] -= (et + OperatorTime);
      if (--TimeControl.moves[side] == 0)
        SetTimeControl();
    }
#ifdef MSDOS
  if (kbhit()) bothsides = false;
#endif
  if ((root->flags & draw) && bothsides) quit = true;
  if (GameCnt > 238) quit = true;
  player = xside;
  Sdepth = 0;
  fflush(stdin);
  return(0);
}


#ifdef CHESSTOOL
#define illegalmsg printz("illegal move\n")
#else
#define illegalmsg {gotoXY(50,24); printz("illegal move!"); ClrEoln();}
#endif

int VerifyMove(s,iop,mv)
char s[];
short iop;
unsigned short *mv;
{
static short pnt,tempb,tempc,temps,cnt;
static struct leaf xnode;
struct leaf *node;

  *mv = 0;
  if (iop == 2)
    {
      UnmakeMove(opponent,&xnode,&tempb,&tempc,&temps);
      return(false);
    }
  cnt = 0;
  MoveList(opponent,2);
  pnt = TrPnt[2];
  while (pnt < TrPnt[3])
    {
      node = &Tree[pnt]; pnt++;
      algbr(node->f,node->t,node->flags & cstlmask);
      if (strcmp(s,mvstr1) == 0 || strcmp(s,mvstr2) == 0)
        {
          cnt++; xnode = *node;
        }
    }
  if (cnt == 0)
    if (isdigit(s[1]) || isdigit(s[2])) illegalmsg;
  if (cnt == 1)
    {
      MakeMove(opponent,&xnode,&tempb,&tempc,&temps);
      if (SqAtakd(PieceList[opponent][0],computer))
        {
          UnmakeMove(opponent,&xnode,&tempb,&tempc,&temps);
          illegalmsg;
          return(false);
        }
      else
        {
          if (iop == 1) return(true);
          if (xnode.flags & epmask) UpdateDisplay(0,0,1,0);
          else UpdateDisplay(xnode.f,xnode.t,0,xnode.flags & cstlmask);
          if (xnode.flags & cstlmask) Game50 = GameCnt;
          else if (board[xnode.t] == pawn || (xnode.flags & capture)) 
            Game50 = GameCnt;
          GameList[GameCnt].depth = GameList[GameCnt].score = 0;
          GameList[GameCnt].nodes = 0;
          ElapsedTime(1);
          GameList[GameCnt].time = (short)et;
          TimeControl.clock[opponent] -= et;
          --TimeControl.moves[opponent];
          *mv = (xnode.f << 8) + xnode.t;
          return(true);
        } 
    }
  else if (cnt > 1)
    {
      gotoXY(50,24); printz("ambiguous move!"); ClrEoln();
    }
  return(false);
}

      
input_command()
{
short ok,i,f,t,c,p,tc,tp;
unsigned short mv;
char s[80],fname[20];

  ok=quit=false;
  player = opponent;
  ataks(white,atak[white]); ataks(black,atak[black]);
#ifdef DISPLAY
  gotoXY(50,19); printz("Your move is? "); ClrEoln();
#endif
  ft = 0;
  if (hint > 0 && !easy && BookDepth < 0)
    {
      fflush(stdout);
      time0 = time((long *)0);
      algbr(hint>>8,hint & 0xFF,false);
      strcpy(s,mvstr1);
      f = epsquare;
      if (VerifyMove(s,1,&mv))
        {
          SelectMove(computer,2);
          VerifyMove(mvstr1,2,&mv);
          if (Sdepth > 0) Sdepth--;
        }
      ft = time((long *)0) - time0;
      epsquare = f;
    }
  fflush(stdin);
  
  while (!(ok || quit))
    {
#ifndef CHESSTOOL
      gotoXY(50,19); printz("Your move is? "); ClrEoln();
#endif CHESSTOOL
      scanz("%s",s);
      player = opponent;
      ok = VerifyMove(s,0,&mv);
      if (ok && mv != hint)
        {
          Sdepth = 0;
          ft = 0;
        }
      if (strcmp(s,"bd") == 0)
        {
#ifdef DISPLAY
          ClrScreen();  UpdateDisplay(0,0,1,0);
#else
          PrintBoard();
#endif DISPLAY
        }
      if (strcmp(s,"quit") == 0) quit = true;
      if (strcmp(s,"post") == 0) post = !post;
      if (strcmp(s,"set") == 0) SetBoard();
      if (strcmp(s,"go") == 0) ok = true;
      if (strcmp(s,"help") == 0) help();
      if (strcmp(s,"force") == 0) force = !force;
      if (strcmp(s,"random") == 0) randflag = !randflag; 
      if (strcmp(s,"book") == 0) BookDepth = -1;
      if (strcmp(s,"undo") == 0 && GameCnt >= 0) Undo();
      if (strcmp(s,"new") == 0) NewGame();
      if (strcmp(s,"list") == 0) ListGame();
      if (strcmp(s,"clock") == 0) SetTimeControl();
      if (strcmp(s,"hash") == 0) hashflag = !hashflag;
      if (strcmp(s,"beep") == 0) beep = !beep;
      if (strcmp(s,"Awindow") == 0)
        {
          gotoXY(50,24); printz("window: "); ClrEoln();
          scanz("%hd",&Awindow);
        }
      if (strcmp(s,"Bwindow") == 0)
        {
          gotoXY(50,24); printz("window: "); ClrEoln();
          scanz("%hd",&Bwindow);
        }
      if (strcmp(s,"hint") == 0)
        {
          algbr((short)(hint>>8),(short)(hint & 0xFF),false);
          gotoXY(50,24); printz("try  %5s",mvstr1); ClrEoln();
        }
      if (strcmp(s,"both") == 0)
        {
          bothsides = !bothsides;
          Sdepth = 0;
          SelectMove(opponent,1);
          ok = true;
        }
      if (strcmp(s,"reverse") == 0)
        {
          reverse = !reverse;
          ClrScreen();
          UpdateDisplay(0,0,1,0);
        }
      if (strcmp(s,"switch") == 0)
        {
          computer = otherside[computer];
          opponent = otherside[opponent];
          UpdateDisplay(0,0,1,0);
          force = false;
          ok = true;
          Sdepth = 0;
        }
      if (strcmp(s,"white") == 0)
        {
          computer = white; opponent = black;
          UpdateDisplay(0,0,1,0);
          ok = true; force = false;
          Sdepth = 0;
        }
      if (strcmp(s,"black") == 0)
        {
          computer = black; opponent = white;
          UpdateDisplay(0,0,1,0);
          ok = true; force = false;
          Sdepth = 0;
        }
      if (strcmp(s,"time") == 0)
        {
          gotoXY(50,24); printz("enter time: "); ClrEoln();
          scanz("%ld",&Level);
          TCflag = false;
        }
      if (strcmp(s,"remove") == 0 && GameCnt >= 1) 
        {
          Undo(); Undo();
        }
      if (strcmp(s,"get") == 0)
        {
          gotoXY(50,24); printz("fname? "); ClrEoln(); scanz("%s",fname);
          GetGame(fname);
          InitializeStats();
          UpdateDisplay(0,0,1,0);
          Sdepth = 0;
        }
      if (strcmp(s,"save") == 0)
        {
          gotoXY(50,24); printz("fname? "); ClrEoln(); scanz("%s",fname);
          SaveGame(fname);
        }
      if (strcmp(s,"depth") == 0)
        {
          gotoXY(50,24); printz("depth= "); ClrEoln();
          scanz("%hd",&MaxSearchDepth);
        }
      if (strcmp(s,"verify") == 0) VerifyBook();
      if (strcmp(s,"easy") == 0) easy = !easy;
      if (strcmp(s,"contempt") == 0)
        {
          gotoXY(50,24); printz("contempt= "); ClrEoln();
          scanz("%hd",&c);
          contempt[computer] = c;
          contempt[opponent] = -c;
        }
      if (strcmp(s,"prune") == 0)
        {
          gotoXY(50,24); printz("prune= "); ClrEoln();
          scanz("%hd",&prune);
        }
      if (strcmp(s,"debug") == 0)
        {
          ExaminePosition();
          gotoXY(50,24); printz("enter piece: "); ClrEoln();
          scanz("%s",s);
          if (s[0] == 'w') c = white; else c = black;
          if (s[1] == 'p') p = pawn;
          else if (s[1] == 'n') p = knight;
          else if (s[1] == 'b') p = bishop;
          else if (s[1] == 'r') p = rook;
          else if (s[1] == 'q') p = queen;
          else if (s[1] == 'k') p = king;
          else p = no_piece;
          for (i = 0; i < 64; i++)
            {
              gotoXY(4+5*col[i],5+2*(7-row[i]));
              tp = board[i]; tc = color[i];
              board[i] = p; color[i] = c;
              c1 = c; c2 = otherside[c1];
              printz("%3d ",SqValue(i,opponent));
              board[i] = tp; color[i] = tc;
            }
          refresh();
        }
    }
#ifdef CHESSTOOL
  printf("%d. %s\n",++mycnt2,s);
#endif CHESSTOOL
  gotoXY(50,24); ClrEoln();
  ElapsedTime(1);
  if (force)
    {
      computer = opponent; opponent = otherside[computer];
    }
}


gotoXY(x,y)
short x,y;
{
#ifdef DISPLAY
#ifdef MSDOS
union REGS r1,r2;
  r1.h.ah = 0x02; r1.h.bh = 0x00;
  r1.h.dl = x-1; r1.h.dh = y-1;
  int86(0x10,&r1,&r2);
#else
  move(y-1,x-1);
#endif MSDOS
#else
#ifndef CHESSTOOL
  printz("\n");
#endif CHESSTOOL
#endif DISPLAY
}


ClrScreen()
{
#ifdef DISPLAY
#ifdef MSDOS
union REGS r1,r2;
  gotoXY(1,1);
  r1.h.ah = 0x08; r1.h.bh = 0x00;
  int86(0x10,&r1,&r2);
  r1.h.ah = 0x09; r1.h.al = 0x20;
  r1.h.bh = 0x00; r1.h.bl = r2.h.ah; 
  r1.h.ch = 0x07; r1.h.cl = 0xD0;
  int86(0x10,&r1,&r2);
  gotoXY(1,1);
#else
  clear(); refresh();
#endif MSDOS
#else
  printz("\n\n");
#endif DISPLAY
}


ClrEoln()
{
#ifdef DISPLAY
#ifdef MSDOS
union REGS r1,r2;
char x,y;
  r1.h.ah = 0x03; r1.h.bh = 0x00;
  int86(0x10,&r1,&r2);
  x = r2.h.dl+1; y = r2.h.dh+1;
  r1.h.ah = 0x08; r1.h.bh = 0x00;
  int86(0x10,&r1,&r2);
  r1.h.ah = 0x09; r1.h.al = 0x20;
  r1.h.bh = 0x00; r1.h.bl = r2.h.ah; 
  r1.h.ch = 0x00; r1.h.cl = 81-x;
  int86(0x10,&r1,&r2);
  gotoXY(x,y);
#else
  clrtoeol(); refresh();
#endif MSDOS
#endif DISPLAY
}


algbr(f,t,iscastle)
short f,t,iscastle;
{
  mvstr1[0] = cx[col[f]]; mvstr1[1] = rx[row[f]];
  mvstr1[2] = cx[col[t]]; mvstr1[3] = rx[row[t]];
  mvstr2[0] = qx[board[f]];
  mvstr2[1] = mvstr1[2]; mvstr2[2] = mvstr1[3];
  mvstr1[4] = '\0'; mvstr2[3] = '\0';
  if (iscastle)
    if (t > f) strcpy(mvstr2,"o-o");
    else strcpy(mvstr2,"o-o-o");
}


Undo()
{
short f,t;
  f = GameList[GameCnt].gmove>>8;
  t = GameList[GameCnt].gmove & 0xFF;
  if (board[t] == king && distance(t,f) > 1)
    castle(GameList[GameCnt].color,f,t,2);
  else
    {
      board[f] = board[t]; color[f] = color[t];
      board[t] = GameList[GameCnt].piece;
      color[t] = GameList[GameCnt].color;
      if (board[f] == king) --kingmoved[color[f]];
    }
  if (TCflag) ++TimeControl.moves[color[f]];
  GameCnt--; mate = false; Sdepth = 0;
  UpdateDisplay(0,0,1,0);
  InitializeStats();
}


parse(s,m,j)
unsigned short *m; char s[];
short j;
{
short r1,r2,c1,c2;
  if (s[4] == 'o')
    if (j & 1) *m = 0x3C3A; else *m = 0x0402;
  else if (s[0] == 'o')
    if (j & 1) *m = 0x3C3E; else *m = 0x0406;
  else
    {
      c1 = s[0] - 'a'; r1 = s[1] - '1';
      c2 = s[2] - 'a'; r2 = s[3] - '1';
      *m = (locn[r1][c1]<<8) + locn[r2][c2];
    }
}


GetOpenings()
{
FILE *fd;
int c,j;
char s[80],*p;
  if ((fd = fopen("gnuchess.book","r")) != NULL)
    {
      BookSize = 0; j = 0; c = '?'; p = s;
      while ((c = getc(fd)) != EOF)
        if (c != '\n') *(p++) = c;
        else
          {
            *p = '\0';
            if (s[0] == '!')
              {
                if (j > 0)
                  {
                    while (j < BookDepth) Book[BookSize][j++] = 0; 
                    BookSize++; j = 0;
                  }
              }
            else
              {
                parse(&s[0],&Book[BookSize][j],j); j++;
                parse(&s[6],&Book[BookSize][j],j); j++;
              } 
            p = s;
          }
      fclose(fd);
    }
}


GetGame(fname)
char fname[20];
{
FILE *fd;
int c;
short loc;
unsigned short m;

  if (fname[0] == '\0') strcpy(fname,"chess.000");
  if ((fd = fopen(fname,"r")) != NULL)
    {
      fscanf(fd,"%hd%hd%hd",&computer,&opponent,&Game50);
      fscanf(fd,"%hd%hd%hd%hd",
             &castld[white],&castld[black],
             &kingmoved[white],&kingmoved[black]);
      fscanf(fd,"%hd%hd",&TCflag,&OperatorTime);
      fscanf(fd,"%ld%ld%hd%ld",
             &TimeControl.clock[white],&TimeControl.clock[black],
             &TimeControl.moves[white],&TimeControl.moves[black]);
      for (loc = 0; loc < 64; loc++)
        {
          fscanf(fd,"%hd",&m); board[loc] = (m >> 8); color[loc] = (m & 0xFF);
        }
      GameCnt = -1; c = '?';
      while (c != EOF)
        {
          ++GameCnt;
          c = fscanf(fd,"%hd%hd%hd%ld%hd%hd%hd",&GameList[GameCnt].gmove,
                     &GameList[GameCnt].score,&GameList[GameCnt].depth,
                     &GameList[GameCnt].nodes,&GameList[GameCnt].time,
                     &GameList[GameCnt].piece,&GameList[GameCnt].color);
        }
      GameCnt--;
    }
  fclose(fd);
}


SaveGame(fname)
char fname[20];
{
FILE *fd;
short loc,i;

  if (fname[0] == '\0') strcpy(fname,"chess.000");
  fd = fopen(fname,"w");
  fprintf(fd,"%d %d %d\n",computer,opponent,Game50);
  fprintf(fd,"%d %d %d %d\n",
          castld[white],castld[black],kingmoved[white],kingmoved[black]);
  fprintf(fd,"%d %d\n",TCflag,OperatorTime);
  fprintf(fd,"%ld %ld %d %ld\n",
          TimeControl.clock[white],TimeControl.clock[black],
          TimeControl.moves[white],TimeControl.moves[black]);
  for (loc = 0; loc < 64; loc++)
    fprintf(fd,"%d\n",256*board[loc] + color[loc]);
  for (i = 0; i <= GameCnt; i++)
    fprintf(fd,"%d %d %d %ld %d %d %d\n",
            GameList[i].gmove,GameList[i].score,GameList[i].depth,
            GameList[i].nodes,GameList[i].time,
            GameList[i].piece,GameList[i].color);
  fclose(fd);
}


ListGame()
{
FILE *fd;
short i,f,t;
  fd = fopen("chess.lst","w");
  fprintf(fd,"\n");
  fprintf(fd,"       score  depth  nodes  time         ");
  fprintf(fd,"       score  depth  nodes  time\n");
  for (i = 0; i <= GameCnt; i++)
    {
      f = GameList[i].gmove>>8; t = (GameList[i].gmove & 0xFF);
      algbr(f,t,false);
      if ((i % 2) == 0) fprintf(fd,"\n"); else fprintf(fd,"         ");
      fprintf(fd,"%5s  %5d     %2d %6ld %5d",mvstr1,
              GameList[i].score,GameList[i].depth,
              GameList[i].nodes,GameList[i].time);
    }
  fprintf(fd,"\n\n");
  fclose(fd);
} 


VerifyBook()
{
short i,j,side,found,pnt,tempb,tempc,temps;
unsigned short mv;
struct leaf *node;
  ClrScreen();
  for (i = 0; i < BookSize; i++)
    {
      CopyBoard(Stboard,board); CopyBoard(Stcolor,color);
      InitializeStats(); GameCnt = 0;
      kingmoved[white] = kingmoved[black] = false;
      castld[white] = castld[black] = false;
      side = white;
      for (j = 0; Book[i][j] > 0; j++)
        {
          MoveList(side,1);
          found = false;
          for (pnt = TrPnt[1]; pnt < TrPnt[2]; pnt++)
            {
              node = &Tree[pnt];
              mv = (node->f << 8) + node->t;
              if (mv == Book[i][j])
                {
                  found = true;
                  MakeMove(side,node,&tempb,&tempc,&temps);
                  break;
                }
            }
          if (!found) break;
          side = otherside[side];
        }
      if (found) printz("%d   ok\n",i);
      else printz("%d   bad (%d)\n",i,j);
    }
}


SetTimeControl()
{
long minutes;
  
  TCmoves = 0; OperatorTime = 30000;
  while (TCmoves <= 0)
    {
      gotoXY(50,24); printz("Enter #moves #minutes: "); ClrEoln();
      scanz("%hd %ld",&TCmoves,&minutes);
    }
  while (OperatorTime * TCmoves > 60*minutes + TCmoves)
    {
      gotoXY(50,24); printz("Operator time= "); ClrEoln();
      scanz("%ld",&OperatorTime);
    }
  TimeControl.moves[white] = TimeControl.moves[black] = TCmoves;
  TimeControl.clock[white] = TimeControl.clock[black] = 60*minutes;
  TCflag = true;
  et = 0;
#ifndef MSDOS
  (void) times(&tmbuf1);
#endif MSDOS
  ElapsedTime(1);
}


ElapsedTime(iop)
short iop;
{
short m,s;
  et = time((long *)0) - time0;
  if (et < 0) et = 0;
  ETnodes += 50;
  if (et > et0 || iop == 1)
    {
      if (et > ResponseTime+ExtraTime) timeout = true;
#ifdef MSDOS
      if (kbhit()) timeout = true;
#endif MSDOS
      et0 = et;
      if (iop == 1)
        {
          time0 = time((long *)0); et0 = 0;
        }
#ifdef DISPLAY
      m = et/60; s = (et - 60*m);
      if (TCflag)
        {
          m = (TimeControl.clock[player] - et) / 60;
          s = absv(TimeControl.clock[player]) - et - 60*m;
        }
      if (player == white)
        if (reverse) gotoXY(20,2); else gotoXY(20,23);
      else
        if (reverse) gotoXY(20,23); else gotoXY(20,2);
      printz("%d:%2d   ",m,s);
#endif DISPLAY
#ifdef MSDOS
      cputimer = 100*et;
      if (et > 0) evrate = NodeCnt/(et+ft); else evrate = 0;
#else
      (void) times(&tmbuf2);
      cputimer = 100*(tmbuf2.tms_utime - tmbuf1.tms_utime) / HZ;
      if (cputimer > 0) evrate = (100*NodeCnt)/(cputimer+100*ft);
      else evrate = 0;
#endif MSDOS
      ETnodes = NodeCnt + 50;
      if (post)
        {
          gotoXY(50,22); printz("Nodes=   %6ld",NodeCnt);
          gotoXY(50,23); printz("Nodes/Sec= %4ld",evrate);
        }
      refresh();
    }
}


post_move(node)
struct leaf *node;
{
short c,d,e,ply;
  e = lpost; c = player;
  gotoXY(50,3); printz("Score= %6d",node->score);
  if (c == white) d = 4; else d = 5;
  for (ply = 1; PrVar[ply] > 0; ply++)
    {
      algbr((short)(PrVar[ply]>>8),(short)(PrVar[ply] & 0x00FF),false);
      if (c == white) gotoXY(50,++d); else gotoXY(60,d);
      printz("%5s",mvstr1);
      c = otherside[c];
    }
  ClrEoln();
  lpost = d;
  while (++d <= e)
    {
      gotoXY(50,d); ClrEoln();
    }
}


DrawPiece(loc)
short loc;
{
#ifdef DISPLAY
short r,c; char x;
  if (reverse) r = 7-row[loc]; else r = row[loc];
  if (reverse) c = 7-col[loc]; else c = col[loc];
  if (color[loc] == black) x = '*'; else x = ' ';
  gotoXY(5+5*c,5+2*(7-r)); printz("%c%c ",x,px[board[loc]]);
#endif DISPLAY
}


UpdateDisplay(f,t,flag,iscastle)
short f,t,flag,iscastle;
{
#ifdef DISPLAY
short i,l,c,z; 
  if (flag)
    {
      i = 3;
      gotoXY(3,++i);
      printz("|----|----|----|----|----|----|----|----|");
      while (i<19)
        {
          gotoXY(1,++i);
          if (reverse) z = (i/2)-1; else z = 10-(i/2);
          printz("%d |    |    |    |    |    |    |    |    |",z);
          gotoXY(3,++i);
          printz("|----|----|----|----|----|----|----|----|");
        }
      gotoXY(3,21);
      if (reverse) printz("  h    g    f    e    d    c    b    a");
              else printz("  a    b    c    d    e    f    g    h");
      if (reverse) gotoXY(5,23); else gotoXY(5,2);
      if (computer == black) printz("Computer"); else printz("Human   ");
      if (reverse) gotoXY(5,2); else gotoXY(5,23);
      if (computer == white) printz("Computer"); else printz("Human   ");
      for (l = 0; l < 64; l++) DrawPiece(l);
    }
  else
    {
      DrawPiece(f); DrawPiece(t);
      if (iscastle)
        if (t > f)
          { DrawPiece(f+3); DrawPiece(t-1); }
        else
          { DrawPiece(f-4); DrawPiece(t+1); }
    }
  refresh();
#endif DISPLAY
}


PrintBoard()
{
short r,c,l;
#ifndef DISPLAY
printz("\n");
for (r = 7; r >= 0; r--)
  {
    for (c = 0; c <= 7; c++)
      {
        if (reverse) l = locn[7-r][7-c]; else l = locn[r][c];
        if (color[l] == neutral) printz(" -");
        else if (color[l] == white) printz(" %c",qx[board[l]]);
        else printz(" %c",px[board[l]]);
      }
    printz("\n");
  }
#endif DISPLAY
}


SetBoard()
{
short a,r,c,loc;
char s[80];

  ClrScreen(); UpdateDisplay(0,0,1,0);
  gotoXY(50,2); printz(".   exit to main");
  gotoXY(50,3); printz("#   clear board");
  a = white;
  do
  {
    gotoXY(49,5);
    printz("enter piece & location: "); ClrEoln(); scanz("%s",s);
    if (s[0] == '#')
      {
        for (loc = 0; loc < 64; loc++)
          { board[loc] = no_piece; color[loc] = neutral; }
        UpdateDisplay(0,0,1,0);
      }
    if (s[0] == 'c' || s[0] == 'C') a = otherside[a];
    c = s[1]-'a'; r = s[2]-'1';
    if ((c >= 0) && (c < 8) && (r >= 0) && (r < 8))
      {
        loc = locn[r][c];
        color[loc] = a;
        if (s[0] == 'p') board[loc] = pawn;
        else if (s[0] == 'n') board[loc] = knight;
        else if (s[0] == 'b') board[loc] = bishop;
        else if (s[0] == 'r') board[loc] = rook;
        else if (s[0] == 'q') board[loc] = queen;
        else if (s[0] == 'k') board[loc] = king;
        else { board[loc] = no_piece; color[loc] = neutral; }
        DrawPiece(loc);
      }
  }
  while (s[0] != '.');
  if (board[4] != king) kingmoved[white] = 10;
  if (board[60] != king) kingmoved[black] = 10;
  GameCnt = -1; Game50 = 0; BookDepth = -1; Sdepth = 0;
  InitializeStats(); ClrScreen(); UpdateDisplay(0,0,1,0);
}
  

NewGame()
{
short l,r,c,p;

  mate = quit = reverse = bothsides = post = randflag = TCflag = false;
  hashflag = force = easy = false;
  beep = true;
  mycnt1 = mycnt2 = 0;
  lpost =  NodeCnt = epsquare = et0 = 0;
  Awindow = 9000;
  Bwindow = 90;
  MaxSearchDepth = 30;
  prune = 50;
  contempt[white] = contempt[black] = 0;
#ifdef MSDOS
  BookDepth = 24;
#else
  BookDepth = 50;
#endif MSDOS
  GameCnt = -1; Game50 = 0;
  Zwmtl = Zbmtl = 0;
  Developed[white] = Developed[black] = false;
  castld[white] = castld[black] = false;
  kingmoved[white] = kingmoved[black] = 0;
  PawnThreat[0] = CptrFlag[0] = Threat[0] = false;
  Pscore[0] = 12000; Tscore[0] = 12000;
  TimeControl.clock[white] = TimeControl.clock[black] = 0;
  TimeControl.moves[white] = TimeControl.moves[black] = 0;
  opponent = white; computer = black;
  for (r = 0; r < 8; r++)
    for (c = 0; c < 8; c++)
      {
        l = 8*r+c; locn[r][c] = l;
        row[l] = r; col[l] = c;
        board[l] = Stboard[l]; color[l] = Stcolor[l];
      }
  for (c = white; c <= black; c++)
    for (p = pawn; p <= king; p++)
      for (l = 0; l < 64; l++)
        {
          hashcode[c][p][l].key = (unsigned short)rand();
          hashcode[c][p][l].bd = ((unsigned long)rand() << 16) +
                                 (unsigned long)rand();
        }
#ifdef CHESSTOOL
  printf("Chess\n");
  easy = true;
#else
  ClrScreen(); gotoXY(1,20);
  printz("enter response time: "); ClrEoln();
  scanz("%ld",&Level);
  ClrScreen();
#endif CHESSTOOL
  UpdateDisplay(0,0,1,0);
  InitializeStats();
  time0 = time((long *)0);
  ElapsedTime(1);
  GetOpenings();
}


help()
{
  ClrScreen();
  gotoXY(28,1); printz("CHESS command summary");
  gotoXY(1,3); printz("g1f3      move from g1 to f3");
  gotoXY(1,4); printz("nf3       move knight to f3");
  gotoXY(1,5); printz("o-o       castle king side");
  gotoXY(1,6); printz("o-o-o     castle queen side");
  gotoXY(1,7); printz("set       edit board");
  gotoXY(1,8); printz("switch    sides with computer");
  gotoXY(1,9); printz("white     computer plays white");
  gotoXY(1,10); printz("black     computer plays black");
  gotoXY(1,11); printz("reverse   board display");
  gotoXY(1,12); printz("both      computer match");
  gotoXY(1,13); printz("random    randomize play");
  gotoXY(1,14); printz("undo      undo last move");
  gotoXY(42,3); printz("time      change level");
  gotoXY(42,4); printz("depth     set search depth");
  gotoXY(42,5); printz("post      principle variation");
  gotoXY(42,6); printz("hint      suggest a move");
  gotoXY(42,7); printz("bd        redraw board");
  gotoXY(42,8); printz("clock     set time control");
  gotoXY(42,9); printz("force     enter game moves");
  gotoXY(42,10); printz("list      game to chess.lst");
  gotoXY(42,11); printz("save      game to file");
  gotoXY(42,12); printz("get       game from file");
  gotoXY(42,13); printz("new       start new game");
  gotoXY(42,14); printz("quit      exit CHESS");
  gotoXY(10,21); printz("Computer: ");
  if (computer == white) printz("WHITE"); else printz("BLACK");
  gotoXY(10,22); printz("Opponent: ");
  if (opponent == white) printz("WHITE"); else printz("BLACK");
  gotoXY(10,23); printz("Response time: %ld",Level," sec.");
  gotoXY(10,24); printz("Easy mode: ");
  if (easy) printz("ON"); else printz("OFF");
  gotoXY(40,21); printz("Depth: %d",MaxSearchDepth);
  gotoXY(40,22); printz("Random: "); 
  if (randflag) printz("ON"); else printz("OFF");
  gotoXY(40,23); printz("Transposition table: ");
  if (hashflag) printz("ON"); else printz("OFF");
  refresh();
  while (getchar() != '\n');
  ClrScreen();
  UpdateDisplay(0,0,1,0);
}


UpdateHashbd(side,piece,f,t)
short side,piece,f,t;
/*
   hashbd contains a 32 bit "signature" of the board position.
   hashkey contains a 16 bit code used to address the hash table.
   When a move is made, XOR'ing the hashcode of moved piece on the from and
   to squares with the hashbd and hashkey values keeps things current.
*/
{
  if (f >= 0)
    {
      hashbd ^= hashcode[side][piece][f].bd;
      hashkey ^= hashcode[side][piece][f].key;
    }
  if (t >= 0)
    {
      hashbd ^= hashcode[side][piece][t].bd;
      hashkey ^= hashcode[side][piece][t].key;
    }
}


UpdatePieceList(side,loc,iop)
short side,loc,iop;

/*
    Array PieceList[side][Pindx] contains the location of all the pieces of
    either side. Array Pindex[loc] contains the indx into PieceList for a
    given square.
*/

{
register short i;
  if (iop == 1)
    {
      PieceCnt[side]--;
      for (i = Pindex[loc]; i <= PieceCnt[side]; i++)
        {
          PieceList[side][i] = PieceList[side][i+1];
          Pindex[PieceList[side][i]] = i;
        }
    }
  else
    {
      PieceCnt[side]++;
      PieceList[side][PieceCnt[side]] = loc;
      Pindex[loc] = PieceCnt[side];
    }
}


InitializeStats()
{
register short i,loc;
  epsquare = -1;
  for (i = 0; i < 8; i++)
    PawnCnt[white][i] = PawnCnt[black][i] = 0;
  mtl[white] = mtl[black] = pmtl[white] = pmtl[black] = 0;
  PieceCnt[white] = PieceCnt[black] = 0;
  hashbd = hashkey = 0;
  for (loc = 0; loc < 64; loc++)
    if (color[loc] != neutral)
      {
        mtl[color[loc]] += value[board[loc]];
        if (board[loc] == pawn)
          {
            pmtl[color[loc]] += valueP;
            ++PawnCnt[color[loc]][col[loc]];
          }
        if (board[loc] == king) Pindex[loc] = 0;
          else Pindex[loc] = ++PieceCnt[color[loc]];
        PieceList[color[loc]][Pindex[loc]] = loc;
        hashbd ^= hashcode[color[loc]][board[loc]][loc].bd;
        hashkey ^= hashcode[color[loc]][board[loc]][loc].key;
      }
}


sort(p1,p2)
short p1,p2;
{
register short p,p0,s;
struct leaf temp;

  s = 32000;
  while (p1 < p2)
    if (Tree[p1].score >= s) p1++;
    else
      {
        s = Tree[p1].score; p0 = p1;
        for (p = p1+1; p <= p2; p++)
          if (Tree[p].score > s)
            {
              s = Tree[p].score; p0 = p;
            }
        if (p0 != p1)
          {
            temp = Tree[p1]; Tree[p1] = Tree[p0]; Tree[p0] = temp;
          }
        p1++;
      }
}


repetition(cnt)
short *cnt;

/*
    Check for draw by threefold repetition.
*/

{
register short i,f,t;
short c,b[64];
unsigned short m;
  *cnt = c = 0;
  for (i = 0; i < 64; b[i++] = 0);
/*
  bzero((char *)b,sizeof(b));
  memset((char *)b,0,64*sizeof(short));
*/
  for (i = GameCnt; i > Game50; i--)
    {
      m = GameList[i].gmove; f = m>>8; t = m & 0xFF;
      if (++b[f] == 0) c--; else c++;
      if (--b[t] == 0) c--; else c++;
      if (c == 0) (*cnt)++;
    }
}


int SqAtakd(sq,side)
short sq,side;

/*
  Generate moves from 'sq' for each piece and see if the appropriate
  piece with color 'side' is encountered.
*/

{
register short i,u,m,d;
short m0;

  m0 = map[sq];
  
  if (HasPawn[side])
    {
      d = Dpwn[otherside[side]];
      for (i = d; i <= d+1; i++)
        if ((u = unmap[m0+Dir[i]]) >= 0)
          if (board[u] == pawn && color[u] == side) return(true);
    }
      
  if (HasKnight[side])
    for (i = 8; i <= 15; i++)
      if ((u = unmap[m0+Dir[i]]) >= 0)
        if (board[u] == knight && color[u] == side) return(true);
      
  if (HasRook[side] || HasQueen[side])
    for (i = 0; i <= 3; i++)
      {
        d = Dir[i]; m = m0+d; u = unmap[m];
        while (u >= 0)
          if (color[u] == neutral)
            {
              m += d; u = unmap[m];
            }
          else if (color[u] == side && sweep1[board[u]]) return(true);
          else break;
      }
    
  if (HasBishop[side] || HasQueen[side])
    for (i = 4; i <= 7; i++)
      {
        d = Dir[i]; m = m0+d; u = unmap[m];
        while (u >= 0)
          if (color[u] == neutral)
            {
              m += d; u = unmap[m];
            }
          else if (color[u] == side && sweep2[board[u]]) return(true);
          else break;
      }
    
  if (distance(sq,PieceList[side][0]) == 1) return(true);
    
  return(false);
}


ataks(side,a)
short side,a[];

/*
    Fill array atak[][] with info about ataks to a square.  Bits 8-15
    are set if the piece (king..pawn) ataks the square. Bits 0-7
    contain a count of total ataks to the square.
*/

{
register short u,m,d,c;
short j,piece,i,m0,loc;
 
  for (u = 0; u < 64; a[u++] = 0);
/*
  memset((char *)a,0,64*sizeof(short));
  bzero((char *)a,sizeof(a));
*/
  Dstart[pawn] = Dpwn[side]; Dstop[pawn] = Dstart[pawn] + 1;
  for (i = 0; i <= PieceCnt[side]; i++)
    {
      loc = PieceList[side][i];
      piece = board[loc]; m0 = map[loc]; c = control[piece];
      if (sweep[piece])
        for (j = Dstart[piece]; j <= Dstop[piece]; j++)
          {
            d = Dir[j]; m = m0+d; u = unmap[m];
            while (u >= 0)
              {
                a[u] = ++a[u] | c;
                if (color[u] == neutral)
                  {
                    m += d; u = unmap[m];
                  }
                else break;
              }
          }
      else
        for (j = Dstart[piece]; j <= Dstop[piece]; j++)
          if ((u = unmap[m0+Dir[j]]) >= 0)
            a[u] = ++a[u] | c;
    }
}

  
int castle(side,kf,kt,iop)
short side,kf,kt,iop;
{
short i,rf,rt,c1,c2,t0,xside;

  xside = otherside[side];
  if (kt > kf)
    {
      rf = kf+3; rt = kt-1; c1 = kf; c2 = rf;
    }
  else
    {
      rf = kf-4; rt = kt+1; c1 = rf; c2 = kf;
    }
  if (iop == 0)
    {
      if (board[kf] != king || board[rf] != rook || color[rf] != side)
        return(false);
      for (i = c1+1; i < c2; i++)
        if (color[i] != neutral) return(false);
      for (i = c1; i <= c2; i++)
        if (SqAtakd(i,xside)) return(false); 
    }
  else
    {
      if (iop == 1) castld[side] = true; else castld[side] = false;
      if (iop == 2)
        {
          t0 = kt; kt = kf; kf = t0;
          t0 = rt; rt = rf; rf = t0;
        }
      board[kt] = king; color[kt] = side; Pindex[kt] = 0;
      board[kf] = no_piece; color[kf] = neutral;
      board[rt] = rook; color[rt] = side; Pindex[rt] = Pindex[rf];
      board[rf] = no_piece; color[rf] = neutral;
      PieceList[side][Pindex[kt]] = kt;
      PieceList[side][Pindex[rt]] = rt;
      UpdateHashbd(side,king,kf,kt);
      UpdateHashbd(side,rook,rf,rt);
    }
  return(true);
}


en_passant(xside,f,t,iop)
short xside,f,t,iop;
{
short l;
  if (t > f) l = t-8; else l = t+8;
  if (iop == 1)
    {
      board[l] = no_piece; color[l] = neutral;
    }
  else 
    {
      board[l] = pawn; color[l] = xside;
    }
  InitializeStats();
}


LinkMove(ply,f,t,xside)
short ply,f,t,xside;

/*
    Add a move to the tree.  Assign a bonus to order the moves
    as follows:
      1. Principle variation
      2. Capture of last moved piece
      3. Other captures (major pieces first)
      4. Killer moves
      5. "history" killers    
*/

{
register short s;
register unsigned short mv;
struct leaf *node;

  node = &Tree[TrPnt[ply+1]];
  ++TrPnt[ply+1];
  node->flags = node->reply = 0;
  node->f = f; node->t = t; mv = (f<<8) + t;
  s = 0;
  if (mv == PV || mv == Swag0) s = 2000;
  else if (mv == Swag1) s = 80;
  else if (mv == Swag2) s = 70;
  else if (mv == Swag3) s = 60;
  else if (mv == Swag4) s = 40;
  else if (mv == Swag5) s = 30;
  if (color[t] != neutral)
    {
      node->flags |= capture;
      if (t == TOsquare) s += 800;
      s += value[board[t]] - board[f];
    }
  if (board[f] == pawn)
    {
      if (row[t] == 0 || row[t] == 7)
        {
          node->flags |= promote;
          s += 500;
        }
      else if (row[t] == 1 || row[t] == 6)
        {
          node->flags |= pwnthrt;
          s += 500;
        }
      else if (t == epsquare) node->flags |= epmask;
    }
  if (InChk)
    {
      if (board[f] == king)
        if (SqAtakd(t,xside)) s -= 200; else s += 400;
      if (mv == Qkillr[ply]) s += 200;
    }
  s += history[otherside[xside]-1][f][t];
  node->score = s-20000;
}


GenMoves(ply,loc,side,xside)
short ply,loc,side,xside;

/*
     Generate moves for a piece. The from square is mapped onto a 12 by 
     12 board and offsets (taken from array Dir[]) are added to the 
     mapped location. Array unmap[] maps the move back onto array 
     board[] (yielding a value of -1 if the to square is off the board). 
     This process is repeated for bishops, rooks, and queens until a 
     piece is encountered or the the move falls off the board. Legal 
     moves are then linked into the tree. 
*/
    
{
register short m,u,d;
short i,m0,piece; 

  piece = board[loc]; m0 = map[loc];
  if (sweep[piece])
    for (i = Dstart[piece]; i <= Dstop[piece]; i++)
      {
        d = Dir[i]; m = m0+d; u = unmap[m];
        while (u >= 0)
          if (color[u] == neutral)
            {
              LinkMove(ply,loc,u,xside);
              m += d; u = unmap[m];
            }
          else if (color[u] == xside)
            {
              LinkMove(ply,loc,u,xside);
              break;
            }
          else break;
      }
  else if (piece == pawn)
    {
      if (side == white && color[loc+8] == neutral)
        {
          LinkMove(ply,loc,loc+8,xside);
          if (row[loc] == 1)
            if (color[loc+16] == neutral)
              LinkMove(ply,loc,loc+16,xside);
        }
      else if (side == black && color[loc-8] == neutral)
        {
          LinkMove(ply,loc,loc-8,xside);
          if (row[loc] == 6)
            if (color[loc-16] == neutral)
              LinkMove(ply,loc,loc-16,xside);
        }
      for (i = Dstart[piece]; i <= Dstop[piece]; i++)
        if ((u = unmap[m0+Dir[i]]) >= 0)
          if (color[u] == xside || u == epsquare)
            LinkMove(ply,loc,u,xside);
    }
  else
    {
      for (i = Dstart[piece]; i <= Dstop[piece]; i++)
        if ((u = unmap[m0+Dir[i]]) >= 0)
          if (color[u] != side)
            LinkMove(ply,loc,u,xside);
    }
}


MoveList(side,ply)
short side,ply;

/*
    Fill the array Tree[] with all available moves for side to
    play. Array TrPnt[ply] contains the index into Tree[]
    of the first move at a ply.
*/
    
{
register short i;
short xside,f;

  xside = otherside[side];
  Swag0 = killr0[ply]; Swag1 = killr1[ply]; Swag2 = killr2[ply];
  Swag3 = killr3[ply]; Swag4 = Swag5 = 0;
  if (ply > 2)
    {
      Swag4 = killr1[ply-2];
      Swag5 = killr3[ply-2];
    }
  TrPnt[ply+1] = TrPnt[ply];
  Dstart[pawn] = Dpwn[side]; Dstop[pawn] = Dstart[pawn] + 1;
  for (i = PieceCnt[side]; i >= 0; i--)
    GenMoves(ply,PieceList[side][i],side,xside);
  if (kingmoved[side] == 0)
    {
      f = PieceList[side][0];
      if (castle(side,f,f+2,0))
        {
          LinkMove(ply,f,f+2,xside);
          Tree[TrPnt[ply+1]-1].flags |= cstlmask;
        }
      if (castle(side,f,f-2,0))
        {
          LinkMove(ply,f,f-2,xside);
          Tree[TrPnt[ply+1]-1].flags |= cstlmask;
        }
    }
  sort(TrPnt[ply],TrPnt[ply+1]-1);
}


#define LinkCapture\
{\
  node = &Tree[TrPnt[ply+1]];\
  ++TrPnt[ply+1];\
  node->f = loc; node->t = u;\
  node->reply = 0;\
  node->flags = capture;\
  node->score = value[board[u]] + svalue[board[u]] - piece;\
  if (piece == pawn && (u < 8 || u > 55))\
    {\
      node->flags |= promote;\
      node->score = valueQ;\
    }\
}


CaptureList(side,xside,ply)
short side,xside,ply;

/*
    Generate a list of captures similiarly to GenMoves.
*/

{
register short m,u,d;
short loc,i,j,m0,piece;
struct leaf *node;

  TrPnt[ply+1] = TrPnt[ply];
  Dstart[pawn] = Dpwn[side]; Dstop[pawn] = Dstart[pawn] + 1;
  for (i = 0; i <= PieceCnt[side]; i++)
    { 
      loc = PieceList[side][i];
      piece = board[loc]; m0 = map[loc];
      if (sweep[piece])
        for (j = Dstart[piece]; j <= Dstop[piece]; j++)
          {
            d = Dir[j]; m = m0+d; u = unmap[m];
            while (u >= 0)
              if (color[u] == neutral)
                {
                  m += d; u = unmap[m];
                }
              else
                {
                  if (color[u] == xside) LinkCapture;
                  break;
                }
          }
      else
        {
          for (j = Dstart[piece]; j <= Dstop[piece]; j++)
            if ((u = unmap[m0+Dir[j]]) >= 0)
              if (color[u] == xside) LinkCapture;
          if (piece == pawn && row[loc] == rank7[side])
            {
              if (side == white) u = loc+8; else u = loc-8;
              if (color[u] == neutral) LinkCapture;
            }
        }
   }
  sort(TrPnt[ply],TrPnt[ply+1]-1);
}


MakeMove(side,node,tempb,tempc,temps)
short side,*tempc,*tempb,*temps;
struct leaf *node;

/*
    Update Arrays board[], color[], and Pindex[] to reflect the new
    board position obtained after making the move pointed to by
    node.  Also update miscellaneous stuff that changes when a move
    is made.
*/
    
{
register short f,t;
short xside;

  xside = otherside[side];
  f = node->f; t = node->t; epsquare = -1;
  TOsquare = t; cptrval = 0;
  GameList[++GameCnt].gmove = (f<<8) + t;
  if (node->flags & cstlmask)
    {
      GameList[GameCnt].piece = no_piece;
      GameList[GameCnt].color = side;
      castle(side,f,t,1);
    }
  else
    {
      *tempc = color[t]; *tempb = board[t]; *temps = svalue[t];
      GameList[GameCnt].piece = *tempb;
      GameList[GameCnt].color = *tempc;
      if (*tempc != neutral)
        {
          UpdatePieceList(*tempc,t,1);
          if (*tempb == pawn) --PawnCnt[*tempc][col[t]];
          if (board[f] == pawn)
            {
              --PawnCnt[side][col[f]];
              ++PawnCnt[side][col[t]];
            }
          mtl[xside] -= value[*tempb];
          if (*tempb == pawn) pmtl[xside] -= valueP;
          UpdateHashbd(xside,*tempb,-1,t);
          cptrval = *temps;
        }
      color[t] = color[f]; board[t] = board[f]; svalue[t] = svalue[f];
      Pindex[t] = Pindex[f]; PieceList[side][Pindex[t]] = t;
      color[f] = neutral; board[f] = no_piece;
      if (board[t] == pawn)
        if (t-f == 16) epsquare = f+8;
        else if (f-t == 16) epsquare = f-8;
      if (node->flags & promote)
        {
          board[t] = queen;
          --PawnCnt[side][col[t]];
          mtl[side] += valueQ - valueP;
          pmtl[side] -= valueP;
          HasQueen[side] = true;
          cptrval -= svalue[f];
          UpdateHashbd(side,pawn,f,-1);
          UpdateHashbd(side,queen,f,-1);
        } 
      if (board[t] == king) ++kingmoved[side];
      if (node->flags & epmask) en_passant(xside,f,t,1);
      else UpdateHashbd(side,board[t],f,t);
    }
}


UnmakeMove(side,node,tempb,tempc,temps)
short side,*tempc,*tempb,*temps;
struct leaf *node;

/*
    Take back the move pointed to by node.
*/

{
register short f,t;
short xside;

  xside = otherside[side];
  f = node->f; t = node->t; epsquare = -1;
  GameCnt--;
  if (node->flags & cstlmask) castle(side,f,t,2);
  else
    {
      color[f] = color[t]; board[f] = board[t]; svalue[f] = svalue[t];
      Pindex[f] = Pindex[t]; PieceList[side][Pindex[f]] = f;
      color[t] = *tempc; board[t] = *tempb; svalue[t] = *temps;
      if (node->flags & promote)
        {
          board[f] = pawn;
          ++PawnCnt[side][col[t]];
          mtl[side] += valueP - valueQ;
          pmtl[side] += valueP;
          UpdateHashbd(side,queen,-1,t);
          UpdateHashbd(side,pawn,-1,t);
        } 
      if (*tempc != neutral)
        {
          UpdatePieceList(*tempc,t,2);
          if (*tempb == pawn) ++PawnCnt[*tempc][col[t]];
          if (board[f] == pawn)
            {
              --PawnCnt[side][col[t]];
              ++PawnCnt[side][col[f]];
            }
          mtl[xside] += value[*tempb];
          if (*tempb == pawn) pmtl[xside] += valueP;
          UpdateHashbd(xside,*tempb,-1,t);
        }
      if (board[f] == king) --kingmoved[side];
      if (node->flags & epmask) en_passant(xside,f,t,2);
      else UpdateHashbd(side,board[f],f,t);
    }
}


distance(a,b)
short a,b;
{
short d1,d2;

  d1 = absv(col[a]-col[b]);
  d2 = absv(row[a]-row[b]);
  if (d1 > d2) return(d1); else return(d2);
}


BlendBoard(a,b,c)
short a[64],b[64],c[64];
{
register int sq;
  for (sq = 0; sq < 64; sq++)
    c[sq] = (a[sq]*(10-stage) + b[sq]*stage) / 10;
}


CopyBoard(a,b)
short a[64],b[64];
{
register int sq;
  for (sq = 0; sq < 64; sq++)
    b[sq] = a[sq];
}


UpdateWeights()
{
short tmtl;

  if (mtl[white] != Zwmtl || mtl[black] != Zbmtl)
    {
      Zwmtl = mtl[white]; Zbmtl = mtl[black];
      emtl[white] = Zwmtl - pmtl[white] - valueK;
      emtl[black] = Zbmtl - pmtl[black] - valueK;
      tmtl = emtl[white] + emtl[black];
      if (tmtl > 5700) stage = 0;
      else if (tmtl < 1300) stage = 10;
      else stage = (5700-tmtl) / 440;
      
      PEDRNK2B = -15;         /* centre pawn on 2nd rank & blocked */
      PBKWRD = -6;            /* backward pawns */
      PWEAKA  = -3;           /* each attack to weak pawn */
      PWEAKH  = -3;           /* weak pawn on half open file */
      PAWNSHIELD = 10-stage;  /* pawn near friendly king */
      PADVNCM =  (10+stage)/2;    /* advanced pawn multiplier */
      
      KNIGHTPOST = (stage+2)/3;   /* knight near enemy pieces */
      KNIGHTSTRONG = (stage+6)/2; /* occupies pawn hole */
      
      BISHOPSTRONG = (stage+6)/2; /* occupies pawn hole */
      
      RHOPN    = 10;          /* rook on half open file */
      RHOPNX   = 4;
      
      XRAY     = 8;           /* Xray attack on major piece */
      
      KHOPN    = (3*stage-30) / 2; /* king on half open file */
      KHOPNX   = KHOPN / 2;
      KCASTLD  = 10 / (stage+1);   /* king castled */
      KMOVD    = -40 / (stage+1);  /* king moved before castling */
      KATAK    = 5;                /* B,R attacks near enemy king */
      if (stage < 8) KSFTY = 8-stage; else KSFTY = 0;
      
      ATAKD    = -6;          /* defender > attacker */
      HUNGP    = -8;          /* each hung piece */
      HUNGX    = -10;         /* extra for >1 hung piece */
    }
}


ExaminePosition()
/*
  This is done one time before the search is started.
  Set up arrays Mwpawn, Mbpawn, Mknight, Mbishop, Mking which are used
  in the SqValue() function to determine the positional value of each
  piece.
*/
{
register short i,sq;
short r,wpadv,bpadv,z,side,pp,j;
long stage2,tpmtl,Pd,val;

  wking = PieceList[white][0]; bking = PieceList[black][0];
  ataks(white,atak[white]); ataks(black,atak[black]);
  Zwmtl = Zbmtl = 0;
  UpdateWeights();
  stage2 = stage*stage; tpmtl = pmtl[white] + pmtl[black];
  HasPawn[white] = HasPawn[black] = false;
  HasKnight[white] = HasKnight[black] = false;
  HasBishop[white] = HasBishop[black] = false;
  HasRook[white] = HasRook[black] = false;
  HasQueen[white] = HasQueen[black] = false;
  for (side = white; side <= black; side++)
    for (i = 0; i <= PieceCnt[side]; i++)
      switch (board[PieceList[side][i]])
        {
          case pawn : HasPawn[side] = true; break;
          case knight : HasKnight[side] = true; break;
          case bishop : HasBishop[side] = true; break;
          case rook : HasRook[side] = true; break;
          case queen : HasQueen[side] = true; break;
        }
  if (!Developed[white])
    Developed[white] = (board[1] != knight && board[2] != bishop &&
                        board[5] != bishop && board[6] != knight);
  if (!Developed[black])
    Developed[black] = (board[57] != knight && board[58] != bishop &&
                        board[61] != bishop && board[62] != knight);
    
  for (sq = 0; sq < 64; sq++)
    {
      WeakSq[white][sq] = WeakSq[black][sq] = true;
      for (i = sq; i >= 0; i -= 8)
        if (atak[white][i] >= ctlP) WeakSq[white][sq] = false;
      for (i = sq; i < 64; i += 8)
        if (atak[black][i] >= ctlP) WeakSq[black][sq] = false;
      Kfield[white][sq] = Kfield[black][sq] = 0;
    }
  
  CopyBoard(pknight,Mknight[white]);
  CopyBoard(pknight,Mknight[black]);
  CopyBoard(pbishop,Mbishop[white]);
  CopyBoard(pbishop,Mbishop[black]);
  BlendBoard(KingOpening,KingEnding,Mking[white]);
  BlendBoard(KingOpening,KingEnding,Mking[black]);
  
  if (!Developed[white])
    {
       Mknight[white][1] -= 5;
       Mbishop[white][2] -= 5;
       Mbishop[white][5] -= 5;
       Mknight[white][6] -= 5;
    }
  if (!Developed[black])
    {
       Mknight[black][57] -= 5;
       Mbishop[black][58] -= 5;
       Mbishop[black][61] -= 5;
       Mknight[black][62] -= 5;
    }
    
  for (sq = 0; sq < 64; sq++)
    {
      wpadv = bpadv = PADVNCM;
      Mwpawn[sq] = (wpadv*PawnAdvance[sq]) / 10;
      Mbpawn[sq] = (bpadv*PawnAdvance[63-sq]) / 10;
      if (distance(sq,wking) < 3 && (col[sq] < 3 || col[sq] > 4))
        Mwpawn[sq] += PAWNSHIELD;
      if (distance(sq,bking) < 3 && (col[sq] < 3 || col[sq] > 4))
        Mbpawn[sq] += PAWNSHIELD;
        
      Mknight[white][sq] += 5 - distance(sq,bking);
      Mknight[white][sq] += 5 - distance(sq,wking);
      Mknight[black][sq] += 5 - distance(sq,wking);
      Mknight[black][sq] += 5 - distance(sq,bking);
      Mbishop[white][sq] += stage;
      Mbishop[black][sq] += stage;
      for (i = 0; i <= PieceCnt[black]; i++)
        if (distance(sq,PieceList[black][i]) < 3)
          Mknight[white][sq] += KNIGHTPOST;
      for (i = 0; i <= PieceCnt[white]; i++)
        if (distance(sq,PieceList[white][i]) < 3)
          Mknight[black][sq] += KNIGHTPOST;
      if (WeakSq[black][sq]) Mknight[white][sq] += KNIGHTSTRONG;
      if (WeakSq[white][sq]) Mknight[black][sq] += KNIGHTSTRONG;
      if (WeakSq[black][sq]) Mbishop[white][sq] += BISHOPSTRONG;
      if (WeakSq[white][sq]) Mbishop[black][sq] += BISHOPSTRONG;
      
      Pd = 0;
      for (i = 0; i < 64; i++)
        if (board[i] == pawn)
          {
            if (color[i] == white)
              {
                r = row[i]+3; pp = true;
                if (row[i] == 6) z = i+8; else z = i+16;
                for (j = i+8; j < 64; j += 8)
                  if (!WeakSq[black][j]) pp = false;
              }
            else
              {
                r = 10-row[i]; pp = true;
                if (row[i] == 1) z = i-8; else z = i-16;
                for (j = i-8; j >= 0; j -= 8)
                  if (!WeakSq[white][j]) pp = false;
              }
            if (pp) r *= 4;
            Pd += r*distance(sq,z);
          }
      if (tpmtl > 0)
        {
          val = (Pd*stage2) / (2*tpmtl);
          Mking[white][sq] -= (short)val;
          Mking[black][sq] -= (short)val;
        }

      if (distance(sq,wking) == 1) Kfield[black][sq] += KATAK;
      if (distance(sq,bking) == 1) Kfield[white][sq] += KATAK;
    }
}


int trapped(loc,piece)
short loc,piece;
/*
  See if the attacked piece has unattacked squares to move to.
  If it is trapped, increment the hung[] array so that the search
  will be extended.
*/
{
register short u,m,d;
short i,m0;

  m0 = map[loc];
  if (sweep[piece])
    for (i = Dstart[piece]; i <= Dstop[piece]; i++)
      {
        d = Dir[i]; m = m0+d; u = unmap[m];
        while (u >= 0)
          if (color[u] == c1) break;
          else if (atak[c2][u] == 0 || board[u] >= piece) return(false);
          else if (color[u] == c2) break;
          else
            {
              m += d; u = unmap[m];
            }
      }
  else if (piece == pawn)
    {
       if (c1 == white) u = loc+8; else u = loc-8;
       if (color[u] == neutral && atak[c1][u] >= atak[c2][u])
         return(false);
       for (i = 0; i <= 1; i++)
         if ((u = unmap[m0+Dir[Dpwn[c1]+i]]) >= 0)
           if (color[u] == c2) return(false);
    }
  else
    {
      for (i = Dstart[piece]; i <= Dstop[piece]; i++)
        if ((u = unmap[m0+Dir[i]]) >= 0)
          if (color[u] != c1)
            if (atak[c2][u] == 0 || board[u] >= piece) return(false);
    }
  return(true);
}


#define ScoreThreat\
  if (color[u] != c2)\
    if (atak[c1][u] == 0 || (atak[c2][u] & 0xFF) > 1) ++cnt;\
    else *s -= 3


KingScan(loc,s)
short loc,*s;
/*
  Assign penalty if king can be threatened by checks or if
  squares adjacent to the king have a bad attack/defence balance.
*/
{
register short m,u,d;
short i,m0,cnt,z;

  cnt = z = 0;
  m0 = map[loc];
  if (HasBishop[c2] || HasQueen[c2])
    for (i = Dstart[bishop]; i <= Dstop[bishop]; i++)
      {
        d = Dir[i]; m = m0+d; u = unmap[m];
        if (u >= 0 && atak[c2][u] > atak[c1][u]) z++;
        while (u >= 0)
          {
            if (atak[c2][u] & ctlBQ) ScoreThreat;
            if (color[u] != neutral) break;
            m += d; u = unmap[m];
          }
      }
  if (HasRook[c2] || HasQueen[c2])
    for (i = Dstart[rook]; i <= Dstop[rook]; i++)
      {
        d = Dir[i]; m = m0+d; u = unmap[m];
        if (u >= 0 && atak[c2][u] > atak[c1][u]) z++;
        while (u >= 0)
          {
            if (atak[c2][u] & ctlRQ) ScoreThreat;
            if (color[u] != neutral) break;
            m += d; u = unmap[m];
          }
      }
  if (HasKnight[c2])
    for (i = Dstart[knight]; i <= Dstop[knight]; i++)
      if ((u = unmap[m0+Dir[i]]) >= 0) 
        if (atak[c2][u] & ctlNN) ScoreThreat;

  if (z > 2) cnt++;
  *s += (KSFTY*Kthreat[cnt]) / 8;
}


BRscan(loc,s,mob)
short loc,*s,*mob;
/*
  Find B,R mobility, XRAY attacks, and pins.
  Increment the hung[] array if a pin is found.
*/
{
register short m,u,d;
short j,m0,s0,piece,pin;

  *mob = 0;
  piece = board[loc]; m0 = map[loc];
  for (j = Dstart[piece]; j <= Dstop[piece]; j++)
    {
      s0 = 2;
      d = Dir[j]; m = m0+d; u = unmap[m];
      while (u >= 0)
        {
          *s += Kfield[c1][u];
          if (color[u] == neutral)
            {
              *mob += s0;
              m += d; u = unmap[m];
            }
          else if (board[u] == pawn) break;
          else if (s0 == 2)
            {
              pin = u; s0 = 1;
              m += d; u = unmap[m];
            }
          else if (color[u] == c2 && (board[u] > piece || atak[c2][u] == 0))
            {
              *s += XRAY;
              if (color[pin] == c2)
                if (atak[c2][pin] == 0 ||
                    atak[c1][pin] > control[board[pin]]+1)
                  ++hung[c2];
              break;
            }
          else break;
        }
    }
}


int SqValue(loc,side)

/* Calculate the positional value for the piece on 'loc'. */

short loc,side;
{
register short j,rank,column;
short s,piece,a1,a2,e,m0,u,in_square,r,mob;

  piece = board[loc];
  a1 = (atak[c1][loc] & 0x4FFF); a2 = (atak[c2][loc] & 0x4FFF);
  rank = row[loc]; column = col[loc];
  s = 0;
  if (piece == pawn && c1 == white)
    {
      s = Mwpawn[loc];
      if (loc == 11 || loc == 12)
        if (color[loc+8] != neutral) s += PEDRNK2B;
      if ((column == 0 || PawnCnt[white][column-1] == 0) &&
          (column == 7 || PawnCnt[white][column+1] == 0))
        s += ISOLANI[column];
      else if (PawnCnt[white][column] > 1) s += DOUBLED[column];
      if (a1 < ctlP && atak[white][loc+8] < ctlP)
        {
          s += PBKWRD;
          s += PWEAKA * (a2 & 0xFF);
          if (PawnCnt[black][column] == 0) s += PWEAKH;
          if (color[loc+8] != neutral) s -= 3;
        }
      if (PawnCnt[black][column] == 0)
        {
          if (side == black) r = rank-1; else r = rank;
          in_square = (row[bking] >= r && distance(loc,bking) < 8-r);
          if (a2 == 0 || side == white) e = 0; else e = 1;
          for (j = loc+8; j < 64; j += 8)
            if (atak[black][j] >= ctlP) j = 99;
            else if (atak[black][j] > 0 || color[j] != neutral) e = 1; 
          if (j == 99) s += (stage*passed_pawn3[rank]) / 10;
          else
            {
              if (in_square || e == 1) s += (stage*passed_pawn2[rank]) / 10;
              else s += (stage*passed_pawn1[rank]) / 10;
              if (color[loc+8] != neutral) s -= 8;
            }
        }
    }
  else if (piece == pawn && c1 == black)
    {
      s = Mbpawn[loc];
      if (loc == 51 || loc == 52)
        if (color[loc-8] != neutral) s += PEDRNK2B;
      if ((column == 0 || PawnCnt[black][column-1] == 0) &&
          (column == 7 || PawnCnt[black][column+1] == 0))
        s += ISOLANI[column];
      else if (PawnCnt[black][column] > 1) s += DOUBLED[column];
      if (a1 < ctlP && atak[black][loc-8] < ctlP)
        {
          s += PBKWRD;
          s += PWEAKA * (a2 & 0xFF);
          if (PawnCnt[white][column] == 0) s += PWEAKH;
          if (color[loc-8] != neutral) s -= 3;
        }
      if (PawnCnt[white][column] == 0)
        {
          if (side == white) r = rank+1; else r = rank;
          in_square = (row[wking] <= r && distance(loc,wking) < r+1);
          if (a2 == 0 || side == black) e = 0; else e = 1;
          for (j = loc-8; j >= 0; j -= 8)
            if (atak[white][j] >= ctlP) j = -99;
            else if (atak[white][j] > 0 || color[j] != neutral) e = 1;
          if (j == -99) s += (stage*passed_pawn3[7-rank]) / 10;
          else
            {
              if (in_square || e == 1) s += (stage*passed_pawn2[7-rank]) / 10;
              else s += (stage*passed_pawn1[7-rank]) / 10;
              if (color[loc-8] != neutral) s -= 8;
            }
        }
    }
  else if (piece == knight)
    {
      s = Mknight[c1][loc];
    }
  else if (piece == bishop)
    {
      s = Mbishop[c1][loc];
      BRscan(loc,&s,&mob);
      s += BMBLTY[mob];
    }
  else if (piece == rook)
    {
      if (PawnCnt[c1][column] == 0) s += RHOPN;
      if (PawnCnt[c2][column] == 0) s += RHOPNX;
      BRscan(loc,&s,&mob);
      if (stage < 3) s += RMBLTY[mob] / 2;
      else
        {
          s += RMBLTY[mob];
          if (c1 == white) s += 14 - taxicab(loc,bking);
          else s += 14 - taxicab(loc,wking);
        }
    }
  else if (piece == queen)
    {
      if (stage > 3)
        if (c1 == white) s += 14 - taxicab(loc,bking);
        else s += 14 - taxicab(loc,wking);
    }
  else if (piece == king)
    {
      if (Developed[c2] && KSFTY > 0) KingScan(loc,&s);
      if (castld[c1]) s += KCASTLD;
      else if (kingmoved[c1] > 0) s += KMOVD;

      if (PawnCnt[c1][column] == 0) s += KHOPN;
      if (PawnCnt[c2][column] == 0) s += KHOPNX;
      if (column == 1 || column == 2 || column == 3 || column == 7)
        {
          if (PawnCnt[c1][column-1] == 0) s += KHOPN;
          if (PawnCnt[c2][column-1] == 0) s += KHOPNX;
        }
      if (column == 4 || column == 5 || column == 6 || column == 0)
        {
          if (PawnCnt[c1][column+1] == 0) s += KHOPN;
          if (PawnCnt[c2][column+1] == 0) s += KHOPNX;
        }

      if (stage < 8) s += Mking[c1][loc];
      else
        {
          e = Mking[c1][loc];
          m0 = map[loc];
          for (j = Dstart[king]; j <= Dstop[king]; j++)
            if ((u = unmap[m0+Dir[j]]) >= 0)
              if (atak[c2][u] == 0)
                if (Mking[c1][u] > e) e = Mking[c1][u];
          s += (Mking[c1][loc] + e) / 2;
        }
    }
  if (a2 > 0) 
    if (a1 == 0 || a2 > control[piece]+1)
      {
        s += HUNGP;
        ++hung[c1];
        if (piece != king && trapped(loc,piece)) ++hung[c1];
      }
    else if (a2 > a1) s += ATAKD;
  return(s);
}


ScoreLoneKing(side,score)
short side,*score;
/* 
   static evaluation when loser has only a king and winner has no pawns
   and mating material.
*/
{
short winner,loser,s;

  UpdateWeights();
  if (mtl[white] > mtl[black]) winner = white; else winner = black;
  loser = otherside[winner];
  if (emtl[winner] <= valueB) *score = 0;
  else
    {
      s = 4995 + emtl[winner] -
      DyingKing[PieceList[loser][0]] -
      2*distance(PieceList[winner][0],PieceList[loser][0]);
      if (side == winner) *score = s; else *score = -s;
    }
}


ScorePosition(side,score)
short side,*score;

/* Perform static evaluation of board position.  */

{
register short loc,i;
short s,xside,pscore[3];

  wking = PieceList[white][0]; bking = PieceList[black][0];
  UpdateWeights();
  xside = otherside[side];
  pscore[white] = pscore[black] = 0;

  for (c1 = white; c1 <= black; c1++)
    {
      c2 = otherside[c1];
      for (i = 0; i <= PieceCnt[c1]; i++)
        {
          loc = PieceList[c1][i];
          s = SqValue(loc,side);
          pscore[c1] += s;
          svalue[loc] = s;
        }
    }
  if (hung[side] > 1) pscore[side] += HUNGX;
  if (hung[xside] > 1) pscore[xside] += HUNGX;
  *score = mtl[side] - mtl[xside] + pscore[side] - pscore[xside] + 10;
  if (randflag) *score += rand() % 5;
  if (*score > 0 && pmtl[side] == 0 && emtl[side] <= valueB)
    *score = 0;
  if (*score < 0 && pmtl[xside] == 0 && emtl[xside] <= valueB)
    *score = 0;
  if (mtl[xside] == valueK && emtl[side] > valueB) *score += 200;
  if (mtl[side] == valueK && emtl[xside] > valueB) *score -= 200;
}


#define UpdateSearchStatus\
{\
  if (post)\
    {\
      algbr(node->f,node->t,false);\
      gotoXY(65,1); printz("%5s ",mvstr1);\
    }\
  if (pnt > TrPnt[1])\
    {\
      d = best-Zscore; e = best-node->score;\
      if (Sdepth == 1) ExtraTime = 10;\
      else if (best < alpha)\
        {\
          if (alpha - best > 800) ExtraTime = 5*ResponseTime;\
          else ExtraTime = 3*ResponseTime;\
        }\
      else if (d > -zwndw && e > 4*zwndw) ExtraTime = -ResponseTime/3;\
      else if (d > -zwndw) ExtraTime = 0;\
      else if (d > -3*zwndw) ExtraTime = ResponseTime;\
      else if (d > -9*zwndw) ExtraTime = 3*ResponseTime;\
      else ExtraTime = 5*ResponseTime;\
    }\
}


int evaluate(side,xside,ply,depth,alpha,beta)
short side,xside,ply,depth,alpha,beta;

/*
   Compute an estimate of the score by adding the positional score
   from the previous ply to the material difference.  If in the
   quiescence search return this estimate, otherwise call
   ScorePosition() to determine the score.
*/

{
short s;

  hung[white] = hung[black] = 0;
  slk = ((mtl[white] == valueK && pmtl[black] == 0) ||
         (mtl[black] == valueK && pmtl[white] == 0));
  s = -Pscore[ply-1] - cptrval + mtl[side] - mtl[xside];
  if (ply == 1 || slk ||
     (ply == Sdepth+1 && s-250 < beta) ||
     (ply == Sdepth+2 && s-prune < beta) ||
     (ply == Sdepth+3 && board[TOsquare] == pawn && s-prune < beta)) 
    {
      EvalNodes++;
      ataks(side,atak[side]);
      ataks(xside,atak[xside]);
      if (atak[side][PieceList[xside][0]] > 0) s = 10001-ply;
      else if (slk) ScoreLoneKing(side,&s);
      else ScorePosition(side,&s);
      InChk = (atak[xside][PieceList[side][0]] > 0);
    }
  else
    {
      if (CptrFlag[ply-1] && board[TOsquare] == pawn) s += 10;
      if (SqAtakd(PieceList[xside][0],side)) s = 10001-ply;
      InChk = SqAtakd(PieceList[side][0],xside);
    }
  Pscore[ply] = s - mtl[side] + mtl[xside];
  if (InChk) ChkFlag[ply-1] = Pindex[TOsquare];
  else ChkFlag[ply-1] = 0;
  Threat[ply-1] = (hung[side] > 1 && ply == Sdepth+1);
  return(s);
}


int FoundInTTable(side,depth,alpha,beta,score)
short side,depth,*alpha,*beta,*score;
{
short hindx;
  if (!hashflag) return(false);
  if (side == white) hashkey |= 1; else hashkey &= 0xFFFE;
  hindx = (hashkey & (ttblsz-1));
  ptbl = (ttable+hindx);
  if (ptbl->depth >= depth && ptbl->hashbd == hashbd)
    {
      HashCnt++;
      PV = ptbl->reply;
      if (ptbl->flags & truescore)
        {
          *score = ptbl->score;
          return(true);
        }
/*
      else if (ptbl->flags & upperbound)
        {
          if (ptbl->score < *alpha)
            {
              *score = ptbl->score;
              return(true); 
            }
        }
*/
      else if (ptbl->flags & lowerbound)
        {
          if (ptbl->score > *alpha) *alpha = ptbl->score;
        }
    }
  return(false);
}


PutInTTable(side,score,depth,alpha,beta,reply)
short side,score,depth,alpha,beta;
unsigned short reply;
{
short hindx;
  if (side == white) hashkey |= 1; else hashkey &= 0xFFFE;
  hindx = (hashkey & (ttblsz-1));
  ptbl = (ttable+hindx);
  ptbl->hashbd = hashbd;
  ptbl->depth = depth;
  ptbl->score = score; 
  ptbl->reply = reply;
  ptbl->flags = 0;
  if (score < alpha)
    ptbl->flags |= upperbound;
  else if (score > beta)
    ptbl->flags |= lowerbound;
  else ptbl->flags |= truescore;
}


int search(side,ply,depth,alpha,beta,bstline,rpt)
short side,ply,depth,alpha,beta,*rpt;
unsigned short bstline[];

/*
   Perform an alpha-beta search to determine the score for the 
   current board position. If depth <= 0 only capturing moves are 
   generated and searched, otherwise all moves are processed. The 
   search depth is modified for check evasions, some forcing checks 
   and for certain threats. Extensions may continue for up to 11 
   ply beyond the nominal search depth. 
*/

{
#define cut (cf && score+node->score < alpha)

register short j;
short best,tempb,tempc,temps,xside,pnt,pbst,d,e,cf,score,in_check,xdepth,rcnt;
unsigned short mv,nxtline[maxdepth];
struct leaf *node,tmp;

  NodeCnt++;
  xside = otherside[side];
  xdepth = depth;
  
  if (ply <= Sdepth+1) repetition(rpt); else *rpt = 0;
  if (*rpt >= 2) return(0);

  score = evaluate(side,xside,ply,depth,alpha,beta);
  in_check = InChk;
  if (score > 9000) return(score);
  
  if (in_check && depth > 0) ++depth;
  else if (depth < 1 && score >= alpha &&
          (in_check || PawnThreat[ply-1] || Threat[ply-1]))
    depth = 1;
  else if (depth < 1 && score <= beta && ply < Sdepth+4 &&
           ply > 4 && ChkFlag[ply-2] && ChkFlag[ply-4] &&
           ChkFlag[ply-2] != ChkFlag[ply-4])
    depth = 1;
  if (depth > 0)
    {
      if (FoundInTTable(side,depth,&alpha,&beta,&score))
        return(score);
      if (alpha > beta) return(alpha);
    }
    
  if (Sdepth == 1) d = 7; else d = 11;
  if (ply > Sdepth+d || (depth < 1 && score > beta)) return(score);

  if (ply > 1)
    if (depth > 0) MoveList(side,ply);
    else CaptureList(side,xside,ply);
    
  if (TrPnt[ply] == TrPnt[ply+1]) return(score);
    
  cf = (depth < 1 && !ChkFlag[ply-2] && !slk);

  if (depth > 0) best = -12000; else best = score;
  if (best > alpha) alpha = best;
  
  for (pnt = pbst = TrPnt[ply];
       pnt < TrPnt[ply+1] && best <= beta && !timeout;
       pnt++)
    {
      node = &Tree[pnt];
      nxtline[ply+1] = 0;
      
      if (cut) continue;
      if (ply == 1) UpdateSearchStatus;

      if (!(node->flags & exact))
        {
          MakeMove(side,node,&tempb,&tempc,&temps);
          CptrFlag[ply] = (node->flags & capture);
          PawnThreat[ply] = (node->flags & pwnthrt);
          Tscore[ply] = node->score;
          PV = node->reply;
          node->score = -search(xside,ply+1,depth-1,-beta,-alpha,
                                nxtline,&rcnt);
          if (absv(node->score) > 9000) node->flags |= exact;
          else if (rcnt == 1)
            if (node->score > contempt[side]+20) node->score -= 20;
            else if (node->score < contempt[side]-20) node->score += 20;
          if (rcnt >= 2 || GameCnt-Game50 > 99 ||
             (node->score == 9999-ply && !ChkFlag[ply]))
            {
              node->flags |= draw; node->flags |= exact;
              node->score = contempt[side];
            }
          node->reply = nxtline[ply+1];
          UnmakeMove(side,node,&tempb,&tempc,&temps);
        }
      if (node->score > best && !timeout)
        {
          if (depth > 0)
            if (node->score > alpha && !(node->flags & exact))
              node->score += depth;
          best = node->score; pbst = pnt;
          if (best > alpha) alpha = best;
          for (j = ply+1; nxtline[j] > 0; j++) bstline[j] = nxtline[j];
          bstline[j] = 0;
          bstline[ply] = (node->f<<8) + node->t;
          if (ply == 1)
            {
              if (post) post_move(node);
              if (best == alpha)
                {
                  tmp = Tree[pnt];
                  for (j = pnt-1; j >= 0; j--) Tree[j+1] = Tree[j];
                  Tree[0] = tmp;
                }
              if (Sdepth > 2)
                if (best > beta) ShowResults('+');
                else if (best < alpha) ShowResults('-');
                else ShowResults('&');
            }
        }
      if (NodeCnt > ETnodes) ElapsedTime(0);
    }
  if (ply == 1) node = &Tree[0]; else node = &Tree[pbst];
  mv = (node->f<<8) + node->t;
  if (ply <= Sdepth && *rpt == 0 && !timeout && hashflag)
    PutInTTable(side,best,xdepth,alpha,beta,bstline[ply+1]);
  if (depth > 0)
    if (history[side-1][node->f][node->t] < 180)
      history[side-1][node->f][node->t] += depth;
  if (node->t != (GameList[GameCnt].gmove & 0xFF))
    if (best <= beta) killr3[ply] = mv;
    else if (mv != killr1[ply])
      {
        killr2[ply] = killr1[ply];
        killr1[ply] = mv;
      }
  if (in_check && best > -9000) Qkillr[ply] = mv;
  if (best > 9000) killr0[ply] = mv; else killr0[ply] = 0;
  if (timeout || (best < alpha && -Tscore[ply-1] < best))
    best = -Tscore[ply-1];
  return(best);
}

