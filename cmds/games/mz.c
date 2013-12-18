#include <stdio.h>
/*
 *  Copyright:  This program is	copyrighted by yours truly.  
 *              However, it may be used and distributed freely.  
 *              You even have my permission to sell it, 
 *              or include in it a system that you sell.  I only
 *	        ask that my name be retained on this program, or any direct
 *	        decendents of this program with approximately the same
 *	        visibility as in this posting.
 *
 *  Mail:	I'm interested in any comments you have on this program.
 *		I can be mailed at "smithj@mbf.UUCP".  Home address is
 *              John Smith
 *              28032 Singleleaf
 *              Mission Viejo
 *              Calif. 92692
 *
 *		Have fun,
 */

#define max_computer 10
extern unsigned long *mz_adrs[];
unsigned int time_2_move_p;
unsigned int time_2_move_c[max_computer];
unsigned int time_2_dsplay_time;
int nbr_p_moves;
int nbr_c_moves;
int nbr_computers;

unsigned int speed_move_p = 35;
unsigned int speed_move_c[max_computer] = {25};
unsigned int speed_dsplay_time = 100;

unsigned int basetime;
unsigned int curtime;		/*converted time from get_time */

#define max_col 80
#define max_row 23

unsigned long buf1[max_col+2];
unsigned long buf2[max_col+2];
unsigned long dots[max_col+2];
int ndots; /* number of dots */
int n_dots_eaten; 
int use_dots = 1;
int s_spr_ndx;
int n_spr_ndx;
unsigned long q_chk[max_computer];
int found_this_computer[max_computer];
int q_chk_ndx[max_computer];
int c_cp_col[max_computer];
int c_cp_row[max_computer];

int gtime;

int nbrcalcs;

unsigned long mz[max_col+2];

unsigned long row_tbl[] =
{
	0x000001,
	0x000002,
	0x000004,
	0x000008,
	0x000010,
	0x000020,
	0x000040,
	0x000080,
	0x000100,
	0x000200,
	0x000400,
	0x000800,
	0x001000,
	0x002000,
	0x004000,
	0x008000,
	0x010000,
	0x020000,
	0x040000,
	0x080000,
	0x100000,
	0x200000,
	0x400000,
	0x800000
};


int old_pp_row;
int old_pp_col;
int pp_row;
int pp_col;
int pp_dir;
int pp_count;

int old_cp_row[max_computer];
int old_cp_col[max_computer];
int cp_row[max_computer];
int cp_col[max_computer];
int cp_dir[max_computer];
int contact;
int ctrlc = 0;
int dots_gone;

/*
     7 0 4
     2   3
     6 1 5
*/
int mov_col[] =
{
	0,0,-1,1,1,1,-1,-1
};

int mov_row[] =
{
	-1,1,0,0,-1,1,1,-1
};

#define nbr_mazes 5
int playing_at_skill_level = 1;

char *msg_ver = "v3.1";


int 
main(argc, argv) 
int argc;
char **argv;
{
	int i,j;
	int nr;
	int number_lives;
	int mz_number;
	int nbr_dots;
	int level;
	char keystroke,c;
	char *program_name;

	i=j=0;
	nbrcalcs = 0;
	nbr_p_moves = 0;
	nbr_c_moves = 0;
	nbr_dots = 0;
	n_dots_eaten = 0;
	level = 0;


	init_io();
/* make computers different speeds */
	for(i=0;i<max_computer;i++)
	{
		/*speed_move_c[i] = speed_move_c[i] + i;*/
		speed_move_c[i] = 25 + 3*i;
	}
	program_name = argv[0];
	erase_screen();
	curpos(0,0);

	printf( "                              Maze %s\n\n",msg_ver);
	printf( "instructions?\n" );
	c=getchar();
	{
		if( c != 'n' )
		{
		instruct();

		printf("\nReturn to continue\n");
		getchar();
		}
	}

	sel_level_of_play();

	nbr_computers = playing_at_skill_level;

	log_in();
	number_lives = 3;
	basetime = 0; /* setup to calculate new base time */
	get_time();	  /* calculate new base time */
	basetime = curtime; /* new basetime */
	get_time(); /* curtime - base time probably = 0 */
	cursor_off();
	srand(basetime);
	for(;(ctrlc == 0) && (number_lives > 0);)
	{
		nbr_dots = nbr_dots + 10;
		level = level + 1;
		for(mz_number=0; (mz_number < nbr_mazes) && 
			 (number_lives > 0) && (ctrlc == 0); mz_number++)
		    {
			cp_mz(mz_adrs[mz_number]);
			init_dots(nbr_dots);
			while((number_lives > 0) && (ctrlc == 0))
			{
				init_screen();
				/* display number of lives */
				curpos(0,0);
				printf("Lives left %d ",number_lives-1);
				/* display room and level */
				curpos(0,30);
				printf(" Room %d, level %d ",mz_number+1,level);
				curpos(0,50);
				printf(" Pellets eaten %d ",n_dots_eaten);
				old_pp_row=1;
				old_pp_col=1;
				pp_row=1;
				pp_col=1;
				pp_dir = 5;
				pp_count = 1;

				position_computers();
				contact = 0;
				dots_gone= 0;

				get_time();
				/* srand(basetime);*/
				for(i=0;i < nbr_computers;i++)
				{
				time_2_move_c[i] = curtime-(1+i);
				}
				time_2_move_p = curtime-1;
				time_2_dsplay_time = curtime-1;

				for(;;)
				{
					get_time();
					dsplay_time();
					move_p();
					if(contact == 1) break;
					move_c();
					if(contact == 1) break;
					calc();
					if(contact == 1) break;
				} 
				if(ndots == 0) /* done because got all dots */
				{
					contact = 0;
					number_lives++;
					break; /* to next screen */
				}
				else
				{ /* done because computer got him */
					number_lives--;
				}
			}
		}
	}
	erase_screen();
	cursor_on();
#if 0
	get_time();	  /* calculate new base time */
	time_2_dsplay_time = time_2_dsplay_time - speed_dsplay_time;
	dsplay_time();
	/* display room and level */
	curpos(0,30);
	printf("      %d,       %d ",mz_number,level);
	curpos(0,65);
	printf("%d ",n_dots_eaten);
#if 0
	printf("\n%d\n",nbrcalcs);
	printf("\n%d c ",nbr_c_moves);   
	printf("\n%d p ",nbr_p_moves);   
#endif
	printf("\n\n\n");
#endif
	log_out();
	close_io();
}

