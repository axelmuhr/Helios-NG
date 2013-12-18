/*------------------------------------------------------------------------
--                                                                      --
--				 CBIN.C					--
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1989, Active Book Company Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
--                                                                      --
-- Converts an ARM assembler a.out file into a pure binary file.	--
--                                                                      --
-- The assembly output file must not contain "data" or a "bss" areas. 	--
-- 									--
--	Author:  JGS 891011						--
--									--
------------------------------------------------------------------------*/

static char *SccsId = " %W% %G% Copyright (C) 1989, Active Book Company Ltd.\n";

#include <stdio.h>
#include <a.out.h>
#include <stab.h>
#include <alloc.h>

#define	FALSE	0
#define TRUE	1

/* macros to determine the file position of different segments in a.out */
#define TextREL(x) (N_TXTOFF(x) + (x).a_text+(x).a_data)
#define DataREL(x) (N_TXTOFF(x) + (x).a_text+(x).a_data + (x).a_trsize)
#define SYMTAB(x) (N_TXTOFF(x) + (x).a_text+(x).a_data + (x).a_trsize + \
	(x).a_drsize)
#define STRTAB(x) (N_TXTOFF(x) + (x).a_text+(x).a_data + (x).a_trsize + \
	(x).a_drsize + (x).a_syms)

#define PARTCMP(x,y)	(strncmp(x, y, sizeof(y) -1 ) == 0)

#define ENC_MORE 0x80	/* number encoding - another byte to follow */
#define ENC_NEG  0x40	/* number encoding - number is neg */
#define BOT6BITS 0x3f
#define BOT7BITS 0x7f

void write_enc() ;
unsigned Enc() ;
int read_enc() ;
void read_and_check_header() ;
void printsyms() ;
int cmp_symbols() ;
void sort_symbols() ;
void munge() ;

/*--------------------------------------------------------------------------*/

void usage()
{
	fprintf(stderr,"Usage: cbin [-d] [-o binaryfile] objectfile\n") ;
	exit(1) ;
}
/*--------------------------------------------------------------------------*/

int main(argc, argv)
int argc ;
char **argv ;
{
	FILE	*outf = stdout ;
	FILE	*inf = stdin ;
	struct exec ohead ;
	int	i ;
	char	*cp ;
	char	*text_seg ;
	int	opt_dumpsize = FALSE ;

	for (i=1; i < argc ; i++)
	{
		if (argv[i][0] == '-')
		{
			switch(argv[i][1])
			{
			case 'o':
				if (argv[i][2] == '\0')
					cp = argv[++i] ;
				else
					cp = &argv[i][2] ;
#ifdef __STDC__ /* ANSI */
				if ((outf = fopen(cp,"wb")) == NULL)
#else
				if ((outf = fopen(cp,"w")) == NULL)
#endif
				{
					fprintf(stderr,"cbin: Cannot open output file\n") ;
					exit(2) ;
				}
				break ;
			
			case 'd':
				opt_dumpsize = TRUE ;
				break ;
				
		default:
				usage() ;

			}
		}
		else
		{
			if((inf=fopen(argv[i],"r")) == NULL)
			{
				fprintf(stderr,"cbin: cannot open input file\n") ;
				exit(3) ;
			}
		}
	}
	/* end of arg processing */

	/* read in a.out text segment */
	read_and_check_header(inf, &ohead) ;
	if (ohead.a_text != 0)
	{
		if ((text_seg = malloc((unsigned)ohead.a_text)) == NULL)
		{
			fprintf(stderr,"cbin: out of memory\n") ;
			exit(4) ;
		}

		if (fread(text_seg, ohead.a_text, 1, inf) != 1)
		{
			fprintf(stderr,"cbin: Couldn't read text segment from input file\n") ;
			exit(4) ;
		}
	}
	else
		fprintf(stderr,"cbin: Warning no text segment in input file\n") ;

	/* write out the binary data */
	if (opt_dumpsize)
		if (fwrite(&(ohead.a_text),sizeof(ohead.a_text),1,outf) != 1)
			{
				fprintf(stderr,"cbin: Unable to write image size with -d option\n") ;
				exit(9) ;
			}

	if (fwrite(text_seg,ohead.a_text,1,outf) != 1)
		{
			fprintf(stderr,"cbin: Couldn't write binary data to file\n") ;
			exit(10) ;
		}

	/* tidy up */
	free(text_seg) ;
	fclose(inf) ;
	fclose(outf) ;
	exit(0) ;
	SccsId = SccsId ; /* remove warning */
}

/*--------------------------------------------------------------------------*/

void read_and_check_header(FILE *inf, struct exec *ohead)
{
	if(fread((char *)ohead, sizeof(struct exec), 1, inf) != 1)
	{
		fprintf(stderr,"cbin: Error - Could not read input file header\n") ;
		exit(4) ;
	}

	if (ohead->a_magic != OMAGIC)
	{
		fprintf(stderr,"cbin: Error - Input file is not RISC-iX assembler a.out file\n") ;
		exit(5) ;
	}

#ifdef oldCODE
	if (ohead->a_trsize != 0)
	{
		fprintf(stderr,"cbin: Error - Input file contains text relocation information\n") ;
		exit(6) ;
	}
#endif /* oldCODE */

	if (ohead->a_drsize != 0)
	{
		fprintf(stderr,"cbin: Error - Input file contains data relocation information\n") ;
		exit(6) ;
	}

	if (ohead->a_data + ohead->a_bss != 0)
	{
		fprintf(stderr,"cbin: Error - Input file contains data and/or bss segments\n") ;
		exit(7) ;
	}
}

/*--------------------------------------------------------------------------*/
/*> EOF cbin.c <*/
