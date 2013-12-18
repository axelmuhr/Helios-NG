/*
 *  	The program takes a command line as input. It then splits it into  the
 *  separate associated conditions and commands storing them in a list.  It
 *  checks the arguments are valid while setting up the list.
 * 	After creating the list the program recursively descends the diretory
 *  hierarchy specified in the pathname-list.  For each file found the program
 *  steps down the linked list to see if any of the conditions match.  If they
 *  do it carries out the commands in the same element of the list.
 */
#ifdef __TRAN
static char *rcsid = "$Header: /users/nickc/RTNucleus/cmds/com/RCS/find.c,v 1.9 1994/03/14 14:49:10 nickc Exp $";
#endif

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <sys/time.h>

#include <memory.h>
#include <string.h>

/* List element */
struct oper_data{
	char **conditions;
	char **commands;
	struct oper_data *next;
};

typedef struct oper_data ELEMENT;
typedef ELEMENT *LINK;


/* Posix files - used to open and read directories */
#ifdef UNIX
#include <sys/dir.h>
#define DIRECT direct
#endif

#ifdef __HELIOS
#include <time.h>
#undef CLK_TCK
#include <dirent.h>
#define DIRECT dirent 
#endif

/* Function return values */

#define ERROR    2
#define OK       3
#define DEFAULT  4
#define STOP     5

/* Error values */

#define NO_PAR     1
#define UNREC_ARG  2
#define USAGE      3
#define BAD_BRACK  4
#define NO_PATH    5
#define CANT_OPEN  6
#define BAD_OR     7

/* Types of strings in the input line */

#define COND     1
#define COMM     2
#define END      3
#define UNKNOWN  4

#define ACC_TM  1	/* access time   */
#define MOD_TM  2	/* modified time */

/* List head */

LINK Head = NULL;

/* Pathname-list head */

char **Path_list = NULL;

/* Current pathname */

char *Buffer;

/* No. of allowable conditions & commands */

int Cnd_num = 12;
int Cmm_num = 4;

/* Allowable conditions and commands */

char *Conditions[12] = { "-name", "-links", "-size", "-inum", "-atime", 
		         "-mtime", "-type", "!", "-o", "(", ")", "-newer" };
char *Commands[4] = { "-exec", "-ok", "-print", "-cont" };

char Name[50];

/*
 *	search_dir() shouldn't return an error since it should be trapped
 *  in set_up().  The check was made for debugging purposes.
 */
 
#include "find.h"

void lfree ( LINK p )
{

/* Safely assumes atleast one item on Head */

	if( p->next != NULL)
		lfree( p->next );
	free (p);

}



/*
 *	set_up() first calls make_path_list() to create the pathname-list,
 *  checking afterwards that a pathname-list was created.
 *	It then goes down the rest of the input line, placing all conditions
 *  into the list until it finds a command, then placing all commands into 
 *  the list until it reaches a condition.  The function examine() returns
 *  either COND - condition, COMM - command, END - end of the input line or
 *  UNKNOWN - anything else. 
 * 	If examine() returns UNKNOWN, the error function print_error() is
 *  called and the program will halt.
 */

