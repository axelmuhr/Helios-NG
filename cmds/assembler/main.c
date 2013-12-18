/*
 * File:	main.c
 * Author:	P.A.Beskeen
 * Date:	Aug '91
 *
 * Description: Helios generic assembler controlling body and miscellaneous
 *		routines.
 *
 * RcsId: $Id: main.c,v 1.6 1992/11/20 15:31:57 paul Exp $
 *
 * (C) Copyright 1991 Perihelion Software Ltd.
 * 
 * $RcsLog$
 *
 */

#include "gasm.h"
#include "ghof.h"
#include <signal.h>

/* Exported Data: */

/* hold counts for number of errors and warnings */
int		errors = 0;
int		warnings = 0;

/* where the lexer reads input from */
FILE		*InputFile = NULL;

/* where the object formatter writes its output */
FILE		*OutputFile = NULL;

/* Input and output file names */
char		*InputFileName = NULL;
char		*OutputFileName = NULL;

/* default module header information */
int		ModuleHeadTail = TRUE;	/* create module head/tail by default */
int		ModuleNumber = -1;	/* default to any module */
char		ModuleName[32] = { 0 };
int		ModuleVersion = 1;


/* Local Data: */

char		*ProgramName = NULL;

#ifdef DEBUG
static int	debug = FALSE;
#endif

/* private functions */
static void TidyUp(void);
static void Usage(void);
static void Signal_Handler(int);


/****************************************************************/
/* Usage							*/
/*								*/
/* Print assembler usage information and exits			*/
/*								*/
/****************************************************************/

static void Usage(void)
{
	fprintf(stderr, "%s [-h] [-d] [-n modulename] [-v moduleversion] [-x modulenumber] \n"
	"[-o objectfile] [sourcefiles]\n", ProgramName);
	fprintf(stderr,
		"\n-h                 This help.\n"
		"-d                 Disable module header / tailer generation.\n"
		"-n modulename      Supply module name, rather than use objectfile name (+).\n"
		"-v moduleversion   Set version number other than 1 (+).\n"
		"-x modulenumber    Fix module table slot number (+).\n"
		"-o objectfile      Send object output to this file, else stdout.\n"
		"   sourcefiles     Multiple source files can be included in one object module.\n"
		"\nOptions marked (+) are only useful for shared library construction.\n"
	);

	exit(1);
}


/****************************************************************/
/* Main								*/
/*								*/
/* Command argument parsing and controlling body of assembler	*/
/*								*/
/****************************************************************/

int main(int argc, char **argv)
{
	ProgramName = *argv++;

	InitSymbTab();
	InitLex();

	/* Process arguments */
	while (*argv != NULL && **argv == '-') {
		switch ((*argv)[1]) {
		case 'H':
		case 'h':
		case '?':
			Usage();
			break;

		case 'd':
			ModuleHeadTail = FALSE;
			break;

		case 'n':
			if ((*argv)[2] == '\0') {
				if (strlen(*++argv) > 31)
					Warn("-n Module name cannot be greater than 31 characters: truncating");
				strncpy(ModuleName, *++argv, 31);
				ModuleName[31] = '\0';
			}
			else {
				if (strlen(&((*argv)[2])) > 31)
					Warn("-n Module name cannot be greater than 31 characters: truncating");
				strncpy(ModuleName, &((*argv)[2]), 31);
				ModuleName[31] = '\0';
			}
			break;

		case 'v':
			if ((*argv)[2] == '\0')
				ModuleVersion = atoi(*++argv);
			else
				ModuleVersion = atoi(&((*argv)[2]));
			break;

		case 'x':
			if ((*argv)[2] == '\0')
				ModuleNumber = atoi(*++argv);
			else
				ModuleNumber = atoi(&((*argv)[2]));

			if (ModuleNumber == 0)
				Error("Illegal Helios module number: 0");

			break;

		case 'o':
			if ((*argv)[2] == '\0') {
				OutputFile = fopen(*++argv, "wb");
				OutputFileName = *argv;
			}
			else {
				OutputFile = fopen(&((*argv)[2]), "wb");
				OutputFileName = &((*argv)[2]);
			}

			if (OutputFile == NULL) {
				char Err[80];

				sprintf(Err, "cannot open output file: \"%s\"", OutputFileName);
				Error(Err);
			}
			break;
#ifdef DEBUG
		case 'd':
			debug = TRUE;
			break;
#endif
		default:
			{
				char Err[80];

				sprintf(Err, "ignoring unknown option: '-%c'", (*argv)[1]);
				Warn(Err);

				break;
			}
		}
		argv++;
	}

/*
-- crf: 10/11/92 - install signal handler
*/
	if (signal (SIGINT, Signal_Handler) == SIG_ERR)
		Warn ("failed to set up signal handler") ;
	if (signal (SIGTERM, Signal_Handler) == SIG_ERR)
		Warn ("failed to set up signal handler") ;

	if (*argv == NULL) {
		/* no input files, so use stdin as input to the parser */
		InputFile = stdin;
		InputFileName = "stdin";

		/* first pass of the assembler */
		if (yyparse() || errors) {
			TidyUp();
			exit(2);
		}

	}
	else while (*argv != NULL)
	{
		/* pass all remaining args as input to the parser */
	  
		if ((InputFile = fopen(*argv, "r")) == NULL) {
			char Err[80];

			sprintf(Err, "Cannot read inputfile: %s", *argv);
			Fatal(Err);
		}

		InputFileName = *argv;

		/* first pass of the assembler */

		if (yyparse() || errors) {
		    TidyUp();
		    exit(2);
		}

		fclose(InputFile);
		argv++;		
	}

	/* if no outputfile specified, send output to stdout */
	if (OutputFile == NULL) {
		OutputFile = stdout;
	}

	/* if dont know specific line of error, don't print line numbers */
	CurLine = 0; StartLine = 0;

#ifdef DEBUG
	if (debug)
		DebugOutputGHOF(HeaderParseTreeItem.next);
#endif
	OutputGHOF(HeaderParseTreeItem.next);

	TidyUp();

	return errors;
}


