#include "stdlib.h"
#include "string.h"
#include "math.h"
#include "ctype.h"
#include "hydra.h"


#define LINE_LEN (50)
#define COMM_LEN (10)
#define NUM_PARMS (8)
#define TEMP_LEN (15)


extern reg_set call_set;
extern unsigned long breaks[];
extern unsigned long brk_addrs[];



void monitor( hydra_conf *config )
{
	static char in_line[LINE_LEN];
	static int i=0;
	char in_char, command[COMM_LEN], temp[TEMP_LEN];
	unsigned long parms[NUM_PARMS];
	int j, k, l, ok;



	while( 1 )
	{

		while( (in_char = c40_getchar()) != '\n' )
		{
			if( in_char == '\b' )
				i--;
			else
				in_line[i++] = in_char;

			if( i >= LINE_LEN )
			{
				c40_printf( "\nLine too long.\n" );
				i=0;
			}
		}
		in_line[i] = '\0';

		/*****  Parse input line *****/

		i=0;
		for( j=0 ; iswhite( in_line[j] ) ; j++ ); /* Skip leading white space */
		for( l=0 ; (!isspace(in_line[j])) && (l < COMM_LEN) ; j++, l++ )  /* extract command from line */
			command[l] = (char)tolower( in_line[j] );
		command[l] = '\0';

		for( l=0 ; l < NUM_PARMS ; l++ )  /* 0xFFFFFFFF is end of parms list */
			parms[l] = 0xFFFFFFFF;

		l=0;
		while( (in_line[j] != '\0') && (l < NUM_PARMS) )   /* break will end this loop */
		{
			while( iswhite( in_line[j] ) )  /* Find next parameter */
				j++;

			for( k=0 ; (isalnum(in_line[j])) && (k < TEMP_LEN) ; k++, j++ )
				temp[k] = in_line[j];
			temp[k] = '\0';

         parms[l++] = (temp[k-1]=='h')||(temp[k-1]=='H') ?
									atox( temp, &ok ) : atod( temp, &ok );
			if( ok == FAILURE )
			{
				c40_printf( "\nNot a valid number : %s\n", temp );
				i=0;
				command[0] = '\0';
				break;
			}
		}

		if( !strcmp(command, "?") )
			help();
		else if( !strcmp(command, "c") )
			compare( parms, 't' );
		else if( !strcmp(command, "d") )
			dump( parms, 't' );
		else if( !strcmp(command, "f") )
			fill( parms );
		else if( !strcmp(command, "e") )
			enter( parms, 't' );
		else if( !strcmp(command, "cp") )
			copy( parms );
		else if( !strcmp(command, "s") )
			search( parms, 't' );
		else if( !strcmp(command, "cf") )
			configure( config );
		else if( !strcmp(command, "t") )
			test( *config, 't' );
		else if( !strcmp(command, "sbp") )
			set_brk( parms, 't' );
		else if( !strcmp(command, "dbp") )
			del_brk( parms, 't' );
		else if( !strcmp(command, "lbp") )
			list_brks( 't' );
		else if( !strcmp(command, "rd") )
			reg_dump( call_set, 't' );
		else if( !strcmp(command, "st") )
			step( &call_set );
		else if( !strcmp(command, "g") )
		{
      	if( parms[0] )
				call_set.ret_add = parms[0];
			go( &call_set );
		}
		else if( !strcmp( command, "" ) );
		else c40_printf( "\nUnknown command : %s\n", command );

		c40_printf( "Ariel_Mon => " );

	}
}





int iswhite( char chr )
/*********************************
 **** Detects spaces and tabs ****
 *********************************/
{
	if( chr == ' ' )
		return( 1 );
	else if(chr == '\t')
			  return( 1 );
	else return( 0 );
}




unsigned long atox( char *str, int *ok )
/****************************************
 **** Converts hex strings into ints ****
 ****************************************/
{
	unsigned long j, xnum;
	unsigned long num;
	int i;

   *ok = SUCCESS;

	for( i=0 ; str[i] != '\0' ; i++ );  /* Find end of string */

	i -= 2;

	for( j=0, num=0 ; i >= 0 ; i--, j++ )
	{
		if( !isxdigit( str[i] ) )
		{
			*ok = FAILURE;
			return( 0 );
		}

		xnum = isdigit(str[i]) ? str[i]-'0' : toupper(str[i])-'A'+10;
		num += xnum * (unsigned long)pow( 16, j );
	}

	return( num );
}




unsigned long atod( char *str, int *ok )
/********************************************
 **** Converts decimal strings into ints ****
 ********************************************/
{
	unsigned long j, dnum;
	unsigned long num;
	int i;

	*ok = SUCCESS;

	for( i=0 ; str[i] != '\0' ; i++ );  /* Find end of string */

	i -= 1;

	for( j=0, num=0 ; i >= 0 ; i--, j++ )
	{
		if( !isdigit( str[i] ) )
		{
			*ok = FAILURE;
			return( 0 );
		}

		dnum = str[i]-'0';
		num += dnum * (unsigned long)pow( 10.0, (double)j );
	}

	return( num );
}