int set_up(
	   int argc,
	   char **argv )
{
	LINK add_to_list( char** , char** , LINK );

	char **conds, **comms, **pntr;  /* pointers to condition and command */
					/* lists, and a temp pointer used to */
					/* set them up.                      */
	int result;	/* result of a called function */
	int s_len;	/* length of a condition.  Used to tell if   */
			/* the condition is '!', '(', ')' or '-o' in */
			/* which take no arguments.                  */
	int bracket_cnt = 0;	/* keeps a track of the number of open   */
				/* brackets there are still to be closed */

	/* Place program name into 'name' */
	strcpy( Name, *argv );

	/* Check for no input line */
	if( argc == 1 )
		return( print_error( USAGE, "" ) );

	/* Create the pthname-list */
	result = make_path_list( argc, argv );

	/* Reset argv and argc */
	argv += argc - result;
	argc = result;


	/* Check for no pathname-list */
	if( *Path_list == NULL )
		return( print_error( NO_PATH, "" ) ); 

	/* Get memory for condition and command lists */
	conds = (char **)( malloc( argc * sizeof( *argv ) ) );
	comms = (char **)( malloc( argc * sizeof( *argv ) ) );
	*conds = *comms = NULL;

	/* Loop until argc = 0, ie the end of the list is reached */

	while( argc ){
		/* Initialize temp. pointer to the head of the condition list */
		pntr = conds; 

		/* Loop while argv is a condition */
		while( ( result = examine( *argv ) ) == COND ){
			/* Add the condition to the list */
			*pntr++ = *argv;
			
			/* Check for open or close brackets */
			if( strcmp( *argv, "(" ) == 0 )
				++bracket_cnt;
			if( strcmp( *argv, ")" ) == 0 ){
				if( bracket_cnt == 0 )
					return( print_error( BAD_BRACK, "" ) );
				--bracket_cnt;
			}
			/* Check if the condition takes an argument */
			if( ( s_len = strlen( *argv ) ) != 1 && s_len != 2 ){
				/* Check that a parameter is present */
				if( --argc == 0 )
					return( print_error( NO_PAR, *argv ) );

				/* Add the arguments to the list until */
				/* it is recognized.                   */
				while( examine( *(++argv) ) == UNKNOWN && *argv != NULL ){
					*pntr++ = *argv;
					argc--;
				}
				/* Reset argv, argc */
				argc++, argv--;
			}
			/* Reset argv, argc */
			argc--, argv++;
			*pntr = NULL;
		}

		/* Check for unknown input */
		if( result == UNKNOWN ) {
			return( print_error( UNREC_ARG, *argv ) );
		}
		/* Check bracket_cnt */
		if( bracket_cnt != 0 )
			return( print_error( BAD_BRACK, "" ) );

		/* Initialize temp. pointer to the head of the command list */
		pntr = comms;

		/* Loop while argv is a command */
		while( ( result = examine( *argv ) ) == COMM ){
			/* Add the condition to the list */
			*pntr++ = *argv;

			/* Check if the condition takes an argument */
			if( strcmp( *argv, "-print" ) !=  0 &&
			    strcmp( *argv, "-cont" ) != 0 ){
				/* Check that a parameter is present */
				if( --argc == 0 )
					return( print_error( NO_PAR, *argv ) );

				/* Add the arguments to the list until */
				/* it is recognized.                   */
				while( examine( *(++argv) ) == UNKNOWN && *argv != NULL ){
					*pntr++ = *argv;
					argc--;
				}
				/* Reset argc, argv */
				argc++, argv--;
			}
			/* Reset argc, argv */
			argc--, argv++;
			*pntr = NULL;
		}

		/* Check for unknown input */
		if( result == UNKNOWN ) {
			return( print_error( UNREC_ARG, *argv ) );
		}

		/* Add the conditions and commands to the linked list */
		Head = add_to_list( conds, comms, Head );

		/* Get more memory for condition and command lists */
		conds = (char **)( malloc( argc * sizeof( *argv ) ) );
		comms = (char **)( malloc( argc * sizeof( *argv ) ) );
		*conds = *comms = NULL;
	}
	return OK;
}


/* 
 *	search_dir() first opens the directory passed to it and then examines 
 *  all the files in the directory.  If another directory is found, search_dir()
 *  calls itself.  The parameter pos passed to it is the location in Buffer
 *  where the names of the files being examined start from.  When going down
 *  into another directory pos is incremented so that Buffer holds the full 
 *  pathname of the directory currently being examined.  On coming back up
 *  old pathnames are overwritten.
 *	For each file, search_dir() calls test_conds() which applies the
 *  appropriate commands where necessary.
 */
