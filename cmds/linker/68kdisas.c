/*
 * 68kdisas.c - A stand alone disassembler for the Helios 68020
 *
 * Copyright (c) 1993 Perihelion Software Ltd.
 *  All Rights Reserved.
 *
 * Author: 	N Clifton
 * Version:	$Revision: 1.4 $
 * Date:	$Date: 1993/06/22 08:51:08 $
 * Header:	$Header: /hsrc/cmds/linker/RS6000/RCS/68kdisas.c,v 1.4 1993/06/22 08:51:08 nickc Exp $
 */

/*{{{ Header Files    */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <helios.h>
#include <module.h>

/*}}}*/
/*{{{ Types           */

/* 68000 instruction formats */

typedef enum
  {
    none=0,  dbit,   sbit,    movep,  ori,     andi,     subi, addi,
    eori,    cmpi,   move,    negx,   movefsr, clr,      neg,  move_ccr,
    not,     move_sr,nbcd,    pea,    swap,    movemfreg,tst,  tas,
    movem_reg,trap,  link,    unlk,   move_usp,movefusp, reset,nop,
    stop,    rte,    rts,     trapv,  rtr,     jsr,      jmp,  chk,
    lea,     addq,   subq,    scc,    dbcc,    bcc,      moveq,or,
    divu,    divs,   sbcd,    sub,    subx,    cmp,      cmpm, eor,
    and,     mulu,   muls,    abcd,   exgd,    exga,     exgm, add,
    addx,    dshift, mshift,  adda,   cmpa,    movea,    suba, ill,
    ori_ccr, ori_sr, andi_ccr,andi_sr,eori_ccr,eori_sr,  extw, extl,
    
/* 68010 instructions */
    
    moves_1, movefccr_1,rtd_1,movec_1,bkpt_1,
    
/* 68020 instructions */
    
    chm_2,    callm_2,rtm_2, cas_2, linkl_2, extbl_2, mulsl_2, divsl_2,
    trapcc_2, pack_2, unpk_2,bfld_2,fp_2
  }
instrenum;

typedef unsigned long int instrword;
typedef unsigned long int int32;

/*}}}*/
/*{{{ Constants       */

/* #define BRANCH_TRACING */

#ifdef SUN4
#include <unistd.h>	/* for SEEK_SET */
#define EXIT_SUCCESS	0
#define EXIT_FAILURE	-1
#define strtoul		strtol
#endif

#ifndef TRUE
#define TRUE	1
#define FALSE	0
#define bool	int
#endif

#define COL_ADDR	1
#define COL_HEX_VALUE	9
#define COL_BYTE_VALUES	26
#define COL_OP_CODE	45
#define COL_OPERANDS	51
#define COL_DESTINATION	66
#define COL_TRACE	83

/*}}}*/
/*{{{ Local Variables */

static unsigned char * 		ProgName     = (unsigned char *)"disas68k"; /* the name of the program */
static unsigned char *		output_file  = (unsigned char *)"-";	    /* name of output destination */
static FILE *			output	     = NULL;			    /* file handle for output */
static FILE *			input	     = NULL;			    /* file handle for input */
static unsigned long int	address      = 0;			    /* address of op code */
static unsigned long int	address_base = 0;			    /* offset to address */

/*}}}*/
/*{{{ Functions       */

/*{{{ Utilities            */

/*{{{ inform     */

/*
 * display an error or information message
 */

#ifdef __CC_NORCROFT
#pragma -v3
#endif

static void
inform(
       char *	message,
       ...		)
{
  va_list	args;


  va_start( args, message );

  if (ProgName)
    fprintf( stderr, "%s: ", ProgName );

  vfprintf( stderr, message, args );

  fprintf( stderr, "\n" );

  va_end( args );

  return;
  
} /* inform */

#ifdef __CC_NORCROFT
#pragma -v0
#endif

/*}}}*/
/*{{{ myprintf   */

#ifdef __CC_NORCROFT
#pragma -v3
#endif

/*
 * display a message, keeping a track of where we are
 */

static int 	current_column = 0;		/* number of characters printed out on current line */
static char *	print_buffer   = NULL;		/* if set, print into this buffer */


static void
myprintf( const char * format, ... )
{
  static char	buffer[ 1024 ];			/* XXX */
  va_list	args;
  char *	start;
  char *	ptr;
  

  va_start( args, format );

  if (print_buffer == NULL)
    { 
      vsprintf( buffer, format, args );
    }
  else
    {
      vsprintf( print_buffer + strlen( print_buffer ), format, args );

      va_end( args );
      
      return;
    }
  
  va_end( args );

  if (output == NULL)
    return;

  current_column += strlen( buffer );

  start = buffer;
  
  while ((ptr = (char *)strchr( start, '%' )) != NULL)
    {
      *ptr = '\0';
      
      fprintf( output, start );
      
      fprintf( output, "%%" );
      
      start = ptr + 1;
    }

  fprintf( output, start );
  
  return;
    
} /* myprintf */

#ifdef __CC_NORCROFT
#pragma -v0
#endif

/*}}}*/
/*{{{ pad_to     */

/*
 * output spaces until the given column is reached
 */

static void
pad_to( int goal_column )
{
  if (current_column < goal_column)
    {
      while (current_column < goal_column)
	{
	  fputc( ' ', output );

	  ++current_column;
	}
    }
  else if (current_column > goal_column)
    {
      fputc( ' ', output );
    }

  return;

} /* pad_to */

/*}}}*/
/*{{{ new_line   */

/*
 * send a newline character, adjusting column count aswell
 */

static void
new_line( void )
{
  fputc( '\n', output );

  current_column = 0;

  return;
  
} /* new_line */
  

/*}}}*/
/*{{{ print_byte */

static void
print_byte( long int value )
{
  if (value < 0)
    value = -value;

  value &= 0xff;

  if (value == '\n')
    {
      myprintf( "\\n " );
    }
  else if (value == '\t')
    {
      myprintf( "\\t " );
    }
  else if (value == '\v')
    {
      myprintf( "\\v " );
    }
  else if (value == '\f')
    {
      myprintf( "\\f " );
    }
  else if (value >= ' ' && value <= '~' )
    {
      myprintf( " %c ", (char)value );
    }
  else
    {
      myprintf( "..." );
    }

  return;
  
} /* print_byte */
  

/*}}}*/
/*{{{ bitreverse */

int32
bitreverse( int32 n )
{
  int32 m1 = 0x0001;
  int32 m2 = 0x8000;
  int32 i;
  int32 r = 0;

  for ( i = 1; i <= 16; m1 <<= 1, m2 >>= 1, i++)
    if (n & m1) r |= m2;

  return(r);
}

/*}}}*/

/*}}}*/
/*{{{ List Manipulation    */

#ifdef BRANCH_TRACING

/*{{{ Types       */

/*
 * list manipulation routines - stolen from Helios
 */

typedef struct Node
  {
    struct Node *	next;
    struct Node *	prev;
  }
Node;

typedef struct
  {
    Node *	head;	/* points to first real item on list */
    Node *	earth;	/* always NULL */
    Node *	tail;	/* points to last real item on list */
  }
List;

/*}}}*/

/*{{{ search_list */

/*
 * search a list using a supplied boolean function
 * returns the first node that "matches" or NULL
 */

static Node *
search_list(
	    register List *	plist,		/* the list to scan */
	    register bool (*	pfunc)(),	/* the function to apply */
	    register long int 	arg )		/* the argument to the applied function */
{
  register Node *	pnode;


  for (pnode = plist->head; pnode->next != NULL; pnode = pnode->next)
    {
      if ((*pfunc)( pnode, arg ))
	return pnode;
    }
  
  return NULL;

} /* search_list */

/*}}}*/
/*{{{ init_list   */

/*
 * initialise a list
 */

static void
init_list( register List * plist )
{
  plist->head  = (Node *)&plist->earth;
  plist->earth = NULL;
  plist->tail  = (Node *)&plist->head;
  
  return;
  
} /* init_list */

/*}}}*/
/*{{{ pre_insert  */

/*
 * insert a node before another node in a list
 */

static void
pre_insert(
	   register Node *	pnext,
	   register Node *	pnode )
{
  pnode->prev       = pnext->prev;
  pnode->next       = pnext;
  pnext->prev       = pnode;
  pnode->prev->next = pnode;
	
  return;
  
} /* pre_insert */

/*}}}*/
/*{{{ add_tail    */

/*
 * add a node to the end of the list
 */

static void
add_tail(
	register List *	plist,
	register Node *	pnode )
{
  pnode->prev       = plist->tail;
  pnode->next       = (Node *)&plist->earth;
  plist->tail->next = pnode;
  plist->tail       = pnode; 
  
  return;

} /* add_tail */

/*}}}*/
/*{{{ remove_head */

/*
 * remove the first node on a list
 * returns the node or NULL
 */

static Node *
remove_head( register List * plist )
{
  register Node *	pnode;
  

  pnode = plist->head;
  
  if (pnode->next == NULL)
    return NULL;
  
  pnode->next->prev = (Node *)plist;
  plist->head       = pnode->next;
  pnode->next       = NULL;
  pnode->prev       = NULL;
	
  return pnode;

} /* remove_head */

/*}}}*/

#endif /* BRANCH_TRACING */

/*}}}*/
/*{{{ Disasembler Routines */

/*{{{ read_word         */

#define MAX_NUM_BYTES	8

static int32		iPutBackVal = 0;	/* (bigendian) value pushed back onto input queue */
static int32		iPutBackLen = 0;	/* number of bytes in the value                   */
static char		byte_read[ MAX_NUM_BYTES ];	  /* buffer recording bytes read	  */
static int		num_bytes_read = -1;	 /* negative disables recording			  */


#define get_val_( val )				\
  if (iPutBackLen > 0)				\
    {						\
      val = iPutBackVal >> 24;			\
      iPutBackVal <<= 8;	    		\
      iPutBackLen  -= 1;	    		\
    }				    		\
  else if (fread( &val, 1, 1, input ) != 1)	\
    {						\
      return FALSE;				\
    }						\
  if (num_bytes_read >= 0 && num_bytes_read < MAX_NUM_BYTES)	\
    {								\
      byte_read[ num_bytes_read++ ] = val;			\
    }
  

