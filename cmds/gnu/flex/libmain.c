/* libmain - flex run-time support library "main" function */

/* $Header: /dsl/HeliosRoot/Helios/cmds/gnu/flex/libmain.c,v 1.1 1993/10/19 12:59:12 tony Exp $ */

extern int yylex();

int main( argc, argv )
int argc;
char *argv[];

    {
    return yylex();
    }
