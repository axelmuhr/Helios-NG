/****************************************************************/
/* File: link.c                                                 */
/*                                                              */
/*                                                              */
/* Author: NHG 26/9/88                                          */
/****************************************************************/
static char *SccsId = "%W%\t%G% Copyright (C) Perihelion Software Ltd.";

#include "link.h"
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include "hash.c"

#ifdef LATTICE
#include <fcntl.h>
#endif

PUBLIC  FILE *infd;
PUBLIC  FILE *outfd;
PUBLIC  FILE *verfd;
PUBLIC  int *error_level;
PRIVATE jmp_buf default_level;
PUBLIC  WORD errors;
PRIVATE WORD phase = 0;
PUBLIC  WORD traceflags;
PRIVATE WORD maxerr = 0;
PUBLIC  WORD hexopt = FALSE;
PRIVATE WORD image  = i_helios;
PUBLIC  WORD defopt = FALSE;
PUBLIC  WORD verbose = FALSE;
PUBLIC  WORD inlib = FALSE;
PUBLIC  FILE *deffd;
PUBLIC  WORD vmpagesize = 8*1024;

PRIVATE WORD needobjed = FALSE;
PRIVATE char *progname = NULL;
PRIVATE WORD stack_size = -1;
PRIVATE WORD heap_size = -1;

PUBLIC  BYTE infile[128];
PRIVATE BYTE outfile[128];

/* void unload(void); */
PRIVATE WORD parsearg(ellipsis);
WORD max(ellipsis);
PRIVATE void tidy1(ellipsis);
PRIVATE void tidyup(ellipsis);
PRIVATE void _fprintf(ellipsis);

#ifdef MWC
long _stksize = 20000;
#endif
#ifdef IBMPC
int _stack = 4096;
#endif

/********************************************************/
/* main                                                 */
/*                                                      */
/* Initialise everything and call the linker            */
/*                                                      */
/********************************************************/

PUBLIC int main(argc,argv)
int argc;
BYTE **argv;
{
   WORD infiles = 0;
   clock_t start=0, inend=0, end=0;

        infd = stdin;
        outfd = stdout;
#ifdef LATTICE
        verfd = stdout;
#else
        verfd = stderr;
#endif

   deffd = (FILE *) NULL;
        infile[0] = 0;
        error_level = default_level;
        errors = 0;

        argv++;
   argc--;

   start = clock();
   
        if( setjmp(error_level) == 0 )
        {
                initmem();
                initcode();
                initmodules();
                initsym();

                report("Helios Linker V1.0");
                report("Copyright (C) 1987, Perihelion Software Ltd.");

                phase = 1;
                while( *argv != NULL ) 
                {
                        if ( parsearg(&argv) == TRUE )
                        {
            infiles++;
                                readfile();
                                tidy1();
                                inlib = FALSE;
                        }
                }

      if( infiles == 0 )   /* if no command line files, use stdin */
      {
         readfile();
      }

                genend();   /* finish off last module */

      inend = clock();
      
      setmodules();

      if( errors == 0 )
      {
         phase = 2;
         scancode();
      }   
         
                if( errors == 0 )
                {
                        phase = 3;

                        genimage();

         fflush(outfd);
         
                        report("Image file generated");
         
                        if( needobjed ) objed(outfd,progname,stack_size,heap_size);
                }

        }

   end = clock();
   
report("Times (secs): Total %d Input %d Output %d",
   (end-start)/CLK_TCK,(inend-start)/CLK_TCK,(end-inend)/CLK_TCK);
report("Memory used:   Code     %8ld Local Heap  %8ld",codesize,heapsize);
report("               Symbols  %8ld\n",symsize);

   VMStats();
      
        tidyup();

        exit(maxerr);

          SccsId = SccsId;
          
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
                switch( (int)locase(*arg) )
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
                                case 'c': traceflags ^= db_gencode;  break;
                                case 'o': traceflags ^= db_genimage; break;
            case 'f': traceflags ^= db_files;    break;
            case 'm': traceflags ^= db_mem;      break;
            case 'y': traceflags ^= db_sym;      break;
            case 'd': traceflags ^= db_modules;  break;
            case 's': traceflags ^= db_scancode;  break;
                                }
                                arg++;
                        }
                        break;

                case 'x':
                        hexopt = TRUE;
                        break;

                case 'i':
                        image = *arg;
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

#ifdef VM
      case 'm':
         _ARG_;
         VMfilename = arg;
         break;
         
      case 'z':
         _ARG_;
         vmpagesize = (long)atol(arg);
         break;
#endif      
                case 'o':
                   _ARG_;
                        strcpy(outfile,arg);