/* enter with row, column, returns non 0 if dot position, 0 if not */

int is_dot(row,col)
register int row,col;
{
	col++; /* because of guard column */
	if((dots[col] & row_tbl[row]) != 0)
	return 1;
	else
	return 0;
}

/* enter with row, column, returns non 0 if dot position, 0 if not */
/* if was dot position, removes it and decrements ndots */

int rem_dot(row,col)
register int row,col;
{
	long i;
	col++; /* because of guard column */
	i = dots[col] & row_tbl[row];
	if(i==0) return 0; /* not a dot position */
	--ndots;
	if(ndots == 0)
	{
		contact = 1;
		dots_gone = 1;
	}
	n_dots_eaten++;
	dots[col] &= ~row_tbl[row];
	return i;
}

/* enter with row, column, returns 0 if wall position, non 0 if not */

int is_wall(row,col)
register int row,col;
{
	col++; /* because of guard column */
	if((mz[col] & row_tbl[row]) == 0)
	return 0;
	else
	return 1;
}


disp_wall_char(row,col)
int row,col;
{
	int graph_type;
	/* offsets to determine graphic char to use on current point */
	static int col_off[] ={
		0,1,0,-1			};
	static int row_off[] ={
		-1,0,1,0			};
	int i,j;
	graph_type = 0;
	for(i=0; i < 4;i++)
	{
		if((row+row_off[i] >= 0) && 
		    (col+col_off[i] >= 0) &&
		    (row+row_off[i] < max_row) && 
		    (col+col_off[i] < max_col))
		{
			if(is_wall(row+row_off[i],col+col_off[i])== 0)
			{
				graph_type = graph_type | 
				    (unsigned int)(1 << i);
			}
		}
	}
	switch(graph_type)
	{
	case 0:
	case 2:
	case 8:
	case 10:			/* horizontal line */
		display_horiz_line();
		break;
	case 1:
	case 4:
	case 5:  			/* vertical line */
		display_vert_line();
		break;
	case 9:           	/* upper left angle */
		display_upper_left_angle();
		break;
	case 3:		    	/* upper right angle */

		display_upper_right_angle();
		break;
	case 6:			   	/* lower right angle */
		display_lower_right_angle();
		break;
	case 12: 			/* lower left angle */
		display_lower_left_angle();
		break;
	case 13: 			/* vertical and left line */
		display_vert_and_left_line();
		break;
	case 11: 			/* horizontal and upper vert line */
		display_hor_and_upper_vert_line();
		break;
	case 7: 			/* vertical and right line */
		display_vert_and_right_line();
		break;
	case 14: 			/* horizontal and bottom vert line */
		display_hor_and_bottom_vert_line();
		break;
	case 15: 			/* vertical and horizontal line */
		display_hor_and_vert_line();
		break;
	default:
		printf("?"); /* * */
		break;
	}
}


int init_dots(nbr_dots)
int nbr_dots;
{
	int i,row,col;

	/* select dots */
	for(i=0;i< max_col+2;i++)
	{
		dots[i] = 0;
	}
	for (i=0; i < nbr_dots;i++)
	{
		/* select legal row col */
		while((is_wall(row=(rand() % max_row),
		col=(rand() % max_col)) == 0) &&
		 (is_dot(row,col) == 1)) {}
		dots[col+1] = dots[col+1] | row_tbl[row];
	}
}
int position_computers()
{
	int i,row,col;

	/* position computers*/
	i = 0;  /* computer 0 always at same position */
	old_cp_row[i]=21;
	old_cp_col[i]=78;
	cp_row[i]=21;
	cp_col[i]=78;
	cp_dir[i] = 2;
	for (i=1; i < nbr_computers;i++)
	{
		/* select legal row col */
		while(is_wall(row=(rand() % max_row),
			col=(30+(rand() % (max_col-30)))) == 0)
		{
		}
		old_cp_row[i]=row;
		old_cp_col[i]=col;
		cp_row[i]=row;
		cp_col[i]=col;
		cp_dir[i] = 2;
	}
}