int search_dir( int pos )
{
	DIR *opendir( char* ), *dirp;
	struct DIRECT *readdir( DIR* ), *dir_info;
	struct stat stbuf;
	int result;

	/* Open the directory */
	if( ( dirp  = opendir( Buffer ) ) == NULL )
		return OK;

	/* Loop until there are no more files to be read */
	while( ( dir_info = readdir( dirp ) ) != NULL ){
		/* Ignore parent */
		if( strcmp( "..", dir_info -> d_name ) != 0 ){

			/* Make up file pathname unless self */
			if( strcmp( ".", dir_info -> d_name ) != 0 )
				form_name( dir_info -> d_name, pos );

			/* test conditions list */
			stat( Buffer, &stbuf );

			result = test_conds( stbuf, dir_info -> d_name, Head );

			if( result == ERROR || result == STOP )
				return result;

			/* check if directory */
			if( strcmp( ".", dir_info -> d_name ) != 0 &&
			    ( stbuf.st_mode & S_IFMT ) == S_IFDIR ){
				result = search_dir( strlen( Buffer ) );
				if( result == ERROR || result == STOP )
					return result;
			}
		}
	}
	closedir( dirp );
	return OK;
}


/*
 *  	print_error() is called when an error is detected.  The error message
 *  printed depends on the parameter which, while str holds any other 
 *  information which can be printed to help the user.
 */
int print_error(
		int which,
		char *str )
{
	switch( which ){
	case NO_PAR : /* No paramter for a condition or command */
		fprintf( stderr, "%s : No parameter for %s\n", Name, str );
		break;
	case UNREC_ARG : /* Unknown argument */
		fprintf( stderr, "%s : Unrecognized argument \"%s\"\n", Name, str );
		break;
	case USAGE : /* No command line */
		fprintf( stderr, "Usage : %s <pathname>... [boolean operand expression]\n", Name );
		break;
	case BAD_BRACK : /* Badly placed bracket */
		fprintf( stderr, "%s : Badly placed ()'s\n", Name );
		break;
	case NO_PATH : /* No pathname-list created */
		fprintf( stderr, "%s : No path list\n", Name );
		break;
	case CANT_OPEN : /* Can't open a directory ( This error message */
			 /* may not be checked for )                    */
		fprintf( stderr, "%s : Can't open %s\n", Name, str );
		break;
	}

	return ERROR;
}


/*
 *	make_path_list() creates the pathname-list by using examine() to
 *  tell when a string is a condition or a command or when the end of the
 *  command line is reached.  It assumes all other strings are the pathnames
 *  to be used by the program.            
 */
int make_path_list(
		   int argc,
		   char **argv )
{
	char **pntr;

	/* Get memory for the pathname-list */
	Path_list = pntr = (char **)( malloc( argc * sizeof( *argv ) ) );
	*Path_list = NULL;

	for( argc--, argv++; examine (*argv) == UNKNOWN; argc--, argv++ ){
		/* Add the pathname to the list */
		*pntr++ = *argv;
		*pntr = NULL;
	}
	return argc;
}


/*
 *	examine() checks a string to see whether it is a condition, a command,
 *  a "" or something else.  It uses the global arrays Conditions and Commands
 *  which hold all the allowable conditions and commands.  Cnd_num and Com_num
 *  are the array sizes.
 */
int examine( char *str )
{
	int i;

	if( str == NULL || strcmp( str, "" ) == 0 || strcmp(str, "\n") == 0)
		return END;

	for( i = 0; i < Cnd_num; ++i ){
		if( strcmp( str, Conditions[i] ) == 0 )
			return COND;
	}
	for( i = 0; i < Cmm_num; ++i ){
		if( strcmp( str, Commands[i] ) == 0 )
			return COMM;
	}

	return UNKNOWN;
}


/*
 *	add_to_list() creates a new element of the list using the parameters
 *  passed to it and appends it on the end of the linked list. 
 */
LINK add_to_list(
		 char **conds,
		 char **comms,
		 LINK pntr )
{
	LINK temp;

	if( pntr == NULL ){
		/* Get memory for new element */
		temp = (LINK)( malloc( sizeof( ELEMENT ) ) );

		/* Set up new element */
		temp -> conditions = conds;
		temp -> commands = comms;
		temp -> next = NULL;

		/* Return the address of the new element */
		return temp;
	}
	else{
		pntr -> next = add_to_list( conds, comms, pntr -> next );
		return( pntr );
	}
}


