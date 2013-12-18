/*
 * Dump the contents of an Helios object file
 *
 * objdump.c
 *
 * Authors:
 *   Criss Selwyn 	December '88
 *   Paul Beskeen 	March    '89 ...
 *   Tony Cruickshank 	June     '89
 *   James G Smith 	May     1990	Added ARM disassembler (and other tweaks)
 *   Nick Clifton	August  1991	Added C40 tweaks
 *   James G Smith  January 1994    Use new "ARM Ltd clx" disassembler
 *
 * RCS:
 *  $Header: /dsl/HeliosARM/jamie/RTNucleus/cmds/linker/RCS/objdump.c,v 1.33 1994/02/08 11:23:14 vlsi Exp $
 *
 * @@@ Note that this command assumes that the input is in little endian format
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#if defined(SUN4) || defined(__unix)
#include <unistd.h>	/* for SEEK_SET */
#endif

#ifdef __ARM
#if defined(SUN4) || defined(RS6000) /* FIXME: we need a native Helios-ARM version of clxlib.a */
#include "disass.h" /* currently held in "armltd.binaries/include" */
#else
#include "ARMshape.h"
#endif
#endif

#include "link.h"	/* for OBJCODE etc */

#ifndef FALSE
#define TRUE	~0
#define FALSE	0
#endif

#define MORE		0x80
#define NFLAG		0x40

char *	ProgName     = NULL;
FILE *	f            = NULL;
FILE *	outfd        = stdout;
int 	pc           = 0;
int 	dataoff      = 0;
int 	codetableoff = 0; 	/* split mod tab support */
int 	ch;
int 	ShortOutput  = 1;
int 	sizeinfo     = 0;	/* provide image and static data area information */
int 	nameinfo     = 0;	/* provide symbol definition information */

#if defined __ARM || defined __C40
int 	dodisass     = 0;	/* No disassembly by default */
int	pcsregs	     = FALSE;	/* default to std machine register names */

#if defined(__ARM) && (defined(SUN4) || defined(RS6000))
static char *local_decode(dis_cb_type type,int32 offset,unsigned32 address,int width,void *arg,char *buffer)
{
 /* We provide no more decoding at the moment */
 return(buffer) ;
}

static void disassemble(unsigned long instruction,unsigned long pc,int flag)
{
 static char buffer[256] ; /* disassembly line */

 disass_32or26((unsigned32)instruction,(unsigned32)pc,buffer,NULL,local_decode,TRUE) ;
 printf(buffer) ;
 return ;
}
#else
extern void	disassemble( unsigned long instruction, unsigned long pc, int flag );
#endif
#endif


/*-------------------------start of code---------------------------------------------*/

/*
 * display an error or information message
 */

void
inform(
       char *	message,
       ...		)
{
  va_list	args;


  va_start( args, message );

#if 0
  fflush( stderr );

  fseek( stderr, 0L, SEEK_END );
#endif
  
  if (ProgName)
    fprintf( stderr, "%s: ", ProgName );

  vfprintf( stderr, message, args );

  fprintf( stderr, "\n" );

#if 0
  fflush( stderr );
#endif
  
  va_end( args );

  return;
  
} /* inform */


void
tidyup( int n )
{
  if (f)
    fclose( f );
  
  exit( n );

} /* tidyup */


int
readobjnum( void )
{
  int 	ch     = getc( f );
  int 	nflag  = (ch & NFLAG) != 0;
  int 	r      = ch & 0x3f;

  
  if (ch == EOF)
    {
      inform( "Error - Unexpected EOF" );

      tidyup( 1 );
    }
  
   while ((ch & MORE) != 0)
     {
       if ((ch = getc( f )) == EOF)
	 {
	   inform( "Error - Unexpected EOF" );
	   
	   tidyup( 1 );
	 }

       r  = (r << 7) + (ch & 0x7f);
     }

  return nflag ? -r : r;

} /* readobjnum */


void
show_string( void )
{
  char	ch;
  

  while ((ch = getc( f )) != 0)
    {
      if (!sizeinfo)
	fputc( ch, outfd );
    }

  return;
  
} /* show_string */


void
show_label_string( void )
{
  char	ch;

  
  while ((ch = getc( f )) != 0)
    {
      if (!sizeinfo || nameinfo)
	fputc( ch, outfd );
    }

  fputc( ':', outfd ); 

  return;
  
} /* show_label_string */