#ifdef ORION
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
#ifdef ORION
         outfd = fopen(outfile,"w+");
#endif
#ifdef SUN
         outfd = fopen(outfile,"w+");
#endif
#ifdef IBMPC
         outfd = fopen(outfile,"w+");
#endif
#ifdef NORCROFT
#ifdef TRIPOS
         outfd = fopen(outfile,"w+");
#else
         outfd = fopen(outfile,hexopt?"w+":"wb+");
#endif
#endif
                        if( outfd == 0 ) error("Cannot open %s for output",outfile);
                        break;


      case 'l':
         _ARG_;
         inlib = TRUE;
         goto inputfile;
         
                }
                *aargv = ++argv;
                return FALSE;
        }
        else {
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
#ifdef ORION
      infd = fopen(infile,"r");
#endif
#ifdef SUN
      infd = fopen(infile,"r");
#endif
#ifdef IBMPC
      infd = fopen(infile,"r");
#endif
#ifdef NORCROFT
#ifdef TRIPOS
      infd = fopen(infile,"r");
#else
      infd = fopen(infile,"rb");
#endif
#endif
                if( infd == 0 )
      {
       error("Cannot open %s for input",infile);
      }
      if( verbose )
      {
         fprintf(verfd,"Reading %s                                     \r",infile);
         fflush(verfd);
      }
                *aargv = ++argv;
                return TRUE;
        }
}


PUBLIC void error(str,a,b,c,d,e,f)
BYTE *str;
INT a,b,c,d,e,f;
{
#ifdef LINENO
        _fprintf(verfd,"Fatal Error: %s %ld :",infile,lineno);
#else
        _fprintf(verfd,"Fatal Error: %s :",infile);
#endif
        _fprintf(verfd,str,a,b,c,d,e,f);
        putc('\n',verfd);
        maxerr = max(maxerr,20);
/*        longjmp(&default_level,1);*/
   exit(maxerr);
}


PUBLIC void warn(str,a,b,c,d,e,f)
BYTE *str;
INT a,b,c,d,e,f;
{
#ifdef LINENO
        _fprintf(verfd,"Warning: %s %ld :",infile,lineno);
#else
        _fprintf(verfd,"Warning: %s :",infile);
#endif
        _fprintf(verfd,str,a,b,c,d,e,f);
        putc('\n',verfd);
        errors++;
        maxerr = max(maxerr,10);
}


PUBLIC void report(str,a,b,c,d,e,f)
BYTE *str;
INT a,b,c,d,e,f;
{
   if( !verbose ) return;
        _fprintf(verfd,str,a,b,c,d,e,f);
        putc('\n',verfd);
}


PUBLIC void _trace(str,a,b,c,d,e,f)
BYTE *str;
INT a,b,c,d,e,f;
{
       _fprintf(verfd,"Trace: ");
       _fprintf(verfd,str,a,b,c,d,e,f);
       putc('\n',verfd);
}


PUBLIC void recover(str,a,b,c,d,e,f)
BYTE *str;
INT a,b,c,d,e,f;
{
        warn(str,a,b,c,d,e,f);
        longjmp(error_level,1);
}


PRIVATE void _fprintf(fd,str,a,b,c,d,e,f)
FILE *fd;
BYTE *str;
INT a,b,c,d,e,f;
{
   static BYTE fbuf[128];
   BYTE *t = fbuf;
   BYTE *s = str;
   while( *s != '\0' )
   {
      if ( *s == '%' ) 
      {
         *t = *s;
         t++;
         s++;
         while( '0' <= *s && *s <= '9' ) 
         {
             *t = *s;
             t++;
             s++;
         }
         switch( *s ) 
         {
         case 'x': 
         case 'X': *t = 'l'; 
            t++;
            *t = 'x'; 
            t++;
            break;

         case 'd': 
         case 'D': *t = 'l'; 
            t++;
            *t = 'd'; 
            t++;
            break;

         default: *t = *s; 
            t++;
            break;
         }
         s++;
      }
      *t = *s;
      t++;
      s++;
   }
   *t = '\0';
   fprintf(fd,fbuf,a,b,c,d,e,f);
}



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
        if(infd != stdin && infd != NULL) { fclose(infd); infd = stdin; }
}

PUBLIC WORD uch = 0;
PUBLIC WORD unreadready = FALSE;

#ifdef MWC

/* The following function was formerly called rdch()- but this clashes
 * with the system function of the same name, causing the compiler to moan.
 * Cannot use system definition of rdch() because it is not same- so I have
 * renamed this function so that I can use it.
 * AEv AEv */
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
        return c;
   }
   uch = -2;
   return c;
}


PUBLIC WORD unrdch(c)
WORD c;
{
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