static bool
read_word( int32 *	pDestination )
{
  unsigned char	val;
  int32		tmp;
  

  if (input == NULL || pDestination == NULL)
    return FALSE;

  get_val_( val );
  
  tmp = val;

  get_val_( val );
  
  address += 2;
  
  *pDestination = tmp << 8 | (int32)val;
  
  return TRUE;
  
} /* read_word */

/*}}}*/
/*{{{ read_long         */

static bool
read_long( int32 *	pDestination )
{
  unsigned char	val;
  int32		tmp;
  

  if (input == NULL || pDestination == NULL)
    return FALSE;

  get_val_( val );
  
  tmp = val;

  get_val_( val );
  
  tmp = tmp << 8 | (int32)val;
  
  get_val_( val );
  
  tmp = tmp << 8 | (int32)val;
  
  get_val_( val );

  *pDestination = tmp << 8 | (int32)val;

  address += 4;
  
  return TRUE;
  
} /* read_long */

/*}}}*/
/*{{{ put_back_word     */

static void
put_back_word( int32 val )
{
  if (iPutBackLen > 2)
    {
      inform( "Error: attempting to put back a long word when put back queue is not empty" );
    }
  else if (iPutBackLen > 0)
    {
      if (iPutBackLen == 1)
	{
	  iPutBackVal |= ((val & 0xFFFF) << 8);
	}
      else
	{
	  iPutBackVal |= (val & 0xFFFF);
	}
      
      iPutBackLen += 2;
    }
  else
    {
      iPutBackLen = 2;

      iPutBackVal = val << 16;
    }

  address        -= 2;
  num_bytes_read -= 2;
  
  return;
  
} /* put_back_word */

/*}}}*/
/*{{{ put_back_long     */

static void
put_back_long( int32 val )
{
  if (iPutBackLen > 0)
    {
      inform( "Error: attempting to put back a long word when put back queue is not empty" );
    }
  else
    {
      iPutBackLen     = 4;
      iPutBackVal     = val;
      address        -= 4;
      num_bytes_read -= 4;
    }
  
  return;
  
} /* put_back_long */

/*}}}*/
/*{{{ readword		*/

static int32
readword( void )
{
  int32 	a;

  
  if (read_word( &a ))
    {
      return a;
    }
  else
    {
      inform( "read failure" );
      
      return 0;
    }
  
} /* readword */
  

/*}}}*/
/*{{{ readlong		*/

static int32
readlong( void )
{
  int32 	a;

  
  if (read_long( &a ))
    {
      return a;
    }
  else
    {
      inform( "read failure" );
      
      return 0;
    }
  
} /* readlong */

/*}}}*/

/*{{{ show_value        */

static void
show_value( void )
{
  int	i;


  pad_to( COL_HEX_VALUE );

  /* left pad with zeros */
  
  if (num_bytes_read < MAX_NUM_BYTES)
    {
      myprintf( "%*c", (MAX_NUM_BYTES - num_bytes_read) * 2, ' ' ); 
    }

  /* display number */
  
  for (i = 0; i < num_bytes_read; i++)
    {
      myprintf( "%02.2lx", byte_read[ i ] );
    }

  /* move to start of byte display */
  
  pad_to( COL_BYTE_VALUES );

  /* display bytes */
  
  for (i = 0; i < num_bytes_read; i++)
    {
      print_byte( byte_read[ i ] );
    }

  return;
  
} /* show_value */

/*}}}*/
/*{{{ bits		*/

#define _BITS_PER_WORD	32

/*
 * mask out all bits outside of the specified
 * range (inclusive), and return the resultant
 * word
 */

static unsigned long int
bits(
     int32 	iValue,
     int	iTopBit,
     int	iBottomBit )
{
  if (iTopBit < iBottomBit)
    {
      int	temp = iTopBit;

      
      iTopBit    = iBottomBit;
      iBottomBit = temp;
    }

  /* mask out bottom bits */
  
  iValue = iValue << (_BITS_PER_WORD - 1 - iTopBit);

  /* mask out top bits */
  
  return ( iValue >> (_BITS_PER_WORD - 1 - iTopBit + iBottomBit));
  
} /* bits */

/*}}}*/

/*{{{ writereg		*/

static void
writereg( int32 reg )
{
  if ( reg > 7 )
    myprintf( "A%d", reg - 8);
  else
    myprintf( "D%d", reg );
}

/*}}}*/
/*{{{ short_value	*/

static int32
short_value( int32 n )
{
  return ((n == 0) ? 8 : n);
}

/*}}}*/
/*{{{ byte_extend	*/

static int32
byte_extend( int32 n )
{
  return ((n > 127) ? (n | ~0XFF) : n);
}

/*}}}*/
/*{{{ word_extend	*/

static int32
word_extend( int32 n )
{
  return ((n & 0x8000) != 0 ? (n | 0xffff0000) : n);
}

/*}}}*/
/*{{{ op_size		*/

static int
op_size(
	instrenum  op,
	instrword  a,
	instrword  w )
{
  switch (op)
    {
    case ori:
    case andi:
    case subi:
    case addi:
    case eori:
    case cmpi:
    case negx:
    case clr:
    case neg:
    case not:
    case tst:
    case addq:
    case subq:
    case or:
    case sub:
    case subx:
    case cmp:
    case cmpm:
    case eor:
    case and:
    case add:
    case addx:
    case dshift:
      return bits( a, 7, 6 );
      
    case move:
      switch (bits( a, 13, 12 ))
        {
	case 1: return 0;
	case 2: return 2;
	case 3: return 1;
	}
    case movea:
      return (bits( a, 13, 12 ) == 3) ? 1 : 2;
      
    case adda:
    case suba:
    case cmpa:
      return (bits( a, 8, 6 ) == 3) ? 1 : 2;
      
    case movem_reg:
    case movemfreg:
    case movep:
      return (bits( a, 6, 6 ) == 0) ? 1 : 2;
      
    case fp_2:    /* This case w = 2nd word of instr */
      switch( bits( w, 12, 10 ) )
        {
	case 0:  return 2;
	case 1:  return 2;
	case 2:  return 4;
	case 3:  return 4;
	case 4:  return 1;
	case 5:  return 3;
	case 6:  return 4;
        } 
      
    case mulsl_2:
    case divsl_2:
      return 2;
      
    default:
      return 1;
    }
  
} /* op_size */

/*}}}*/
/*{{{ condition_code	*/

static char *
condition_code(
	       instrenum	op,
	       instrword	a )
{
  switch (bits( a, 11, 8 ))
    {
    case  0: return (op == bcc ? "RA": "T");
    case  1: return (op == bcc ? "SR": "RA");
    case  2: return "HI";
    case  3: return "LS";
    case  4: return "CC";
    case  5: return "CS";
    case  6: return "NE";
    case  7: return "EQ";
    case  8: return "VC";
    case  9: return "VS";
    case 10: return "PL";
    case 11: return "MI";
    case 12: return "GE";
    case 13: return "LT";
    case 14: return "GT";
    case 15: return "LE";
    }
  
  return(0); /* Artificial (never executed) */
  
} /* condition_code */

/*}}}*/
/*{{{ write_op		*/

static void
write_op(
	 char * instruction,
	 int	n )
{
  int	s;

  
  myprintf( "%s", instruction );
  
  if ( n == 0 ) myprintf( ".B" );
  if ( n == 1 ) myprintf( ".W" );
  if ( n == 2 ) myprintf( ".L" );

  s = strlen( instruction );

  if (n >= 0 && n <= 2) s += 2;

  if (s < 10)
    myprintf( "%*c", (10 - s) - 1, ' ' );

  return;
  
} /* write_op */

/*}}}*/
/*{{{ w_index		*/

static void
w_index(
	int32 a,
	int32 reg,
	int32 l,
	int32 scl )
{
  myprintf( (a == 1 ? "A%d" : "D%d"), reg);
  
  if (l == 1) myprintf( ".L" );
  
  if (scl != 0)
    myprintf( "*%d", (1 << scl) );
}

/*}}}*/
/*{{{ mode6orpc3	*/

static void
mode6orpc3(
	   int32 arg,
	   int32 mode )
{
  int32 b      = readword();
  int32 d      = b & 0xff;
  int32 scl    = bits( b, 10, 9  );
  int32 reg    = bits( b, 14, 12 );
  int32 a      = bits( b, 15, 15 );
  int32 l      = bits( b, 11, 11 );
  int32 pcmode = mode == 7;
  int32 pcx    = address - 2;
  int32 iis, bds, is, bs;
  

  if (bits( b, 8, 8 ) == 0)
    {
      if (d > 127)
	d = byte_extend( d );
      
      myprintf( (pcmode ? "$%04x(PC,":"%d(A%d,"), d/*+(pcmode ? pcx : 0)*/, arg );
      
      w_index( a, reg, l, scl );
      
      myprintf( ")" );
    }
  else
    {
      iis = bits( b, 2, 0 );
      bds = bits( b, 5, 4 );
      is  = bits( b, 6, 6 );
      bs  = bits( b, 7, 7 );
      
      myprintf( "(" );
      
      if (iis != 0)
	myprintf( "[" );
      
      if (bds != 1)
	{
          if (bds == 3)
	    myprintf( "$%x,", readlong() + (pcmode ? pcx : 0) );
          else
	    myprintf( (pcmode ? "%04x," : "%d,"),
		      readword() + (pcmode ? pcx : 0) );
	}
      
      if (bs == 1)
	myprintf( "Z" );
      
      myprintf( (pcmode ? "PC" : "A%1d"), arg );
      
      if (iis <= 7 && iis >= 5)
	myprintf( "]" );
      
      if (is == 0)
	{
          myprintf( "," );
	  
          w_index( a, reg, l, scl );
	}
      
      if (iis <= 3 && iis >= 1)
	myprintf( "]" );
      
      if (bits( is, 1, 0 ) > 1 )
	{
          if (bits( is, 1, 0 ) == 3)
	    myprintf( "%08x", readlong() );
          else
	    myprintf( "%d", readword() );
	}
      
      if (iis != 0)
	myprintf( "]" );
      
      myprintf( ")" );
    }
  
  return;
  
} /* mode6orpc3 */

