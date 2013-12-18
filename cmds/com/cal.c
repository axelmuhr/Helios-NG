/*{{{  Header */

/***********************************************************************/
/*                            Calender Program:                        */
/*                            ~~~~~~~~~~~~~~~~~                        */
/*	This program prints out a calender for either an entire year   */
/* or for one month of a given year.                                   */
/*	Both the year and the month must be given as numbers.          */
/* eg - cal 8 1988 	August 1988                                    */
/*     	cal 1700        1700                                           */
/*	The year must be greater than 0, and the month must be between */
/* 1 and 12.                                                           */
/*                                                                     */
/* Program Steps -                                                     */
/* ---------------                                                     */
/*	i)	Check the input.                                       */
/*	ii)	Set up the base date using the given year.             */
/*	iii)	Work out the first day of the required month or for    */
/*		every month if no month was specified.                 */
/*	iv)	Set the flags so that the program either prints out    */
/*		all the months in batches of 3 or prints out the       */
/*		required month.                                        */
/*	v)	Print out the calender.                                */
/*                                                                     */
/* Special Cases -                                                     */
/* ---------------                                                     */
/*	In 1752 the month of September lost 11 days when the day       */
/* following September 2nd was September 14th.                         */
/*	This must be taken into account when calculating the first     */
/* days, and when printing out the calender.                           */
/*                                                                     */
/*	The rule for leap years also depends on the year 1752.  Before */
/* then every year which could be divided by 4 was a leap year.  After */
/* 1752 the rule became every year which could be divided by 4 and not */
/* by 100 was a leap year, except that if a year could be divided by   */
/* 400 it was a leap year.                                             */
/*                                                                     */
/* Copyright (c) 1994 by Perihelion Software Ltd.		       */
/*								       */
/***********************************************************************/

#ifdef __TRAN
static char *rcsid = "$Header: /hsrc/cmds/com/RCS/cal.c,v 1.7 1994/03/09 16:34:59 nickc Exp $";
#endif

/*}}}*/
/*{{{  Includes */

# include <stdio.h>

/*}}}*/
/*{{{  Constants */

# define  TRUE   1
# define  FALSE  0

# define  FEB    1
# define  SEP    8

/*}}}*/
/*{{{  Types */

typedef enum
  {
    SUN, MON, TUE, WED, THU, FRI, SAT
  }
DAYS;

typedef union set_ind
  {
    DAYS 	D;
    int 	I;
  }
INDEX;

typedef struct
  {
    char *	name;		/* Name of the month                       */
    int 	days;		/* Number of days in the month             */
    int 	total;		/* Total days in the year before the month */
  }
MONTH_DATA;

/*}}}*/
/*{{{  Variables */

extern DAYS 	Base; 		/* Base day  from which the first day(s) are calculated */
extern int 	Base_year;	/* Base year from which the first day(s) are calculated */
extern int 	Year;		/* Input Year  */
extern int 	Month;		/* Input Month */

extern MONTH_DATA Months[ 12 ];	/* Data for each month of the year */

extern int 	No_month;	/* Flag : Set if no month was specified */
extern int 	Leap_year;	/* Flag : Set if the specified year is a leap year */


MONTH_DATA Months[12] =  /* Initialize months */
  {
    { "January",   31,   0 },
    { "February",  28,  31 },
    { "March",     31,  59 },
    { "April",     30,  90 },
    { "May",       31, 120 },
    { "June",      30, 151 },
    { "July",      31, 181 },
    { "August",    31, 212 },
    { "September", 30, 243 },
    { "October",   31, 273 },
    { "November",  30, 304 },
    { "December",  31, 334 }
};


/* Initialize extern variables - */

int 	Year      = 0;
int 	Month     = 0;
int 	Base_year = 0;
DAYS 	Base      = SAT;
int 	No_month  = TRUE;
int 	Leap_year = TRUE;

/*}}}*/
/*{{{  Code */

/*{{{  set_base() */

/* 	The program uses the following base days and years - 	*/
/*		1st January  Year 1  -  Saturday                */
/*		1st January  Year 1000  -  Monday               */
/* 	The program also uses the fact that from the year 2000  */
/* onwards the first day of each millenia cycles between a      */
/* Saturday and a Wednesday.                                    */
/* ie: 		1st January 2000/4000/...  -  Saturday          */
/*		1st January 3000/5000/...  -  Wednesday         */

