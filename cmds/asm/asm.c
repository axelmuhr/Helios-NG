/****************************************************************/
/* File: asm.c                                                  */
/*                                                              */
/*                                                              */
/* Author: NHG 19-Feb-87                                        */
/* Modified to compile under Norcroft C: AEv 21-Jan-88		*/
/****************************************************************/
/* $Id: asm.c,v 1.15 1994/11/08 09:47:15 nickc Exp $ */

#include "asm.h"
#include <ctype.h>

#ifdef __STDC__
#include <stdlib.h>
#endif

#ifdef LATTICE
#include <fcntl.h>
#endif

#ifdef __HELIOS
#include <task.h>
#include <memory.h>
#include <nonansi.h>
#include <module.h>
#endif

#include "getargs.h"

PUBLIC  FILE *infd;
PUBLIC  FILE *outfd;
PUBLIC  FILE *verfd;
PUBLIC  jmp_buf error_level;
PRIVATE jmp_buf default_level;
PUBLIC  WORD errors;
PRIVATE WORD phase = 0;
PUBLIC  WORD traceflags;
PUBLIC  WORD warnflags;
PRIVATE WORD maxerr = 0;
PUBLIC  WORD preasm = FALSE;
PUBLIC  WORD hexopt = FALSE;
PRIVATE WORD image  = i_helios;
PUBLIC  WORD defopt = FALSE;
PUBLIC  WORD fastopt = TRUE;
PUBLIC  WORD verbose = FALSE;
PUBLIC  WORD inlib = FALSE;
PUBLIC  FILE *deffd;
PUBLIC  WORD vmpagesize = 8*1024;
PUBLIC  WORD instr_size = 3;

PRIVATE WORD needobjed = FALSE;
PRIVATE WORD noname = FALSE;
PRIVATE char *progname = NULL;
PRIVATE WORD stack_size = -1;
PRIVATE WORD heap_size = -1;

PUBLIC  BYTE infile[128];
PRIVATE BYTE outfile[128];
PUBLIC  char *curfile;

#ifdef __STDC__
#define BUFSIZE (8*1024)
PRIVATE BYTE inbuf[BUFSIZE];
PRIVATE BYTE outbuf[BUFSIZE];
#endif

WORD filemod;
WORD filepos = -1;

PUBLIC WORD waste;

/* void unload(void); */
#ifdef __DOS386

#undef max
#undef min
WORD max(WORD a, WORD b);
PRIVATE WORD parsearg(char ***aargv);
PRIVATE void tidy1(void);
PRIVATE void tidyup(void);

#else /* !__DOS386 */

WORD max(ellipsis);
PRIVATE WORD parsearg(ellipsis);
PRIVATE void tidy1(ellipsis);
PRIVATE void tidyup(ellipsis);

#endif /* __DOS386 */

#ifdef MWC
long _stksize = 20000;
#endif
#ifdef IBMPC
int _stack = 4096;
#endif

