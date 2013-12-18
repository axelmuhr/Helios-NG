#ifdef XENIX
#include <tinfo.h>
#include <term.h>
#else
#include <curses.h>
#endif

#ifdef __HELIOS
#define logname cuserid
#endif

#define SPACE 32
#define SCOREF "/helios/local/games/lib/puzzle.score"

int playfield[4][4];
int ok[4][4] = { '0', '1', '2', '3',
		 '4', '5', '6', '7',
		 '8', '9', 'a', 'b',
		 'c', 'd', 'e', 32 };
int spacex, spacey;
long t1, t0, moves;

struct tscore {
				long t;
				char u[30];
};

compare(d1,d2)
struct tscore *d1, *d2;
{
	return(d1->t - d2->t);
}

char *gettstr(s)
long s;
{
	static char tstr[8];
	sprintf(tstr,"%02ld:%02ld:%02ld",s/60/60,s/60%60,s%60);
	return(tstr);
}

long gettl(st)
char st[8];
{
	long h, s, m;
	sscanf(st,"%02ld:%02ld:%02ld",&h,&m,&s);
	return(h*60L*60L+m*60L+s);
}

display_field(y0,y1)
{
	int i;
	for(i=y0;i<=y1;++i) {
		mvprintw(i+8,30,"%c  %c  %c  %c",
			playfield[i][0], playfield[i][1], playfield[i][2], playfield[i][3]);
	}
	refresh();
}

getindex(py,px,ch)
int *py, *px;
char ch;
{
	int i, j;
	for(i=0;i<=3;++i) 
		for(j=0;j<=3;++j)
			if(playfield[i][j]==ch) { *py = i; *px = j; return(1); }
	return(-1);
}


ready()
{
	int i, j, ret;
	for(i=0;i<=3;++i) 
		for(j=0;j<=3;++j) 
			if(playfield[i][j] != ok[i][j]) return(0);
	return(1);
}


lets_play()
{
	char ch;
	int x, y;

	cbreak();
	t1 = time((char *)0);
	mvprintw(23,25,"your choice (0-9,a-e) -->"); refresh();
	do {
		move(23,51); refresh();
		ch =getch();
		if ((ch == 'q') || (ch == 'Q'))
		 { endwin(); return(1); }
		moves++;
		getindex(&y,&x,ch);	
		if(x-1 == spacex && y == spacey) {
			playfield[y][x-1] = playfield[y][x]; playfield[y][x] = SPACE;
			spacex = x; display_field(y,y);
		}
		if(x+1 == spacex && y == spacey) {
			playfield[y][x+1] = playfield[y][x]; playfield[y][x] = SPACE;
			spacex = x; display_field(y,y);
		}
		if(y-1 == spacey && x == spacex) {
			playfield[y-1][x] = playfield[y][x]; playfield[y][x] = SPACE;
			spacey = y; display_field(y-1,y);
		}
		if(y+1 == spacey && x == spacex) {
			playfield[y+1][x] = playfield[y][x]; playfield[y][x] = SPACE;
			spacey = y; display_field(y,y+1);
		}
		refresh();
	} while(!ready());
	return(0);
}

look_score(stat)
int stat;
{
	FILE *sc;
	char tstr[9], s[80], *logname();
	int compare(), i=0;
	long gettl();
	struct tscore score[11];

	t0 = time((char *)0);
	strcpy(tstr,gettstr(t0-t1));
	clear();
	mvprintw(10,20,"thats all folx... your time is %s",tstr); refresh();

	if((sc=fopen(SCOREF,"r"))==NULL) { perror("can't open scorefile"); exit(1); }
	for(i=0;i<=9;i++) {
		fgets(s,80,sc);
		sscanf(s,"%s%s",tstr,score[i].u);
		score[i].t = gettl(tstr);
	}
	fclose(sc);

	score[10].t = t0-t1; 
	sprintf(score[10].u,"%s_[%07ld]_%s",(stat)?"__quit":"solved",moves,logname(NULL));
	qsort((char *)score,11,sizeof(struct tscore), compare);

	if((sc=fopen(SCOREF,"w"))==NULL) { 
		perror("can't open scorefile"); exit(1); 
	}
	for(i=1;i<=10;++i)
		fprintf(sc,"%s %s\n",gettstr(score[i].t),score[i].u);
	fclose(sc);

	getch();
	clear();
	mvprintw(5,23," L O O S E R L I S T "); refresh();
	for(i=1;i<=10;++i) 
		mvprintw(20-i,17,"%s %s",gettstr(score[i].t),score[i].u);
	refresh();
	getch();
}

initscore()
{
	FILE *nsc;
	int i;

	if(getuid() != 0) return(0);
	if((nsc = fopen(SCOREF,"w"))==NULL) {
		perror("unable to open scorefile"); exit(1);
	}
	for(i=0;i<=9;++i) {
		fprintf(nsc,"00:00:00 _undef_[0000000]_anonymous\n");
	}
	fclose(nsc);
}	

init()
{
	int testfield[16], i, j, x=0, y=0, test;

	srand(time(0));
	for(i=0;i<=15;) {
		test = rand()%16;
		for(j=0;j<=i&&test!=testfield[j];++j) continue;
		if(j>=i) { 
			testfield[i++] = test;
			if(test <= 9) playfield[y][x] = test + '0';
			else {
				playfield[y][x] = test + 'a' - 10;
				if(playfield[y][x]=='f') playfield[y][x]=SPACE, spacex=x, spacey=y;
			}
			if(++x == 4) { x = 0; y++; }
		}
	}
	display_field(0,3);
}

main(argc,argv)
int argc;
char *argv[];
{
	moves=0;
	if(argc == 2 ) 
		if(!strcmp(argv[1],"-i")) initscore();
	initscr();
	init();
	look_score(lets_play());
	endwin();
}	
