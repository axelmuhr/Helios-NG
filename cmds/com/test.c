
static char *rcsid = "$Header: /hsrc/cmds/com/RCS/test.c,v 1.4 1993/07/12 11:35:46 nickc Exp $";

#include <stdio.h>
#include <setjmp.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <posix.h>

enum {
	t_nenums,	t_gtnums,	 t_genums,
	t_readablefile, t_writeablefile, t_executablefile,
	t_isregfile,    t_isdir,         t_ischarspecfile,
	t_isblkspecfile,t_ispipe,        t_uidset,
	t_gidset,       t_stickyset,     t_non0file,
	t_islink,       t_isatty,        t_zerostr,
	t_nzerostr,     t_eqstr,         t_neqstr,
	t_nullstr,      t_eqnums,        t_ltnums,
	t_lenums,	t_not,           t_and,
	t_or,           t_lparen,        t_rparen,
	t_file,         t_fildes,        t_string,
	t_num,          t_logical,       t_end 
};


struct toktype {
	char *switchstr;
	int  op;
} ptable[]  =
{	{ "-ne" , t_nenums},
	{ "-gt" , t_gtnums},
	{ "-ge" , t_genums},
	{ "-r", t_readablefile},
	{ "-w", t_writeablefile},
	{ "-x", t_executablefile},
	{ "-f", t_isregfile},
	{ "-d", t_isdir},
	{ "-c", t_ischarspecfile},
	{ "-b", t_isblkspecfile},
	{ "-p", t_ispipe},
	{ "-u", t_uidset},
	{ "-g", t_gidset},
	{ "-k", t_stickyset},
	{ "-s", t_non0file},
	{ "-h", t_islink},
	{ "-t", t_isatty},
	{ "-z", t_zerostr},
	{ "-n", t_nzerostr},
	{ "=",  t_eqstr},
	{ "!=", t_neqstr},
	{ "-eq", t_eqnums},
	{ "-lt", t_ltnums},
	{ "-le", t_lenums},
	{ "!",  t_not},
	{ "-a", t_and},
	{ "-o", t_or},
	{ "(",  t_lparen },
	{ ")",  t_rparen },
	{ NULL }
};

struct node {
	int n_op;
	char *n_val;
} token;

jmp_buf badparse;
int argpos=1;
int gargc;
char **gargv;

int primary(void);

int compare_nums(int op, char *n1, char *n2)
{	char *p;
	long v1;
	long v2;
	v1 = strtol(n1, &p, 10);
	if( *p != '\0' ) return 0;
	v2 = strtol(n2, &p, 10);
	if( *p != '\0' ) return 0;

	switch( op )
	{
	case t_eqnums: return v1 == v2;
	case t_nenums: return v1 != v2;
	case t_gtnums: return v1 > v2;
	case t_genums: return v1 >= v2;
	case t_ltnums: return v1 < v2;
	case t_lenums: return v1 <= v2;
	}

	return 0;	
}

int x_fileop(int op, char *name)
{	struct stat st;
	int res;
	if( stat(name,&st) == 0 )
	{
		switch( op )
		{
		case t_readablefile:
			res = !access( name, R_OK );
			break;
		case t_writeablefile:
			res = !access( name, W_OK );
			break;
		case t_executablefile:
			res = !access( name, X_OK );
			break;
		case t_isregfile:
			res = S_ISREG( st.st_mode )!=0;
			break;
		case t_isdir:
			res = S_ISDIR( st.st_mode )!=0;
			break;
		case t_ischarspecfile:
			res = S_ISCHR( st.st_mode )!=0;
			break;
		case t_isblkspecfile:
			res = S_ISBLK( st.st_mode )!=0;
			break;
		case t_ispipe:
			res = S_ISFIFO( st.st_mode )!=0;
			break;
		case t_uidset:
			res = (st.st_mode & S_ISUID) != 0;
			break;
		case t_gidset:
			res = (st.st_mode & S_ISGID) != 0;
			break;
		case t_stickyset:
			res = 0;
			break;
		case t_non0file:
			res = st.st_size != 0;
			break;
		      default:
		case t_islink:
			res = 0;
			break;
		}
	}
	else
		res = 0;
	return res;
}