PRIVATE void
_asm( argc, argv )		/* NB cannot call the function asm() as this is a reserved word in some C compilers */
  int 		argc;
  char ** 	argv;
{
	WORD infiles = 0;
	int start, inend, growend, end;
	ArgStack	*argstack = NULL;    

        infd = stdin;
        outfd = stdout;
	
#ifdef LATTICE
        verfd = stdout;
#else
        verfd = stderr;
#endif

	deffd = (FILE *) NULL;
        infile[0] = 0;
        memcpy(error_level,default_level,sizeof(jmp_buf));
        errors = 0;

        argv++;
	argc--;

	inend = growend = end = start = time(NULL);
	
        if( setjmp(error_level) == 0 )
        {
	  
                initmem();
                initcode();
                initmodules();
                initsym();
                initasm();

                report("Transputer Assembler V1.9");
                report("Copyright (C) 1987, Perihelion Software Ltd.");

                phase = 1;
                while( (*argv != NULL) || (argstack != NULL) )
                {
			if (*argv == NULL)
			{
				popargs(&argstack, &argc, &argv);
				continue;
			}

			if ( **argv == '@')
			{
				char **argfile;

				argfile = getargs((*argv)+1, &argc);
				if (argfile == NULL)
				{
					error("Unable to open or create arguments from %s\n", (*argv)+1);
					argv++;
				} else {
					/* argv+1 is pushed so we return to the next argv */
					pushargs(&argstack, argc-1, argv+1);
					argv = argfile;
				}

				continue;
			}

                        if ( parsearg(&argv) == TRUE )
                        {
				infiles++; filemod = 0; filepos = -1;
                                assemble();
                                tidy1();
                                tempval++;
                                inlib = FALSE;
                        }
                }

		if( infiles == 0 )	/* if no command line files, use stdin */
		{
			assemble();
		}

                genend();	/* finish off last module */

		inend = growend = end = time(NULL);
				
                report("\nSyntax analysis complete");

		strcpy(infile,outfile);
		if( warnflags & warn_unref ) show_unref();
	
                if( !preasm && errors == 0 )
                {
                        phase = 2;
                        setmodules();
			growdata();
			report("Semantic analysis complete");
                        growcode();
			growend = end = time(NULL);
                        report("\nCode growing complete");

                }

                if( !preasm && errors == 0 )
                {
                        phase = 3;
                        switch( (int)image )
                        {
                        case i_helios: genimage();      break;
                        }

			if( !noname && progname == NULL && outfd != stdout )
			{
				char *p = &outfile[strlen(outfile)];
				while( *(p-1) != '/' && p != outfile ) 
				{
					if( *p == '.' ) *p = '\0';
					p--;
				}
				progname = p;
				needobjed = 1;
			}
			
			fflush(outfd);
			
                        report("Image file generated");
			
                        if( needobjed ) objed(outfd,progname,stack_size,heap_size);
                }

                if( preasm && errors == 0 )
                {
                        phase = 2;
                        genpreasm();

                        report("Object code generated");

                }

        }
                                                               
	end = time(NULL);

report("Code Density:    Percent        %8ld Waste          %8ld",((simPc+4-waste)*100)/(simPc+4),waste);
report("Times (secs):    Total          %8ld Input          %8ld",end-start,inend-start);
report("                 Growing        %8ld Output         %8ld",growend-inend,end-growend);
report("Memory used:     Code           %8ld Local Heap     %8ld",codesize,heapsize);
report("                 Symbols        %8ld Expressions    %8ld",symsize,exprsize);

	VMStats();
	
	modstats();

	if( traceflags&db_refs ) printtab(Symtab);

	if( traceflags&db_vm ) showvm();
			
        tidyup();

        exit(maxerr);

        return;
}

/********************************************************/
/* main                                                 */
/*                                                      */
/* Initialise everything and call the assembler         */
/*                                                      */
/********************************************************/

PUBLIC int
main( argc, argv )
  int argc;
  BYTE **argv;
{
#if defined(__HELIOS) && defined(__TRAN)
	int			stack_size = 3000;
	static Carrier *	stack      = NULL;

	
	stack = AllocFast( stack_size , &MyTask->MemPool );

	if (stack)
	  Accelerate( stack, _asm, sizeof (argc) + sizeof (argv), argc, argv );
	else
#endif
	  _asm( argc, argv );

	return 0;
}