/*}}}*/
/*{{{ ea		*/

static void
ea(
   instrenum	op,
   instrword	a,
   int32	w )
{
  int32 	mode = bits( a, 5, 3 );
  int32		arg  = bits( a, 2, 0 );
  int		size;

  
  switch (mode)
    {
    case 0: myprintf( "D%d", arg );     break;
    case 1: myprintf( "A%d", arg );     break;
    case 2: myprintf( "(A%d)", arg );   break;
    case 3: myprintf( "(A%d)+", arg );  break;
    case 4: myprintf( "-(A%d)", arg );  break;
    case 5: myprintf( "%d(A%d)", word_extend( readword() ), arg ); break;
    case 6: mode6orpc3( arg, mode );    break;
    case 7:
      switch (arg)
	{
	case 0: /* absolute short */
	    {
	      unsigned long int offset = readword();
	      
	      if ((offset & 0x8000) == 0)
		myprintf( "$%04x.W", offset );
	      else
		myprintf( "$%08lx.W", offset | 0xffff0000 );
	    }
	    break;
	    
	  case 1: /* absolute long (e.g. extref) */
	      {
		int32  w = readlong();

		
		myprintf( "$%08lx", (long)w );
	      }
	    break;
	    
	  case 2: /* PC indirect with disp */
	      {
		int32 old_address = address;
		
		myprintf( "$%04x", (word_extend( readword() ) + old_address) & 0XFFFFFF);
	      }
	    break;
	    
	    /* PC mode 3 not complete for 020 yet */
	    
	  case 3:
	    mode6orpc3( arg, mode );
	    break;
	    
	  case 4:
	    size = op_size( op, a, w );
	    switch( size )
	      {
		int32 a,b,c;
		    
	      case 0:
	      case 1:
		myprintf( "#%d", readword() );
		break;
		
	      case 2:
		myprintf( "#$%08x", readlong() );
		break;
		
	      case 3:
		a = readlong();
		b = readlong();
		
		myprintf( "#$%08x%08x", a, b );
		break;
		
	      case 4:
		a = readlong();
		b = readlong();
		c = readlong();
		
		myprintf( "#$%08x%08x%08x", a, b, c );
		break;
	      }
	    break;
	    
	  default:
	    myprintf( "???" );
	  }
    }
  
} /* ea */

/*}}}*/
/*{{{ write_opi		*/

static void
write_opi(
	  char *	instruction,
	  int32		op,
	  instrword	a )
{
  int32 	size;


  size = op_size( op, a, 0 );

  write_op( instruction, size );

  if (size == 0)  myprintf( "#%d,",    readword() );
  if (size == 1)  myprintf( "#%d,",    readword() );
  if (size == 2)  myprintf( "#$%08X,", readlong() );

  ea( op, a, 0 );
}

/*}}}*/
/*{{{ register_list     */

static void
register_list(
	      int32 	m,
	      void (*	preg)( int32 ) )
{
  int32 	i, freg = 0;
  int32		state   = 0;
  int32		list_started = 0;

  
  for (i = 0; i <= 16; i++)     /* 16 to flush A7 if set */
    {
      int32 x;
      
      if ((x = bits(m,i,i)) != state)
	{
	  state = x;
	  
	  if (state)
	    {
	      if (list_started )
		myprintf("/");
	      
	      preg(i);
	      freg = i;               /* Record first reg in list */
	      list_started = 1;
	    }
	  else
	    {
	      if ( (i-1) != freg )
		{
		  myprintf("-");
		  preg(i-1);
		}
	    }
	}
    }
} /* register_list */


/*}}}*/
/*{{{ fpcond		*/

static void
fpcond(
       char * 	op,
       int32	pred,
       int32	sz )
{
  char *	bop;
  char		s[ 10 ];


  s[ 0 ] = 0;
  strcat( s, op );

  switch( pred )
    {
    default:
    case 0x00:  bop = "F";   break;
    case 0x01:  bop = "EQ";  break;
    case 0x02:  bop = "OGT"; break;
    case 0x03:  bop = "OGE"; break;
    case 0x04:  bop = "OLT"; break;
    case 0x05:  bop = "OLE"; break;
    case 0x06:  bop = "OGL"; break;
    case 0x07:  bop = "OR";  break;
    case 0x08:  bop = "UN";  break;
    case 0x09:  bop = "UEQ"; break;
    case 0x0a:  bop = "UGT"; break;
    case 0x0b:  bop = "UGE"; break;
    case 0x0c:  bop = "ULT"; break;
    case 0x0d:  bop = "NLE"; break;
    case 0x0e:  bop = "NE";  break;
    case 0x0f:  bop = "T";   break;
    case 0x10:  bop = "SF";  break;
    case 0x11:  bop = "SEQ"; break;
    case 0x12:  bop = "GT";  break;
    case 0x13:  bop = "GE";  break;
    case 0x14:  bop = "LT";  break;
    case 0x15:  bop = "LE";  break;
    case 0x16:  bop = "GL";  break;
    case 0x17:  bop = "GLE"; break;
    case 0x18:  bop = "NGLE"; break;
    case 0x19:  bop = "NGL"; break;
    case 0x1a:  bop = "NLE"; break;
    case 0x1b:  bop = "NLT"; break;
    case 0x1c:  bop = "NGE"; break;
    case 0x1d:  bop = "NGT"; break;
    case 0x1e:  bop = "SNE"; break;
    case 0x1f:  bop = "ST";  break;
    }
  
  strcat( s, bop );
  
  write_op( s, sz );
  
  return;
  
} /* fpcond */

/*}}}*/
/*{{{ fpdbcc		*/

static void
fpdbcc( instrword a )
{
  int fpr  = bits( a, 2, 0 );
  int pcx  = address;
  int pred = readword();
  int off  = readword();


  fpcond( "FDB", pred, -1 );
  
  myprintf( "D%d,%06x", fpr, off + pcx );

  return;
  
} /* fpdbcc */

/*}}}*/
/*{{{ fpbcc		*/

static void
fpbcc( instrword a )
{
  int pred = bits( a, 5, 0 );
  int s    = bits( a, 6, 6 );
  int pcx  = address;
  int addr;
  

  fpcond( "FB", pred, s + 1 );

  addr = s == 1 ? readlong() : readword();
  
  myprintf( "%06x", addr + pcx );

  return;
  
} /* fpbcc */

/*}}}*/
/*{{{ fptcc		*/

static void
fptcc( instrword a )
{
  int32 pred = readword();
  int32 mode = bits( a, 2, 0 );


  fpcond( "FTRAP", pred, (mode == 4)? 7: mode - 1 );

  if (mode == 4)
    return;

  if (mode == 2)
    {
      myprintf( "%04x", readword() );
      return;
    }

  if (mode == 3)
   {
     myprintf( "%08x", readlong() );
     return;
   }
  
} /* fptcc */

/*}}}*/
/*{{{ fpscc		*/

static void
fpscc( instrword a )
{
   int32 	pred = readword();

   
   fpcond( "FS", pred, 0 );
   
   ea( fp_2, a, 0 ); /* op size irrelevant */
}

/*}}}*/
/*{{{ fpsave		*/

static void
fpsave( instrword a )
{
   write_op( "FSAVE", -1 );
   ea( fp_2, a, 0 ); /* op size irrelevant */
}

/*}}}*/
/*{{{ fprestore		*/

static void
fprestore( instrword a )
{
  write_op( "FRESTORE", -1 );
  ea( fp_2, a, 0 ); /* op size irrelevant */
}

/*}}}*/
/*{{{ write_opf		*/

static void
write_opf(
	  char *	op,
	  int32		w,
	  int32		ss )
{
  char *	opts = "LSXPWDB";
  int32		l;


  myprintf( "%s.", op );

  myprintf( "%c", ((w & 0x4000) == 0)? *(opts + 2): *(opts + ss) );

  pad_to( COL_OPERANDS );
}

/*}}}*/
/*{{{ fpgen0		*/