void gettoken()
{	int found = 0;
	int i;
	
	if( argpos == gargc )
	{
		token.n_op = t_end;
		return;
	}

	for( i=0; ptable[i].switchstr != NULL; i++ )
	{	if( strncmp(gargv[argpos], ptable[i].switchstr, 
				strlen(ptable[i].switchstr)) == 0 )
		{	found = 1;
			break;
		}
	}

	if( found )
		token.n_op = ptable[i].op;
	else
	{
		if( gargv[argpos][0] == '-' )
			longjmp(badparse,1);
		else
			token.n_op = t_string;
		token.n_val = gargv[argpos];
	}
	argpos++;
}

int quarternary()
{	int res;
	int op;
 
	switch( op = token.n_op )
	{
	case t_readablefile: case t_writeablefile:
	case t_executablefile: case t_isregfile:
	case t_isdir: case t_ischarspecfile:
	case t_isblkspecfile: case t_ispipe:
	case t_uidset: case t_gidset:
	case t_stickyset: case t_non0file:
	case t_islink:
		gettoken();
		if( token.n_op != t_string )
			longjmp(badparse,1);
		res = x_fileop(op, token.n_val);
		gettoken();
		break;

	case t_isatty:
		gettoken();
		if( token.n_op != t_string )
			res = isatty(1);
		else
			{
			char *rest;
			int val;

			val = (int) strtol(token.n_val, &rest, 10);
			if(*rest != '\0')
				res = 0;
			else
				res = isatty(val);
			}
		gettoken();
		break;
	
	case t_zerostr:
		gettoken();
		if(token.n_op == t_end)
			res = 1;
		else
			res = ( strlen ( token.n_val ) == 0 );
		gettoken();
		break;
		
	case t_nzerostr:
		gettoken();
		if(token.n_op == t_end)
			res = 0;
		else
			res = ( strlen ( token.n_val ) != 0 );
		gettoken();
		break;

	      default:
	case t_end:
		res = 0;
		break;

	case t_lparen:
		gettoken();
		res = primary();
		if( token.n_op != t_rparen )
			longjmp(badparse,1);
		gettoken();
		break;
	case t_not:
		gettoken();
		res = !primary();
		break;
	case t_string:
		res = (int)token.n_val;
		gettoken();
		break;
	}
	return res;
}	

int tertiary()
{
	int n1 = quarternary();
	int n2;
	int op;
	int res;
	
	switch( op = token.n_op )
	{
	case t_neqstr:
	case t_eqstr:
		gettoken();
		n2 = quarternary();
		if( n1 != 1 && n2 != 1 )
			res = strcmp((char *)n1,(char *)n2)==0;
		else
			res = 0;
		if( op == t_neqstr ) res = !res;
		break;
	case t_eqnums: case t_nenums: case t_gtnums:
	case t_genums: case t_ltnums: case t_lenums:
		gettoken();
		n2 = quarternary();
		if( n1 != 1 && n2 != 1 )
			res = compare_nums( op, (char *)n1, (char *)n2);
		else
			res = 0;
		break;
	default:
		res = n1!=0;
		break;
	}
	return res;
}

int secondary()
{	int n1 = tertiary();
	
	switch( token.n_op )
	{
	case t_and:
	{	int n2;
		gettoken();
		n2  = secondary();
		return n1 & n2;
	}

	default:
		return n1;
	}
}

int primary()
{	int n1 = secondary();
	int res;
	
	switch( token.n_op )
	{
	case t_or:
	{	int n2;
		gettoken();
		n2 = primary();
		return n1 | n2;
	}	

	default:
		res = n1;
		break;
	}

	return res;
}

int parseargs()
{
	gettoken();
	return primary();
}

int main( int argc, char *argv[])
{	int r;
	gargc = argc;
	gargv = argv;
	
	if( setjmp(badparse) != 0 )
	{
		return 2;
	}
	r = parseargs();
	if( token.n_op != t_end ) return 2;
	return !r;
}