/*
 *	test_conds() recursively goes down the linked list, calling the 
 *  functions check_conds() and do_commands() with the conditions and
 *  commands within each element of the list.  The first checks to see
 *  whether the conditions hold for the file passed to it and, if so 
 *  the second is called to perform the commands.
 */
int test_conds(
	       struct stat file_info,
	       char *name,
	       LINK pntr )
{
	int result;

	if( pntr == NULL )
		return OK;
	else{
		/* Stops the program examining the pathname '.' twice */
		if( strcmp( Buffer, "." ) != 0 && strcmp( name, "." ) == 0 )
			return FALSE;

		/* Check the conditions within the list element */
		result = check_conds( file_info, name, pntr -> conditions, 0 );

		if( result == ERROR )
			return ERROR;
		else if( result == TRUE ){
			/* If necessary perform the commands */
			result = do_commands( pntr -> commands );

			if( result == ERROR || result == STOP )
				return result;
		}

		/* Check the next element of the linked list */
		return( test_conds( file_info, name, pntr -> next ) );
	}
}


/*
 *	do_commands() checks each element of the command list passed to it
 *  and performs the necessary commands.
 */
int do_commands( char **comms )	/* head of the command list */
{
	char **comm_pntr;	/* pointer to the command list */
	char command[80];	/* array for the command to be used */
				/* in system()                      */
	char *temp;		/* temp. holding variable used to     */
				/* concatenate several arguments in   */
				/* list into one string which is then */
				/* passed to form_command()           */
	char c;	/* reply variable used in '-ok' */

	/* get memory for temp */
	temp = (char *)(malloc( 50 * sizeof( char ) ) );

	/* Loop until *comm_pntr = NULL, ie the end of the list */
	for( comm_pntr = comms; *comm_pntr; comm_pntr++ ){
		/* reset command and temp */
		command[0] = '\0';
		*temp = '\0';
		/* check for '-exec' */
		if( strcmp( *comm_pntr, "-exec" ) == 0 ){
			/* concatenate the arguments of '-exec' */
			while( *(++comm_pntr) != NULL &&
				examine( *comm_pntr ) == UNKNOWN ){
				strcat( temp, *comm_pntr );
				strcat( temp, " " );
			}
			/* make up the command to be carried out */
			form_command( temp, command );

			system( command );

			/* reset comm_pntr */
			--comm_pntr;
		}
		/* check for '-ok' */
		else if( strcmp( *comm_pntr, "-ok" ) == 0 ){
			/* concatenate the arguments of '-exec' */
			while( *(++comm_pntr) != NULL &&
			       examine( *comm_pntr ) == UNKNOWN ){
				strcat( temp, *comm_pntr );
				strcat( temp, " " );
			}
			/* make up the command to be carried out */
			form_command( temp, command );

			/* display command */
			printf("< %s ? > ", command );

			/* get input from stdin */
			if( ( c = getchar() ) == 'y' ) {
				/* if reply = 'y', perform the command */	
		
				system( command );
			}

			/* clear buffer */
			while( c != '\n' )
				c = getchar();

			/* reset comm_pntr */
			--comm_pntr;
		}
		/* check for '-cont' */
		else if( strcmp( *comm_pntr, "-cont" ) == 0 ){
			do {
				fprintf( stderr, "Continue ? (y/n) " );
				fflush (stderr);
			} while ( ( c = getchar() ) != 'y' && c != 'n' );

			if( c == 'n' )
				return STOP;

			/* clear buffer */
			while( c != '\n' )
				c = getchar();
		}
		/* check for '-print' */
		else if( strcmp( *comm_pntr, "-print" ) == 0 )
			printf("%s\n", Buffer );
		else {
			return( print_error( UNREC_ARG, *comm_pntr ) );
		}
	}
	free( temp );
	return OK;
}


/*
 *	match() takes two strings and checks whether or not they match.  
 *  Special characters are - 
 *		'*'   -   matches any number of characters
 *		'?'   -   matches any one character
 *              [...] -   matches any of the characters within the brackets
 */