static void fpgen0(instrword a, int32 w)
{
   int32 argtype = 3;
   int32 s = bits(w,12,10);
   int32 d = bits(w,9,7);
   int32 m = bits(w,14,14);
   char *insstr;

   if( ( s == 7 ) && ( m == 1 ) )
   {
      write_opf("FMOVECR",w,0);
      myprintf("#$%02x,FP%d", w & 0x3f, bits(w,9,7));
      return;
   }
   else
   {
      switch( w & 0x7f )
      {
      case 0x00: argtype = 2; insstr = "FMOVE"; break;
      case 0x02: insstr = "FSINH"; break;
      case 0x03: insstr = "FINTRZ"; break;
      case 0x04: insstr = "FSQRT"; break;
      case 0x06: insstr = "FLOGNP1"; break;
      case 0x08: insstr = "FETOXM1"; break;
      case 0x09: insstr = "FTANH"; break;
      case 0x0a: insstr = "FATAN"; break;
      case 0x0c: insstr = "FASIN"; break;
      case 0x0d: insstr = "FATANH"; break;
      case 0x0e: insstr = "FSIN"; break;
      case 0x0f: insstr = "FTAN"; break;
      case 0x10: insstr = "FETOX"; break;
      case 0x11: insstr = "FTWOTOX"; break;
      case 0x12: insstr = "FTENTOX"; break;
      case 0x14: insstr = "FLOGN"; break;
      case 0x15: insstr = "FLOG10"; break;
      case 0x16: insstr = "FLOG2"; break;
      case 0x19: insstr = "FCOSH"; break;
      case 0x1a: insstr = "FNEG"; break;
      case 0x1c: insstr = "FACOS"; break;
      case 0x1e: insstr = "FGETEXP"; break;
      case 0x1f: insstr = "FGETMAN"; break;
      case 0x20: argtype = 2; insstr = "FDIV"; break;
      case 0x21: argtype = 2; insstr = "FMOD"; break;
      case 0x22: argtype = 2; insstr = "FADD"; break;
      case 0x23: argtype = 2; insstr = "FMUL"; break;
      case 0x24: argtype = 2; insstr = "FSGLDIV"; break;
      case 0x25: argtype = 2; insstr = "FREM"; break;
      case 0x26: argtype = 2; insstr = "FSCALE"; break;
      case 0x27: argtype = 2; insstr = "FSGLMUL"; break;
      case 0x28: argtype = 2; insstr = "FSUB"; break;
      case 0x30: argtype = 4; insstr = "FSINCOS"; break;
      case 0x31: argtype = 4; insstr = "FSINCOS"; break;
      case 0x32: argtype = 4; insstr = "FSINCOS"; break;
      case 0x33: argtype = 4; insstr = "FSINCOS"; break;
      case 0x34: argtype = 4; insstr = "FSINCOS"; break;
      case 0x35: argtype = 4; insstr = "FSINCOS"; break;
      case 0x36: argtype = 4; insstr = "FSINCOS"; break;
      case 0x37: argtype = 4; insstr = "FSINCOS"; break;
      case 0x38: argtype = 2; insstr = "FCMP"; break;
      case 0x3a: argtype = 1; insstr = "FTST"; break;
      case 0x3d: argtype = 2; insstr = "FCOS"; break;
      default:
         myprintf("????");
         return;
      }
   }
   write_opf(insstr,w,s);

   if( m != 0 )
      ea( fp_2, a, w);
   else
      myprintf("FP%d", s);

   switch( argtype )
   {
   case 1:
      break;

   case 2:
      myprintf(",FP%d", d);
      break;

   case 3:
      if( (m != 0) || ( (m == 0) && (s != d) ) )
         myprintf(",FP%d",d);
      break;

   case 4:
      myprintf(",FP%d:FP%d",bits(w,2,0),d);
      break;
   }
}

/*}}}*/
/*{{{ writefpcr		*/

static void writefpcr(int32 n)
{
   char s[15];
   bool started = 0;

   s[0] = 0;
   if( n & 1 )
   {  strcat(s,"FPIAR");
      started = 1;
   }
   if( n & 2 )
   {  if( started ) strcat(s,"/");
      else started = 1;

      strcat(s,"FPSR");
   }

   if( n & 4 )
   {  if( started ) strcat(s,"/");
      else started = 1;

      strcat(s,"FPCR");
   }
   myprintf(s);
}

/*}}}*/
/*{{{ fpmovefpcr	*/

static void fpmovefpcr(instrword a, int32 w)
{
   int32 dir = bits(w,13,13);
   int32 fpr = bits(w,12,10);

   write_op("FMOVE",2);

   if( dir == 1 )
   {
      writefpcr(fpr);
      myprintf(",");
      ea(fp_2,a, w);
   }
   else
   {
      ea(fp_2,a, w);
      myprintf(",");
      writefpcr(fpr);
   }
}

/*}}}*/
/*{{{ fpmovefromfp	*/

static void fpmovefromfp(instrword a, int32 w)
{
   int32 s = bits(w,9,7);
   int32 d = bits(w,12,10);
   int32 k = bits(w,6,0);

   write_opf("FMOVE",w,d);

   myprintf("FP%d,",s);
   ea(fp_2,a, w);

   if( d == 3 )
      myprintf("{#%d}",k);

   if( d == 7 )
      myprintf("{D%d}",bits(k,6,4));
}

/*}}}*/
/*{{{ writefpreg        */

static void
writefpreg( int32 n ) 
{
  myprintf( "FP%d", n );
}

/*}}}*/
/*{{{ fpmovem		*/

static void fpmovem(instrword a, int32 w)
{
   int32 rlist = bits(w,7,0);           /* AM changed 6 to 7 Aug 89 (check) */
   int32 mode  = bits(w,12,11);
   int32 dir   = bits(w,13,13);
   int32 dreg  = bits(w,6,4);

   write_op("FMOVEM.X",-1);

   switch( mode )
   {
   case 0:
   case 2:
      if ( mode == 2 )
	rlist = bitreverse( rlist ) >> 8;
      if ( dir == 1 )
      {
         register_list( rlist, writefpreg );
         myprintf(",");
         ea(fp_2, a, 0);
      }
      else
      {
         ea(fp_2, a, 0);
         myprintf(",");
         register_list( rlist, writefpreg );
      }
      break;

   case 3:
   case 1:
      if( dir == 1 )
      {
         myprintf("D%d,",dreg);
         ea(fp_2, a, 0);
      }
      else
      {
         ea(fp_2, a, 0);
         myprintf(",D%d",dreg);
      }
   }
}

/*}}}*/
/*{{{ fpgen		*/

static void
fpgen( instrword a )
{
  int32 	w = readword();


  switch( bits(w, 15, 13) )
    {
    case 0:
    case 2:
      fpgen0(a,w);
      break;

    case 1:
      myprintf("????");
      break;

    case 3:
      fpmovefromfp( a, w );
      break;

    case 4:
    case 5:
      fpmovefpcr( a, w );
      break;
      
   case 6:
   case 7:
      fpmovem( a, w );
      break;
    }
  
  return;
  
} /* fpgen */

/*}}}*/
/*{{{ write_fp_ins	*/

static void
write_fp_ins( instrword a )
{
  switch (bits( a, 8, 6 ))
    {
    case 0:
      fpgen( a );
      break;

    case 1:
      switch (bits( a, 5, 3 ))
	{
	case 1:
	  fpdbcc( a );
	  break;
	  
	case 7:
	  fptcc( a );
	  break;
	  
	default:
	  fpscc( a );
	  break;
	} 
      break;

    case 2:
    case 3:
      fpbcc( a );
      break;

    case 4:
      fpsave( a );
      break;

    case 5:
      fprestore( a );
      break;
      
    case 6:
    case 7:
      myprintf( "????" );
      break;
    }
  
  return;
  
} /* write_fp_ins */

/*}}}*/
/*{{{ write_mulsl	*/

static void
write_mulsl( instrword a )
{
  instrword 	w   = readword();

  
  write_op( bits( w, 11, 11 ) ? "MULS": "MULU", 2 );

  ea( mulsl_2, a, 0 );

  myprintf( "," );

  if (bits( w, 10, 10 ) != 0)
    {
      writereg( bits( w,  2,  0 ) );
      myprintf( ":" );
    }

  writereg( bits( w, 14, 12 ) );

  return;
  
} /* write_mulsl */

/*}}}*/
/*{{{ write_divsl	*/

static void
write_divsl( instrword a )
{
  instrword 	w = readword();
  char *	ins;
  int32		dr  = bits( w, 2,  0  );
  int32		dq  = bits( w, 14, 12 );
  int32		sz  = bits( w, 10, 10 );

  
  ins = bits( w, 11, 11) ? ( (sz != 0)? "DIVSL": "DIVS" ):
                           ( (sz != 0)? "DIVUL": "DIVU" );
  write_op( ins, 2 );

  ea( divsl_2, a, 0 );

  myprintf( "," );
  
  writereg( dr );
  
  if (( sz != 0 ) | (dr != dq))
    {
      myprintf( ":" );
      writereg( dq );
    }

  return;
  
} /* write_divsl */

/*}}}*/
/*{{{ write_instruction */