int init_screen()
{
	char line[max_col+1];
	register int row,col;
	int i,j;

	graph_on();
	ndots = 0; /* number of dot (spaces) characters */
	curpos(0,0);
	for (row=0; row < max_row; row++)
	{
		for(col=0;col < max_col; col++)
		{
			if((col != (max_col -1)) || (row != (max_row -1)))
				if(is_wall(row,col)== 0)
				{
					disp_wall_char(row,col);
				}
				else
				{
					if(use_dots && (is_dot(row,col) != 0))
					{
						display_dot();
						ndots++;
					}
					else
						printf(" ");
					/*line[col] = ' ';*/
				}
		}
	}
	curpos(max_row-1,max_col-1);
	disp_wall_char(max_row-1,max_col-1);
	graph_off();
	fflush(stdout);		/* flush the output buffer */
}

move_p()
{
	char keystroke;
	int canmove;

	keystroke = '\0';
	while ((time_2_move_p < curtime) || (keystroke != '\0'))
	{
		keystroke = chk_for_key();
		canmove = 0;
		if(keystroke != '\0')
		{
			keystroke = set_p_direction(keystroke);
		}

		if(is_wall(old_pp_row+mov_row[pp_dir],
		old_pp_col+mov_col[pp_dir]) != 0)
		{
			pp_row = old_pp_row + mov_row[pp_dir];
			pp_col = old_pp_col + mov_col[pp_dir];
			canmove = 1;
		}
		else
		{
			if(is_wall(old_pp_row+mov_row[pp_dir],old_pp_col) != 0)
			{
				pp_row = old_pp_row + mov_row[pp_dir];
				canmove = 1;
			}
			else
			{
				if(is_wall(old_pp_row,
				old_pp_col+mov_col[pp_dir]) != 0)
				{
					pp_col = old_pp_col + mov_col[pp_dir];
					canmove = 1;
				}
			}
		}

		if(canmove == 1)
		{
			if(use_dots)
			{
				rem_dot(old_pp_row,old_pp_col);
				mov_chr(old_pp_row,old_pp_col,
					pp_row,pp_col,' ','p');
			}
			else
			{
				mov_chr(old_pp_row,old_pp_col,
					pp_row,pp_col,' ','p');
			}
			nbr_p_moves++;   
			old_pp_row = pp_row;
			old_pp_col = pp_col;
		}
		if(keystroke == '\0')
		{
			time_2_move_p = time_2_move_p + speed_move_p;
		}
		if(chk_4_contact() == 1) break;
	}
}


move_c()
{
	int i;
	for(i=0;i < nbr_computers;i++)
	{
	while (time_2_move_c[i] < curtime)
	{

		if(is_wall(old_cp_row[i]+mov_row[cp_dir[i]],
		old_cp_col[i]+mov_col[cp_dir[i]]) != 0)
		{
			cp_row[i] = old_cp_row[i] + mov_row[cp_dir[i]];
			cp_col[i] = old_cp_col[i] + mov_col[cp_dir[i]];
		}
		else
		{
			if(is_wall(old_cp_row[i]+mov_row[cp_dir[i]],
				old_cp_col[i]) != 0)
				cp_row[i] = old_cp_row[i] + mov_row[cp_dir[i]];
			else
				if(is_wall(old_cp_row[i],
				old_cp_col[i]+mov_col[cp_dir[i]]) != 0)
					cp_col[i] = old_cp_col[i] + 
					mov_col[cp_dir[i]];
		}

		if(use_dots)
		{
			if(is_dot(old_cp_row[i],old_cp_col[i]))
			{
			    curpos(old_cp_row[i],old_cp_col[i]);
				display_dot();
			    curpos(cp_row[i],cp_col[i]);
			    printf("c");
			}
			else
			{
				mov_chr(old_cp_row[i],old_cp_col[i],
				     cp_row[i],cp_col[i],' ','c');
			}
		}
		else
		{
			mov_chr(old_cp_row[i],old_cp_col[i],cp_row[i],
			     cp_col[i],' ','c');
		}
		nbr_c_moves++;   
		old_cp_row[i] = cp_row[i];
		old_cp_col[i] = cp_col[i];
		time_2_move_c[i] = time_2_move_c[i] + speed_move_c[i];
		if(chk_4_contact() == 1) break;
	}
	}
}
 
dsplay_time()
{
	int i;
	static int speedup = 10; /* speed up players every 10 seconds */
	static int old_dots_eaten = 99;

	if(old_dots_eaten != n_dots_eaten)
	{
	curpos(0,65);
	printf("%d ",n_dots_eaten);
	old_dots_eaten = n_dots_eaten;
	}
	while (time_2_dsplay_time < curtime)
	{
#if 0
		curpos(0,70);
		printf("%d",gtime++);
#endif
		time_2_dsplay_time = time_2_dsplay_time + speed_dsplay_time;
		/*speedup--;*/
		if (speedup == 0)
		{
			speedup = 10;
			if(speed_move_p > 11)
				speed_move_p--;
			for(i=0;i < nbr_computers;i++)
			{
			if(speed_move_c[i] > 10)
				speed_move_c[i]--;
			}
		}
	}
}