int match(
	  char *str,
	  char *pattern )
{
  int c;

  until ((c = *pattern++) == '\0')
  {
    switch (c)
    {
      default:
      if (*str++ == c) continue;
      return FALSE;

      case '?':
      if (*str++) continue;
      return FALSE;

      case '[':
      until (((c = *pattern++) == ']') || (c == '\0'))
      {
        if (*pattern == '-')
        {
          pattern++;
          if ((*str >= c) && (*str <= *pattern++))
          {
            c = *str;
            break;
          }
        }
        else if (*str == c) break;
      }
      if (*str++ == c)
      {
        until (((c = *pattern++) == ']') || (c == '\0'));
        continue;
      }
      return FALSE; 

      case '*':
      if (*pattern == '\0') return TRUE;
      while (*str)
      {
        if (match(str, pattern)) return TRUE;
        str++;
      }
      return FALSE;
    }
  }
  if (*str) return FALSE;
  return TRUE;
}


/*
 *	form_name() appends the current filename to Buffer at the position
 *  specified to make up the current pathname 
 */
int form_name(
	      char *str,
	      int pos )
{
	/* if Buffer = '/', do not need to append another '/' */
	if( pos == 1 && Buffer[0] == '/' )
		strcpy( Buffer + pos, str );
	else{
		strcpy( Buffer + pos, "/" );
		strcpy( Buffer + pos + 1, str );
	}

	return OK;
}


/*
 *	form_command() makes up the command to be performed by either '-exec'
 *  or '-ok'.  It replces any occurences of '_' or '{}' with the current pathname.
 *  The final command is placed in the array command[] passed to it.
 */ 
int form_command(
		 char *comm,	/* argument string containing the command and '_' */
		 char command[] ) /* array to be used to hold the final command */
{
	int len;	/* length of comm */
	int i, j;	/* positions in comm and command (resp) */
			/* being checked and written to (resp)  */

	/* initialize len - comm has an extra space at the end */
	len = strlen( comm ) - 1;

	/* initialize array positions */
	i = j = 0;

	/* initialize command */
	command[j] = '\0';

	/* loop while i < length of comm */
	while( i < len ){

		/* write the chars in comm to command until '_' is found, */
		/* '{' is found (start of '{}') or the end of comm is     */
		/* reached                         			  */
		for( ; *(comm+i) != '_' && *(comm+i) != '{' && i < len; ++i, ++j )
			command[j] = *(comm+i);
		command[j] = '\0';

		/* if a '_' was found place the current file name */
		/* into command                                   */
		if( *(comm+i) == '_' ){
			++i;
			strcat( command, Buffer );
			j += strlen( Buffer );
		}
		
		/* if a '{' was found, check the following character; */
		/* If it is '}', replace '{}' by the pathname, else   */
		/* add '{' to the command.			      */
		if( *(comm+i) == '{' ){
			++i;
			if( *(comm+i) == '}' ){
				++i;
				strcat( command, Buffer );
				j += strlen( Buffer );
			}
			else
				command[j++] = '{';
		}
		
		command[j] = '\0';
	}

	return OK;
}


/*
 * 	num_check() takes two numbers and returns TRUE or FALSE depending
 *  on the test performed.  The test is specified by the parameter cmp_flg
 */
int num_check(int num1, int num2, char cmp_flg )
{
	switch( cmp_flg ){
	case '+' :
		if( num1 > num2 )
			return TRUE;
		break;
	case '-' :
		if( num1 < num2 )
			return TRUE;
		break;
	default :
		if( num1 == num2 )
			return TRUE;
		break;
	}
	return FALSE;
}


/*
 *	time_check() compares two times.  One taken from the file being 
 *  examined specified by the parameter which, the other being the present
 *  time.  The two times are then compared using the num_check() function
 *  which uses the check specified by cmp_flg
 */