static void
write_instruction(
		  instrenum op,
		  instrword a )
{
  int32 	dir;
  int32		addr;
  int32		pcx;
  int32		b;
  int32		mode;
  int32		val;
  char		s[ 10 ];
  char *	type;
  char *	str;
  char *	sh_op;
  char *	condition;

  
  switch (op)
    {
    default:
    case none:
      write_op("?DC", 1);
      myprintf("$%04x", a);
      break;
      
    case abcd:
      write_op("ABCD", -1);
      if (bits(a, 3, 3) == 0)
	myprintf("-(A%d),-(A%d)", bits(a, 2, 0), bits(a, 11, 9));
      else
	myprintf("D%d,D%d", bits(a, 2, 0), bits(a, 11, 9));
      break;
      
    case add:
      write_op("ADD", bits(a, 8, 6));
      if (bits(a, 8, 6) <= 2 && bits(a, 8, 6) >= 0)
	{
	  ea(op, a, 0);
	  myprintf(",D%d", bits(a, 11, 9));
	}
      if (bits(a, 8, 6) <= 6 && bits(a, 8, 6) >= 4)
	{
	  myprintf("D%d,", bits(a, 11, 9));
	  ea(op, a, 0);
	}
      break;
      
    case adda:
      write_op("ADDA", op_size(op, a, 0));
      ea(op, a, 0);
      myprintf(",A%d", bits(a, 11, 9));
      break;
      
    case addi:
      write_opi("ADDI",op,  a);
      break;
      
    case addq:
      write_op("ADDQ", bits(a, 7, 6));
      myprintf("#%d,", short_value(bits(a, 11, 9)));
      ea(op, a, 0);
      break;
      
    case addx:
      write_op("ADDX", bits(a, 7, 6));
      if (bits(a, 3, 3) == 0)
	myprintf("-(A%d),-(A%d)", bits(a, 2, 0), bits(a, 11, 9));
      else myprintf("D%d,D%d", bits(a, 2, 0), bits(a, 11, 9));
      break;
      
    case and:
      write_op("AND", bits(a, 8, 6));
      if (bits(a, 8, 6) <= 2 && bits(a, 8, 6) >= 0)
	{
	  ea(op, a, 0);
	  myprintf(",D%d", bits(a, 11, 9));
	}
      if (bits(a, 8, 6) <= 6 && bits(a, 8, 6) >= 4)
	{
	  myprintf("D%d,", bits(a, 11, 9));
	  ea(op, a, 0);
	}
      break;
      
    case andi:
      write_opi("ANDI",op,  a);
      break;
      
    case andi_sr:         /* not complete */
      myprintf("ANDI_SR");
      break;
      
    case andi_ccr:        /* not complete */
      myprintf("ANDI_CCR");
      break;
      
    case dshift:
	{
	  dir = bits(a, 8, 8);
	  switch (bits(a, 4, 3))
	    {
	    default:
            case 0: sh_op =  ((dir == 0 ) ?  "ASR" : "ASL"); break;
            case 1: sh_op =  ((dir == 0 ) ?  "LSR" : "LSL"); break;
            case 2: sh_op =  ((dir == 0 ) ?  "ROXR": "ROXL"); break;
            case 3: sh_op =  ((dir == 0 ) ?  "ROR" : "ROL");break;
	    }
	  write_op(sh_op, bits(a, 7, 6));
	  if ( bits(a, 5, 5) == 0 )
	    myprintf("#%d,D%d", short_value(bits(a, 11, 9)), bits(a, 2, 0));
	  else myprintf("D%d,D%d", bits(a, 11, 9), bits(a, 2, 0));
	  break;
	}
      
    case mshift:
	{
	  dir = bits(a, 8, 8);
	  
	  switch (bits(a, 10, 9))
	    {
	    default:
            case 0: sh_op =  ((dir == 0 ) ?  "ASR" : "ASL"); break;
            case 1: sh_op =  ((dir == 0 ) ?  "LSR" : "LSL"); break;
            case 2: sh_op =  ((dir == 0 ) ?  "ROXR": "ROXL"); break;
            case 3: sh_op =  ((dir == 0 ) ?  "ROR" : "ROL"); break;
	    }
	  write_op(sh_op, -1);
	  ea(op, a, 0);
	  break;
	}
      
    case scc:
	{
	  condition = condition_code(op, a);
	  
	  strcpy(s,"S");
	  strcat(s,condition);
	  write_op(s,-1);
	  ea(op,a, 0);
	  break;
	}
      
    case bcc:
    case dbcc:
	{
	  addr      = bits( a, 7, 0 );
	  pcx       = address;
	  condition = condition_code( op, a );
	  
	  if (op == bcc )
	    strcpy( s,"B" );
	  
	  if (op == dbcc ) 
	    strcpy( s, "DB" );
	  
	  strcat( s, condition );
	  
	  if (op == bcc)
	    {
	      if (addr > 127 )
		addr = byte_extend( addr );
	      
	      if (addr != 0)
		strcat( s, ".S" );
	    }
	  else
	    addr = 0;
	  
	  write_op(s,-1);
	  
	  if (addr == 0 )  addr = word_extend(readword());
	  
	  if (addr == 0XFF)
	    addr = readlong();
	  
	  /* Decode this as a label */
	  
	  if (op == dbcc)
	    myprintf("D1,"); /* Atari HiSoft requires this! */
	  
	  break;
	}
      
    case dbit:
	{
	  switch (bits(a, 7, 6))
	    {   default:
		case 0: type = "BTST"; break;
		case 1: type = "BCHG"; break;
		case 2: type = "BCLR"; break;
		case 3: type = "BSET"; break;
		}
	  write_op(type, -1);
	  myprintf("D%d,", bits(a, 11, 9));
	  ea(op, a, 0);
	  break;
	}
      
    case sbit:
	{
	  b = readword();
	  switch (bits(a, 7, 6))
	    {
	    default:
            case 0: type = "BTST"; break;
            case 1: type = "BCHG"; break;
            case 2: type = "BCLR"; break;
            case 3: type = "BSET"; break;
	    }
	  write_op(type, -1);
	  myprintf("#%d,", b);
	  ea(op, a, 0);
	  break;
	}
      
    case chk:
      write_op("CHK", -1);
      ea(op, a, 0);
      myprintf(",D%d", bits(a, 11, 9));
      break;
      
    case clr:
      write_op("CLR", bits(a, 7, 6));
      ea(op, a, 0);
      break;
      
    case cmp:
      write_op("CMP", bits(a, 8, 6));
      ea(op, a, 0);
      myprintf(",D%d", bits(a, 11, 9));
      break;
      
    case cmpa:
      write_op("CMPA", op_size(op, a, 0));
      ea(op, a, 0);
      myprintf(",A%d", bits(a, 11, 9));
      break;
      
    case cmpi:
      write_opi("CMPI",op,  a);
      break;
      
    case cmpm:
      write_op("CMPM", bits(a, 7, 6));
      myprintf("(A%d)+,(A%d)+", bits(a, 2, 0), bits(a, 11, 9));
      break;
      
    case divs:
      write_op("DIVS", -1);
      ea(op, a, 0);
      myprintf(",D%d", bits(a, 11, 9));
      break;
      
    case divu:
      write_op("DIVU", -1);
      ea(op, a, 0);
      myprintf(",D%d", bits(a, 11, 9));
      break;
      
    case eor:
      write_op("EOR", bits(a, 7, 6)); /* This used to be 8,6 but seems wrong */
      myprintf("D%d,", bits(a, 11, 9));
      ea(op, a, 0);
      break;
      
    case eori:
      write_opi("EORI",op,  a);
      break;
      
    case eori_ccr:         /* notcomplete */
      myprintf("EORI_CCR");
      break;
      
    case eori_sr:         /* notcomplete */
      myprintf("EORI_CCR");
      break;
      
    case exgm:
    case exgd:
    case exga:
	{
	  mode = bits(a, 7, 3);
	  str = "D%d,D%d";
	  write_op("EXG", -1);
	  if (mode == 9 )  str = "A%d,A%d";
	  if (mode == 17 )  str = "D%d,A%d";
	  myprintf(str, bits(a, 11, 9), bits(a, 2, 0));
	  break;
	}
      
    case extbl_2:
    case extw:
    case extl:
      if ( op == extbl_2 )
	write_op("EXTBL",2);
      else
	write_op("EXT", bits(a, 8, 6) == 2  ?  1 : 2);
      writereg(bits(a, 2, 0));
      break;
      
      
    case ill:
      write_op("ILLEGAL", -1);
      break;
      
    case jmp:
      write_op("JMP", -1);
      ea(op, a, 0);
      break;
      
    case jsr:
      write_op("JSR", -1);
      ea( op, a, 0 );
      break;
      
    case lea:
      write_op("LEA", -1);
      ea(op, a, 0);
      myprintf(",A%d", bits(a, 11, 9));
      break;
      
    case link:
	{
	  int32 off = bits(a,7,3) == 1 ? readlong() : word_extend(readword());
	  write_op("LINK", -1);
	  myprintf("A%d,#%d", bits(a, 2, 0), off);
	}
      break;
      
    case move:
	{
	  b = (bits(a, 8, 6) << 3) | bits(a, 11, 9) | (bits(a, 15, 12) << 12);
	  write_op("MOVE", op_size(op, a, 0));
	  ea(op, a, 0);
	  myprintf(",");
	  ea(op, b, 0);
	  break;
	}
      
    case movea:
      write_op("MOVEA", op_size(op, a, 0));
      ea(op, a, 0);
      myprintf(",A%d", bits(a, 11, 9));
      break;
      
    case move_ccr:
      write_op("MOVE", -1);
      ea( op, a, 0 );
      myprintf(",CCR");
      break;
      
    case move_sr:
      write_op("MOVE", -1);
      ea(op, a, 0);
      myprintf(",SR");
      break;
      
    case movefsr:
      write_op("MOVE", -1);
      myprintf("SR,");
      ea(op, a, 0);
      break;
      
    case move_usp:
    case movefusp:
      write_op("MOVE", -1);
      if (bits(a, 3, 3) == 0)
	myprintf("A%d,USP", bits(a, 2, 0));
      else myprintf("USP,A%d", bits(a, 2, 0));
      break;
      
    case movemfreg:
    case movem_reg:
      write_op("MOVEM", bits(a, 6, 6) == 0  ?  1 : 2);
      if (  bits(a, 10, 10) == 0 )
	{
	  register_list( (bits(a,5,3) == 4)? bitreverse(readword()):
			readword(), writereg);
	  myprintf(",");
	  ea(op, a, 0);
	}
      else
	{
	  ea(op, a, 0);
	  myprintf(",");
	  register_list(readword(), writereg);
	}
      break;
      
    case movep:
      write_op("MOVEP", bits(a, 6, 6) == 0  ?  1 : 2);
      if ( bits(a, 7, 7))
	myprintf("%d(A%d),D%d", readword(), bits(a, 2, 0), bits(a, 11, 9));
      else myprintf("D%d,%d(A%d)", bits(a, 11, 9), readword(), bits(a, 2, 0));
      break;
      
    case moveq:
	{
	  val = a & 0XFF;
	  if (val > 127 )  val = byte_extend(val);
	  write_op("MOVEQ", -1);
	  myprintf("#%d,D%d", val, bits(a, 11, 9));
	  break;
	}
      
    case muls:
      write_op("MULS", -1);
      ea(op, a, 0);
      myprintf(",D%d", bits(a, 11, 9));
      break;
      
    case mulu:
      write_op("MULU", -1);
      ea(op, a, 0);
      myprintf(",D%d", bits(a, 11, 9));
      break;
      
    case nbcd:
      write_op("NBCD", -1);
      ea(op, a, 0);
      break;
      
    case neg:
      write_op("NEG", bits(a, 7, 6));
      ea(op, a, 0);
      break;
      
    case negx:
      write_op("NEGX", bits(a, 7, 6));
      ea(op, a, 0);
      break;
      
    case nop:
      myprintf("NOP");
      break;
      
    case not:
      write_op("NOT", bits(a, 7, 6));
      ea(op, a, 0);
      break;
      
    case or:
      write_op("OR", bits(a, 7, 6));
      if (bits(a, 8, 6) <= 2 && bits(a, 8, 6) >= 0)
	{
	  ea(op, a, 0);
	  myprintf(",D%d", bits(a, 11, 9));
	}
      if (bits(a, 8, 6) <= 6 && bits(a, 8, 6) >= 4)
	{
	  myprintf("D%d,", bits(a, 11, 9));
	  ea(op, a, 0);
	}
      break;
      
    case ori:
      write_opi("ORI", op,  a);
      break;
      
    case ori_sr:        /* not complete */
      myprintf("ORI_SR");
      break;
      
    case ori_ccr:       /* not complete */
      myprintf("ORI_CCR");
      break;
      
    case pea:
      write_op("PEA", -1);
      ea(op, a, 0);
      break;
      
    case reset:
      myprintf("RESET");
      break;
      
    case rtd_1:
      write_op("RTD", -1);
      myprintf("#%d", word_extend(readword()));
      break;
      
    case rte:
      myprintf("RTE");
      break;
      
    case rtr:
      myprintf("RTR");
      break;
      
    case rts:
      myprintf("RTS");
      break;
      
    case sbcd:
      write_op("SBCD", -1);
      if ( bits(a, 3, 3) == 0 )
        myprintf("-(A%d),-(A%d)", bits(a, 2, 0), bits(a, 11, 9));
      else myprintf("D%d,D%d", bits(a, 2, 0), bits(a, 11, 9));
      break;
      
    case stop:
      myprintf("STOP");
      break;
      
    case sub:
      write_op("SUB", bits(a, 8, 6));
      if (bits(a, 8, 6) <= 2 && bits(a, 8, 6) >= 0)
	{
	  ea(op, a, 0);
	  myprintf(",D%d", bits(a, 11, 9));
	}
      if (bits(a, 8, 6) <= 6 && bits(a, 8, 6) >= 4)
	{
	  myprintf("D%d,", bits(a, 11, 9));
	  ea(op, a, 0);
	}
      break;
      
    case suba:
      write_op("SUBA", op_size(op, a, 0));
      ea(op, a, 0);
      myprintf(",A%d", bits(a, 11, 9));
      break;
      
    case subi:
      write_opi("SUBI", op, a);
      break;
      
    case subq:
      write_op("SUBQ", bits(a, 7, 6));
      myprintf("#%d,", short_value(bits(a, 11, 9)));
      ea(op, a, 0);
      break;
      
    case subx:
      write_op("SUBX", bits(a, 7, 6));
      if ( bits(a, 3, 3) == 0 )
	myprintf("-(A%d),-(A%d)", bits(a, 2, 0), bits(a, 11, 9));
      else myprintf("D%d,D%d", bits(a, 2, 0), bits(a, 11, 9));
      break;
      
    case swap:
      write_op("SWAP", -1);
      myprintf("D%d", bits(a, 2, 0));
      break;
      
    case tas:
      write_op("TAS", -1);
      ea(op, a, 0);
      break;
      
    case trap:
      write_op("TRAP",-1);
      myprintf("#%d", a & 0XF);
      break;
      
    case trapv:
      write_op("TRAPV", -1);
      break;
      
    case tst:
      write_op("TST", bits(a, 7, 6));
      ea(op, a, 0);
      break;
      
    case unlk:
      write_op("UNLK", -1);
      myprintf("A%d", bits(a, 2, 0));
      break;
      
    case mulsl_2:
      write_mulsl(a);
      break;
      
    case divsl_2:
      write_divsl(a);
      break;
      
    case fp_2:
      write_fp_ins(a);
      break;
    }
} /* write_instruction */