/* init buffers */
init_calc()
{
	register int i;
	register unsigned long *j, *k;
	/* zero out buffers */
	for(i=max_col+2, j = &buf1[0],k = &buf2[0];i > 0;i--)
	{
		*j = 0;
		j++;
		*k = 0;
		k++;
	}
	buf1[pp_col+1] = row_tbl[pp_row];
	/* init spread indexes */
	s_spr_ndx = pp_col+1;
	n_spr_ndx = 1;
	for(i=0;i < nbr_computers;i++)
	{
	found_this_computer[i] = 0;
	q_chk[i] = row_tbl[cp_row[i]] +(row_tbl[cp_row[i]] << 1) + 
	    (row_tbl[cp_row[i]] >> 1);
	q_chk_ndx[i] = cp_col[i];
	c_cp_col[i] = cp_col[i]+1;
	c_cp_row[i] = cp_row[i];
	}
}

calc()
{
	register unsigned long *j,*k;
	register unsigned int i;
	register unsigned long m;
	register unsigned long *o;
	char keystroke;
	static int prog =0;
	int pass_through_calc = 0;

	init_calc();
	while(quick_chk()== 0)
	{
	/* spread bits forward and back one column and one row */
	/* takes a single bit * and turns into */
	/*1                */
	/*2       *        */
	/*3                */
	/* becomes         */
	/*1       *        */
	/*2      * *       */
	/*3       *        */
	/* Note that the original bit is gone.  The spread is a shell */
	/* which allows the if((m = *j) != 0) to skip more often for speed. */

		for(i = n_spr_ndx, j = &buf1[s_spr_ndx+n_spr_ndx-1],
			k = &buf2[s_spr_ndx+n_spr_ndx]; i > 0; i--)
		    {
			if((m = *j) != 0)
			{
				*k = *k | m;
				k = k - 2;
				*k = *k | m;
				k++;
				*k = *k | ((m << 1) | (m >> 1));
			}
			else
			{
				k--;
			}
			j--;
		}
		/* and with mz and back to buf1 */
		for(i = n_spr_ndx+2, o = &mz[s_spr_ndx-1], 
			j = &buf1[s_spr_ndx-1],
				k = &buf2[s_spr_ndx-1]; i > 0; i--)
		    {
			*j = *j ^ (*k & *o);
			o++;
			j++;
			k++;
		}
		if(buf1[s_spr_ndx+n_spr_ndx] != 0)
			n_spr_ndx++;

		if(buf1[s_spr_ndx-1] != 0)
		{
			s_spr_ndx--;
			n_spr_ndx++;
		}
		pass_through_calc++; 
		if(pass_through_calc & 1)
		{
			get_time();
			dsplay_time();
			move_p();
			if(contact == 1) return;
			move_c();
			if(contact == 1) return;
			fflush(stdout);		/* flush the output buffer */
		}
	}
	/*set_c_dir();*/
	prog++;
	nbrcalcs++;
}