void
set_base( void )
{
  int temp;
  

  if ( Year < 1000 )		/* 1/1/1 = Saturday */
    {
      Base      = SAT;
      Base_year = 1;
    }
  else if ( Year < 2000 )		/* 1/1/1000 = Monday */
    {
      Base      = MON;
      Base_year = 1000;
    }
  else
    {
      /* Work out the nearest, previous millenia - */

      temp = (int)( Year / 1000 );
      
      if ( temp % 2 == 0 )
	Base = SAT;
      else
	Base = WED;
      
      Base_year = temp * 1000;
    }

  return;
  
} /* set_base */

/*}}}*/
/*{{{  make_title() */

/* 	make_title() produces the header for the beginning of each month. */
/* The title is 20 characters long and is padded with spaces at each end  */
/* so that when printed the title is over the centre of the calender      */
/* printed for each month.                                                */

void
make_title(
	   int 	m,		/* required month - 0 = Jan, 1 = Feb, ... , 11 = Dec */
	   char title[ 21 ] )	/* character string to be returned */
{
  int strlen( char* );
  int strcpy( char*, char* );
  int strcat( char*, char* );
  
  int 	m_len;
  int 	i;
  char 	year[ 5 ];

  
  strcpy( title, "" );
  
  sprintf( year, " %04d", Year );
  
  m_len = strlen( Months[ m ].name );
  
  if ( !No_month )
    m_len += strlen( year );
  
  for ( i = 0; i < (int)((20 - m_len) / 2); ++i )
    strcat( title, " " );
  
  strcat( title, Months[ m ].name );
  
  if ( !No_month )
    strcat( title, year );
  
  for (i += m_len ; i < 20; ++i )
    strcat( title, " " );

  return;
  
} /* make_title */

/*}}}*/
/*{{{  print_hdr() */

/* 	The header printed depends on whether a month was specified */
/* if not, the header is of the form -                              */
/*			November                                    */
/* 	 	  Su Mo Tu We Th Fr Sa                              */
/* Otherwise, it is of the form -                                   */
/*		     November 1988                                  */
/*		  Su Mo Tu We Th Fr Sa                              */
/*                                                                  */
/*	If no month was specified the year to be printed is put at  */
/* the top of the calender.                                         */

void
print_hdr( void )
{
  int 	i; 
  char 	title[ 21 ];

  
  if ( No_month )
    {
      printf("\n");
      
      if ( Month == 0 )
	printf("\n\t\t\t       %04d\n\n", Year );
      
      for ( i = 0; i < 3; ++i )
	{
	  make_title( Month + i, title );
	  
	  printf("%s   ", title );
	}
      
      printf("\n");
      
      for ( i = 0; i < 3; ++i )
	printf("Su Mo Tu We Th Fr Sa   ");
    }
  else
    {
      make_title( Month, title );
      
      printf("%s\n", title );
      printf("Su Mo Tu We Th Fr Sa   ");
    }

  printf("\n");

  return;
  
} /* print_hdr */

/*}}}*/
/*{{{  not_end() */

/*	not_end() checks whether the count passed to it exceeds the */
/* number of days in the month passed to it.  This may depend on    */
/* which mont was passed, and whether the year is a leap year.      */

int
not_end(
	int 	cnt,
	int 	mnth )
{
  int o;

  
  if ( mnth == FEB && Leap_year )
    o = 2;
  else
    o = 1; 
  
  if ( cnt < Months[mnth].days + o )
    return TRUE;
  else
    return FALSE;

} /* not_end */

/*}}}*/
/*{{{  unfinished() */

/*	unfinished checks whether all the counts passed to it exceeds */
/* the number of days in the months currently being printed out.  The */
/* number of checks to be made depends on whether a month was given.  */
/*	It does this by calling not_end() repeatedly for each month.  */

int
unfinished( int cnt[] )
{
  int i;
  int times;

  
  if ( No_month )
    times = 3;
  else
    times = 1;
  
  for( i = 0; i < times; ++i )
    {
      if ( not_end( cnt[i], Month + i ) )
	return TRUE;
    }

  return FALSE;

} /* unfinished */