void
show_code( int n )
{
#define CODEMAX 16
  int 		i;
#if defined __ARM || defined __C40
  unsigned int 	instruction;
  int          	localpc = pc;
  int		doprint = 1;

  
  if (dodisass)
    {
      /*
       * Provide disassembled code sections in the C40/ARM versions.
       * Note: This cannot synchronise on the instructions if the CODE is
       *	    actually non-word-aligned data.
       *
       * We are called with the text "PC = 0xZZZZ " already displayed.
       */
      
      if (!sizeinfo)
	fprintf( outfd, "CODE %#x\n", n );
     
      i = 0;
      
      while ((n - i) >= 4)		/* enough for an C40/ARM instruction */
	{
	  int	j;

	  
	  instruction = 0x00000000;

#ifdef __BIGENDIAN
	  for (j = 0; (j < 4); j++)
	    instruction = (instruction << 8) | (getc( f ) & 0xFF);
#else
	  for (j = 0; (j < 4); j++)
	    instruction |= (getc( f ) << (j * 8));
#endif

	  if (!sizeinfo) 
	    {
#ifdef __C40
	      fprintf( outfd, "PC = %#010x ", localpc / 4 );
	      fprintf( outfd, "     %#010x ", instruction );
#else
	      fprintf( outfd, "PC = %08X ", localpc );
	      fprintf( outfd, "     %08X ", instruction );
#endif
	      
	      for (j = 0; (j < 4); j++)
		{
#ifdef __BIGENDIAN
		  char	dc = (instruction >> ((3 - j) * 8)) & 0xFF;
#else
		  char  dc = (instruction >> (j * 8)) & 0xff;
#endif		  
		  fputc( ((dc < ' ') || (dc > 0x7E)) ? '.' : dc, outfd );
		}
	      
	      fprintf( outfd, "     " );
	      
	      disassemble( instruction, localpc, FALSE );
	      
	      fputc( '\n', outfd );
	    }
	  
	  localpc += 4;		/* nice byte addressed machine */
	  i       += 4;
	}

      /* deal with any non-word aligned data at the end */
      
      if (i < n)
	{
	  int j;

	  if (!sizeinfo)
	    {
#ifdef __C40
	      fprintf( outfd, "PC = %#010x        ", localpc / 4 );
#else
	      fprintf( outfd, "PC = %08X      ", localpc );
#endif
	      
	      for (j = 0; (j < (4 - (n - i))); j++)
		fprintf( outfd, "  " );
	    }

	  instruction = 0x00000000;
	  
#ifdef __BIGENDIAN
	  for (j = 0; (j < n - i); j++)
	    instruction = (instruction << 8) | (getc( f ) & 0xFF);
#else
	  for (j = 0; (j < (n - i)); j++)
	    instruction |= (getc( f ) & 0xff) << (j * 8);
#endif

	  if (!sizeinfo)
	    {
	      for (j = (n - i); (j > 0);)
		{
		  j--;
		  
		  fprintf( outfd, "%02x",(instruction & (0xFF << (j * 8))) >> (j * 8));
		}

	      fputc( ' ', outfd );

	      for (j = 0; (j < (4 - (n - i))); j++)
		fputc( ' ', outfd );

	      for (j = 0; (j < (n - i)); j++)
		{
#ifdef __BIGENDIAN
		  char	dc = (instruction >> ((3 - j) * 8)) & 0xFF;
#else
		  char	dc = (instruction >> (j * 8)) & 0xFF;
#endif
		  
		  fputc( ((dc < ' ') | (dc > 0x7E)) ? '.' : dc, outfd );
		}

#ifdef __C40
	      fprintf( outfd, " :\n" );
#else
          fprintf( outfd, "\n" );
#endif
	    }
	}
    }
  else
    {
      if (!sizeinfo)
	fprintf( outfd, "CODE %#x :", n );
      else
	doprint = 0;

      for (i = 1; (i <= n); i++)
	{
	  if ((i % CODEMAX) == 0)
	    {
	      if (!sizeinfo)
		{
		  if (ShortOutput)
		    doprint = 0 ;
		  else
		    fprintf( outfd, "\n\t" );
		}
	    }

	  if (doprint)
	    fprintf( outfd, " %02X", getc( f ) );
	  else
	    fseek( f, SEEK_CUR, 1 );
	}

      if (!sizeinfo)
	fputc( '\n', outfd );
    }
#else /* neither __ARM nor __C40 */
  int	doprint = 2;


  fprintf( outfd, "CODE %#x :", n );

  for (i = 0; i < n; i++)
    {
      if (i % CODEMAX == 0)
	{
	  if (ShortOutput)
	    --doprint;
	  else
	    fprintf( outfd, "\n\t" );
	}

      if (doprint > 0)
	fprintf( outfd, " %02X", getc( f ) );
      else
	fseek( f, SEEK_CUR, 1 );
    }

  fputc( '\n', outfd );
  
#endif /* __ARM || __C40 */

  return;
  
} /* show_code */