/*}}}*/
/*{{{ find_opcode	*/

static instrenum
find_opcode( instrword op_code )
{
  switch (bits( op_code, 15, 12 ))
    {
    case 0:
      switch (bits( op_code, 11, 8 ))
	{
        case 14: return ((bits( op_code, 7, 6 ) == 3) ? cas_2 : moves_1);
        case 12: return ((bits( op_code, 7, 6 ) == 3) ? cas_2 : cmpi);
        case 10:
	  if (bits( op_code, 7, 6 ) == 3)
	    return cas_2;
	  else
	    {
	      if (bits( op_code, 5, 0 ) == 0X3C)
		switch (bits( op_code, 7, 6 ))
		  {
		  case 0: return (eori_ccr);
		  case 1: return (eori_sr);
		  }
	      else
		return (eori);
	    }
	  
        case  8:
	  return (sbit);
	  
        case  6:
	  if (bits( op_code, 7, 6 ) == 3)
	    return ((bits( op_code, 5, 4 ) == 0) ? rtm_2 : callm_2);
	  else
	    return (addi);
	  
        case  4:
	  return ((bits( op_code, 7, 6 ) == 3) ? chm_2 : subi);
	  
        case  2:
	  if (bits( op_code, 7, 6 ) == 3)
	    return (chm_2);
	  else
	    {
	      if (bits( op_code, 5, 0 ) == 0X3C)
		switch (bits( op_code, 7, 6 ))
		  {
		  case 0: return (andi_ccr);
		  case 1: return (andi_sr);
		  }
	      else return (andi);
	    }
	  
        case  0:
	  if (bits( op_code, 7, 6 ) == 3)
	    return (chm_2);
	  else
	    {
	      if (bits( op_code, 5, 0 ) == 0X3C)
		switch (bits( op_code, 7, 6 ))
		  {
		  case 0: return (ori_ccr);
		  case 1: return (ori_sr);
		  }
	      else return (ori);
	    }
	  
        default:
	  if ((bits( op_code, 5, 3 ) == 1) &&  bits( op_code, 8, 6) <= 7
	      && bits( op_code, 8, 6 ) >= 4 )
	    return (movep);

	  if (bits( op_code, 8, 8 ) == 1)
	    return (dbit);

	  return (none);
	}

    case 1:
    case 2:
    case 3:
      return ((bits( op_code, 8, 6 ) == 1) ? movea : move);
      
    case 4:
      switch (bits( op_code, 11, 8 ))
	{
        case  0:
	  if (bits( op_code, 7, 6 ) == 3)
	    return (movefsr);
	  else
	    return (negx);
	  
        case  2:
	  return (bits( op_code, 7, 6 ) == 3) ? movefccr_1 : clr;
	  
        case  4:
	  if (bits( op_code, 7, 6 ) == 3)
	    return move_ccr;
	  else
	    return neg;
	  
        case  6:
	  if (bits( op_code, 7, 6 ) == 3)
	    return move_sr;
	  else
	    return not;
	  
        case  8:
	  switch (bits( op_code, 7, 6 ))
            {
            case  0:
	      return (bits( op_code, 5, 3 ) == 1) ? linkl_2 : nbcd;
	      
            case  1:
	      switch (bits( op_code, 5, 3 ))
		{
		case 0 :return swap;
		case 1 :return bkpt_1;
		default :return pea;
		}
	      
            case  2:
	      if (bits( op_code, 5, 3 ) == 0)
		return extw;
	      else
		return movemfreg;
	      
            case  3:
	      if ((bits( op_code, 5, 3 )) == 0)
		return extl;
	      else
		return (movemfreg);
            }

        case 10:
	  if (bits( op_code, 7, 6 ) == 3)
	    return (bits( op_code, 5, 0 ) == 0X3C) ? ill : tas;
	  else
	    return tst;
	  
        case 12:
	  if (bits( op_code, 7, 7 ) == 1)
	    return movem_reg;
	  else
	    {
	      if (bits( op_code, 6, 6 ) == 0)
		return mulsl_2;
	      else
		return divsl_2;
	    }
	  
        case 14:
	  switch (bits( op_code, 7, 3 ))
	    {
            case  1: return link;
            case  8:               /* fall through */
            case  9: return trap;
            case 10: return link;
            case 11: return unlk;
            case 12: return move_usp;
            case 13: return movefusp;
            case 14:
	      switch (bits( op_code, 2, 0 ))
		{
                case 0: return reset;
                case 1: return nop;
                case 2: return stop;
                case 3: return rte;
                case 4: return rtd_1;
                case 5: return rts;
                case 6: return trapv;
                case 7: return rtr;
		}
            case 15: return movec_1;
            default:
	      if (bits( op_code, 7, 6 ) == 2)
		return jsr;
	      if (bits( op_code, 7, 6 ) == 3)
		return jmp;
	      return none;
	    }
	  
        default:
	  if (bits( op_code, 8, 6 ) == 6)
	    return chk;
	  
	  if ((op_code & 0xfff8) == 0x49c0)
	    return extbl_2;
	  
	  if (bits( op_code, 8, 6 ) == 7)
	    return lea;
	  
	  return none;
	}

    case  5:
      if (bits( op_code, 7, 3) == 25)
	return dbcc;
      if (bits( op_code, 7, 6) == 3)
	{
	  if ( bits( op_code, 5, 0) <= 074 && bits( op_code, 5, 0) >= 072 )
	    return trapcc_2;
	  else
	    return scc;
	}
      if (bits( op_code, 8, 8) == 0)
	return addq;
      else return subq;
      
    case  6:
      return bcc;

    case  7:
      if (bits( op_code, 8, 8) == 0)
	return moveq;
      else return none;

    case  8:
      switch (bits( op_code, 8, 4))
	{
	case 16: return sbcd;
	case 20: return pack_2;
	case 24: return unpk_2;
	}

      switch (bits( op_code, 8, 6))
	{
	case 7: return divs;
	case 3:return divu;
	default: return or;
	}

    case  9:
      if (bits( op_code, 5, 4) == 0 && bits( op_code, 8, 8) == 1 &&
	  bits( op_code, 7, 6) <= 2 && bits( op_code, 7, 6) >= 0)
	return subx;

      if (bits( op_code, 8, 6) == 3 || bits( op_code, 8, 6) == 7)
	return suba;
      
      return sub;

    case 11:
      if (bits( op_code, 8, 8) == 1 && bits( op_code, 5, 3) == 1 &&
	  bits( op_code, 7, 6) <= 2 && bits( op_code, 7, 6) >= 0)
	return cmpm;

      if (bits( op_code, 8, 6) <= 6 && bits( op_code, 8, 6) >= 4)
	return eor;

      if (bits( op_code, 8, 6) == 3 || bits( op_code, 8, 6) == 7)
	return cmpa;

      return cmp;

    case 12:
      switch (bits( op_code, 8, 6))
	{
        case  3: return mulu;
        case  7: return muls;
        case  4:
	  if (bits( op_code, 5, 4) == 0) 
	    return abcd;
	  else
	    return  and;

        case  5:
	  if (bits( op_code, 5, 3) == 0)
	    return exgd;
	  if (bits( op_code, 5, 3) == 1)
	    return exga;
	  return and;
	  
        case  6:
	  if (bits( op_code, 5, 3) == 1)
	    return exgm;
        default: return and;
	}

    case 13:
      if ((bits( op_code, 8, 8) == 1) && bits( op_code, 5, 4) == 0 &&
	  bits( op_code, 7, 6) <= 2 && bits( op_code, 7, 6) >= 0)
	return addx;

      if (bits( op_code, 8, 6) == 3 || bits( op_code, 8, 6) == 7)
	return adda;

      return add;

    case 14:
      if (bits( op_code, 7, 6) == 3) {
	  if (bits( op_code, 11, 11) == 0)
	    return mshift;
	  else
	    return bfld_2;
	}
      else
	return dshift;

    case 15:
      return fp_2;
    }
  
  return 0; /* Artificial (never executed) */
}