/****************************************************************/
/* Warn								*/
/* Error							*/
/* Fatal							*/
/*								*/
/* All of these fn's print the error text they are passed,	*/
/* along with an optional line number. They also increment the	*/
/* count of errors seen so far.					*/
/*								*/
/* Fatal() tidys up the world, printing the accumulated error	*/
/* stats and aborts the program.				*/
/*								*/
/*								*/
/* ErrorMsg							*/
/* 								*/
/* Prints the associated error message, complete with any line	*/
/* numbering information available.				*/
/* 								*/
/****************************************************************/

void ErrorMsg(char *et, char *s)
{
	if (CurLine == 0)
		fprintf(stderr, "%s: %s\n", et, s);	
#if 0
	/* for use if you want to errors to print the line the statement */
	/* started on, to the point the error was found */

	else if ((StartLine != 0) && StartLine < CurLine)
		fprintf(stderr, "%s: \"%s\" %s lines %d - %d: %s\n", \
			ProgramName, CurSrcFile, et, StartLine, CurLine, s);
#endif
	else
		fprintf(stderr, "%s: \"%s\" %s line %d: %s\n", \
			ProgramName, InputFileName, et, CurLine, s);
}


void Warn(char *s)
{
	ErrorMsg("Warning", s);

	warnings++;
}

void Error(char *s)
{
	ErrorMsg("Error", s);
	errors++;
}

void Note(char *s)
{
	ErrorMsg("Note", s);
}

void Fatal(char *s)
{
	ErrorMsg("Fatal Error", s);
	errors++;

	fprintf(stderr, "Aborting\n");
	TidyUp();	/* tidy up the world, printing number of errors found */
	exit(3);
}


/****************************************************************/
/* TidyUp							*/
/*								*/
/* Tidy up the world, printing number of errors and warnings	*/
/* found if necessary.						*/
/*								*/
/****************************************************************/

static void TidyUp(void)
{
	fclose(OutputFile);

	if (errors || warnings) {
		fprintf(stderr, "Errors: %d, Warnings %d\n", errors, warnings);

		if (errors != 0 && OutputFileName != NULL) {
			/* remove output file to stop make getting confused */
			remove(OutputFileName);
		}
	}
#if 0
	else {
		fprintf(stderr, "No errors or warnings\n");
	}
#endif
}


/****************************************************************/
/* Signal_Handler						*/
/*								*/
/* Called on reception of SIGINT, SIGTERM - removes output file	*/
/*								*/
/****************************************************************/

static void Signal_Handler (int sig)
{
	if (OutputFile != NULL)
		(void) fclose(OutputFile);
	if (OutputFileName != NULL)
		(void) remove(OutputFileName);
#ifdef DEBUG
	printf ("Signal_Handler: sig = %d\n", sig) ;
#else
	sig = sig ;
#endif
	exit (0) ;
}


/* end of main.c */
