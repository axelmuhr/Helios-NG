/*
 *	The game of Backgammon
 */
static char ID[] = "@(#)back.c	2.1 ";

#include	<stdio.h>

#define TRUE		1
#define FALSE		0
#define	WHITE		0
#define	BROWN		1
#define	NIL		(-1)
#define	MAXGMOV		10
#define	MAXIMOVES	1000
#define RULES		"/sgs/jel/src/backrules"

extern	char	*getenv();

char	level,		/*'b'=beginner, 'i'=intermediate, 'e'=expert*/
	*result,
	s[100];

int	i, j, l, m, die1, die2, bsum, wsum, bdbl, wdbl, selflag, best,
	count, imoves, whose, first;

int	pflg = 1;
int	nobroll = 0;
int	goodmoves[MAXGMOV],
	probmoves[MAXGMOV];

int	brown[] = {		/* brown position table */
	0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 
	0, 0, 0, 0, 3, 0, 5, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0
};

int	white[] = {		/* white position table */
	0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 
	0, 0, 0, 0, 3, 0, 5, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0
};

struct	amove {
	int	first,
		second;
	};

struct probtab {
	int	sum;	/* probability of all possible rolls yielding n */
	struct	amove move[18];/* possible rolls yielding n */
	};

#ifndef __HELIOS
struct	probtab	probability[] = {
/*
 *	{ sum, { first, second }, { first, second } }
 */
/*0*/	{ 0 },
/*1*/	{ 6,{ 1,1 },{ 1,2 },{ 1,3 },{ 1,4 },{ 1,5 },{ 1,6 } },
/*2*/	{ 7,{ 1,1 },{ 1,2 },{ 2,2 },{ 2,3 },{ 2,4 },{ 2,5 },{ 2,6 } },
/*3*/	{ 8,{ 1,3 },{ 2,3 },{ 3,3 },{ 4,3 },{ 5,3 },{ 6,3 }, { 1,1 },{ 1,2} },
/*4*/	{ 9,{ 1,4 },{ 2,4 },{ 3,4 },{ 4,4 },{ 5,4 },{ 6,4 }, { 1,1 },{ 1,3 },{ 2,2 } },
/*5*/	{ 8,{ 1,5 },{ 2,5 },{ 3,5 },{ 4,5 },{ 5,5 },{ 6,5 }, { 1,4 },{ 2,3 } },
/*6*/	{ 10,{ 1,6 },{ 2,6 },{ 3,6 },{ 4,6 },{ 5,6 },{ 6,6 },{ 1,5 },{ 2,2 },{ 2,4 },{ 3,3 } },
/*7*/	{ 3, { 1,6 }, { 2,5 }, { 3,4 } },
/*8*/	{ 4, { 2,2 },{ 2,6 },{ 3,5 },{ 4,4 } },
/*9*/	{ 3, { 3,3 }, { 3,6 }, { 4,5 } },
/*10*/	{ 2, { 4,6 }, { 5,5 } },
/*11*/	{ 1, { 5,6 } },
/*12*/	{ 3, { 3,3 }, { 4,4 }, { 6,6 } },
/*13*/	{0},
/*14*/	{0},
/*15*/	{ 1, { 5,5 } },
/*16*/	{ 1, { 4,4 } },
/*17*/	{0},
/*18*/	{ 1, { 6,6 } },
/*19*/	{0},
/*20*/	{ 1, { 5,5 } },
/*21*/	{0},
/*22*/	{0},
/*23*/	{0},
/*24*/	{ 1, { 6,6 } }
};
#else
struct	probtab	probability[] = {
/*
 *	{ sum, { first, second }, { first, second } }
 */
/*0*/	{ 0 },
/*1*/	{ 6,{ { 1,1 },{ 1,2 },{ 1,3 },{ 1,4 },{ 1,5 },{ 1,6 } } },
/*2*/	{ 7,{ { 1,1 },{ 1,2 },{ 2,2 },{ 2,3 },{ 2,4 },{ 2,5 },{ 2,6 } } },
/*3*/	{ 8,{ { 1,3 },{ 2,3 },{ 3,3 },{ 4,3 },{ 5,3 },{ 6,3 }, { 1,1 },{ 1,2} } },
/*4*/	{ 9,{ { 1,4 },{ 2,4 },{ 3,4 },{ 4,4 },{ 5,4 },{ 6,4 }, { 1,1 },{ 1,3 },{ 2,2 } } },
/*5*/	{ 8,{ { 1,5 },{ 2,5 },{ 3,5 },{ 4,5 },{ 5,5 },{ 6,5 }, { 1,4 },{ 2,3 } } } ,
/*6*/	{ 10,{ { 1,6 },{ 2,6 },{ 3,6 },{ 4,6 },{ 5,6 },{ 6,6 },{ 1,5 },{ 2,2 },{ 2,4 },{ 3,3 } } },
/*7*/	{ 3, { { 1,6 }, { 2,5 }, { 3,4 } } },
/*8*/	{ 4, { { 2,2 },{ 2,6 },{ 3,5 },{ 4,4 } } },
/*9*/	{ 3, { { 3,3 }, { 3,6 }, { 4,5 } } },
/*10*/	{ 2, { { 4,6 }, { 5,5 } } },
/*11*/	{ 1, { { 5,6 } } },
/*12*/	{ 3, { { 3,3 }, { 4,4 }, { 6,6 } } },
/*13*/	{0},
/*14*/	{0},
/*15*/	{ 1, { { 5,5 } } },
/*16*/	{ 1, { { 4,4 } } },
/*17*/	{0},
/*18*/	{ 1, { { 6,6 } } },
/*19*/	{0},
/*20*/	{ 1, { { 5,5 } } },
/*21*/	{0},
/*22*/	{0},
/*23*/	{0},
/*24*/	{ 1, { { 6,6 } } }
};
#endif
struct	{
	int	pos[4],
		mov[4];
} moves[MAXIMOVES];