void
show_patch( char * s )
{
  int	ch = getc( f );


  if (ch == EOF)
    {
      inform( "\nError: unexpected end of file in image, pos = %ld", ftell( f ) );

      return;
    }
  
  if (!sizeinfo)
    fprintf( outfd, "%s ", s );

  if (OBJPATCH <= ch && ch <= OBJPATCHMAX)
    {
      int n = 0;

      
      if (ch != PATCHSWAP)
	n = readobjnum();

      if (!sizeinfo)
	{
	  switch (ch)
	    {
	    case PATCHADD:  
	      fprintf( outfd, "PATCH ADD: %#x ", n );
	      break;
	      
	    case PATCHSHIFT:
	      fprintf( outfd, "PATCH SHIFT by: %d ", n );
	      break;

#ifdef __ARM
	    case PATCHARMDT:
	      fprintf( outfd, "PATCH ARM DT    : ");
	      
	      if (dodisass)
		disassemble( n, pc, FALSE );
              else
		fprintf( outfd, "0x%08X", n );
	      
	      fprintf( outfd, " :" );
	      break;
	      
	    case PATCHARMDP:
	      fprintf( outfd, "PATCH ARM DP    : " );
	      
	      if (dodisass)
		disassemble( n, pc, FALSE );
              else
		fprintf( outfd, "0x%08X", n );
	      
	      fprintf( outfd, " :" );
	      break;
	      
	    case PATCHARMDPLSB:
	      fprintf( outfd, "PATCH ARM DPLSB : " );

	      if (dodisass)
		disassemble( n, pc, FALSE );
              else
		fprintf( outfd, "0x%08X", n );
	      
	      fprintf( outfd, " :" );
	      break;
	      
	    case PATCHARMDPREST:
	      fprintf( outfd, "PATCH ARM DPREST: " );
	      
	      if (dodisass)
		disassemble( n, pc, FALSE );
              else
		fprintf( outfd, "0x%08X", n );
	      
	      fprintf( outfd, " :" );
	      break;
	      
	    case PATCHARMDPMID:
	      fprintf( outfd, "PATCH ARM DPMID : " );
	      
	      if (dodisass)
		disassemble( n, pc, FALSE );
              else
		fprintf( outfd, "0x%08X", n );

	      fprintf( outfd, " :" );
	      break;
	      
	    case PATCHARMJP:
	      fprintf( outfd, "PATCH ARM JP    : " );

	      if (dodisass)
		disassemble( n, -8, FALSE );
              else
		fprintf( outfd, "0x%08X", n );
	      
	      fprintf( outfd, " :" );
	      break;


	    case PATCHARMAOFLSB:
	      fprintf( outfd, "PATCH ARM AOF LSB : r%d :", n );
	      break;
	      
	    case PATCHARMAOFREST:
	      fprintf( outfd, "PATCH ARM AOF REST: r%d :", n );
	      break;
	      
	    case PATCHARMAOFMID:
	      fprintf( outfd, "PATCH ARM AOF MID : r%d :", n );
	      break;
#endif /* __ARM */
	      
#ifdef __C40
	    case PATCHC40DATAMODULE1:
	      fprintf( outfd, "M/C PATCH C40 DATAMODULE1: " );

	      if (dodisass)
		disassemble( n, pc, TRUE );
	      else
		fprintf( outfd, "%#010x ", n );
	      break;

	    case PATCHC40DATAMODULE2:
	      fprintf( outfd, "M/C PATCH C40 DATAMODULE2: " );

	      if (dodisass)
		disassemble( n, pc, TRUE );
	      else
		fprintf( outfd, "%#010x ", n );
	      break;

	    case PATCHC40DATAMODULE3:
	      fprintf( outfd, "M/C PATCH C40 DATAMODULE3: " );

	      if (dodisass)
		disassemble( n, pc, TRUE );
	      else
		fprintf( outfd, "%#010x ", n );
	      break;

	    case PATCHC40DATAMODULE4:
	      fprintf( outfd, "M/C PATCH C40 DATAMODULE4: " );

	      if (dodisass)
		disassemble( n, pc, TRUE );
	      else
		fprintf( outfd, "%#010x ", n );
	      break;

	    case PATCHC40DATAMODULE5:
	      fprintf( outfd, "M/C PATCH C40 DATAMODULE5: " );

	      if (dodisass)
		disassemble( n, pc, TRUE );
	      else
		fprintf( outfd, "%#010x ", n );
	      break;

	    case PATCHC40MASK24ADD:
	      fprintf( outfd, "M/C PATCH C40 MASK 24 ADD: " );

	      if (dodisass)
		disassemble( n, pc, TRUE );
	      else
		fprintf( outfd, "%#010x ", n );
	      
	      break;

	    case PATCHC40MASK16ADD:
	      fprintf( outfd, "M/C PATCH C40 MASK 16 ADD: " );

	      if (dodisass)
		disassemble( n, pc, TRUE );
	      else
		fprintf( outfd, "%#010x ", n );
	      
	      break;

	    case PATCHC40MASK8ADD:
	      fprintf( outfd, "M/C PATCH C40 MASK 8 ADD: " );

	      if (dodisass)
		disassemble( n, pc, TRUE );
	      else
		fprintf( outfd, "%#010x ", n );

	      break;

#endif /* __C40 */

	    case PATCHSWAP:
	      fprintf( outfd, "PATCH SWAP: " );
	      break;
	      
	    case PATCHOR:
	      fprintf( outfd, "PATCH OR: %#x ", n );
	      break;
	      
	    default:
	      inform( "Warning - unknown M/C Patch %#x : %#x\n", ch, n );
	      
	      fprintf( outfd, "Unknown M/C Patch %#x\n", ch );
	    }
	} /* if (!sizeinfo) */

      show_patch( "" );
    }
  else
    {
      switch( ch )
	{
	case OBJDATASYMB:
	  if (!sizeinfo)
	    fprintf( outfd, "DATASYMB " );
	  
	  show_string();
	  break;
	  
	case OBJCODESYMB:
	  if (!sizeinfo)
	    fprintf( outfd, "CODESYMB " );
	  
	  show_string();
	  break;
	  
	case OBJDATAMODULE:
	  if (!sizeinfo)
	    fprintf( outfd, "DATAMODULE " );
	  
	  show_string();
	  break;
	  
	case OBJMODSIZE:
	  if (!sizeinfo)
	    fprintf( outfd, "MODSIZE" );
	  
	  break;
	  
	case OBJMODNUM:
	  if (!sizeinfo)
	    fprintf( outfd, "MODNUM" );
	  
         break;
	  
	case OBJLABELREF:
	  if (!sizeinfo)
	    fprintf( outfd, "LABELREF " );
	  
	  show_string();
	  break;

#ifdef __C40
	case OBJCODESTUB:
	  if (!sizeinfo)
	    fprintf( outfd, "CODESTUB " );
	      
	  show_string();
	      
	  break;	

	case OBJADDRSTUB:
	  if (!sizeinfo)
	    fprintf( outfd, "ADDRSTUB " );
	      
	  show_string();
	      
	  break;	
#endif /* __C40 */
	  
	default:
	  inform( "Warning - bad patch object format: %#x at offset 0x%08X", ch, (ftell( f ) - 1 ) );
	  
	  fprintf( outfd, "Bad patch object format: %#x\n", ch );
#if 0
	  tidyup( 1 );
#endif
	}
    }

  return;
  
} /* show_patch */


