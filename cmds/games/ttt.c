static char ID[] = "@(#)ttt.c	2.1 ";
/*   ttt -- learning tic-tac-toe    */

#include <stdio.h>
#include <signal.h>

#define  SIZE     9
#define  BSIZE    18
#define  BFSIZE   1000
#define  TWO      2
#define  FOUR     4
#define  SIX      6
#define  THREE    3
#define  EIGHT    8
#define  BYTES    4000

int board[SIZE];
int badbuf[BFSIZE];
int singbuf[BSIZE];
int *singp, *badp;
char *info;
int intel1 = 0, intel2 = 0;
	int wins[8][3] = {
		{0, 1, 2},
		{3, 4, 5},
		{6, 7, 8},
		{0, 3, 6},
		{1, 4, 7},
		{2, 5, 8},
		{0, 4, 8},
		{2, 4, 6},
	};

	int maps[8][9] = {
		{0, 1, 2, 3, 4, 5, 6, 7, 8},
		{6, 3, 0, 7, 4, 1, 8, 5, 2},
		{8, 7, 6, 5, 4, 3, 2, 1, 0},
		{2, 5, 8, 1, 4, 7, 0, 3, 6},
		{2, 1, 0, 5, 4, 3, 8, 7, 6},
		{8, 5, 2, 7, 4, 1, 6, 3, 0},
		{6, 7, 8, 3, 4, 5, 0, 1, 2},
		{0, 3, 6, 1, 4, 7, 2, 5, 8},
	};

	int diag[4] = {16200, 15714, 15480, 15498};

	int center[4] = {11826, 11340, 11106, 11124};

main(argc,argv)
char *argv[];
{
	extern char *optarg;
	extern int optind;
	extern addno();
	int *bufa;
	int i, c, d, f1, num, ans, ins;
	int trial, ntrial;
	int flag = 0, flag1 = 0;
	int bits, svi;

	while ((c = getopt(argc, argv, "ie")) != EOF)
		switch(c)  {
		case 'i':
			instruct();
			break;

		case 'e':		/*  Extra intelligence  */
			intel1++;
			intel2++;
			break;

		case '?':
			printf("usage: ttt [-i -e]\n");
			exit(2);
		}

	printf("\n");
	printf("Tic-tac-toe\n");
	printf("Accumulated knowledge? ( Yes or No )\n");

#ifdef __HELIOS
	info = "/helios/local/games/lib/ttt.a";
#else
	info = "/usr/games/ttt.a";
#endif
	if ((d = quest()) == 3)
		exit(1);
	if (d == 1)  {
		if ((f1 = open(info,0)) != -1)  {
			read(f1, badbuf, BYTES);
			close(f1);
		}
	}

	badp = badbuf;
	while(badp <= badbuf + BFSIZE)  {
		if (*badp == 0)
			break;
		++badp;
	}
	bits = badp - badbuf;
	printf("%d 'words' of knowledge\n", bits);

	signal(SIGINT, addno);
	for (;;)  {
		printf("new game\n");

		/*  Initialize board  */

		singp = singbuf;
		for (i = 0; i < SIZE; ++i)
			board[i] = 0;

loop:  
		prboard();      /*   Print board    */

		printf("Your move?\n");
		if (( c = prmove()) == 3)
			exit(2);
		if ( c == 2)  {
			printf("bad move\n");
			goto loop;
		}

		/*  Check if player won  */

		if (( ans = check(SIX,2)) == 1)  {
			loose();
			continue;
		}

		/*  Check if catgame  */

		if (ans = catgam() == 2)
		    continue;

		/*  Select a move  */

		ntrial = trial = 0;
		for ( i = 0; i < SIZE; ++i)  {
			if ( board[i] == 0)  {
				board[i] = 1;
				if ( ans = check(THREE,1) == 1)  {
					printf("I win\n");
					flag++;
					break;
				}

				ins = getsit();
				board[i] = 0;

				if ( intel1 != 0)
					extra1(ins);
				if ( intel2 != 0 )
					extra2(ins);

				bufa = badbuf;
				while (bufa < badp) {
					if ( ins == *bufa)  {
						flag1++;
						break;
					}
					bufa++;
				}
				if ( flag1 != 0)  {
					bufa++;
					flag1 = 0;
					continue;
				}
				if ( ins != trial)  {
					if (ins > trial)  {
						trial = ins;
						svi = i;
					}
					++ntrial;
				}
			}
		}
		if ( flag != 0) {
			flag = 0;
			continue;
		}

		/*  Install move   */

		if ( ntrial == 0)  {
			conced();
			continue;
		}
		if ( ntrial != 1)
		    singp = singbuf;
		*singp = trial;
		++singp;
		board[svi] = 1;
		goto loop;
	}
}

quest()    /*  Checks answer to accumulated knowledge question  */
{
	char *s;
	char answer[100];

	s = answer;
	if (gets(s) != NULL)  {
		if (answer[0] == 'y')
		    return(1);
		else
			return(2);
	}
	return(3);
}