main()
{
	short	tvec[2];

	int	go[5],
		k, n, pid, ret, rpid, t, tmp;

	time(tvec);
	srand((long)((int)tvec[0]+(int)tvec[1]));
	go[5] = NIL;
	bsum=wsum=bdbl=wdbl=selflag=0;
	level='e';

#ifndef MYBACK
	fprintf(stdout, "Instructions? ");
#ifdef __HELIOS
	fflush(stdout);
#endif
	gets(s);
	if(*s == 'y')
		instructions();
	putchar('\n');
	fprintf(stdout, "Opponent's level: b - beginner,\n");
	fprintf(stdout, "i - intermediate, e - expert? ");
#ifdef __HELIOS
	fflush(stdout);
#endif
	gets(s);
	if(*s == 'b')
		level = 'b';
	else if(*s == 'i')
		level = 'i';
	putchar('\n');
	fprintf(stdout, "You will play brown.\n\n");
	fprintf(stdout, "Would you like to roll your own dice? ");
#ifdef __HELIOS
	fflush(stdout);
#endif
	gets(s);
	putchar('\n');
	if(*s == 'y')
		nobroll = 1;
#endif

	first=TRUE;
	fprintf(stdout, "Would you like to go first? ");
#ifdef __HELIOS
	fflush(stdout);
#endif
	gets(s);
	putchar('\n');
	if(*s == 'y')
		goto nowhmove;

whitesmv:
	whose = WHITE;
	roll(WHITE);
	if (first) {
		while (die1 == die2)
			roll(WHITE);
		first=FALSE;
	}
	wdbl += ( die1 == die2 );
	wsum += ( die1 == die2 ? 4 * die1 : die1 + die2 ) ;
	fprintf(stdout, "White rolls %d, %d\n", die1, die2);
	fprintf(stdout, "White's move is:");
	if(nextmove(white, brown) == NIL)
		goto nowhmove;
	if(piececount(white, 0, 24) == 0){
		fprintf(stdout, "White wins");
		result = "wwin";
		if((piececount(brown,0,6)>0) && (piececount(brown,0,24)==15)){
			fprintf(stdout, " with a Backgammon!\n");
			result = "wbck";
		} else if (piececount(brown, 0, 24) == 15) {
			fprintf(stdout, " with a Gammon.\n");
			result = "wgam";
		} else
			fprintf(stdout, ".\n");
		stats();
		exit(0);
	}

nowhmove:
	if(pflg)
		prtbrd();
	whose = BROWN;
	roll(BROWN);
	if (first) {
		while (die1==die2)
			roll(BROWN);
		first=FALSE;
	}
	bdbl += ( die1 == die2 );
	bsum += ( die1 == die2 ? 4 * die1 : die1 + die2 ) ;
	imoves=0;
	movegen(brown,white);
	if (die1 != die2) {
		tmp=die1;
		die1=die2;
		die2=tmp;
		movegen(brown,white);
	}

retry:
	fprintf(stdout, "\nYour roll is %d  %d\n", die1, die2);
	if (imoves == 0) {
		fprintf(stdout, "No moves possible\n");
		goto whitesmv;
	}
	best = strategy(brown,white);
	if ( selflag ) {
		fprintf(stdout, "Brown's move is:");
		nextmove(brown, white);
	}
	else {
		fprintf(stdout, "Move? ");
#ifdef __HELIOS
		fflush(stdout);
#endif
		gets(s);
		switch(*s) {
			case '\0':			/* empty line */
				if (imoves == 0) {
					fprintf(stdout, "Brown's move skipped.\n");
					goto whitesmv;
				}
				else {
					fprintf(stdout,"You must move so long as you have legal moves.\nType 'm' to learn what your options are.\n");
					goto retry;
				}

			case 'o':			/* how many beared off? */
				fprintf(stdout, "Brown:   %d\n", piececount(brown, 0, 24) - 15);
				fprintf(stdout, "White:   %d\n", piececount(white, 0, 24) - 15);
				goto retry;
	
			case 'p':			/* print board */
				prtbrd();
				goto retry;
	
			case 's':			/* stop auto printing of board */
				pflg = 0;
				goto retry;
	
			case 'r':			/* resume auto printing */
				pflg = 1;
				goto retry;
	
			case 'm':			/* print possible moves */
				pmoves();
				goto retry;
	
			case 'b':			/* what is my best move? */
				fprintf(stdout, "\n%d - ",best);
				 for (tmp = 0; tmp<4; tmp++){
					if(moves[best].pos[tmp] == NIL)
						break;
					fprintf(stdout, "(%d, %d) ",moves[best].pos[tmp],moves[best].mov[tmp]);
				}
				goto retry;
	
			case 'i':			/* play yourself */
				selflag = 1 ;
				goto nowhmove;
			case 'q':			/* i give up */
				exit(0);
	
			case '!':			/* escape to Shell */
				if(s[1] != '\0')
					system(s+1);
#ifndef __HELIOS
				else if((pid = fork()) == 0) {
					execl("/bin/sh", "sh", "-", 0);
#else
				else if ((pid = vfork()) == 0) {
					execl("/helios/bin/shell", "shell", "-", 0);
#endif
					fprintf(stderr, "back: cannot exec /bin/sh!\n");
					exit(2);
				}

				while((rpid = wait(&ret)) != pid && rpid != -1)
					;
				goto retry;
	
			case '?':			/* well, what can i do? */
				fprintf(stdout, "o		number beared off\n");
				fprintf(stdout, "p		print board\n");
				fprintf(stdout, "m		print legal moves\n");
				fprintf(stdout, "b		print best move\n");
				fprintf(stdout, "i		play yourself\n");
				fprintf(stdout, "q		quit\n");
				fprintf(stdout, "r		resume auto print of board\n");
				fprintf(stdout, "s		stop auto print of board\n");
				fprintf(stdout, "!		escape to Shell\n");
				goto retry;
		}
		n = sscanf(s,"%d%d%d%d%d",&go[0],&go[1],&go[2],&go[3],&go[4]);
		if((die1 != die2 && n > 2) || n > 4){
			fprintf(stdout, "Too many moves.\n");
			goto retry;
		}
		/* quick and dirty number of moves check */
		if ((die1 != die2 && n < 2 && (moves[0].pos[1] != NIL)) ||
			(die1 == die2 && n < 4 && (moves[0].pos[3] != NIL))) {
			fprintf(stdout,"Too few moves. Type 'm' to learn legal moves.\n");
			goto retry;
		}
		go[n] = NIL;
		if(*s=='-'){
			go[0]= -go[0];
			t=die1;
			die1=die2;
			die2=t;
		}
		for(k = 0; k < n; k++){
			if(0 <= go[k] && go[k] <= 24)
				continue;
			else{
				fprintf(stdout, "Move %d illegal.\n", go[k]);
				goto retry;
			}
		}
		if(play(brown, white, go))
			goto retry;
	}
	if(piececount(brown, 0, 24) == 0){
		fprintf(stdout, "Brown wins");
		result = "bwin";
		if((piececount(white,0,6)>0) && (piececount(white,0,24)==15)){
			fprintf(stdout, " with a Backgammon!\n");
			result = "bbck";
		} else if(piececount(white, 0, 24) == 15) {
			fprintf(stdout, " with a Gammon.\n");
			result = "bgam";
		} else
			fprintf(stdout, ".\n");
		stats();
		exit(0);
	}
	goto whitesmv;
}

play(player,playee,pos)
int *player,*playee,pos[];
{
	int	k, n, die, ipos;

	for(k=0; k < player[0]; k++){  /*blots on player[0] must be moved first*/
		if(pos[k] == NIL)
			break;
		if(pos[k] != 0){
			fprintf(stdout, "Stone on bar must be moved first.\n");
			return(NIL);
		}
	}
	for(k = 0; (ipos=pos[k]) != NIL; k++){
		die = k?die2:die1;
		n = 25-ipos-die;
		if(player[ipos] == 0)
			goto badmove;
		if(n > 0 && playee[n] >= 2)
			goto badmove;
		if(n <= 0){
			if(piececount(player,0,18) != 0)
				goto badmove;
			if((ipos+die) != 25 && piececount(player,19,24-die)!=0)
				goto badmove;
		}
		player[ipos]--;
		player[ipos+die]++;
	}
	for(k = 0; pos[k] != NIL; k++){
		die = k?die2:die1;
		n = 25-pos[k]-die;
		if(n>0 && playee[n]==1){
			playee[n]=0;
			playee[0]++;
		}
	}
	return(0);

badmove:
	fprintf(stdout, "Move %d illegal.\n", ipos);
	while(k--){
		die=k?die2:die1;
		player[pos[k]]++;
		player[pos[k]+die]--;
	}
	return(NIL);
}
nextmove(player,playee)
int *player,*playee;
{
	int	k;

	imoves=0;
	movegen(player,playee);
	if(die1!=die2){
		k=die1;
		die1=die2;
		die2=k;
		movegen(player,playee);
	}
	if(imoves==0){
		fprintf(stdout, "no move possible.\n");
		return(NIL);
	}
	k=strategy(player,playee);		/*select kth possible move*/
	prtmov(k);
	update(player,playee,k);
	return(0);
}
prtmov(k)
int k;
{
	int	n;

	if(k == NIL)
		fprintf(stdout, "No move possible\n");
	else for(n = 0; n < 4; n++){
		if(moves[k].pos[n] == NIL)
			break;
		if (whose==BROWN)
			fprintf(stdout, "    %d, %d",moves[k].pos[n],moves[k].mov[n]);
		else
			fprintf(stdout, "    %d, %d",25-moves[k].pos[n],moves[k].mov[n]);
	}
	fprintf(stdout, "\n");
}
update(player,playee,k)
int *player,*playee,k;
{
	int	n,t;

	for(n = 0; n < 4; n++){
		if(moves[k].pos[n] == NIL)
			break;
		player[moves[k].pos[n]]--;
		player[moves[k].pos[n]+moves[k].mov[n]]++;
		t=25-moves[k].pos[n]-moves[k].mov[n];
		if(t>0 && playee[t]==1){
			playee[0]++;
			playee[t]--;
		}
	}
}
piececount(player,startrow,endrow)
int *player,startrow,endrow;
{
	int	sum;

	sum=0;
	while(startrow <= endrow)
		sum += player[startrow++];
	return(sum);
}
pmoves()
{
	int	i1, i2, tmp;

	fprintf(stdout, "Possible moves are:\n");
	tmp = ( imoves > 30 ? ( best > 10 ? best + 1 : 10 ) : imoves );
	if ( imoves > 30 ){
		fprintf(stdout, "%d possible moves, printing %d\n",imoves,tmp);
	}
	for(i1 = 0; i1 < tmp; i1++){
		fprintf(stdout, "\n%d - ",i1);
		 for (i2 = 0; i2<4; i2++){
			if(moves[i1].pos[i2] == NIL)
				break;
			fprintf(stdout, "(%d, %d) ",moves[i1].pos[i2],moves[i1].mov[i2]);
		}
	}
	fprintf(stdout, "\n");
}

roll(who)
{
	register n;
	char	 s[10];

	if(who == BROWN && nobroll) {
		fprintf(stdout, "Roll? ");
#ifdef __HELIOS
		fflush(stdout);
#endif
		gets(s);
		n = sscanf(s, "%d%d", &die1, &die2);
		if(n != 2 || die1 < 1 || die1 > 6 || die2 < 1 || die2 > 6)
			fprintf(stdout, "Illegal - I'll do it!\n");
		else
			return;
	}
	die1 = ((rand()>>8) % 6) + 1;
	die2 = ((rand()>>8) % 6) + 1;
}

movegen(mover,movee)
int *mover,*movee;
{
	int	k;

	for(i = 0; i <= 24; i++){
		count = 0;
		if(mover[i] == 0)
			continue;
		if((k=25-i-die1) > 0 && movee[k] >= 2)
			if(mover[0] > 0)
				break;
		else
			continue;
		if(k <= 0){
			if(piececount(mover, 0, 18) != 0)
				break;
			if((i+die1) != 25 && piececount(mover,19,i-1) != 0)
				break;
		}
		mover[i]--;
		mover[i+die1]++;
		count = 1;
		for(j = 0; j <= 24; j++){
			if(mover[j]==0)
				continue;
			if((k=25-j-die2) > 0 && movee[k] >= 2)
				if(mover[0] > 0)
					break;
			else
				continue;
			if(k <= 0){
				if(piececount(mover,0,18) != 0)
					break;
				if((j+die2) != 25 && piececount(mover,19,j-1) != 0)
					break;
			}
			mover[j]--;
			mover[j+die2]++;
			count = 2;
			if(die1 != die2){
				moverecord(mover);
				if(mover[0] > 0)
					break;
				else
					continue;
			}
			for(l = 0; l <= 24; l++){
				if(mover[l] == 0)
					continue;
				if((k=25-l-die1) > 0 && movee[k] >= 2)
					if(mover[0] > 0)
						break;
				else
					continue;
				if(k <= 0){
					if(piececount(mover, 0, 18) != 0)
						break;
					if((l+die2) != 25 && piececount(mover,19,l-1) != 0)
						break;
				}
				mover[l]--;
				mover[l+die1]++;
				count=3;
				for(m=0;m<=24;m++){
					if(mover[m]==0)
						continue;
					if((k=25-m-die1) >= 0 && movee[k] >= 2)
						if(mover[0] > 0)
							break;
					else
						continue;
					if(k <= 0){
						if(piececount(mover,0,18) != 0)
							break;
						if((m+die2) != 25 && piececount(mover,19,m-1) != 0)
							break;
					}
					count=4;
					moverecord(mover);
					if(mover[0] > 0)
						break;
				}
				if(count == 3)
					moverecord(mover);
				else{
					mover[l]++;
					mover[l+die1]--;
				}
				if(mover[0] > 0)
					break;
			}
			if(count == 2)
				moverecord(mover);
			else{
				mover[j]++;
				mover[j+die1]--;
			}
			if(mover[0] > 0)
				break;
		}
		if(count == 1)
			moverecord(mover);
		else{
			mover[i]++;
			mover[i+die1]--;
		}
		if(mover[0] > 0)
			break;
	}
}
moverecord(mover)
int *mover;
{
	int	t;

	if(imoves < MAXIMOVES) {
		for(t = 0; t <= 3; t++)
			moves[imoves].pos[t] = NIL;
		switch(count) {
		case 4:
			moves[imoves].pos[3]=m;
			moves[imoves].mov[3]=die1;

		case 3:
			moves[imoves].pos[2]=l;
			moves[imoves].mov[2]=die1;

		case 2:
			moves[imoves].pos[1]=j;
			moves[imoves].mov[1]=die2;

		case 1:
			moves[imoves].pos[0]=i;
			moves[imoves].mov[0]=die1;
			imoves++;
		}
	}
	switch(count) {
	case 4:
		break;

	case 3:
		mover[l]++;
		mover[l+die1]--;
		break;

	case 2:
		mover[j]++;
		mover[j+die2]--;
		break;

	case 1:
		mover[i]++;
		mover[i+die1]--;
	}
}

strategy(player,playee)
int *player,*playee;
{
	int	k, n, nn, bestval, moveval, prob;

	n = 0;
	if(imoves == 0)
		return(NIL);
	goodmoves[0] = NIL;
	bestval = -32000;
	for(k = 0; k < imoves; k++){
		if((moveval=eval(player,playee,k,&prob)) < bestval)
			continue;
		if(moveval > bestval){
			bestval = moveval;
			n = 0;
		}
		if(n<MAXGMOV){
			goodmoves[n]=k;
			probmoves[n++]=prob;
		}
	}
	if(level=='e' && n>1){
		nn=n;
		n=0;
		prob=32000;
		for(k = 0; k < nn; k++){
			if((moveval=probmoves[k]) > prob)
				continue;
			if(moveval<prob){
				prob=moveval;
				n=0;
			}
			goodmoves[n]=goodmoves[k];
			probmoves[n++]=probmoves[k];
		}
	}
	return(goodmoves[(rand()>>4)%n]);
}

eval(player,playee,k,prob)
int *player,*playee,k,*prob;
{
	int	newtry[31], newother[31], *r, *q, *p, n, sum, first;
	int	ii, lastwhite, lastbrown;

/* newtry is player's board, newother is opponents
    home for each is high end of array
 */
	*prob = sum = 0;
	r = player+25;
	p = newtry;
	q = newother;
	while(player<r){
		*p++= *player++;
		*q++= *playee++;
	}
	q=newtry+31;
	for(p = newtry+25; p < q; p++)		/* zero out spaces for hit pieces */
		*p = 0;
	for(n = 0; n < 4; n++){
		if(moves[k].pos[n] == NIL)
			break;
		newtry[moves[k].pos[n]]--;
		newtry[ii=moves[k].pos[n]+moves[k].mov[n]]++;
		if(ii<25 && newother[25-ii]==1){
			newother[25-ii]=0;
			newother[0]++;
			if(ii<=15 && level=='e')		/* hit if near other's home */
				sum+=10;
		}
	}
	for(lastbrown = 0; newother[lastbrown] == 0; lastbrown++);
		;
	for(lastwhite = 0; newtry[lastwhite] == 0; lastwhite++)
		;
/* lastbrown and lastwhite point to pieces furthest from home */
	lastwhite = 25-lastwhite;
	if(lastwhite<=6 && lastwhite<lastbrown)
		sum=1000;
									/* experts running game. */
									/* first priority is to */
									/* get all pieces into */
									/* white's home */
	if(lastwhite<lastbrown && level=='e' && lastwhite>6) {
		for(sum = 1000; lastwhite > 6; lastwhite--)
			sum = sum-lastwhite*newtry[25-lastwhite];
	}
	for(first = 0; first < 25; first++)
		if(newother[first] != 0)		/*find other's first piece*/
			break;
	q = newtry+25;
	for(ii=1,p = newtry+1; p < q;++ii)		/* blocked points are good */
		if((*p++ > 1) && ( ii > 6 ))	/* if not in other's home, especially on 6 and bar points */
			sum += ((ii == 18 || ii == 19 ) ? 2 : 1 );
	if(first > 5) {					/* only stress removing pieces if */
							/* homeboard cannot be hit */
		q = newtry+31;
		p=newtry+25;
		for(n = 6; p < q; n--)
			sum += *p++ * n;			/*remove pieces, but just barely*/
	}
	if(level != 'b'){
		r = newtry+25-first;	/*singles past this point can't be hit*/
		for(ii=7,p = newtry+7; p < r; ++ii)
					/*singles are bad after 1st 6 points if they can be hit*/
			if ((*p++ == 1) && (getprob(newtry,newother,ii,ii) > 5))
				sum--;	/* a certain amount of risk is healthy */
		q = newtry+3;
		for(p = newtry; p < q; )	   /*bad to be on 1st three points*/
			sum -= *p++;
	}

	for(n = 1; n <= 4; n++)
		*prob += n*getprob(newtry,newother,6*n-5,6*n);
	return(sum);
}

instructions()
{
	printf("	To play backgammon, you must type the numbers\n");
	printf("	of the points from which pieces are to be\n");
	printf("	moved. For example, if the roll was '2  6',\n");
	printf("	typing '12 15' will move a piece from point\n");
	printf("	12 two spaces to point 14 and a piece from\n");
	printf("	point 15 six spaces to point 21. If the moves\n");
	printf("	must be made in the opposite order, you must\n");
	printf("	prepend the first number with a minus ('-').\n");
	printf("	Thus, typing '-12 15' will move a piece from\n");
	printf("	point 12 six spaces and a piece from point 15\n");
	printf("	two spaces. If you want to move a single piece\n");
	printf("	several times the sequence of points from\n");
	printf("	which it is to be moved must be typed. For \n");
	printf("	example, if the roll is '3  5', typing\n");
	printf("	'14  17' will move a piece from point 14 to\n");
	printf("	point 17 and then to point 22. If a double\n");
	printf("	is rolled, you should enter four numbers.\n");
	printf("	\n");
	printf("	Brown pieces that have been removed from the\n");
	printf("	board after being hit by white are on point 0\n");
	printf("	and must be brought in before any other moves\n");
	printf("	can be made. White pieces that are hit are \n");
	printf("	removed to point 25.\n");
	printf("	\n");
	printf("	Making illegal, too few or too many moves is\n");
	printf("	detected. Type a '?' when you are asked\n");
	printf("	for your move and a list of available actions \n");
	printf("	will be printed. Good luck!!!\n");
}

getprob(player,playee,start,finish)
int *player,*playee,start,finish;
{			/*returns the probability (times 36) that any
			  pieces belonging to 'player' and lying between
			  his points 'start' and 'finish' will be hit
			  by a piece belonging to playee
			*/
	int	k, n, sum;

	sum = 0;
	for(; start <= finish; start++){
		if(player[start] == 1){
			for(k = 1; k <= 24; k++){
				if((n=25-start-k) < 0)
					break;
				if(playee[n] != 0)
					sum += figprob(player,start,playee,n,k);
			}
		}
	}
	return(sum);
}
figprob(player,us,playee,them,k)

int	*player,
	us,
	*playee,
	them,
	k;
{
	int	sum,move,i,i1,i2;

	sum=0;
	if ( probability[k].sum == 0 )	/* e.g. k == 21 */
		return(0);

	for ( i=0; i < probability[k].sum; ++i )	/* a move */
		if ( probability[k].move[i].first == probability[k].move[i].second ) { /*double*/
			move = probability[k].move[i].first;
			for ( i1=1;i1<=4;++i1)
				if ( 25-them-(move*i1) < us && player[25-them-(move*i1)] >= 2 )
					break;
			if ( i1==5) /* made it all the way */
				sum += 1;
		} else {	/* not double */
			i1 = probability[k].move[i].first;
			i2 = probability[k].move[i].second;
			if ( (i1+i2) == k ) {
				if ( player[25-them-i1] < 2 || player[25-them-i2] < 2 )
					sum += 1;
			} else {
				sum += 1;
				}
			}
		/* end for */

	return(sum);
}
prtbrd()
{
	int	k;

	fprintf(stdout, "\nWhite's Home\n");
	fprintf(stdout, "   1   2   3   4   5   6       7   8   9  10  11  12\n");
	fprintf(stdout, "____________________________________________________\n");
	numline(brown, white, 1, 6);
	fprintf(stdout, "    ");
	numline(brown, white, 7, 12);
	putchar('\n');
	colorline(brown, 'B', white, 'W', 1, 6);
	fprintf(stdout, "    ");
	colorline(brown, 'B', white, 'W', 7, 12);
	putchar('\n');
	if(white[0] != 0)
		fprintf(stdout, "%28dW\n",white[0]);
	else
		putchar('\n');
	if(brown[0] != 0)
		fprintf(stdout, "%28dB\n", brown[0]);
	else
		putchar('\n');
	colorline(white, 'W', brown, 'B', 1, 6);
	fprintf(stdout, "    ");
	colorline(white, 'W', brown, 'B', 7, 12);
	putchar('\n');
	numline(white, brown, 1, 6);
	fprintf(stdout, "    ");
	numline(white, brown, 7, 12);
	fprintf(stdout, "\n____________________________________________________\n");
	fprintf(stdout, "  24  23  22  21  20  19      18  17  16  15  14  13");
	fprintf(stdout, "\nBrown's Home\n\n");
}
numline(upcol,downcol,start,fin)
int *upcol,*downcol,start,fin;
{
	int	k, n;

	for(k = start; k <= fin; k++){
		if((n = upcol[k]) != 0 || (n = downcol[25-k]) != 0)
			fprintf(stdout, "%4d", n);
		else
			fprintf(stdout, "    ");
	}
}
colorline(upcol,c1,downcol,c2,start,fin)
int *upcol,*downcol,start,fin;
char c1,c2;
{
	int	k;
	char 	c;

	for(k = start; k <= fin; k++){
		c = ' ';
		if(upcol[k] != 0)
			c = c1;
		if(downcol[25-k] != 0)
			c = c2;
		fprintf(stdout, "   %c",c);
	}
}

stats()
{
	FILE	*fd;
	char	*home,
		path[100];
	int	header = 1;


	fprintf(stdout,"Would you like postmortem statistics? ");
#ifdef __HELIOS
	fflush(stdout);
#endif
	gets(s);
	putchar('\n');
	if ( *s == 'y' ) {
		fprintf(stdout, "White's dice totaled %d, Brown's totaled %d\n",wsum,bsum);
		fprintf(stdout, "White doubled %d times, Brown doubled %d times\n",wdbl,bdbl);
		if ( (home=getenv("HOME")) != NULL) {
			sprintf(path, "%s/.backlog", home);
			if ( (fd=fopen(path, "r")) != NULL) {
				header = 0;
				fclose(fd);
				}
			if ( (fd=fopen(path, "a")) != NULL) {
				if ( header )
					fprintf(fd, "wsum	wdbl	bsum	bdbl	result\n");
				fprintf(fd, "%d\t%d\t%d\t%d\t%s\n", wsum, wdbl,
							bsum, bdbl, result);
				fclose(fd);
				}
			}

	}
}