PRIVATE WORD parsearg(aargv)
char ***aargv;
{
	char **argv = *aargv;
	char *arg = *argv;

#define _ARG_ if( *++arg == '\0' ) arg = *(++argv);	

        if( *arg == '-' )
        {
        	arg++;
                switch( *arg )
                {
                case 'v':
                    verbose = !verbose;
                    if( *++arg != '\0' )
                    {
                        FILE *vfd = fopen(arg,"w");
                        if( vfd == 0 ) error("Cannot open %s for output",arg+2);
                        verfd = vfd;
                    }
                    break;


                case 't':
                	arg++;
                        while( *arg != '\0' )
                        {
                                switch( (int)locase(*arg) )
                                {
                                case 'l': traceflags ^= db_lex;      break;
                                case 's': traceflags ^= db_syn;      break;
                                case 'c': traceflags ^= db_gencode;  break;
                                case 'g': traceflags ^= db_growcode; break;
                                case 'o': traceflags ^= db_genimage; break;
                                case 'e': traceflags ^= db_expr;     break;
                                case 'v': traceflags ^= db_eval;     break;
                                case 'p': traceflags ^= db_preasm;   break;
                                case 'z': traceflags ^= db_vm; 	     break;
				case 'k': traceflags ^= db_kludge;   break;
				case 'f': traceflags ^= db_files;    break;
				case 'm': traceflags ^= db_mem;      break;
				case 'y': traceflags ^= db_sym;      break;
				case 'd': traceflags ^= db_modules;  break;
				case 'r': traceflags ^= db_refs;     break;
                                }
                                arg++;
                        }
                        break;

		case 'w':
                	arg++;
                        while( *arg != '\0' )
                        {
                                switch( (int)locase(*arg) )
                                {
                                case 'r': warnflags ^= warn_unref;   break;
                                case 'u': warnflags ^= warn_undef;   break;
                                }
                                arg++;
                        }
			break;			

		case 'I':
			_ARG_
			instr_size = (long)atol(arg);
			break;

                case 'p':
                        preasm = TRUE;
                        break;

                case 'x':
                        hexopt = TRUE;
                        break;

                case 'i':
                        image = *arg;
                        break;

		case 'f':
			fastopt = !fastopt;
			break;

		case 'd':
			defopt = TRUE;
			_ARG_;
			deffd = fopen(arg,"w");
			if(deffd == NULL ) error("Cannot open %s for output",arg+2);
			break;

		case 'n':
			_ARG_
			needobjed = 1;
			progname = arg;
			break;
			
		case 's':
			_ARG_
			needobjed = 1;
			stack_size = (long)atol(arg);
			break;

		case 'h':
			_ARG_;
			needobjed = 1;
			heap_size = (long)atol(arg);
			break;			

		case 'N':
			noname = TRUE;
			break;
			
#ifdef VM
		case 'm':
			_ARG_;
			VMfilename = arg;
			break;
			
		case 'z':
			_ARG_;
			vmpagesize = (long)atol(arg);
			break;
			
		case 'M':
			_ARG_;
			VMLeave = (long)atol(arg);
			break;
#endif		
                case 'o':
                	_ARG_;
                        strcpy(outfile,arg);
#if defined(SUN4)
			unlink(outfile);
#else
                        remove(outfile);
#endif
#ifdef LATTICE
                        if( !hexopt ) 
			{
				_iomode ^= O_RAW;
	                        outfd = fopen(outfile,"w");
				_iomode ^= O_RAW;
			}
#endif
#ifdef MWC
			outfd = fopen(outfile,hexopt?"w+":"wb+");
#endif
#if defined(SUN4) || defined(RS6000)
			outfd = fopen(outfile,"w+");
#endif
#ifdef SUN
			outfd = fopen(outfile,"w+");
#endif
#ifdef IBMPC
			outfd = fopen(outfile,"w+");
#endif
#ifdef __DOS386
			outfd = fopen(outfile,"wb+");
#endif
#ifdef __HELIOS
			outfd = fopen(outfile,hexopt?"w+":"wb+");
#endif
                        if( outfd == 0 ) error("Cannot open %s for output",outfile);
#ifdef __STDC__
			setvbuf( outfd, outbuf, _IOFBF, BUFSIZE );
#endif
                        break;


		case 'l':
			_ARG_;
			inlib = TRUE;
			goto inputfile;

		case '\0':
			strcpy( infile, "stdin" );
			infd   = stdin;
			goto openedfile;
			break;
                }
                *aargv = ++argv;
                return FALSE;
        }
        else
	  {
           inputfile:
	        strcpy(infile,arg);
#ifdef LATTICE
                if( !preasm ) 
		{
			_iomode ^= O_RAW;
	                infd = fopen(infile,"r");
                	_iomode ^= O_RAW;
		}
#endif
#ifdef MWC
		infd = fopen(infile,preasm?"r":"rb");
#endif
#if defined(SUN4) || defined(RS6000)
		infd = fopen(infile,"r");
#endif
#ifdef SUN
		infd = fopen(infile,"r");
#endif
#ifdef IBMPC
		infd = fopen(infile,"r");
#endif
#ifdef __DOS386
		infd = fopen(infile,"rb");
#endif
#ifdef __HELIOS
		infd = fopen(infile,"rb");
#endif
                if( infd == 0 )
		{
		 error("Cannot open %s for input",infile);
		}
openedfile:		
		if( verbose )
		{
			fprintf(verfd,"Reading %s                                     \r",infile);
			fflush(verfd);
		}
#ifdef __STDC__
		setvbuf( infd, inbuf, _IOFBF, BUFSIZE );
#endif
                lineno = 1;
                *aargv = ++argv;
                return TRUE;
        }
}


#ifdef __DOS386
PUBLIC void error(BYTE *str, ...)
{
	va_list	argptr;

	va_start(argptr, str);
#ifdef LINENO
        fprintf(verfd,"Fatal Error: %s %ld :",infile,lineno);
#else
        fprintf(verfd,"Fatal Error: %s :",infile);
#endif
        vfprintf(verfd,str,argptr);
        putc('\n',verfd);
        maxerr = max(maxerr,20);
#if defined(SUN4)
	{
		float x = 0.0;
		float y = 0.0;
		float z;

		z = x/y;		/* force fpe to get core dump	*/
	}
#endif
	va_end(argptr);
	exit(maxerr);
}

PUBLIC void warn(BYTE *str, ...)
{
	va_list	argptr;

	va_start(argptr, str);
#ifdef LINENO
        fprintf(verfd,"Warning: %s %ld :",infile,lineno);
#else
        fprintf(verfd,"Warning: %s :",infile);
#endif
        vfprintf(verfd,str,argptr);
        putc('\n',verfd);
	va_end(argptr);
}