int time_check( struct stat file_info, int days, int which,char cmp_flg)
/* struct stat file_info:Information on the file being examined		*/
/* int days:		Number of days being tested for			*/
/* int which:		Flag indicating which time is to be used        */
/*			( eithr\er last accessed or last modified time )*/
/* char cmp_flg		Which comparison is to be made by num_check()   */
/*			( cmp_flg = '+'  =>  diff > days                */
/*			  cmp_flg = '-'  =>  diff < days                */
/*			  otherwise          diff == days where diff is */
/*			  number of days between the two times used. )	*/
{
	int diff; 		/* difference in days between the two times used */
	unsigned int temp; 	/* temp. variable to hold the times */
	int i;			/* loop variable */	
	int curr_year, curr_yday;	/* current year and day number */
	int file_year, file_yday;	/* year and day number of the file */
					/* time being used                 */
	struct tm *curr_time, *file_time;	/* time structures for the   */
						/* current time and the file */
						/* time                      */
	struct timeval *tv;	/* structure returned by gettimeofday() */

	/* get memory for current time of day */
	tv = (struct timeval *)(malloc( sizeof( struct timeval ) ) );

	/* get current time of day */

#ifdef HELIOS
	time( &(tv->tv_usec) );
#else
	gettimeofday( tv, NULL );
#endif

	/* convert current time of day to struct tm format */
	curr_time = gmtime( (const time_t *)  &(tv -> tv_usec) );

	/* get current year and day number */
	curr_year = curr_time -> tm_year;
	curr_yday = curr_time -> tm_yday;

	/* get relevant time from the file - this will be in seconds */
	if( which == ACC_TM )
		temp = file_info.st_atime;
	else
		temp = file_info.st_mtime;

	/* convert file time of day to struct tm format */
	file_time = gmtime( &temp );

	/* get file year and day number */
	file_year = file_time -> tm_year;
	file_yday = file_time -> tm_yday;

	/* calculate the difference between the two times in days */
	diff = curr_yday - file_yday;

	for( i = file_year; i < curr_year; i++ ){
		diff += 365;
		/* check for leap year */
		if( i % 4 == 0 && i % 100 != 0 )  diff++;
		if( i % 400 == 0 )  ++diff;
	}

	/* compare times */
	if( file_year <= curr_year && num_check( diff, days, cmp_flg ) )
		return TRUE;
	else
		return FALSE;
}


/*
 *	get_num() converts a number in string form to a number using sscanf().
 *  This is made more complicated by the possible addition of another char in
 *  front of the number which specifies which test is to be performed.
 */
int get_num( char * num_str )
{
	int num;

	if( *num_str == '+' )
		sscanf( num_str, "+%d", &num );
	else if ( *num_str == '-' )
		sscanf( num_str, "-%d", &num );
	else
		sscanf( num_str, "%d", &num );
	return num;
}


/*
 * 	type_check() simply checks the flags in the file information passed
 *  to it and returns TRUE or FALSE depending whether the file is of the 
 *  type being checked against.
 */
int type_check(
	       struct stat file_info,
	       char *type )
{
	switch( *type ){
#ifndef HELIOS
	case 'b' :	/* check for block special file */
		if( ( file_info.st_mode & S_IFMT ) == S_IFBLK )
			return TRUE;
		break;
	case 'c' :	/* check for character special file */
		if( ( file_info.st_mode & S_IFMT ) == S_IFCHR )
			return TRUE;
		break;
#endif
	case 'd' :	/* check for directory */
		if( ( file_info.st_mode & S_IFMT ) == S_IFDIR )
			return TRUE;
		break;
	case 'f' :	/* check for ordinary file */
		if( ( file_info.st_mode & S_IFMT ) == S_IFREG )
			return TRUE;
		break;
	}
	return FALSE;
}


/*
 *	get_time() returns the time specified by the parameter flg in the
 *  file information passed to it.
 */
long get_time( struct stat file_info, char flg)
{
	switch( flg ){
	case 'a' :	/* last accessed time */
		return( file_info.st_atime );
	case 'c' :	/* creation time */
		return( file_info.st_ctime );
	default :	/* last modified time */
		return( file_info.st_mtime );
	}
}


/* 
 * 	newer() compares two times from the file being examined and from
 *  another specified file, given as an argument.  The times used are
 *  specified by the two flags t_flg, c_flg.
 */
int newer( struct stat test_file, struct stat cmp_file, char t_flg, char c_flg)
{
	/* check if the test time is greater than the compare time */
	/* ie the test time is later than the compare time         */
	if( get_time( test_file, t_flg ) > get_time( cmp_file, c_flg ) )
		return TRUE;
	else  
		return FALSE;
}