/*}}}*/
/*{{{ disassemble       */

static void
disassemble(
#ifdef BRANCH_TRACING
	    bool	trace 		/* TRUE if branches are to be traced to their destinations */
#else
	    void
#endif
	    )
{
  unsigned long int	value;
  bool			show_code;
  long int		name_len      = 0;
  char *		func_name     = NULL;
#ifdef BRANCH_TRACING
  branch_ref *		pnext_branch;
  func_ref *		pnext_func;
#endif
  

  if (input == NULL)
    return;
  
  if ( output_file      == NULL  ||
      *output_file      == '\0'  ||
      (output_file[ 0 ] == '-'  &&
       output_file[ 1 ] == '\0' ) )
    {
      output = stdout;
    }
  else
    {
      output = fopen( (char *)output_file, "w" );
      
      if (output == NULL)
	{
	  inform( "could not open output file '%s' - using stdout instead", output_file );
	  
	  output = stdout;
	}
    }
  
/*{{{ BRANCH_TRACING */

#ifdef BRANCH_TRACING
  if (input == stdin)
    trace = FALSE;
  
  if (trace)
    {
      trace = load_trace_data( input );
    }
  
  (void) fseek( input, 0L, SEEK_SET );

  if (trace)
    {
      pnext_branch = (branch_ref *)remove_head( &branch_trace );
      pnext_func   =   (func_ref *)remove_head( &func_trace   );
      
      if (pnext_func)
	{
	  name_len = pnext_func->length_of_name;
	  
	  func_name = (char *)malloc( (int)(name_len * sizeof (long) + 5) );
	  
	  if (func_name == NULL)
	    {
	      inform( "out of memory allocating buffer for function name" );
	      
	      name_len = 0;
	    }
	}
    }
  else
    {
      pnext_branch = NULL;
      pnext_func   = NULL;
    }
#endif /* BRANCH_TRACING */
  

/*}}}*/

  address   = 0;
  show_code = FALSE;
  
  while (read_long( &value ))
    {
      if (value == T_Module)
	{
	  int	i;

	  
	  /* we have the start of a module header */

	  for (i = sizeof (Module) / sizeof (unsigned long); i--; )
	    {
	      myprintf( "%8lx", address + address_base - 4 );
	      
	      pad_to( COL_HEX_VALUE + ((MAX_NUM_BYTES - 4) * 2) );
	      
	      myprintf( "%08.8lx", value );
	      
	      pad_to( COL_BYTE_VALUES );
	      
	      print_byte( value >> 24 );
	      print_byte( value >> 16 );
	      print_byte( value >>  8 );
	      print_byte( value );
	      
	      new_line();
	      
	      if (!read_long( &value ))
		break;
	    }
	}
      
      myprintf( "%8lx", address + address_base - 4 );

/*{{{ BRANCH_TRACING */

#ifdef BRANCH_TRACING
      if (pnext_func && address == pnext_func->address_of_name)
	{
	  show_code = FALSE;
	  name_len  = pnext_func->length_of_name;
	  
	  free( pnext_func );
	  
	  pnext_func = (func_ref *)remove_head( &func_trace );
	  
	  func_name  = (char *)malloc( (int)(name_len * sizeof (long) + 5 ) );
	  
	  if (func_name == NULL)
	    {
	      inform( "out of memory allocating buffer for function name" );
	      
	      name_len = 0;
	    }
	  else
	    {
	      func_name[ 0 ] = '\0';
	    }
	}
#endif /* BRANCH_TRACING */

/*}}}*/
      
      if (((value & 0xFFFFFF00U) == 0xFF000000U))
	{
	  if (func_name)
	    {
	      pad_to( COL_OP_CODE );

	      myprintf( "%s():", func_name );

	      free( func_name );

	      func_name = NULL;
	      name_len  = 0;
	    }
	  
	  show_code = TRUE;
	  
	  pad_to( COL_HEX_VALUE + ((MAX_NUM_BYTES - 4) * 2) );
	  
	  myprintf( "%08.8lx", value );
	}
      else if (value > 10 && show_code)
	{
	  char	buffer[ 80 ];		/* XXX */

	  
	  put_back_long( value );
	  
	  /* enable byte recording */
	  
	  num_bytes_read = 0;

	  value = readword();

	  buffer[ 0 ] = '\0';
	  
	  print_buffer = buffer;
	  
	  write_instruction( find_opcode( value ), value );

	  print_buffer = NULL;
	  
	  show_value();
	  
	  /* disable byte recording */
  
	  num_bytes_read = -1;

	  pad_to( COL_OP_CODE );

	  myprintf( buffer );
	}
      else
	{
	  pad_to( COL_HEX_VALUE + ((MAX_NUM_BYTES - 4) * 2) );
	  
	  myprintf( "%08.8lx", value );
	  
	  pad_to( COL_BYTE_VALUES );
	      
	  print_byte( value >> 24 );
	  print_byte( value >> 16 );
	  print_byte( value >>  8 );
	  print_byte( value );
	}
      
      if (name_len > 0 && func_name)
	{
	  int 	i = strlen( func_name );
	  
	  
	  func_name[ i++ ] = (char)((value >> 24) & 0xff);
	  func_name[ i++ ] = (char)((value >> 16) & 0xff);
	  func_name[ i++ ] = (char)((value >>  8) & 0xff);
	  func_name[ i++ ] = (char)( value        & 0xff);
	  func_name[ i++ ] = '\0';
	  
	  --name_len;
	}
      
/*{{{ BRANCH_TRACING */

#ifdef BRANCH_TRACING
      if (trace && pnext_branch && address == pnext_branch->address_refered_to)
	{
	  pad_to( COL_TRACE );
	  
	  myprintf( "branched from: " );
	  
	  while (pnext_branch->address_refered_to == address)
	    {
	      myprintf( "%lx ", pnext_branch->address_of_refer + address_base );
	      
	      free( pnext_branch );
	      
	      pnext_branch = (branch_ref *)remove_head( &branch_trace );
	    }
	}
#endif /* BRANCH_TRACING */

/*}}}*/
      
      new_line();
#if 0      
      fflush( output );
#endif
    }
  
  return;
  
} /* disassemble */

/*}}}*/

/*}}}*/
/*{{{ Branch Tracing       */

#ifdef BRANCH_TRACING

/*{{{ Types & Variables */

static List branch_trace;		/* linked list of branch_ref structures */
static List func_trace;			/* linked list of function headers      */

typedef struct branch_ref
  {
    Node		node;			/* link into branch trace list       */
    unsigned long int	address_refered_to;	/* address to which a branch will go */
    unsigned long int	address_of_refer;	/* address of branch instruction     */
  }
branch_ref;

typedef struct func_ref
  {
    Node		node;			/* link into function header list         */
    unsigned long int	address_of_name;	/* word address of start of function name */
    unsigned long int	length_of_name;		/* byte length of function name           */
  }
func_ref;

/*}}}*/

/*{{{ is_branch         */

/*
 * returns non-zero if the instruction is a branch instruction, 0 otherwise
 */

static signed long int
is_branch( unsigned long instruction )
{
  return 0L;
  
} /* is_branch */

/*}}}*/
/*{{{ check_node        */

/*
 * returs TRUE if the address refered to in
 * the node is earlier in memory than arg
 */

static bool
check_node(
	   Node *	pnode,
	   int		arg )
{
  return (((branch_ref *)pnode)->address_refered_to > arg);

} /* check_node */

/*}}}*/
/*{{{ store_func_ref    */

/*
 * add an entry to the function header list
 */

static void
store_func_ref(
	       unsigned long int	address,
	       unsigned long int	length )
{
  func_ref *	pref;


  pref = (func_ref *)malloc( sizeof( *pref ) );

  if (pref == NULL)
    {
      inform( "out of memory allocating function reference" );

      return;
    }

  pref->address_of_name = address;
  pref->length_of_name  = length;

  add_tail( &func_trace, &pref->node );
  
  return;
  
} /* store_func_ref */

/*}}}*/
/*{{{ store_branch      */

/*
 * add an entry to the branch reference list
 */

static void
store_branch(
	     unsigned long int	address,
	     signed long int	offset )
{
  Node *	pprev;
  branch_ref *	pref;


  pref = (branch_ref *)malloc( sizeof( *pref ) );

  if (pref == NULL)
    {
      inform( "out of memory allocating branch list" );

      return;
    }

  pref->address_of_refer   = address;
  pref->address_refered_to = address + offset;

  pprev = search_list( &branch_trace, check_node, address + offset );

  if (pprev)
    {
      pre_insert( pprev, &pref->node );
    }
  else
    {
      add_tail( &branch_trace, &pref->node );
    }
  
  return;
  
} /* store branch */

/*}}}*/
/*{{{ load_trace_data   */

/*
 * get the information needed by the branch tracing code
 * returns TRUE upon success, FALSE otherwise
 */

