/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   U T I L I T Y			--
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1991, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- buildkeymap.c							--
--                                                                      --
--	Utility to build keymaps for the native keyboard server		--
--                                                                      --
--	Author:  PAB 26/3/91						--
--                                                                      --
--	This simple utility reads in a keymap definition file, 		--
--	converting the definition into a binary map. The binary map is 	--
--	the used by the keyboard console server to translate between 	--
--	scancodes (with shift key modifiers) and ASCII.			--
--									--
--	By convention ascii key maps have a '.akm' extension and their	--
--	binary derivatives have '.bkm'. For example the binary keymap	--
--	for a UK keyboard is 'key44.bkm', 44 being the countries	--
--	international telephone access code.				--
--									--
--	# may be used to start a comment (upto and incl. rest of line)	--
--	Tabs, spaces and newlines are used as delimiters.		--
--	0x is used to introduce a hex value.				--
--	A single character is taken as that character.			--
--									--
--	The file starts with a series of scancodes defining the shift	--
--	and toggle keys on the keyboard, and is then followed by 127	--
--	rows consisting of four columns.				--
--									--
--	The shift scancodes to be defined MUST be as follows:		--
--									--
--		left shift, right shift					--
--		left ctrl, right ctrl					--
--		left alt, right alt					--
--		left fn, right fn					--
--		CapsLock, Numlock					--
--		ScrollLoc, Spare					--
--									--
--	The four columns of ASCII are as follows:			--
--									--
--		normal, shifted, alternate, fn				--
--									--
--	Ascii codes with the top bit set are taken to be 'meta' keys	--
--	these will be post processed to transform them into the correct	--
--	multibyte terminal escape sequences.				--
--									--
--	These meta codes are:						--
--									--
--		0x80 == INVALID COMBINATION (no ASCII value)		--
--									--
--		0x81, 0x82...0x8A == F1, F2..F16			--
--									--
--		0x91, 0x92, 0x93, 0x94 == up, down, right, left		--
--									--
--		0x95 == help						--
--		0x96 == undo						--
--		0x97 == home						--
--		0x98 == pageup						--
--		0x99 == end						--
--		0x9A == pagedown					--
--		0x9B == insert						--
--									--
--	Alternatively you can specify !metaname e.g. !right		--
--	Note that there must be no space between the pling and the 	--
--	name, and names must be in lower case.				--
--                                                                      --
------------------------------------------------------------------------*/
/* RCSId: $Id: buildkeymap.c,v 1.4 1993/08/11 14:54:13 bart Exp $ */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DELIMITERS	" \t\n"
#define BUFSIZE		256
#define COLUMNS		4	/* normal, shift, alt, fn */

#define NumShiftKeys	12	/* Shift key scancodes defined at top of file */

#ifndef TRUE
# define TRUE 1
# define FALSE 0
#endif

#ifdef SUN4
#define strtoul strtol
#endif

typedef int bool;

static int line = 0;
static char buf[BUFSIZE];
static bool newstr = TRUE;

static void Usage(void)
{
	fprintf	(stderr,"buildkeymap [-o binkeymapfile] asciisrcfile\n");
	exit(1);
}