catgam()      /*   Checks if catgame    */
{
	int i;

	for ( i = 0; i < SIZE; ++i) {
		if ( board[i] == 0)
		    return(1);
	}

	printf("Draw\n");
	return(2);
}

loose()    /*   The player won   */
{
	printf("You win\n");
	update();
}

conced()     /*   Concedes move   */
{
	printf("I concede\n");
	update();
}

update()    /*   Updates the badbuf array    */
{
	int *p2, *p1;

	p1 = badp;
	p2 = singbuf;
	while ( p2 < singp)
	    *p1++ = *p2++;
	badp = p1;
}

prmove()    /*  Print players' moves  */
{
	int c;
	char answer[100];
	char *s;

	s = answer;
	if ( gets(s) != NULL )  {
		if ( answer[0] != '\0' )  {
			c = answer[0];
			if ( answer[0] < '1' || answer[0] > '9' )
			     return(2);
			if ( board[c - '1'] != 0 )
			     return(2);
			if ( answer[1] != '\0' )
			     return(2);

			board[c - '1'] = 2;
		}
		return(1);
	}
	return(3);
}

prboard()      /* print board */
{
	int i,num;

	for (i = 0; i < SIZE; ++i)  {
		if (board[i] == 0)  {
			num = i + 1;
			printf("%d", num);
		}
		else if (board[i] == 1)
		    printf("O");
		else
			printf("X");

		if ((i + 1) % 3 == 0)
		    printf("\n");
	}
}

check(lim,n)    /*  Check if any of the players won   */
int lim,n;
{
	int i,j,num,cnt = 0;

	for(i = 0; i < EIGHT; ++i)  {
		for(j = 0; j < THREE; ++j)  {
			num = wins[i][j];
			if ( board[num] == n )
			    cnt = cnt + board[num];
			else  {
				cnt = 0;
				break;
			}
		}
		if ( cnt == lim )
		    return(1);    /*  player won  */
	}
	return(0);    /*  player did not win  */
}

getsit()    /*  Get possible position in the board   */
{
	int i,j,r5,num;
	int r3 = 0;

	for (i = 0; i < EIGHT; ++i)  {
		r5 = 0;
		for (j = 0; j < SIZE; ++j)  {
			r5 = r5 * 3;
			num = maps[i][j];
			r5 = r5 + board[num];
		}
		if (r5 > r3)
		    r3 = r5;
	}
	return(r3);
}

addno()    /*  Add knowledge   */
{
	int num,num1,f2;
	int *point, *point1, *point2;
	int tmp;

	if ((f2 = open(info,0)) != -1)  {
		read(f2,badp,BYTES);
		close(f2);
	}

	point = point2 = badbuf;
	while(point2 <= badbuf + BFSIZE)  {
		tmp = *point2;
		if (*point2 == 0)
		    break;
		if (*point2 > 0)  {
			point1 = badbuf;
			while (*point1 != 0)  {
				if (tmp == *point1)  {
         			    *point1 = -1;
				}
				++point1;
			}
			*point++ = tmp;
		}
		point2++;
	}

	num = point - badbuf;
	printf("\n");
	printf("%d 'words' returned\n",num);
        num = num * 4;
	f2 = creat("ttt.a",0775);
	write(f2,badbuf,num);
	exit(0);
}

extra1(sit)   /*  Extra knowledge for diagonal moves  */
int sit;
{
	int i, j;

	for ( i = 0; i < FOUR; ++i) {
		if ( sit == diag[i] )  {
			for ( j = 0; j < FOUR; ++j)
				*badp++ = diag[j];
			intel1 = 0;
			break;
		}
	}
}

extra2(sit)    /*   Extra knowledge for center moves    */
int sit;
{
	int k, l;

	for ( k = 0; k < FOUR; ++k)  {
		if ( sit == center[k] )  {
			for ( l = 0; l < FOUR; ++l)
				*badp++ = center[l];
			intel2 = 0;
			break;
		}
	}
}

/*   Instructions for the game   */

char *inst[] = {
	"Instructions to play Tic-tac-toe:",
	"Two players use a board of nine positions.",
	"Your moves are marked with X's and your",
	"opponent's (the computer) moves are marked",
	"with O's. The player that first achieves",
	"a row, column, or diagonal with just its",
	"marks, wins the game. A new game immediately",
	"follows. If you do not wish to continue",
	"playing, hit the break key.",
	"The computer stores knowledge of the games",
	"played each time you win. If you do not wish",
	"your opponent to use its stored knowledge,",
	"just answer 'no' to the pertinent question.",
	"The computer will act as if it knows nothing",
	"about the game and, as a consequence, is easily",
	"beaten. But if you want it to use its stored",
	"knowledge, answer 'yes', and the game will become",
	"more interesting. If you want the computer to",
	"learn faster then add the -e option when calling",
	"the game.",
	"",
};

instruct()
{
	register char **cpp;

	printf("\n");

	for ( cpp = inst; **cpp != '\0'; ++cpp )
		printf("%s\n", *cpp);
}