static bool
load_trace_data( FILE * input )
{
  unsigned long int	value;

  
  if (fseek( input, 0L, SEEK_SET ) != 0)
    {
      extern int errno;

      
      inform( "failed to rewind input, errno = %d", errno );

      return FALSE;
    }
  
  address = 0;

  init_list( &branch_trace );
  init_list( &func_trace   );
  
  while (read_long( &value ))
    {
      signed long int	offset;


      if ((value & 0xFFFFFF00U) == 0xFF000000U)
	{
	  /* we have found a start of function marker */
	  
	  offset = value & 0xFF;
	  offset /= sizeof (int);

	  /* function name starts at 'address - offset' */

	  store_func_ref( address - offset, offset );
	}
      
      if ((offset = is_branch( value )) != 0)
	{
	  store_branch( address, offset );
	}

      address++;
    }

  return TRUE;
  
} /* load_trace_data */

/*}}}*/

#endif /* BRANCH_TRACING */
  

/*}}}*/
/*{{{ Control Routines     */

/*{{{ usage       */

/*
 * describe the command line options
 */

static void
usage( void )
{
  inform( "valid command line options:" );
#ifdef BRANCH_TRACING
  inform( "-b        enable  branch tracing" );
  inform( "-B        disable branch tracing (the default)" );
#endif
  inform( "-o <file> send output to the named file" );  
  inform( "-help     this message" );
  inform( "-?        this message" );
  inform( "+<offset> addition to current offset for instruction addresses" );
  inform( "-         disassemble from stdin (default)" );
  inform( "<file>    disassemble <file>" );
  inform( "--        treat all futher command line options as file names to disassemble" );
  
  exit( 0 );
  
} /* usage */

/*}}}*/
/*{{{ main        */

/*
 * disassembler entry point
 */
  
int
main(
     int		argc,
     unsigned char **	argv )
{
#ifdef BRANCH_TRACING
  bool		trace         = FALSE;
#endif


  /* find name of program */
  
  if ((ProgName = (unsigned char *)strrchr( (const char *)argv[ 0 ], '/' )) != NULL)
    {
      ProgName++;
    }
  else
    {
      ProgName = argv[ 0 ];
    }

  /*
   * parse command line arguments
   *
   * what we want to achieve is this:
   *
   * disas fred			- disassembles file 'fred' in Helios format
   * disas -t fred		- disassembles file 'fred' in TI format
   * disas fred -t		- disassembles file 'fred' in TI format
   * disas fred -t jim		- disassembles file 'fred' in Helios format and file 'jim' in TI format
   * disas fred jim -t		- disassembles files 'fred' and 'jim' in TI format
   * disas -t fred jim		- disassembles files 'fred' and 'jim' in TI format
   * disas -t fred -T jim	- disaeembles file 'fred' in TI format, file 'jim' in Helios format
   * disas -t fred -T jim -t	- illegal
   *
   * yeah!
   *
   */
   
  if (argc > 1)
    {
      int	arg;
      int	found_switch = -1;


      /*
       * pre-scan arguments for the special case where we have
       * one or more files followed by switches but then no more
       * files ...
       */
      

      for (arg = 1; arg < argc; arg++)
	{
	  if (argv[ arg ][ 0 ] == '-'  &&
	      argv[ arg ][ 1 ] != '\0' &&
	      found_switch     == -1    )
	    {
	      found_switch = arg;
	    }
	  else if (found_switch != -1)
	    {
	      if (arg > 1                     &&
		  argv[ arg - 1 ][ 0 ] == '-' &&
		  argv[ arg - 1 ][ 1 ] == 'o' &&
		  argv[ arg - 1 ][ 2 ] == '\0' )
		{
		  /* catch the very special cases of -o <name> */
		  
		  continue;		  
		}
	      
	      if (arg > 1                     &&
		  argv[ arg - 1 ][ 0 ] == '+' &&
		  argv[ arg - 1 ][ 1 ] == '\0' )
		{
		  /* catch the very special cases of + <offset> */
		  
		  continue;		  
		}
	      
	      /* we have a switch followed by a file name - special case does not apply */

	      found_switch = -1;
	      
	      break;
	    }
	}

      if (found_switch != -1)
	{
	  /* yup special case applies */

	  for (arg = found_switch; arg < argc; arg++ )
	    {
	      if (argv[ arg ][ 0 ] == '+')
		{
		  long int	off = 0;
		      

		  if (argv[ arg ][ 1 ] == '\0')
		    {
		      if (++arg >= argc)
			{
			  inform( "warning - no value following '+' option; option has been ignored" );
			}
		      else
			{
			  if (argv[ arg ][ 1 ] == 'x' || argv[ arg ][ 1 ] == 'X')
			    off = strtoul( (char *)argv[ arg ], (char **)(&argv[ arg ]), 16 );
			  else
			    off = atoi( (char *)argv[ arg ] );

			  if (off == 0)
			    {
			      inform( "warning - no value following '+' option; option has been ignored" );
			  
			      --arg;
			    }
			} 
		    }
		  else
		    {
		      if (argv[ arg ][ 2 ] == 'x' || argv[ arg ][ 2 ] == 'X')
			off = strtoul( (char *)argv[ arg ], (char **)(&argv[ arg ]), 16 );
		      else
			off = atoi( (char *)argv[ arg ] );
		      
		      if (off == 0)
			inform( "warning - string following '+' option is not a number; option has been ignored" );		  
		    }
	      
		  address_base += off;		      
		}
	      else
		{
		  switch (argv[ arg ][ 1 ])
		    {
		    case 'o':
		      if (argv[ arg ][ 2 ] == '\0')
			{
			  if (++arg >= argc)
			    {
			      inform( "no name following '-o' option" );
			      
			      return EXIT_FAILURE;
			    }
			  
			  output_file = argv[ arg ];
			}
		      else
			{
			  output_file = argv[ arg ] + 2;		      
			}
		      
		      break;
		      
		    case 'h':
		    case '?':
		      break;
#ifdef BRANCH_TRACING		      
		    case 'b':
		      trace = TRUE;
		      break;
		      
		    case 'B':
		      trace = FALSE;
		      break;
#endif
		    }
		}	      
	    }
	}
      
      for (arg = 1; arg < argc; arg++)
	{
	  if (argv[ arg ][ 0 ] == '-')
	    {
	      long int	next_arg = TRUE;

	      
	      switch (argv[ arg ][ 1 ])
		{
		case 'o':
		  if (argv[ arg ][ 2 ] == '\0')
		    {
		      if (++arg >= argc)
			{
			  inform( "no name following '-o' option" );

			  return EXIT_FAILURE;
			}		      
		      
		      output_file = argv[ arg ];
		    }
		  else
		    {
		      output_file = argv[ arg ] + 2;		      
		    }
		  break;
		  
		case 'h':
		case '?':
		  usage();
		  break;
		  
		case '\0':
		  input    = stdin;
		  next_arg = FALSE;
		  break;
#ifdef BRANCH_TRACING
		case 'b':
		  trace = TRUE;
		  break;

		case 'B':
		  trace = FALSE;
		  break;
#endif
		case '-':
		  while (++arg < argc)
		    {
		      input = fopen( (char *)argv[ arg ], "rb" );

		      if (input == NULL)
			{
			  inform( "could not open input file '%s' - skipping", argv[ arg ] );
			}
		      else
			{
#ifdef BRANCH_TRACING
			  disassemble( trace );
#else
			  disassemble();
#endif
			}

		      if (input)
			fclose( input );
		    }
		  
		  break;

		default:
		  inform( "unknown command line option '%c'", argv[ arg ][ 1 ] );
		  usage();
		  break;
		}

	      if (next_arg)
		continue;
	    }
	  else if (argv[ arg ][ 0 ] == '+')
	    {
	      unsigned long int	off = 0;
		      

	      if (argv[ arg ][ 1 ] == '\0')
		{
		  if (++arg >= argc)
		    {
		      inform( "warning - no value following '+' option; option has been ignored" );
		    }
		  else
		    {
		      if (argv[ arg ][ 1 ] == 'x' || argv[ arg ][ 1 ] == 'X')
			off = strtoul( (char *)argv[ arg ], (char **)(&argv[ arg ]), 16 );
		      else
			off = (unsigned long int)atoi( (char *)argv[ arg ] );
		      
		      if (off == 0)
			{
			  inform( "warning - no value following '+' option; option has been ignored" );
			  
			  --arg;
			}
		    } 
		}
	      else
		{
		  if (argv[ arg ][ 2 ] == 'x' || argv[ arg ][ 2 ] == 'X')
		    off = strtoul( (char *)argv[ arg ], (char **)(&argv[ arg ]), 16 );
		  else
		    off = (unsigned long int)atoi( (char *)argv[ arg ] );
		      
		  if (off == 0)
		    inform( "warning - string following '+' option is not a number; option has been ignored" );		  
		}
	      
	      address_base += off;		      
	    }	  
	  else
	    {
	      input = fopen( (char *)argv[ arg ], "rb" );

	      if (input == NULL)
		{
		  inform( "could not open input file %s - using stdin instead", argv[ arg ] );

		  /* possible bug - stdin may have been opened in text mode */
		  
		  input = stdin;
		}
	    }

#ifdef BRANCH_TRACING
	  disassemble( trace );
#else
	  disassemble( );
#endif

	  if (input != stdin)
	    {
	      fclose( input );

	      input = NULL;
	    }
	}
    }
  else
    { 
      /* possible bug - stdin may have been opened in text mode */

      input = stdin;
      
#ifdef BRANCH_TRACING
      disassemble( trace );
#else
      disassemble( );
#endif
    }

  return EXIT_SUCCESS;
  
} /* main */

/*}}}*/

/*}}}*/

/*}}}*/
/*{{{ SUN4 hack       */

#ifdef SUN4
/*
 * The gcc compiler appears to generate references to the following functions
 * without providing them in a standard library.  Since the code for the
 * linker does not use either of them, they are provided here as stubs
 */

int ___assert( void ) { return 0; }
int ___typeof( void ) { return 0; }  

#endif /* SUN4 */

/*}}}*/