int	quick_chk()
{
	register unsigned long *j;
	int i,k,l;
	l = 0;
	k = 0;
	for(i=0;i < nbr_computers; i++)
	{
	j = &buf1[q_chk_ndx[i]];
	if(found_this_computer[i] == 0)
	{
	if((q_chk[i] & (*j++ | *j++ | *j++)))
	{
	set_c_dir(i);
	found_this_computer[i] = 1;
	k++;
	}
	}
	else
	{
	k++;
	}
	}
	if((k == nbr_computers) || contact)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/*
       7 0 4
       2   3
       6 1 5
*/
int set_c_dir(computer)
int computer;
{
	register unsigned long i;
	register unsigned long j;
	register unsigned long *k;

	k = &buf1[c_cp_col[computer]];
	cp_dir[computer] = 0;
	i = row_tbl[c_cp_row[computer]];
	j = i;
	if(((i >> 1) & *k) != 0) return; 	/* dir 0 */
	cp_dir[computer]++;
	if(((i << 1) & *k) != 0) return; 	/* dir 1 */
	k--;
	cp_dir[computer]++;
	if(((i) & *k) != 0) return; 		/* dir 2 */
	k++;
	k++;
	cp_dir[computer]++;
	if(((i) & *k) != 0) return;			/* dir 3 */
	cp_dir[computer]++;
	if(((i >> 1) & *k) != 0) return;	/* dir 4 */
	cp_dir[computer]++;
	if(((i << 1) & *k) != 0) return;	/* dir 5 */
	k--;
	k--;
	cp_dir[computer]++;
	if(((i << 1) & *k) != 0) return;	/* dir 6 */
	cp_dir[computer]++;
	if(((i >> 1) & *k) != 0) return;	/* dir 7 */
}

int chk_4_contact()
{
	int i;
	for(i=0;i < nbr_computers;i++)
	{
	if((pp_row == cp_row[i]) && (pp_col == cp_col[i]))
		contact = 1;
	}
	return (contact);
}


/* pp_dir set to the following numbers causes the following vector 

     7 0 4
     2   3
     6 1 5
   Keystrokes interpreted to set these vectors are in the set_p_direction
   string.  returns keystroke if valid key, 0 if not.
*/

set_p_direction(c)
register char c;
{
	static char *dir = "82469317";
	register int i;
	for(i=7; i >= 0;i--)
	{
		if(c == dir[i])
		{
			pp_dir = i;
			return c;
		}
	}
	return 0;
}
/*	print instructions */

char *inst[] = 
{
"Maze is a game against the computer.  Your job is to eat pellets.  The",
"computers job is to eat you!.  You start out with 3 lives and gain ",
"another each time you clean a room of pellets which also advances you",
"to the next room or level.",
" ",
"Your score is the number of pellets you have eaten.",
" ",
"Your player is controlled by the numeric pad characters:",
" ",
"                              7 8 9",
"                              4   6",
"                              1 2 3",
" ",
"For instance, the 7 key would cause your character to move up and",
"to the left.  The 2 would cause you to go down, the 3 would cause",
"you to go down and to the right, etc.",
" ",
" ",
0
};

instruct()
{
	register char **cpp;

	erase_screen();
	curpos(0,0);

	for( cpp = inst; *cpp != NULL; ++cpp )
	{
		printf( "%s\n", *cpp );
	}
}


char *level_of_play[] = 
{
	" SKILL LEVELS OF PLAY",
	" 1. Novice/beginner",
	" 2. Intermediate",
	" 3. Advanced",
	" 4. Obviously not getting any work done",
	" 5. Strange person",
	" 6. Mentally ill",
	0
};
#define max_skill_level 6

sel_level_of_play()
{
	register char **cpp;
	char resp[20];                         

	erase_screen();
	curpos(0,0);
	playing_at_skill_level = 0;
	for( cpp = level_of_play; *cpp != NULL; ++cpp )
	{
		printf( "%s\n", *cpp );
	}
	do
	{
	curpos(20,0);
	printf("Enter skill level ");
	resp[0] = getchar();
	resp[1] = '\0';
	playing_at_skill_level = atoi(resp);
	}while((playing_at_skill_level < 1) || 
		(playing_at_skill_level > max_skill_level ));

}



unsigned long mz1[max_col+2] =
{
	0x000000,					/* guard column */
	0x0,		/* 0 */
	0x3ffff6,		/* 1 */
	0x3fbffe,		/* 2 */
	0x30000a,		/* 3 */
	0x37bfea,		/* 4 */
	0x37bfea,		/* 5 */
	0x34a02a,		/* 6 */
	0x36bfaa,		/* 7 */
	0x36bfaa,		/* 8 */
	0x3280aa,		/* 9 */
	0x36eeaa,		/* 10 */
	0x36eeaa,		/* 11 */
	0x36eaaa,		/* 12 */
	0x32aaaa,		/* 13 */
	0x1ebbba,		/* 14 */
	0x3ebbba,		/* 15 */
	0x300002,		/* 16 */
	0x3ff7fe,		/* 17 */
	0x3ff7fe,		/* 18 */
	0x3fbefe,		/* 19 */
	0x3fbefe,		/* 20 */
	0x200802,		/* 21 */
	0x2febba,		/* 22 */
	0x2febba,		/* 23 */
	0x2feaaa,		/* 24 */
	0x282aaa,		/* 25 */
	0x2faaaa,		/* 26 */
	0x2faaaa,		/* 27 */
	0x20aaaa,		/* 28 */
	0x36aaaa,		/* 29 */
	0x3eaaaa,		/* 30 */
	0x3eaaaa,		/* 31 */
	0x2aaaaa,		/* 32 */
	0x2aaaaa,		/* 33 */
	0x2aaaaa,		/* 34 */
	0x2aaaaa,		/* 35 */
	0x2aaaaa,		/* 36 */
	0x2aaaaa,		/* 37 */
	0x2aaaae,		/* 38 */
	0x2aaaee,		/* 39 */
	0x2aaaec,		/* 40 */
	0x2bbeee,		/* 41 */
	0x2bbeae,		/* 42 */
	0x200002,		/* 43 */
	0x3ffffe,		/* 44 */
	0x3ffffe,		/* 45 */
	0x3ffffe,		/* 46 */
	0x200002,		/* 47 */
	0x2fbefa,		/* 48 */
	0x2fbefa,		/* 49 */
	0x2db6da,		/* 50 */
	0x2db6da,		/* 51 */
	0x2db6da,		/* 52 */
	0x2db6da,		/* 53 */
	0x2db6da,		/* 54 */
	0x2db6da,		/* 55 */
	0x2db6da,		/* 56 */
	0x2db6da,		/* 57 */
	0x2db6da,		/* 58 */
	0x2db6da,		/* 59 */
	0x2db6da,		/* 60 */
	0x2db6da,		/* 61 */
	0x2db6da,		/* 62 */
	0x2db6da,		/* 63 */
	0x2db6da,		/* 64 */
	0x2db6da,		/* 65 */
	0x2db6da,		/* 66 */
	0x2db6da,		/* 67 */
	0x2db6da,		/* 68 */
	0x2db6da,		/* 69 */
	0x2db6da,		/* 70 */
	0x2db6da,		/* 71 */
	0x2db6da,		/* 72 */
	0x2db6da,		/* 73 */
	0x2db6da,		/* 74 */
	0x2df7da,		/* 75 */
	0x3df7de,		/* 76 */
	0x3df7de,		/* 77 */
	0x3df7de,		/* 78 */
	0x0,		/* 79 */
	0x000000
};


unsigned long mz2[max_col+2] =
{
	0x000000,					/* guard column */
	0x0,		/* 0 */
	0x3ffffe,		/* 1 */
	0x3ffffe,		/* 2 */
	0x3ffffe,		/* 3 */
	0x3ffffe,		/* 4 */
	0x3ffffe,		/* 5 */
	0x3ffffe,		/* 6 */
	0x3ffffe,		/* 7 */
	0x3ffffe,		/* 8 */
	0x3ffffe,		/* 9 */
	0x3ffffe,		/* 10 */
	0x3ffffe,		/* 11 */
	0x3ffffe,		/* 12 */
	0x3ffffe,		/* 13 */
	0x3ffffe,		/* 14 */
	0x3ffffe,		/* 15 */
	0x3ffffe,		/* 16 */
	0x3ffffe,		/* 17 */
	0x3ffffe,		/* 18 */
	0x3ffffe,		/* 19 */
	0x3ffffe,		/* 20 */
	0x3ffffe,		/* 21 */
	0x3ffffe,		/* 22 */
	0x3ffffe,		/* 23 */
	0x3ffffe,		/* 24 */
	0x3ffffe,		/* 25 */
	0x3ffffe,		/* 26 */
	0x3ffffe,		/* 27 */
	0x3ffffe,		/* 28 */
	0x3ffffe,		/* 29 */
	0x3ffffe,		/* 30 */
	0x3ffffe,		/* 31 */
	0x3ffffe,		/* 32 */
	0x3ffffe,		/* 33 */
	0x3ffffe,		/* 34 */
	0x3ffffe,		/* 35 */
	0x3ffffe,		/* 36 */
	0x3ffffe,		/* 37 */
	0x3ffffe,		/* 38 */
	0x3ffffe,		/* 39 */
	0x3ffffe,		/* 40 */
	0x3ffffe,		/* 41 */
	0x3ffffe,		/* 42 */
	0x3ffffe,		/* 43 */
	0x3ffffe,		/* 44 */
	0x3ffffe,		/* 45 */
	0x3ffffe,		/* 46 */
	0x3ffffe,		/* 47 */
	0x3ffffe,		/* 48 */
	0x3ffffe,		/* 49 */
	0x3ffffe,		/* 50 */
	0x3ffffe,		/* 51 */
	0x3ffffe,		/* 52 */
	0x3ffffe,		/* 53 */
	0x3ffffe,		/* 54 */
	0x3ffffe,		/* 55 */
	0x3ffffe,		/* 56 */
	0x3ffffe,		/* 57 */
	0x3ffffe,		/* 58 */
	0x3ffffe,		/* 59 */
	0x3ffffe,		/* 60 */
	0x3ffffe,		/* 61 */
	0x3ffffe,		/* 62 */
	0x3ffffe,		/* 63 */
	0x3ffffe,		/* 64 */
	0x3ffffe,		/* 65 */
	0x3ffffe,		/* 66 */
	0x3ffffe,		/* 67 */
	0x3ffffe,		/* 68 */
	0x3ffffe,		/* 69 */
	0x3ffffe,		/* 70 */
	0x3ffffe,		/* 71 */
	0x3ffffe,		/* 72 */
	0x3ffffe,		/* 73 */
	0x3ffffe,		/* 74 */
	0x3ffffe,		/* 75 */
	0x3ffffe,		/* 76 */
	0x3ffffe,		/* 77 */
	0x3ffffe,		/* 78 */
	0x0,		/* 79 */
	0x000000
};


unsigned long mz3[max_col+2] =
{
	0x000000,					/* guard column */
	0x0,		/* 0 */
	0x3ffffe,		/* 1 */
	0x3ffffe,		/* 2 */
	0x200002,		/* 3 */
	0x2ffffa,		/* 4 */
	0x2ffffa,		/* 5 */
	0x28080a,		/* 6 */
	0x2bffea,		/* 7 */
	0x2bffea,		/* 8 */
	0x2a002a,		/* 9 */
	0x2affaa,		/* 10 */
	0x2affaa,		/* 11 */
	0x2a88aa,		/* 12 */
	0x2abeaa,		/* 13 */
	0x2abeaa,		/* 14 */
	0x2aa2aa,		/* 15 */
	0x2aaaaa,		/* 16 */
	0x2aaaaa,		/* 17 */
	0x2aaaaa,		/* 18 */
	0x2aaaaa,		/* 19 */
	0x2aaaaa,		/* 20 */
	0x2aaaaa,		/* 21 */
	0x2aaaaa,		/* 22 */
	0x2aaaaa,		/* 23 */
	0x2aaaaa,		/* 24 */
	0x2aaaaa,		/* 25 */
	0x2aaaaa,		/* 26 */
	0x2aaaaa,		/* 27 */
	0x2aaaaa,		/* 28 */
	0x2aaaaa,		/* 29 */
	0x2aaaaa,		/* 30 */
	0x2aaaaa,		/* 31 */
	0x2aaaaa,		/* 32 */
	0x2aaaaa,		/* 33 */
	0x2aaaaa,		/* 34 */
	0x2aaaaa,		/* 35 */
	0x2aaaaa,		/* 36 */
	0x2aaaaa,		/* 37 */
	0x3bbeee,		/* 38 */
	0x3bbeee,		/* 39 */
	0x3bbeee,		/* 40 */
	0x2aaaaa,		/* 41 */
	0x2aaaaa,		/* 42 */
	0x2aaaaa,		/* 43 */
	0x2aaaaa,		/* 44 */
	0x2aaaaa,		/* 45 */
	0x2aaaaa,		/* 46 */
	0x2aaaaa,		/* 47 */
	0x2aaaaa,		/* 48 */
	0x2aaaaa,		/* 49 */
	0x2aaaaa,		/* 50 */
	0x2aaaaa,		/* 51 */
	0x2aaaaa,		/* 52 */
	0x2aaaaa,		/* 53 */
	0x2aaaaa,		/* 54 */
	0x2aaaaa,		/* 55 */
	0x2aaaaa,		/* 56 */
	0x2aaaaa,		/* 57 */
	0x2aaaaa,		/* 58 */
	0x2aaaaa,		/* 59 */
	0x2aaaaa,		/* 60 */
	0x2aaaaa,		/* 61 */
	0x2aaaaa,		/* 62 */
	0x2aaaaa,		/* 63 */
	0x2aa2aa,		/* 64 */
	0x2abeaa,		/* 65 */
	0x2abeaa,		/* 66 */
	0x2a88aa,		/* 67 */
	0x2affaa,		/* 68 */
	0x2affaa,		/* 69 */
	0x2a002a,		/* 70 */
	0x2bffea,		/* 71 */
	0x2bffea,		/* 72 */
	0x28080a,		/* 73 */
	0x2ffffa,		/* 74 */
	0x2ffffa,		/* 75 */
	0x200002,		/* 76 */
	0x3ffffe,		/* 77 */
	0x3ffffe,		/* 78 */
	0x0,		/* 79 */
	0x000000
};


unsigned long mz4[max_col+2] =
{
	0x000000,					/* guard column */
	0x0,		/* 0 */
	0x3ffffe,		/* 1 */
	0x3ffffe,		/* 2 */
	0x200002,		/* 3 */
	0x2ff7fa,		/* 4 */
	0x2ffffa,		/* 5 */
	0x28080a,		/* 6 */
	0x2bffea,		/* 7 */
	0x2bf7ea,		/* 8 */
	0x2a002a,		/* 9 */
	0x2af7aa,		/* 10 */
	0x2affaa,		/* 11 */
	0x2a88aa,		/* 12 */
	0x2abeaa,		/* 13 */
	0x2ab6aa,		/* 14 */
	0x2aa2aa,		/* 15 */
	0x2aaaaa,		/* 16 */
	0x2aaaaa,		/* 17 */
	0x2aaaaa,		/* 18 */
	0x2aaaaa,		/* 19 */
	0x2aaaaa,		/* 20 */
	0x2aaaaa,		/* 21 */
	0x2aaaaa,		/* 22 */
	0x2aaaaa,		/* 23 */
	0x2aaaaa,		/* 24 */
	0x2aaaaa,		/* 25 */
	0x2aaaaa,		/* 26 */
	0x2aaaaa,		/* 27 */
	0x2aaaaa,		/* 28 */
	0x2aaaaa,		/* 29 */
	0x2aaaaa,		/* 30 */
	0x2aaaaa,		/* 31 */
	0x2aaaaa,		/* 32 */
	0x2aaaaa,		/* 33 */
	0x2aaaaa,		/* 34 */
	0x2aaaaa,		/* 35 */
	0x2aaaaa,		/* 36 */
	0x2aaaaa,		/* 37 */
	0x3bbeee,		/* 38 */
	0x111c44,		/* 39 */
	0x3bbeee,		/* 40 */
	0x2aaaaa,		/* 41 */
	0x2aaaaa,		/* 42 */
	0x2aaaaa,		/* 43 */
	0x2aaaaa,		/* 44 */
	0x2aaaaa,		/* 45 */
	0x2aaaaa,		/* 46 */
	0x2aaaaa,		/* 47 */
	0x2aaaaa,		/* 48 */
	0x2aaaaa,		/* 49 */
	0x2aaaaa,		/* 50 */
	0x2aaaaa,		/* 51 */
	0x2aaaaa,		/* 52 */
	0x2aaaaa,		/* 53 */
	0x2aaaaa,		/* 54 */
	0x2aaaaa,		/* 55 */
	0x2aaaaa,		/* 56 */
	0x2aaaaa,		/* 57 */
	0x2aaaaa,		/* 58 */
	0x2aaaaa,		/* 59 */
	0x2aaaaa,		/* 60 */
	0x2aaaaa,		/* 61 */
	0x2aaaaa,		/* 62 */
	0x2aaaaa,		/* 63 */
	0x2aa2aa,		/* 64 */
	0x2ab6aa,		/* 65 */
	0x2abeaa,		/* 66 */
	0x2a88aa,		/* 67 */
	0x2affaa,		/* 68 */
	0x2af7aa,		/* 69 */
	0x2a002a,		/* 70 */
	0x2bf7ea,		/* 71 */
	0x2bffea,		/* 72 */
	0x28080a,		/* 73 */
	0x2ffffa,		/* 74 */
	0x2ff7fa,		/* 75 */
	0x200002,		/* 76 */
	0x3ffffe,		/* 77 */
	0x3ffffe,		/* 78 */
	0x0,		/* 79 */
	0x000000
};


unsigned long mz5[max_col+2] =
{
	0x000000,					/* guard column */
	0x0,		/* 0 */
	0x2,		/* 1 */
	0x3ffffe,		/* 2 */
	0x200002,		/* 3 */
	0x200002,		/* 4 */
	0x2ffffa,		/* 5 */
	0x28080a,		/* 6 */
	0x28080a,		/* 7 */
	0x2bffea,		/* 8 */
	0x2a002a,		/* 9 */
	0x2a002a,		/* 10 */
	0x2affaa,		/* 11 */
	0x2a88aa,		/* 12 */
	0x2a88aa,		/* 13 */
	0x2abeaa,		/* 14 */
	0x2aa2aa,		/* 15 */
	0x2aa2aa,		/* 16 */
	0x2aaaaa,		/* 17 */
	0x2aaaaa,		/* 18 */
	0x2aaaaa,		/* 19 */
	0x2aaaaa,		/* 20 */
	0x2aaaaa,		/* 21 */
	0x2aaaaa,		/* 22 */
	0x2aaaaa,		/* 23 */
	0x2aaaaa,		/* 24 */
	0x2aaaaa,		/* 25 */
	0x2aaaaa,		/* 26 */
	0x2aaaaa,		/* 27 */
	0x2aaaaa,		/* 28 */
	0x2aaaaa,		/* 29 */
	0x2aaaaa,		/* 30 */
	0x2aaaaa,		/* 31 */
	0x2aaaaa,		/* 32 */
	0x2aaaaa,		/* 33 */
	0x2aaaaa,		/* 34 */
	0x2aaaaa,		/* 35 */
	0x2aaaaa,		/* 36 */
	0x2aaaaa,		/* 37 */
	0x2aaaaa,		/* 38 */
	0x3bbeee,		/* 39 */
	0x3bbeee,		/* 40 */
	0x2aaaaa,		/* 41 */
	0x2aaaaa,		/* 42 */
	0x2aaaaa,		/* 43 */
	0x2aaaaa,		/* 44 */
	0x2aaaaa,		/* 45 */
	0x2aaaaa,		/* 46 */
	0x2aaaaa,		/* 47 */
	0x2aaaaa,		/* 48 */
	0x2aaaaa,		/* 49 */
	0x2aaaaa,		/* 50 */
	0x2aaaaa,		/* 51 */
	0x2aaaaa,		/* 52 */
	0x2aaaaa,		/* 53 */
	0x2aaaaa,		/* 54 */
	0x2aaaaa,		/* 55 */
	0x2aaaaa,		/* 56 */
	0x2aaaaa,		/* 57 */
	0x2aaaaa,		/* 58 */
	0x2aaaaa,		/* 59 */
	0x2aaaaa,		/* 60 */
	0x2aaaaa,		/* 61 */
	0x2aaaaa,		/* 62 */
	0x2aaaaa,		/* 63 */
	0x2aa2aa,		/* 64 */
	0x2aa2aa,		/* 65 */
	0x2abeaa,		/* 66 */
	0x2a88aa,		/* 67 */
	0x2a88aa,		/* 68 */
	0x2affaa,		/* 69 */
	0x2a002a,		/* 70 */
	0x2a002a,		/* 71 */
	0x2bffea,		/* 72 */
	0x28080a,		/* 73 */
	0x28080a,		/* 74 */
	0x2ffffa,		/* 75 */
	0x200002,		/* 76 */
	0x200002,		/* 77 */
	0x3ffffe,		/* 78 */
	0x0,		/* 79 */
	0x000000
};


unsigned long *mz_adrs[] = {
	mz1,
	mz5,
	mz3,
	mz4,
	mz2,
	0};


cp_mz(mz_adr)
unsigned long *mz_adr;
{
	int i;

	for(i=0;i< max_col+2;i++)
	{
		mz[i] = *mz_adr++;
	}
}