PUBLIC void warn_error(BYTE *str, ...)
{
	va_list	argptr;

	va_start(argptr, str);
#ifdef LINENO
        fprintf(verfd,"Warning: %s %ld :",infile,lineno);
#else
        fprintf(verfd,"Warning: %s :",infile);
#endif
        vfprintf(verfd,str,argptr);
        putc('\n',verfd);

        errors++;
        maxerr = max(maxerr,10);
	va_end(argptr);
}


PUBLIC void report(BYTE *str, ...)
{
	va_list argptr;

	if( !verbose ) return;

	va_start(argptr, str);
        vfprintf(verfd,str,argptr);
        putc('\n',verfd);
	va_end(argptr);
}


PUBLIC void _trace(BYTE *str, ...)
{
       va_list argptr;

       va_start(argptr, str);
       fprintf(verfd,"%x: ",filepos);
       vfprintf(verfd,str,argptr);
       putc('\n',verfd);
       va_end(argptr);
}


PUBLIC void recover(BYTE *str, ...)
{
	va_list	argptr;

	va_start(argptr, str);
        warn(str, argptr);
	va_end(argptr);
        longjmp(error_level,1);
}

#else /* !__DOS386 */

PUBLIC void error(str,a,b,c,d,e,f)
BYTE *str;
INT a,b,c,d,e,f;
{
#ifdef LINENO
        fprintf(verfd,"Fatal Error: %s %ld :",infile,lineno);
#else
        fprintf(verfd,"Fatal Error: %s :",infile);
#endif
        fprintf(verfd,str,a,b,c,d,e,f);
        putc('\n',verfd);
        maxerr = max(maxerr,20);
#if defined(SUN4)
	{
		float x = 0.0;
		float y = 0.0;
		float z;

		z = x/y;		/* force fpe to get core dump	*/
	}
#endif
	exit(maxerr);
}

PUBLIC void warn(str,a,b,c,d,e,f)
BYTE *str;
INT a,b,c,d,e,f;
{
#ifdef LINENO
        fprintf(verfd,"Warning: %s %ld :",infile,lineno);
#else
        fprintf(verfd,"Warning: %s :",infile);
#endif
        fprintf(verfd,str,a,b,c,d,e,f);
        putc('\n',verfd);
}

PUBLIC void warn_error(str,a,b,c,d,e,f)
BYTE *str;
INT a,b,c,d,e,f;
{
#ifdef LINENO
        fprintf(verfd,"Warning: %s %ld :",infile,lineno);
#else
        fprintf(verfd,"Warning: %s :",infile);
#endif
        fprintf(verfd,str,a,b,c,d,e,f);
        putc('\n',verfd);

        errors++;
        maxerr = max(maxerr,10);
}


PUBLIC void report(str,a,b,c,d,e,f,g,h,i,j,k)
BYTE *str;
INT a,b,c,d,e,f,g,h,i,j,k;
{
	if( !verbose ) return;
        fprintf(verfd,str,a,b,c,d,e,f,g,h,i,j,k);
        putc('\n',verfd);
}


PUBLIC void _trace(str,a,b,c,d,e,f,g,h,i,j,k)
BYTE *str;
INT a,b,c,d,e,f,g,h,i,j,k;
{
       fprintf(verfd,"%x: ",filepos);
       fprintf(verfd,str,a,b,c,d,e,f,g,h,i,j,k);
       putc('\n',verfd);
}


PUBLIC void recover(str,a,b,c,d,e,f)
BYTE *str;
INT a,b,c,d,e,f;
{
        warn(str,a,b,c,d,e,f);
        longjmp(error_level,1);
}

#endif /* __DOS386 */

PRIVATE void tidyup()
{
        tidy1();
        if(outfd != stdout && outfd != NULL) { fclose(outfd); outfd = stdout; }
        if(verfd != stderr && verfd != NULL) { fclose(verfd); verfd = stderr; }
        if(deffd != NULL) { fclose(deffd); deffd =  NULL; }
        VMTidy();
}

PRIVATE void tidy1()
{
        if(infd != stdin && infd != NULL) 
        {
        	fclose(infd); 
        	infd = stdin; 
        }
}

PUBLIC WORD uch = -2;
PUBLIC WORD unreadready = FALSE;

#ifdef RDCH_FN

PUBLIC WORD asm_rdch()
{
   WORD c = uch;

   if( c == -2 ){
#ifdef MWC
	c = bingetc(infd);
	if( c == 0xffffL ) c = EOF;
#else
        c = getc(infd);
#endif
   }
   else uch = -2;
   filepos++;
   return c;
}


PUBLIC WORD unrdch(c)
WORD c;
{
   filepos--;
   uch = c;
}
#endif
#ifndef LATTICE
PUBLIC WORD min(a,b)
WORD a,b;
{
        return a<b ? a : b;
}

PUBLIC WORD max(a,b)
WORD a,b;
{
        return a>b ? a : b;
}
#endif