/*}}}*/
/*{{{  print_days() */

/*	print_days() prints out the calenders for either 1 or 3 months */
/* depending on whether a month was specified or not.                  */
/* 	It uses three nested loops which, from the inside out, perform */
/* the following tasks and tests -                                     */ 
/* 	i)	prints out one line for each month                     */
/*		ends when - one line has been printed                  */
/*		  or when - the calender for that month has been       */
/*			    printed                                    */
/*	ii)	this outside loop terminates when the inside loop has  */
/*		printed the calender for one week of either 1 month    */
/* 		or 3 months                                            */
/*	iii)	the outer loop terminates when the whole calender for  */
/*		the required number of months has been printed         */
/*	To print the calender an array of day counts is initialized to */
/* 1, and the each one is incrementedn as a date is printed out.       */

void
print_days( int offset[] )
  /* offset keeps a track of how far along a line for */
  /* each month has been printed.                     */
  /* The days have been set up so that the first days */
  /* of each month which it initially holds is also   */
  /* the number of times spaces must be printed so    */
  /* that the calender starts in the correct place.   */
{
  int 	day_cnt[ 3 ];	/* day_counts for each month to be printed */
  int 	num;		/* the number of months to be printed */
  int 	i, j;		/* Loop indexes */
  

  /* Work out the number of months to be printed */

  if ( No_month )
    num = 3;
  else
    num = 1; 
  
  /* Initialize the day counts for the 3 months to be printed */

  for ( i = 0; i < 3; ++i )
    day_cnt[i] = 1;
  
  /* while the day counts are still less than the number of dates */
  /* to be printed -                                              */

  while ( unfinished( day_cnt ) )
    {
      /* print out each block of seven days for each month */
      /* to be printed -                                   */
      
      for ( i = 0; i < num; ++i )
	{
	  /* pad with spaces until the required day is */
	  /* reached -                                 */
	  
	  for ( j = 0; j < offset[ Month + i ]; ++j )
	    printf("   ");
	  
	  /* print a line for each month - */
	  
	  while ( offset[ Month + i ] < 7 && not_end( day_cnt[ i ], Month + i ) )
	    {
	      /* check special case (Sept 1752) - */
	      
	      if ( day_cnt[ i ] == 3 && Year == 1752 && Month + i == SEP )
		day_cnt[i] += 11;
	      
	      /* print extra space if day count < 10 - */
	      
	      if ( day_cnt[ i ] < 10 )
		printf(" ");
	      
	      /* print out the date - */
	      
	      printf( "%d ", day_cnt[ i ] );
	      
	      /* Increment day count and the offset */
	      
	      ++day_cnt[ i ];
	      ++offset[ Month + i ];
	    }
	  
	  /* when all the dates for a particular month */
	  /* have been printed, this ensures that the  */
	  /* lines for the next month start in the     */
	  /* correct place.                            */
	  
	  for ( ; offset[ Month + i ] < 7; ++offset[ Month + i ] )
	    printf("   ");
	  
	  printf("  ");
	  
	  /* Reset the offset */
	  
	  offset[ Month + i ] = 0;
	}
      
      /* newline required */
      
      printf( "\n" );
    }

  return;
  
} /* print_days */ 

/*}}}*/
/*{{{  is_leap_year() */

/* 	is_leap_year() checks whether a given year is a leap year, using */
/* the rules given in the header comments.                               */

int
is_leap_year( int year )
{
  /* Check for leap year */

  if ( year <= 1752 )
    {
      if ( year % 4 == 0 )
	return TRUE;
      else
	return FALSE;
    }
  else
    {
      if ( year % 4 == 0 && year % 100 != 0 )
	return TRUE;

      if ( year % 400 == 0 )
	return TRUE;
      else
	return FALSE;
    }
  
} /* is_leap_year */

/*}}}*/
/*{{{  first_day() */

/* 	first_day() calculates the first day of the given year by using   */
/* the base calculated by set_base().  It does this by using a loop to    */
/* go from the base year to the given year.  For each year it adds the    */
/* number of days in the year to a count and the takes the modulus when   */
/* this is divided by 7.  The DAYS have been set up so that this results  */
/* in the first day of the given year being returned.                     */
/*	The special case of Sept 1752 must be taken into account, as must */
/* leap years.  It calls the function is_leap_year() to test each year in */
/* the loop.                                                              */
/*	If a month was specified it returns the first day of the given    */
/* month.  ( If no month was given, Month = 0 )                           */