static int GetNext(FILE *infile) {
	char *token, *dummy;
	int x;

	if (newstr) {
		if (fgets(buf, BUFSIZE, infile) == NULL)
			return EOF;
		line++;
		token = strtok(buf, DELIMITERS);
		newstr = FALSE;
	}
	else
		token = strtok(NULL, DELIMITERS);

	while (token == NULL || *token == '#') {
		if (fgets(buf, BUFSIZE, infile) == NULL) {
			return EOF;
		}
		line++;
		token = strtok(buf, DELIMITERS);
	}

	if (token[1] == '\0')			/* single ascii char input */
		return token[0];

	else if (token[0] != '!') {		/* hex/dec input */
		x = (int)strtoul(token,&dummy,0);
		if (x > 255) {
			fprintf(stderr,"buildkeymap: illegal numeric ASCII value %s @ line %d\n", token, line);
			exit(6);
		}
		if (x == 0) {
			fprintf(stderr,"buildkeymap: warning ASCII 0 (^@) detected (%s) @ line %d\n", token, line);
		}
		return x;
	}

	if (strcmp(&token[1],"null") == 0)	/* metachar name input */
		return(0x80);

	if (strcmp(&token[1],"f1") == 0)
		return(0x81);
	if (strcmp(&token[1],"f2") == 0)
		return(0x82);
	if (strcmp(&token[1],"f3") == 0)
		return(0x83);
	if (strcmp(&token[1],"f4") == 0)
		return(0x84);
	if (strcmp(&token[1],"f5") == 0)
		return(0x85);
	if (strcmp(&token[1],"f6") == 0)
		return(0x86);
	if (strcmp(&token[1],"f7") == 0)
		return(0x87);
	if (strcmp(&token[1],"f8") == 0)
		return(0x88);
	if (strcmp(&token[1],"f9") == 0)
		return(0x89);
	if (strcmp(&token[1],"f10") == 0)
		return(0x8A);
	if (strcmp(&token[1],"f11") == 0)
		return(0x8B);
	if (strcmp(&token[1],"f12") == 0)
		return(0x8C);
	if (strcmp(&token[1],"f13") == 0)
		return(0x8d);
	if (strcmp(&token[1],"f14") == 0)
		return(0x8e);
	if (strcmp(&token[1],"f15") == 0)
		return(0x8f);
	if (strcmp(&token[1],"f16") == 0)
		return(0x90);

	if (strcmp(&token[1],"up") == 0)
		return(0x91);
	if (strcmp(&token[1],"down") == 0)
		return(0x92);
	if (strcmp(&token[1],"right") == 0)
		return(0x93);
	if (strcmp(&token[1],"left") == 0)
		return(0x94);

	if (strcmp(&token[1],"help") == 0)
		return(0x95);
	if (strcmp(&token[1],"undo") == 0)
		return(0x96);
	if (strcmp(&token[1],"home") == 0)
		return(0x97);
	if (strcmp(&token[1],"pageup") == 0)
		return(0x98);
	if (strcmp(&token[1],"end") == 0)
		return(0x99);
	if (strcmp(&token[1],"pagedown") == 0)
		return(0x9A);
	if (strcmp(&token[1],"insert") == 0)
		return(0x9B);

	fprintf(stderr,"buildkeymap: unknown !metaname: %s @ line %d\n", token, line);
		exit(3);

	return 0; /* make compiler happy */
}


int main(int argc, char **argv)
{
	int i, j, x;
	FILE *infile, *outfile;

	if (argc < 2)
		Usage();

	if (strncmp(argv[1],"-o", 2) == 0) {
		char *x = &argv[1][2];
		char *y = argv[2];

		if (*x == '\0') {
			x = argv[2];
			y = argv[3];
		}

		if ((outfile = fopen(x ,"wb")) == NULL) {
			fprintf(stderr,"buildkeymap: cannot create %s\n", x);
			exit(2);
		}
		if ((infile = fopen(y, "r")) == NULL) {
			fprintf(stderr,"buildkeymap: cannot open %s\n", y);
			exit(2);
		}
	}
	else
	{
		outfile = stdout;
		if ((infile = fopen(argv[1], "r")) == NULL) {
			fprintf(stderr,"buildkeymap: cannot open %s\n", argv[1]);
			exit(2);
		}
	}

	/* read the shift keys */
	for (i = 0; i < NumShiftKeys; i++) {
		if ((x = GetNext(infile)) == EOF) {
			fprintf(stderr,"buildkeymap: unexpected EOF @ line %d\n",line);
			exit(4);
		}
		if (x > 0x80) {
			fprintf(stderr,"buildkeymap: error, metachar in shift key scancodes @ line %d\n",line);
			exit(5);
		}
#ifdef DEBUG
		fprintf(stderr,"shift char %d %#x\n",i,x); /*debug*/
#endif
		fputc((char)x, outfile);
	}

	/* read the scancode conversion table */
	for (i = 0; i < 128; i++) {
#ifdef DEBUG
		fprintf(stderr,"Scancode %#x:",i); /*debug*/
#endif
		for (j = 0; j < COLUMNS; j++) {
			if ((x = GetNext(infile)) == EOF) {
				fprintf(stderr,"buildkeymap: unexpected EOF @ line %d\n",line);
				exit(4);
			}
#ifdef DEBUG
			fprintf(stderr," %#x",x); /*debug*/
#endif
			fputc((char)x, outfile);
		}
#ifdef DEBUG
		fputc('\n',stderr); /*debug*/
#endif
	}
	
	fclose(outfile);
	fclose(infile);

	return 0;
}