void
usage( void )
{
#if defined __ARM || defined __C40
  inform( "Usage: [-h] [-l] [-d] [-p] [-i] [-n] [+pcinc] [-o output-file] input-files" );
#else
  inform( "Usage: [-h] [-l] [-i] [-n] [+pcinc] [-o output-file] input-files" );
#endif
  
  inform( "-h :         Provide this help information" );
  inform( "-l :         Give full information (long)"  );
  
#if defined __ARM || defined __C40
  inform( "-d :         Provide disassembly" );
  inform( "-p :         Use PCS register names in disassembly" );
#endif

  inform( "-i :         Provide image size and static allocation info only" );
  inform( "-n :         Provide symbol definition information only" );
  inform( "+pcinc:      Allows you to add an address offset. This can" );
  inform( "             be used between the input files." );
  inform( "-o <file> :  Specifies the name of a file to contain the program's output" );
  
  tidyup( 1 );

  return;
  
} /* usage */

#if defined(__ARM) && (defined(SUN4) || defined(RS6000))
char *ARM_regnames[] = {"a1","a2","a3","a4","v1","v2","v3","v4","v5","dp","sl","fp","ip","sp","lr","pc"} ;
#endif

int
main(
     int 	argc,
     char **	argv )
{
  char *	s;
  int 		i;
  int 		file = 0;

#if defined(__ARM) && (defined(SUN4) || defined(RS6000))
  disass_setregnames(ARM_regnames,NULL) ;
#endif

  if ((s = (char *)strrchr( argv[ 0 ], '/' )) != NULL)
    {
      ProgName = s + 1;
    }
  else
    {
      ProgName = argv[ 0 ];
    }
  
  if (argc < 2)
    usage();

  /* Parse Arguments */
  
  for (i = 1; i < argc; ++i)
    {
      s = argv[ i ];
      
      if (*s != '-' && *s != '+')
	{
	  file++;
	  
	  if ((f = fopen( argv[ i ], "rb" )) == 0)
	    {
	      inform( "Error - Unable to open input file: %s\n", argv[ i ] );
	      
	      tidyup( 1 );
	    }

	  fprintf( outfd, "OBJECTFILE: %s\n", argv[ i ] );
	}
      else
	{
	  if (*s++ == '-')
	    {
	      switch (*s++)
		{
		case 'l' :
		  ShortOutput = 0;
		  break;

		case 'h' :
		  usage();
		  break;		/* should never get here */

		case 'i' :
		  sizeinfo = -1;
		  break;

		case 'n' :
		  sizeinfo = -1;	/* stop full object dump */
		  nameinfo = -1;	/* but provide name information */
		  break;
#if defined __ARM || defined __C40
		case 'd' :
		  dodisass = -1;
		  break;
		case 'p':
		  pcsregs = TRUE;
		  break;		  
#endif
		case 'o':
		  if (*s == '\0')
		    s = argv[ ++i ];		  

		  outfd = fopen( s, "w" );

		  if (outfd == NULL)
		    {
		      inform( "Warning - unable to open output file %s", s );

		      outfd = stdout;
		    }
		  
		  break;

		default:
		  inform( "Warning - Unknown switch %c", *s );
		  usage();
		  break;
		}
	    }
	  else /* must be + */
	    {       
	      if (*s != '\0')
		pc += atoi( s );
	      else
		pc += atoi( argv[ ++i ] );
	    }
	  
	  continue;
	}

      /* Quick check.
       * If the first byte of the file is not a GHOF directive
       * then we are not parsing a GHOF file and we can abort.
       */

      ch = getc( f );

      if (ch == EOF)
	{
	  inform( "Empty input file!" ); 
	}
      else if (ch >= NFLAG)
	{
	  inform( "Input file is not in GHOF format" );

	  if (ch == 0xc5) inform( "Input file might be in ARM Object Format" );
	  else if (ch == 0xc3) inform( "Input file might be in byte swapped ARM Object Format" );
	}
      else
	{
	  do
	    {
	      int	n;
	      
	      
	      if (!sizeinfo)
#ifdef __C40
		fprintf( outfd, "PC = %#010x ", pc / 4 );
#else
	      fprintf( outfd, "PC = %08X ", pc );
#endif
	      
	      switch (ch)
		{
		case OBJCODE:
		  n = readobjnum();
		  
		  if (n < 0)
		    inform( "CODE directive encountered with negative size (%x)", n );
		  else
		    show_code( n );
		  
		  pc += n;
		  
		  break;
		  
		case OBJBSS:
		  n = readobjnum();
		  
		  if (!sizeinfo)
		    fprintf( outfd, "BSS: %#x\n", n );
		  
		  pc += n;
		  
		  break;
		  
		case OBJINIT:
		  if (!sizeinfo)
		    fprintf( outfd, "INIT\n" );
		  
		  pc += 4;
		  
		  break;
		  
		case OBJBYTE:
		  show_patch( "BYTE" );
		  
		  if (!sizeinfo)
		    fputc( '\n', outfd );
		  
		  pc += 1;
		  
		  break;
		  
		case OBJSHORT:
		  show_patch( "SHORT" );
		  
		  if (!sizeinfo)
		    fputc( '\n', outfd );
		  
		  pc += 2;
		  
		  break;
		  
		case OBJWORD:
		  show_patch( "WORD" );
		  
		  if (!sizeinfo)
		    fputc( '\n', outfd );
		  
		  pc += 4;
		  
		  break;
		  
		case OBJMODULE:
		  n = readobjnum();
		  
		  if (!sizeinfo)
		    fprintf( outfd, "MODULE %d\n", n );
		  
		  break;
		  
		case OBJGLOBAL:
		  if (!sizeinfo)
		    fprintf( outfd, "GLOBAL " );
		  
		  show_string();
		  
		  if (!sizeinfo)
		    fputc( '\n', outfd );
		  
		  break;
		  
		case OBJLABEL:
		  if (nameinfo)
#ifdef __C40
		    fprintf( outfd, "PC = %#010x ", pc / 4 );
#else
		  fprintf( outfd, "PC = %08X ", pc );
#endif		    
		  if (!sizeinfo || nameinfo)
		    fprintf( outfd, "LABEL " );
		  
		  show_label_string();
		  
		  if (!sizeinfo || nameinfo)
		    fputc( '\n', outfd );
		  
		  break;
		  
		case OBJDATA:
		  n = readobjnum();
		  
		  if (!sizeinfo)
		    {
		      fprintf( outfd, "DATA %#5x ", n );
		      
		      fprintf( outfd, "STATIC OFFSET = %#5x ", dataoff );
		    }
		  
		  show_string();
		  
		  if (!sizeinfo)
		    fputc( '\n', outfd );
		  
		  dataoff += n;
		  
		  break;
		  
		case OBJCOMMON:
		  n = readobjnum();
		  
		  if (!sizeinfo)
		    {
		      fprintf( outfd, "COMMON %#x ", n );
		      
		      fprintf( outfd, "STATIC OFFSET = %#x (may now be erroneous) ", dataoff );
		    }
		  
		  show_string();
		  
		  if (!sizeinfo)
		    fputc( '\n', outfd );
		  
		  dataoff += n;
		  
		  break;
		  
		case OBJCODETABLE: /* split module table support */
		  if (!sizeinfo)
		    fprintf( outfd, "CODETABLE " );
		  
		  show_string();
		  
		  if (!sizeinfo)
		    fputc( '\n', outfd );
		  
		  codetableoff += 4;	/* uses 4 bytes for address in static data */
		  
		  break;
		  
		case OBJREF:
		  if (!sizeinfo)
		    {
		      fprintf( outfd, "REF " );
		      
		      show_string();
		      
		      fputc( '\n', outfd );
		    }
		  break;	
		  
		default:
		  inform( "Warning - bad object format %#x at offset 0x%08X", ch, (ftell( f ) - 1) );
		  
		  fprintf( outfd, "Bad object format %#x\n (skipping entire word)", ch );
		  
		  (void) getc( f );
		  (void) getc( f );
		  (void) getc( f );
		  
#if 0
		  tidyup( 1 );
#endif
		}
	    }
	  while ((ch = getc( f )) != EOF);
	}
      
      fclose( f );

      if (sizeinfo && !nameinfo)
	{
	  fprintf( outfd, "Image size       = 0x%08X\n", pc );
	  fprintf( outfd, "Static data size = 0x%08X\n", dataoff );	  
	  fprintf( outfd, "Code table size  = 0x%08X\n", codetableoff );
	  
	  pc           = 0;
	  dataoff      = 0;
	  codetableoff = 0;
	  
	  fputc( '\n', outfd );
	}
    }
  
  if (!sizeinfo)
    {
      fprintf( outfd, "\nTotal image size       = %#x\n", pc );
      fprintf( outfd, "Total static data size = %#x\n", dataoff );
      fprintf( outfd, "Total code table size  = %#x\n", codetableoff );
    }

  return( 0 );

} /* main */


#ifdef SUN4
/*
 * The gcc compiler appears to generate references to the following functions
 * without providing them in a standard library.  Since the code for the
 * linker does not use either of them, they are provided here as stubs
 */

int ___assert( void ) { return 0; }
int ___typeof( void ) { return 0; }  

#endif /* SUN4 */