DAYS
first_day( void )
{
  INDEX 	day;	/* count variable */
  int 		i;	/* loop index */

  
  /* Initialize day to the base day - */

  day.D = Base;
  
  /* starting from the base year, work up to the given year - */
  
  for( i = Base_year; i < Year; ++i )
    {
      /* 1752 is a special case */
      
      if ( i == 1752 )
	day.I+= 354;
      else
	day.I+= 365;
      
      /* check for leap year - */
      
      if( is_leap_year( i ) )
	++day.I;
      
      /* take the modulus - */
      
      day.I %= 7;
    }

  /* add on the number of days to the start of the given month - */
  /* ( NB : Months[Month].total = 0 )                            */
  
  day.I += Months[Month].total;
  
  /* check for the special cases of leap years and 1752 - */
  
  if ( is_leap_year( Year ) && Month > FEB )
    ++day.I;
  
  if ( i == 1752 && Month > SEP )
    day.I -= 11;

  /* take the modulus - */
  
  day.I %= 7;
  
  return day.D;
}

/*}}}*/
/*{{{  print_cal() */

/* 	Before the calender can be printed out, the program must work */
/* out how many times print_days() must be called and also what are   */
/* the first day(s) of the required month(s).                         */

void
print_cal( void )
{
  int times;		/* How many times must print_days() be called */
  int offset[12];	/* The start day is equivalent to the number */
  			/* of spaces which must be printed before    */
  			/* the number 1 is printed                   */
  int i;		/* Loop index */
  

  if ( No_month )
    {
      /* print_days() must be called 4 times */
      
      times = 4;
      
      /* Calculate the offset (ie: first days) of every month */
      
      offset[ 0 ] = first_day();
      
      for ( i = 1; i < 12; ++i )
	{
	  /* offset[i] = offset[i-1] + Months[i].days + 1;*/
	  
	  offset[ i ] = offset[ i - 1 ] + Months[ i - 1 ].days;
	  
	  if ( Leap_year && i-1 == FEB )
	    ++offset[ i ];
	  
	  if (Year == 1752 && i-1 == SEP )
	    offset[ i ] -= 11;
	  
	  offset[i] %= 7;
	}
    }
  else
    {
      /* print_days() must only be called once */
      
      times = 1;
      
      /* Calculate the offset (ie: first day) of the given month */
      
      offset[ Month ] = first_day();
    }
  
  /* Call print_hdr() and print_days() the required number of times */

  for ( i = 0; i < times; ++i, Month += 3 )
    {
      print_hdr();

      print_days( offset );
    }

  return;
  
} /* print_cal */

/*}}}*/
/*{{{  main() */

int
main(
     int 	argc,
     char *	argv[] )
{
  /* Assign values to Year and Month - */

  if ( argc == 2 )
    {
      sscanf( argv[ 1 ], "%d", &Year );

      Month    = 1;
      No_month = TRUE;
    }
  else if ( argc == 3 )
    {
      sscanf( argv[ 1 ], "%d", &Month );

      sscanf( argv[ 2 ], "%d", &Year );
      
      No_month = FALSE;
    }

  /* Set Leap_year flag */
  
  Leap_year = is_leap_year( Year );

  /* Check input - */
  
  if ( argc != 2 && argc != 3 )
    {
      fprintf( stderr, "Usage : cal [month] year  -  " );
      fprintf( stderr, "both to be specified by number\n" );
    }
  else if ( Month < 1 || Month > 12 )
    {
      fprintf( stderr, "Bad argument - month out of range\n" );
    }
  else if ( Year < 1 )
    {
      fprintf( stderr, "Bad argument - year out of range\n" );
    }
  else
    {
      --Month;		/* Adjust Month to lie in the range 0 - 11 */

      set_base();	/* Set the base day and year */

      print_cal();	/* Print the calender */
    }

  return 0;
  
} /* main */

/*}}}*/

/*}}}*/

/* end of cal.c */