/*
 *	flag_check() examines the str passed to it to set the flags which are
 *  to be used in newer().  It does this by checking whether the string is 
 *  recognized by examine(), and if not assigns the first two chars of the 
 *  string to the two flags.  Otherwise it defaults both flags to 'm'.
 */
int flag_check(
	       char *str,
	       char *flg1,
	       char *flg2 )
{
	if( examine( str ) == UNKNOWN ){
		*flg1 = *str;
		*flg2 = *(str+1);
		return OK;
	}
	else{
		*flg1 = *flg2 = 'm';
		return DEFAULT;
	}
}

/*
 *	check_conds() examines the line of arguments passed to it to see
 *  whether they apply to the file passed to it.  The parameter lvl is
 *  used to check that there are no out of place brackets.
 */
int check_conds(
		struct stat file_info, 	/* Information on the file being checked      */
		char *name,		/* Name of the file being checked             */
		char **conds,		/* Condition list being used                  */
		int lvl )		/* No. of times check_conds() has been called */
{
	char **pntr;	/* pointer to the arguments in the condition list */
	char c1, c2;
	struct stat cmp_info;	/* Information on the file to be compared */
				/* against in the -newer primary          */
	int ret_val = TRUE;	/* Return value */
	int cmp_val = TRUE;	/* Result to be checked for when examining   */
				/* for a particular condition.  Set to FALSE */
				/* by a '!' argument                         */
	int result;		/* Holding variable for the result of a check */
	int which, num, temp;	/* Various variables used to hold values */
				/* while checking a condition            */

	/* Loop until *pntr = NULL, ie the end of the condition list */
	for( pntr = conds; *pntr; pntr++ ){
		/* Check for -o ( OR ) */
		if( strcmp( *pntr, "-o" ) == 0 ){
			/* if ret_val is currently TRUE, then */
			/* the conditions must hold           */
			if( ret_val == TRUE )
				return TRUE;
			else{
				/* otherwise, check that something follows */
				/* the OR                                  */
				if( *(++pntr) == NULL )
					return( print_error( BAD_OR, "" ) );
				--pntr;
				/* reset ret_val */
				ret_val = TRUE;
			}
			/* reset cmp_val */
			cmp_val = TRUE;
		}
		/* check for '(' */
		else if( strcmp( *pntr, "(" ) == 0 ){
			/* increment pointer over the '(' and - */
			++pntr;
			/* call check_conds() again */
			result = check_conds( file_info, name, pntr, lvl + 1 );

			/* check for an error */
			if( result == ERROR )
				return ERROR;

			/* Read up to next ')' or to the end of the list */
			while( strcmp( *pntr, ")" ) != 0 &&
			       strcmp( *pntr, "" ) != 0 ) 
				++pntr;
			/* check result */
			if( result != cmp_val )
				ret_val = FALSE;

			/* reset cmp_val */
			cmp_val = TRUE;
		}
		/* check for ')' */
		else if( strcmp( *pntr, ")" ) == 0 ){
			/* if a ')' is found but lvl = 0, ie there has */
			/* been no '(', print error message            */
			if( lvl == 0 )
				return( print_error( BAD_BRACK, "" ) );
			else
				return ret_val;
		} 
		/* check for '-name' */
		else if( strcmp( *pntr, "-name" ) == 0 ){
			/* increment pointer over '-name' */
			++pntr;
			/* check whether the file name matches */
			if( match( name, *pntr ) != cmp_val )
				ret_val = FALSE;
			/* reset cmp_val */
			cmp_val = TRUE;
		}
		else if( strcmp( *pntr, "-links" ) == 0 ){
			++pntr;
			/* c1 is used in num_check() to tell which condition */
			/* is being checked for                              */
			/*               c1 = '-'    =>     <                */
			/*               c1 = '+'    =>     >                */
			/*               otherwise   =>    ==                */
			c1 = **pntr;
			/* get the number from the argument */
			num = get_num( *pntr );
			/* check result against cmp_val */
			if( num_check( file_info.st_nlink, num, c1 ) != cmp_val )
				ret_val = FALSE;
			/* reset cmp_val */
			cmp_val = TRUE;
		}
		/* check for '-size' */
		else if( strcmp( *pntr, "-size" ) == 0 ){
			++pntr;
			c1 = **pntr;
			num = get_num( *pntr );
			/* find the size of the file in blocks */
			/*          1 block = 512 chars        */
			temp = (int) (file_info.st_size + 511) / 512;
			/* check result against cmp_val */
			if( num_check( temp, num, c1 ) != cmp_val )
				ret_val = FALSE;
			/* reset cmp_val */
			cmp_val = TRUE;
		}
		/* check for '-inum' */
		else if( strcmp( *pntr, "-inum" ) == 0 ){
			++pntr;
			c1 = **pntr;
			num = get_num( *pntr );
			if( num_check( file_info.st_ino, num, c1 ) != cmp_val )
				ret_val = FALSE;
			cmp_val = TRUE;
		}
		/* check for '-atime' or '-mtime' */
		else if( strcmp( *pntr, "-atime" ) == 0 ||
		         strcmp( *pntr, "-mtime" ) == 0 ){
			if( strcmp( *pntr, "-atime" ) == 0 )
				which = ACC_TM; /* use last access time */
			else
				which = MOD_TM; /* use last modified time */
			++pntr;
			c1 = **pntr;
			num = get_num( *pntr );
			if( time_check( file_info, num, which, c1 ) != cmp_val )
				ret_val = FALSE;
			cmp_val = TRUE;
		}
		/* check for '-newer' */
		else if( strcmp( *pntr, "-newer" ) == 0 ){
			++pntr;
			/* get information on the file to be */
			/* compared against                  */
			stat( *pntr, &cmp_info );
			pntr++;
			/* set c1 and c2 - c1 and c2 tell newer() which two */
			/* times are to be compared.  If not given, they    */
			/* both default to 'm' ( - last modified time )     */
			if( flag_check( *pntr, &c1, &c2 ) == DEFAULT )
				pntr--;
			if( newer( file_info, cmp_info, c1, c2 ) != cmp_val )
				ret_val = FALSE;
			cmp_val = TRUE;
		}
		/* check for '-type' */
		else if( strcmp( *pntr, "-type" ) == 0 ){
			++pntr;
			if( type_check( file_info, *pntr ) != cmp_val )
				ret_val = FALSE;
			cmp_val = TRUE;
		}
		/* check for '!' - logical NOT */
		else if( strcmp( *pntr, "!" ) == 0 )
			/* set cmp_val to FALSE - next check performed */
			/* will be checked for FALSE instead of TRUE   */
			cmp_val = FALSE;
		else {
			/* else print error message and return ERROR      */
			/* ( This shouldn't occur since any bad arguments */
			/* should be spotted by set_up )                  */
			return( print_error( UNREC_ARG, *pntr ) );
		}
	}

	if( lvl != 0 )
		return( print_error( BAD_BRACK, "" ) );

	return ret_val;
}

int main(
	 int argc,
	 char *argv[] )
{
	char **path_pntr;
	int result,i;

	Buffer = (char*) ( malloc ( 256 * sizeof ( char) ) );
	
	/* Set up the pathname-list and linked list */
	if( set_up( argc, argv ) != ERROR ){
		/* Start at the top of the pathname-list */
		path_pntr = Path_list;
		do{
			/* Place pathname into Buffer */
			strcpy( Buffer, *path_pntr );
			/* Search the directory */
			result = search_dir( strlen( Buffer ) );
			/* Descend down pathname-list - */
			++path_pntr;
		/* until an error occurs, or the end of the pathname-list */
		/* is reached.                                            */
		}while( result == OK && *path_pntr != NULL );
	}

	if ( Path_list != NULL ) {
		for (i = -1; *(Path_list+(i+1)) != NULL; ++i) {
		free( *(Path_list+i));
		}
	}

	/* free elements on Head */
	
	if ( Head != NULL )
		lfree( Head );
}

