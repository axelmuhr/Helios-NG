/*{{{  Header */

/*
 * peep.c : 	Peepholer for the Norcroft/Perihelion C40 C Compiler
 *
 * Copyright (c) 1992, 1993, 1994 Perihelion Software Ltd.
 *
 * All rights reserved.
 *
 * Author :	N Clifton
 * Version :	$Revision: 1.6 $
 * Date :	$Date: 1994/03/29 11:25:39 $
 *
 */

/*}}}*/
/*{{{  Includes */

#include "peep.h"	/* for function prototype conformation */
#include "mcdpriv.h"	/* for FALSE and function prototypes   */
#include "ops.h"	/* for OP codes  */
#include "globals.h"	/* for peepholing enabling */
#include "store.h"	/* for SU_Other  */
#include "codebuf.h"	/* for codebase  */
#include "mcdep.h"	/* for in_stubs  */
#include "errors.h"	/* for cc_warn() and syserr() */
#include "regalloc.h"	/* for regmaskvec */

/*}}}*/
/*{{{  Constants and Macros */

#define PEEP_BUF_SIZE	(1 << 3)	/* NB/ must be power of 2 */

#define num_in_peep_()			(peep_end - peep_start + (peep_end < peep_start ? PEEP_BUF_SIZE : 0))
#define previous_( var )		(((var) - 1) & (PEEP_BUF_SIZE - 1))
#define successor_( var )		(((var) + 1) & (PEEP_BUF_SIZE - 1))

#define is_FP_reg_( reg )		 (((reg) >= R_FV1 && ((reg) <= R_FV1 + NFLTVARREGS)) || \
					  ((reg) >= R_FT1 && ((reg) <= R_FT1 + NFLTTEMPREGS)) )

/*}}}*/
/*{{{  Types */

typedef struct Node
  {
    struct Node *	next;		/* points to next     node in list, NEVER NULL */
    struct Node *	prev;		/* points to previous node in list, NEVER NULL */
  }
Node;

typedef struct
  {
    Node *		head;		/* points to first node in list */
    Node *		earth;		/* always NULL                  */
    Node *		tail;		/* points to last node in list  */
  }
list;	/* name changed from List, because List is already used by compiler */

typedef struct addr_reg
  {
    Node		link;		/* links the structure into a list		 */
    RealRegister	addr_reg;	/* the address register controlled by this node  */
    RealRegister	contents;	/* the register held by the address register     */
    int32		offset;		/* the word offset added to the address register */
    bool		biased;		/* non-0 iff R_BASE has been added into the reg  */
    bool		noted;		/* non-0 iff front end has been told about use	 */
  }
addr_reg;

typedef struct
  {
    Node		link;		/* links the structure into a queue		*/
    RealRegister	reg;		/* The register that should be pushed		*/
    push_type		type;		/* The type of push to be performed		*/
  }
pending_push;

typedef struct peep_data
  {
    peep_type		type;		/* e.g. OUT_INSTR 			 	*/
    int32 		op_code;	/* 32 bits of machine instruction		*/
    int32 		reads;		/* registers accessed				*/
    int32 		writes;		/* registers clobbered				*/
    Symstr *		symbol;		/* external symbol       (if needed)		*/
    int32 		reftype;	/* symbol reference type (if needed)		*/
    LabelNumber *	label;		/* label                 (if needed)		*/
  }
peep_data;

/*}}}*/
/*{{{  Local Variables */

static addr_reg		ar0, ar1, ar2, ar3, ar5;	/* potentially available address registers*/
static list		addr_regs;			/* list of available address registers	*/
static int		node_count = 0;			/* number of nodes in addr_registers	*/

static peep_data	peep_buf[ PEEP_BUF_SIZE ];	/* circular buffer of op codes		*/
static signed int	peep_start 	= 0;		/* next instruction to be emitted	*/
static signed int	peep_end   	= 0;		/* next free slot in table		*/
static bool		xref_pending    = FALSE;	/* TRUE if cross reference is pending	*/
static int32		pending_reftype = 0;		/* the cross reference type		*/
static Symstr *		pending_symbol  = NULL;		/* the cross reference's symbol		*/
static bool		fref_pending    = FALSE;	/* TRUE if forward ref is pending	*/
static LabelNumber *	pending_label   = NULL;		/* the forward reference's label	*/
static int		peep_pending_swaps = 0;		/* number of forward intrs to swap	*/

static list		pending_pushes;			/* queue of pushes pending emission	*/
static list		free_pending_pushes;		/* nodes released from pending push queue*/

/*}}}*/
/*{{{  Exported Variables */

int32			peep_protect_pc	   = 0;		/* number of instructions to protect	*/
int32			peep_eliminated    = 0;		/* number of instructions eliminated	*/
int32			peep_transformed   = 0;		/* number of instructions transformed	*/
int32			peep_swapped       = 0;		/* number of instructions swapped	*/

/*}}}*/
/*{{{  Code */

/*{{{  Local Functions */

/*{{{  debug() */

void
#ifdef __STDC__
debug( const char * format, ... )
#else
debug( format, va_alist )
  const char *	format;		/* printf (3) style format string  */
  va_dcl			/* argumnets for the format string */
#endif
{
  va_list	args;
	

  DBG( if (format == NULL) syserr( peep_null_format ); )
  
#ifdef __STDC__
  va_start( args, format );
#else
  va_start( args );
#endif

  vfprintf( stderr, (char *)format, args );

  putc( '\n', stderr );
  
  va_end( args );

  return;
  
} /* debug */

/*}}}*/
/*{{{  peepf() */

/*
 * print a message about peepholing
 */

void
#ifdef __STDC__
peepf( const char * format, ... )
#else
peepf( format, va_alist )
  const char *	format;		/* printf (3) style format string  */
  va_dcl			/* argumnets for the format string */
#endif
{
  va_list	args;
	

  DBG( if (format == NULL) syserr( peep_null_format ); )
  
  if (!annotations)
    return;
  
#ifdef __STDC__
  va_start( args, format );
#else
  va_start( args );
#endif

  asmf( "; peepholer: " );
  
  vfprintf( asmstream, (char *)format, args );

  putc( '\n', asmstream );
  
  va_end( args );

  return;
  
} /* peepf */

/*}}}*/

/*{{{  init_list() */

/*
 * initialise a list (to empty)
 */

static void
init_list( list * plist )
{
  DBG( if (plist == NULL) syserr( peep_null_list ); )
  
  plist->head  = (Node *)&plist->earth;
  plist->earth = NULL;
  plist->tail  = (Node *)&plist->head;

  return;

} /* init_list */

/*}}}*/
/*{{{  add_head() */

/*
 * add a node onto the front of a list
 */

static void
add_head(
	 list *		plist,	/* list onto which to add node	*/
	 Node *		pnode )	/* node to add to list		*/
{
  DBG( if (plist == NULL || pnode == NULL) syserr( peep_null_parameter ); )
  
  pnode->next       = (Node *)plist->head;
  pnode->prev       = (Node *)plist;
  plist->head->prev = pnode;
  plist->head       = pnode;
  
  return;

} /* add_head */

/*}}}*/
/*{{{  add_tail() */

/*
 * adds a node onto the back of a list
 */

static void
add_tail(
	 list *	plist,	/* list onto which to add node	*/
	 Node *	pnode )	/* node to add onto list	*/
{
  DBG( if (plist == NULL || pnode == NULL) syserr( peep_null_parameter ); )
  
  pnode->next       = (Node *)&plist->earth;
  pnode->prev       = plist->tail;
  plist->tail->next = pnode;
  plist->tail       = pnode; 

  return;

} /* add_tail */

/*}}}*/
/*{{{  remove_head() */

/*
 * removes the first node from a list,
 * and returns a pointer to the removed node
 */

static Node *
remove_head( list * plist )
{
  Node *	pnode;
  

  DBG( if (plist == NULL) syserr( peep_null_list ); )
  
  pnode = plist->head;
  
  if (pnode->next == NULL)
    {
      /* This is not an error - it used by the push peepholing */
      
      return NULL;
    }
  
  pnode->next->prev = (Node *)plist;
  plist->head       = pnode->next;
  pnode->next       = NULL;
  pnode->prev       = NULL;
	
  return pnode;

} /* remove_head */

/*}}}*/
/*{{{  remove_tail() */

/*
 * removes the last node from a list, and returns it
 */

static Node *
remove_tail( list * plist )
{
  Node *	pnode;
  

  DBG( if (plist == NULL) syserr( peep_null_list ); )
  
  pnode = plist->tail;

  if (pnode->prev == NULL)
    {
      /* this is not an error - it is used by the push peepholing */

      return NULL;      
    }
  
  pnode->prev->next = (Node *)(&plist->earth);
  plist->tail       = pnode->prev;
  pnode->next       = NULL;
  pnode->prev       = NULL;
  
  return pnode;

} /* remove_tail */

/*}}}*/
#ifdef DEBUG
/*{{{  remove_node() */

/*
 * removes a node from a list
 */

static void
remove_node( Node * pnode )
{
  if (pnode == NULL)
    {
      syserr( peep_null_parameter );
    }
  
  pnode->prev->next = pnode->next;
  pnode->next->prev = pnode->prev;
  
  return;

} /* remove_node */

/*}}}*/
/*{{{  first_node() */

/*
 * returns the first node in a list or NULL
 */

static Node *
first_node( list * plist )
{
  if (plist             == NULL ||
      plist->head       == NULL ||
      plist->head->next == NULL  )
    {
      syserr( peep_null_parameter );
    }

  return plist->head;
	
} /* first_node */

/*}}}*/
/*{{{  last_node() */

 
/*
 * returns the last node in a list or NULL
 */

static Node *
last_node( list * plist )
{
  if (plist             == NULL ||
      plist->tail       == NULL ||
      plist->tail->prev == NULL  )
    {
      syserr( peep_null_parameter );
    }
  
  return plist->tail;
	
} /* last_node */

/*}}}*/
/*{{{  next_node() */

/*
 * returns the node following the given one
 */

static Node *
next_node( Node * pnode )
{
  if (pnode             == NULL ||
      pnode->next       == NULL ||
      pnode->next->next == NULL  )
    {
      syserr( peep_null_parameter );

      return NULL;
    }  
    
  return pnode->next;
	
} /* next_node */

/*}}}*/
/*{{{  previous_node() */

/*
 * returns the node prior to the given one
 */

static Node *
previous_node( Node * pnode )
{
  if (pnode             == NULL ||
      pnode->prev       == NULL ||
      pnode->prev->prev == NULL  )
    {
      syserr( peep_null_parameter );

      return NULL;
    }
  
  return pnode->prev;
	
} /* previous_node */

/*}}}*/
#else
/*{{{  Inline versions of above functions */

#define remove_node( n ) ((n)->prev->next = (n)->next, (n)->next->prev = (n)->prev)
#define first_node( l ) ((l)->head)
#define last_node( l ) ((l)->tail)
#define next_node( n ) ((n)->next)
#define previous_node( n ) ((n)->prev)

/*}}}*/
#endif /* not DEBUG */
/*{{{   walk_list() */

/*
 * traverses a list applying a function to each node in turn
 */

static void
walk_list(
	  list *	plist,		/* list to scan				*/
	  void (* 	pfunc)() )	/* function to apply to each node	*/
{
  Node *	pnode;
  Node *	pnext;


  DBG( if (plist == NULL || pfunc == NULL) syserr( peep_null_parameter ); )
  
  for (pnode = plist->head; pnode->next != NULL; pnode = pnext)
    {
      pnext = pnode->next;
      
      (*pfunc)( pnode );
    }
    
  return;

} /* walk_list */

/*}}}*/
/*{{{  	search_list() */

/*
 * searches a list applying a given function to each node in turn
 * until one node returns a non-0 result.  Returns this node, or
 * NULL if no node was found.
 */

static Node *
search_list(
	    list *	plist,			/* list to search			*/
	    bool (*	pfunc)( Node *, int ),	/* function to apply to each node 	*/
	    int 	arg )			/* (optional) argument to function	*/
{
  Node *	pnode;
   

  DBG( if (plist == NULL || pfunc == NULL) syserr( peep_null_parameter ); )
  
  for (pnode = plist->head; pnode->next != NULL; pnode = pnode->next)
    {
      if ((*pfunc)( pnode, arg ))
	return pnode;
    }
  
  return NULL;

} /* search_list */

/*}}}*/

/*}}}*/
/*{{{  Address Register Peepholing */

/*{{{  register_p() */

/*
 * returns true if the given node matches the given register
 */

static bool
register_p(
	   Node *	pnode,
	   int		reg )
{
  DBG( if (pnode == NULL) syserr( peep_null_parameter ); )
  
  return ((addr_reg *)pnode)->addr_reg == (RealRegister)reg;
  
} /* register_p */

/*}}}*/
/*{{{  contents_p() */

/*
 * returns true if the given node holds the given register
 */

static bool
contents_p(
	   Node *	pnode,
	   int		reg )
{
  DBG( if (pnode == NULL) syserr( peep_null_parameter ); )
  
  return ((addr_reg *)pnode)->contents == (RealRegister)reg;

} /* contents_p */

/*}}}*/
/*{{{  forget() */

/*
 * forget about the contents of the given address register
 */

static void
forget( addr_reg * preg )
{
  DBG( if (preg == NULL) syserr( peep_null_parameter ); )
  
  preg->contents = GAP;
  preg->offset   = 0;
  preg->biased   = FALSE;

  return;

} /* forget */

/*}}}*/
/*{{{  init_reg() */

/*
 * prepares a node for use in a function
 */

static void
init_reg( addr_reg * preg )
{
  DBG( if (preg == NULL) syserr( peep_null_parameter ); )
    
  /* contents of register are unknown */
  
  forget( preg );

  /* add to list of available registers */
  
  add_tail( &addr_regs, (Node *)preg );

  /* keep track of number of nodes available */
  
  ++node_count;

  /* front end has not been told about our use of this register */
  
  preg->noted = FALSE;

 return;
  
} /* init_reg */

/*}}}*/
/*{{{  peep_change_addr_offset() */

/*
 * The value in the indicated reg has changed by the
 * given amount.  Update any records based upon the
 * contents of that register
 */

void
peep_change_addr_offset(
			RealRegister	reg,		/* register (probably R_SP) which has changed */
			int32		increment )	/* change to previous offset in words */
{
   Node *	pnode;


  if ((pnode = search_list( &addr_regs, contents_p, (int)reg )) != NULL)
    {
      addr_reg *	preg = (addr_reg *)pnode;


      /*
       * subtract increment because new register value is that much
       * closer to our cached value that the old register value
       */
      
      preg->offset -= increment;
    }

  return;
  
} /* peep_change_addr_offset */

/*}}}*/
/*{{{  peep_forget_about() */

/*
 * prevent any address registers from remembering that it
 * contains the contents of the given register adjusted for 
 * machine accessing.
 */

void
peep_forget_about( RealRegister reg )
{
   Node *	pnode;


  while ((pnode = search_list( &addr_regs, contents_p, (int)reg )) != NULL)
    {
      forget( (addr_reg *)pnode );
      
      if (node_count > 1)
	{
	  /* move the node to the end of the list, so that it will be used again soon */
	  
	  remove_node( pnode );

	  add_tail( &addr_regs, pnode );
	}
    }

  return;
  
} /* peep_forget_about */

/*}}}*/
/*{{{  peep_corrupt_addr_reg() */

/*
 * locate the structure for the given address register, and clear it
 */

void
peep_corrupt_addr_reg( RealRegister reg )
{
  Node *	pnode;


  /* locate the register mentioned */
   
  if ((pnode = search_list( &addr_regs, register_p, (int)reg )) != NULL)
    {
      /* forget about the contents of the register */
      
      forget( (addr_reg *)pnode );

      if (node_count > 1)
	{
	  /* move the node to the end of the list, so that it will be used again soon */
	  
	  remove_node( pnode );

	  add_tail( &addr_regs, pnode );
	}
    }

  return;
  
} /* peep_corrupt_addr_reg */

/*}}}*/
/*{{{  peep_corrupt_all_addr_regs() */

/*
 * indicate that all of the address registers (may) have been corrupted
 */

void
peep_corrupt_all_addr_regs( void )
{
  walk_list( &addr_regs, forget );

  return;
  
} /* peep_corrupt_all_addr_regs */

/*}}}*/
/*{{{  peep_note_addr_reg_loaded() */

  

/*
 * inform the peepholer that the given address register
 * has been loaded with the word offset equivalent of
 * the contents of the given register, offset by the
 * given amount (in words), and possibly biased by the
 * contents of the base addressing register
 */

void
peep_note_addr_reg_loaded(
			  RealRegister	reg,		/* address register that has been loaded 	*/
			  RealRegister	contents,	/* register whoes contents have been loaded	*/
			  int32		offset,		/* word offset applied to the 'contents'	*/
			  bool		biased )	/* non-zero if 'contents' has had R_BASE added	*/
{
  Node *	pnode;


  if ((pnode = search_list( &addr_regs, register_p, (int)reg )) != NULL)
    {
      ((addr_reg *)pnode)->contents = contents;
      ((addr_reg *)pnode)->offset   = offset;
      ((addr_reg *)pnode)->biased   = biased;

      if (node_count > 1)
	{
	  /* the node has been used, so put it at the head of the list */

	  remove_node( pnode );
	  
	  add_head( &addr_regs, pnode );
	}
    }

  /* it is not an error for no match to be found */
  
  return;
    
} /* peep_note_addr_reg_loaded */

/*}}}*/
/*{{{  peep_find_loaded_addr_reg() */

/*
 * determines if the indicated register in loaded into an
 * address register, and returns the relevant register if
 * it has been found, as well as setting up the return
 * values for the offset and bias.  Returns GAP (and does
 * not set offset and bias) if no register could be found.
 */

RealRegister
peep_find_loaded_addr_reg(
			  RealRegister	contents,		/* register whoes contents are sought	*/
			  int32 *	offset_return,		/* return value for offset included	*/
			  bool *	biased_return )		/* return value indicating bias status	*/
{
   Node *	pnode;


  if ((pnode = search_list( &addr_regs, contents_p, (int)contents )) != NULL)
    {
      if (offset_return != NULL)
	*offset_return = ((addr_reg *)pnode)->offset;

      if (biased_return != NULL)
	*biased_return = ((addr_reg *)pnode)->biased;

      return ((addr_reg *)pnode)->addr_reg;
    }

  return GAP;
  
  
} /* peep_find_loaded_addr_reg */

/*}}}*/
/*{{{  peep_get_free_addr_reg() */

  

/*
 * returns the least recently used address
 * register that is not 'excludes'.  Returns
 * GAP upon failure
 */

RealRegister
peep_get_free_addr_reg( RealRegister exclude )
{
  Node *	pnode;
  addr_reg *	reg;
  

  if (node_count > 1)
    {
      if (exclude == GAP)
	{
	  /* remove the last node from the list */
	  
	  pnode = remove_tail( &addr_regs );

	  DBG( if (pnode == NULL) syserr( peep_urg ); )
	  
	  reg = (addr_reg *)pnode;
	}
      else
	{
	  /* examine last node on list */
	  
	  pnode = last_node( &addr_regs );

	  DBG( if (pnode == NULL) syserr( peep_urg ); )

	  reg = (addr_reg *)pnode;
	  
	  if (reg->addr_reg == exclude)
	    {
	      pnode = previous_node( pnode );

	      DBG( if (pnode == NULL) syserr( peep_urg ); )

	      remove_node( pnode );

	      reg = (addr_reg *)pnode;
	    }
	  else
	    {
	      pnode = remove_tail( &addr_regs );
	      
	      DBG( if (pnode == NULL) syserr( peep_urg ); )
	    }
	}

      /* place removed node at start of list */
      
      add_head( &addr_regs, pnode );
    }
  else
    {
      pnode = first_node( &addr_regs );

      DBG( if (pnode == NULL) syserr( peep_urg ); )
      
      reg = (addr_reg *)pnode;
      
      if (reg->addr_reg == exclude)
	{
	  return GAP;
	}
    }

  /* invalidate contents of address register */
  
  reg->contents = GAP;

  if (!reg->noted)
    {
      augment_RealRegSet( &regmaskvec, reg->addr_reg );

      reg->noted = TRUE;
    }

  return reg->addr_reg;
    
} /* peep_get_free_addr_reg */

/*}}}*/
/*{{{  peep_init_addr_regs() */

  
/*
 * initialise the queue of available address registers
 */

void
peep_init_addr_regs( int32 reg_mask )
{
  static bool	inited = FALSE;

  
  if (!inited)
    {
      /* initialise the address register structures */

      ar0.addr_reg = RR_AR0;
      ar1.addr_reg = RR_AR1;
      ar2.addr_reg = RR_AR2;
      ar3.addr_reg = RR_AR3;
      ar5.addr_reg = R_ATMP;

      inited = TRUE;
    }

  /* empty the register list */
  
  init_list( &addr_regs );

  node_count = 0;
  
  /* register AR5 is always available */

  init_reg( &ar5 );
  
  ar5.noted = TRUE;	/* never tell front end about our use of this register */
  
  if (no_peepholing)
    return;
  
  /* check the other address registers */
  
  if (!(reg_mask & regbit( RR_AR0 )))
    {
      init_reg ( &ar0 );
    }
  
  if (!(reg_mask & regbit( RR_AR1 )))
    {
      init_reg( &ar1 );
    }
  
  if (!(reg_mask & regbit( RR_AR2 )))
    {
      init_reg( &ar2 );
    }
  
  if (!(reg_mask & regbit( RR_AR3 )))
    {
      init_reg( &ar3 );
    }

  return;
  
} /* peep_init_addr_regs */

/*}}}*/

/*}}}*/
/*{{{  Push Peepholing */

/*{{{  push_reg_p() */

/*
 * returns TRUE if 'pnode' is a pending push of register 'reg'
 */
   
static bool
push_reg_p(
	   Node *	pnode,
	   int		reg )
{
  return ((pending_push *)pnode)->reg == reg;
 
} /* push_reg_p */

/*}}}*/
/*{{{  flush_push() */

/*
 * emits a pending push, and removes the push from the push queue
 */

static void
flush_push( Node * pnode )
{
  pending_push *	push = (pending_push *)pnode;
  RealRegister		reg  = push->reg;
  

  /* remove from the list */
  
  remove_node( pnode );

  /* place node in free pool */

  add_tail( &free_pending_pushes, pnode );
  
  /* ignore already emitted pushes */
      
  if (reg == GAP)
    return;
  
  /* push the register */

  /* debug( "flush push of reg %d", reg ); */
  
  switch (push->type)
    {
    case PUSH_INT:	ipush( reg ); break;
    case PUSH_FLOAT:	fpush( reg ); break;
    case PUSH_DOUBLE:	dpush( reg ); break;
    default:
      syserr( peep_unknown_push );
      break;
    }

  return;
  
} /* flush_push */

/*}}}*/
/*{{{  add_pending_push() */

  
/*
 * add an entry in the pending pushes array
 * The entry is added to the TAIL of the array
 */

void
add_pending_push(
		 RealRegister	reg,	/* the register to be pushed 		*/
		 push_type	type )	/* the kind of push being performed	*/
{
  pending_push *	push;

  
  /* check that there is room */

  peepf( "adding push of reg %ld (type %d)", reg, type );

  push = (pending_push *)remove_head( &free_pending_pushes );

  if (push == NULL)
    {
      push = (pending_push *)GlobAlloc( SU_Other, sizeof (pending_push) );
  
      if (push == NULL)
	{
	  syserr( peep_out_of_memory );
	}
    }

  /* adjust the stack offset (BEFORE adding the pending push) */
  
  if (stack_move != 0)
    {
      correct_stack( FALSE );
    }
  
  /* add the push to the pending queue */

  push->reg  = reg;
  push->type = type;
  
  add_tail( &pending_pushes, (Node *)push );

  /* adjust the stack offset - XXX why ? */
  
  switch (type)
    {
    case PUSH_INT:	stack_offset += sizeof_int;    break;
    case PUSH_FLOAT:	stack_offset += sizeof_float;  break;
    case PUSH_DOUBLE:	stack_offset += sizeof_double; break;
    default:
      syserr( peep_unknown_push );
      break;
    }

  /* if we are not peepholing then emit this push immediately */
  
  if (no_peepholing)
    {
      flush_pending_pushes();
    }

  return;
  
} /* add_pending_push */

/*}}}*/
/*{{{  pop_pending_push() */

/*
 * removes a pending push into register 'reg'
 * returns number of registers poped
 */

int
pop_pending_push(
		 RealRegister	dest,			/* register into which stack value should be placed */
		 bool		can_pop_double )	/* non-zero if two words can be popped */
{
  pending_push *	push;
  RealRegister		src;
  push_type		type;
  

  /* get the last pending push */
  
  push = (pending_push *)remove_tail( &pending_pushes );
  
  if (push == NULL)
    {
      /* if there are no pushes pending then just pop */

      ipop( dest );

      stack_offset -= sizeof_int;

      /* debug( "pop into reg %d, (none stacked)", dest ); */
      
      return 1;
    }
  else
    {
      /* note that this node is now free */
      
      add_tail( &free_pending_pushes, (Node *)push );
    }

  src  = push->reg;
  type = push->type;
  
  /* adjust the stack offset */
  
  switch (type)
    {
    case PUSH_INT:	stack_offset -= sizeof_int;    break;
    case PUSH_FLOAT:	stack_offset -= sizeof_float;  break;
    case PUSH_DOUBLE:	stack_offset -= sizeof_double; break;
    default:
      syserr( peep_unknown_push );
      break;
    }

  /*
   * XXX - NC - 4/2/92
   *
   * Believe it or not, the compiler can ask us to
   * push a double and then pop "half" of it into
   * register!  try the following code ...
   *
   *  func( 1, 2.0, 3,0 );
   *
   * So we must catch the case where we want to pop
   * a double, but only a single word is required
   */
  
  if (type == PUSH_DOUBLE && can_pop_double == 0)
    {
      stack_offset += sizeof_float;

      type = PUSH_FLOAT;

      if (src != GAP)
	{
	  /*
	   * since we are about to push something onto the stack
	   * make sure that all other pending pushes have been
	   * performed
	   */
	  
	  flush_pending_pushes();
	  
	  /* make sure that "low" part of value is pushed onto stack */
      
	  ipush( src );
	}
    }

  if (dest == src)
    {
      /* push followed by pop into same register */

      peepf( "eliminated push/pop pair" );

      if (type == PUSH_DOUBLE)
	{
	  peep_eliminated += 4;
      
	  return 2;
	}
      else
	{
	  peep_eliminated += 2;
	  
	  return 1;
	}
    }
  else if (src == GAP)
    {
      /* we have already pushed the register - now pop it */

      /*
       * NB/ we never really pop floats and doubles -
       * this is only ever done when we want to put FP
       * values into INT registers
       */
      
      /* debug( "pop type %d into reg %d", type, dest ); */
      
      switch (type)
	{
	case PUSH_INT:	  ipop( dest ); return 1;
	case PUSH_FLOAT:  ipop( dest ); return 1;
	case PUSH_DOUBLE: ipop( dest ); ipop( dest + 1 ); return 2;	  
	}
    }
  else
    {
      /* push from one register, pop into another */
      
      /* debug( "reg transfer type %d from %d into reg %d", type, src, dest ); */
      
      switch (type)
	{
	case PUSH_INT:
	  maybe_flush_pending_push( dest );
	  
	  move_register( src, dest, FALSE );
	      
	  peepf( "transformed integer push/pop pair" );

	  ++peep_transformed;
	  ++peep_eliminated;
	  
	  return 1;
	  
	case PUSH_FLOAT:
	  maybe_flush_pending_push( dest );
	      
	  if (is_FP_reg_( dest ))
	    {
	      move_register( src, dest, TRUE );
	      
	      peepf( "transformed float push/pop pair" );

	      ++peep_transformed;
	      ++peep_eliminated;
	    }
	  else
	    {
	      if (stack_move < 0)
		correct_stack( FALSE );

	      /* push floating point value, but pop an integer */

	      fpush( src );

	      ipop( dest );
	    }	  

	  return 1;
	  
	case PUSH_DOUBLE:
	  /* we are about to corrupt dest, so make sure that we are not pushing it */
	  
	  maybe_flush_pending_push( dest );
	  maybe_flush_pending_push( dest + 1 );
	  
	  fpush( src );
	  ipop(  dest );				/* big endian doubles */
	  
	  move_register( src, dest + 1, FALSE );	/* NB/ must transfer integer part as well */
	  
	  peepf( "transformed double push/pop pair" );
	  
	  ++peep_transformed;
	  ++peep_eliminated;
	  
	  return 2;
	}
    }

  return 0;
  
} /* pop_pending_push */

/*}}}*/
/*{{{  flush_pending_pushes() */

/*
 * flushes all pending pushes, (in order)
 */

void
flush_pending_pushes( void )
{
  walk_list( &pending_pushes, flush_push );
  
  return;
  
} /* flush_pending_pushes */

/*}}}*/
/*{{{  maybe_flush_pending_push() */

/*
 * flushes any pending pushes if they
 * push register 'reg'
 */

void
maybe_flush_pending_push( RealRegister reg )
{
  Node *	pnode;


  /* see if the register should be pushed */
  
  pnode = search_list( &pending_pushes, push_reg_p, (int)reg );

  /* if so then emit all the pending pushes up to and including the located one */
  
  if (pnode)
    {
      pending_push *	push;

      
      /* flush back to pnode */

      for (push  = (pending_push *)first_node( &pending_pushes );
	   push != NULL;
	   push  = (pending_push *)next_node( (Node *)push ))
	{
	  reg = push->reg;

	  /* ignore already emited pushes */
      
	  if (reg == GAP)
	    continue;

	  /* push the register */

	  /* debug( "flush push of reg %d because of clash", reg ); */
	  
	  switch (push->type)
	    {
	    case PUSH_INT:	ipush( reg ); break;
	    case PUSH_FLOAT:	fpush( reg ); break;
	    case PUSH_DOUBLE:	dpush( reg ); break;
	    default:
	      syserr( peep_unknown_push );
	      break;
	    }

	  peepf( "flushed push of reg %ld because of clash", reg );
  
	  /* mark this push as having been emitted */
	  
	  push->reg = GAP;

	  /* if we have reached the located node then stop */
	  
	  if (push == (pending_push *)pnode)
	    break;
	}
    }

  return;
  
} /* maybe_flush_pending_push */

/*}}}*/

/*}}}*/
/*{{{  Instruction Peepholing */

/*{{{  can_swap() */

/*
 * returns TRUE if the two instructions
 * can be swapped FALSE otherwise
 *
 * XXX - beware - we cannot ignore the ST register.
 */

static bool
can_swap(
	 int32	a,		/* first instruction 				*/
	 int32	b )		/* second instruction 				*/
{
  unsigned32	a_reads;
  unsigned32	b_reads;
  unsigned32	a_writes;
  unsigned32	b_writes;
  int32		a_op;
  int32		b_op;
  

  a_reads  = peep_buf[ a ].reads;
  b_reads  = peep_buf[ b ].reads;
  a_writes = peep_buf[ a ].writes;
  b_writes = peep_buf[ b ].writes;

  if ((a_reads  & b_writes) ||
      (b_reads  & a_writes) ||
      (a_writes & b_writes)  )
    {
      return FALSE;
    }

  a_op = peep_buf[ a ].op_code;
  b_op = peep_buf[ b ].op_code;

  if (is_normal( a_op ) 				    && /* a is diadic or triadic          */
      is_normal( b_op ) 				    && /* b is diadic or triadic          */
      is_mode( a_op, ADDR_MODE_INDIRECT ) 		    && /* a uses indirect addressing      */
      is_mode( b_op, ADDR_MODE_INDIRECT ) 		    && /* b uses indirect addressing      */
      indirect_addr_reg( a_op) == indirect_addr_reg( b_op )  ) /* share the same address register */
    {
      return FALSE;
    }
      
  return TRUE;
  
} /* can_swap */

/*}}}*/
/*{{{  is_delayed() */

/*
 * returns TRUE iff type is a delayed op type
 */

static bool
is_delayed( peep_type type )
{
  switch (type)
    {
    default:
      return FALSE;

    case OUT_DELAYED:
    case OUT_DELLABREF:
    case OUT_DELSYMREF:
    case OUT_DELSYMXREF:
      return TRUE;
    }
  
} /* is_delayed */

/*}}}*/
/*{{{  eliminate_null_op() */

/*
 * try to eliminate a null operation by adjusting
 * the instructions already in the peephole buffer
 * returns TRUE if the instruction was eliminated,
 * FALSE otherwise
 */

static bool
eliminate_null_op( void )
{
  peep_type	type;
  signed int	delayed;
  int		count;
  int		nulls = 0;
      

  if (no_peepholing)    
    return FALSE;
  
  /* see if we have a delayed instruction somewhere in the previous three op-codes */

  delayed = peep_end;

  for (count = 0; count < 3; count++)
    {
      /* get previous instruction */
      
      delayed = previous_( delayed );

      /* have we reached the start of the peephole buffer ? */
      
      if (delayed == previous_( peep_start ))
	{
	  /* end of search */

	  count = 4;

	  break;
	}

      /* extract the type of this instruction */
      
      type = peep_buf[ delayed ].type;

      /* is it a noop ? */
      
      if (type == OUT_NULL)
	{
	  /* found a padding instruction */
	  
	  ++nulls;
	}
      else if (is_delayed( type ))
	{
	  /* found delayed instruction */

	  break;
	}
    }

  /* see if we found anything */
  
  if (count < 3 && delayed != peep_start)
    {
      signed int	prev;
      
      
      /* we have a delayed instruction and one other instruction */
      
      prev = previous_( delayed );
      
      if (can_swap( delayed, prev ))
	{
	  peep_data		tmp;
	  

	  tmp                 = peep_buf[ delayed ];
	  peep_buf[ delayed ] = peep_buf[ prev    ];
	  peep_buf[ prev    ] = tmp;
	  peep_buf[ delayed ].reads |= regbit( RR_PC );	  /* ensure that prev does not get swapped again */
	  
 	  peepf( "eliminated a NOP" );

	  ++peep_eliminated;
	  
	  return TRUE;
	}
/*    else peepf( "could not swap %08x and %08x (prev r = %x, w = %x)(delayed r = %x, w = %x)",
		   peep_buf[ prev ].op_code,  peep_buf[ delayed ].op_code,
		   peep_buf[ prev ].reads,    peep_buf[ prev ].writes,
		   peep_buf[ delayed ].reads, peep_buf[ delayed ].writes ); */
    }

  if (count == 2 && nulls == 2)
    {
      int32	op = peep_buf[ delayed ].op_code;
      int32	off;

      
      /*
       * We failed to swap the delayed instruction
       * but we might be able to convert the delayed insrtuction
       * and the three following NOPs into a non-delayed insrtuction.
       * This does not save code execution time, but it will save
       * code space.
       */
      
      if ((op >> 26) == B_011010)
	{
	  /* we have a conditional branch */
	  
	  /* removed delayed bit */
	  
	  op &= ~(1 << 21);
	  
	  if (op & (1 << 25))
	    {
	      /* branch is PC relative - increase offset by 2 */
	      
	      off = mask_and_sign_extend_word( op, 0xFFFF );

	      if (!off)
		syserr( peep_no_offset );
	      
	      off += 2;
	      
	      op = (op & 0xFFFF0000U) | (off & 0x0000FFFFU);
	    }
	  
	  peepf( "converted a delayed conditional branch into undelayed" );
	}
      else if ((op >> 25) == (B_0110000))
	{
	  /* we have an unconditional branch */
	  
	  /* removed delayed bit */
	  
	  op &= ~(1 << 24);
	      
	  /* branch is PC relative - increase offset by 2 */
	  
	  off = mask_and_sign_extend_word( op, 0x00FFFFFFU );

	  if (!off)
	    syserr( peep_no_offset );

	  off += 2;

	  op = (op & 0xFF000000U) | (off & 0x00FFFFFFU);

	  peepf( "converted a delayed unconditional branch into undelayed" );
	}
      else
	{
	  return FALSE;
	}
#if 0
      /* change the type to reflect new status */
      
      switch (type)
	{
	case OUT_DELLABREF:
	  type = OUT_LABREF;
	  break;
	  
	case OUT_DELSYMREF:
	  type = OUT_SYMREF;

	  off = mask_and_sign_extend_word( op, 0x00ffffffU );
      
	  if (off != 0)
	    {
	      /* subtract codep from offset */
	  
	      op = (op & 0xff000000U) |
		((off - (codep / sizeof_int)) & 0x00ffffffU);
	    }

	  break;
	  
	case OUT_DELSYMXREF:
	  type = OUT_SYMXREF;
	  break;
	  
	case OUT_DELAYED:
	  type = OUT_INSTR;
	  break;
	  
	default:
	  syserr( peep_unknown_delay_type );
	  break;
	}
      peep_buf[ delayed ].type    = type;
#endif      
      peep_buf[ delayed ].op_code = op;
	  
      /* remove following three NOPs */
	  
      peep_end = successor_( delayed );

      peep_eliminated += 3;
      
      return TRUE;
    }
  
  return FALSE;
    
} /* eliminate_null_op */

/*}}}*/
/*{{{  is_potential_parallel_op() */

/*
 * returns 1 if the op_code has the potential to be converted into
 * a parallel operation, returns 0 otherwise
 */

static int
is_potential_parallel_op(
			 int32 	op,
			 int32	potential_op,
			 int32	other_op )
{
  if (is_op(   op, potential_op )	&&	/* matches sought op			*/
      is_mode( op, ADDR_MODE_INDIRECT )	&&	/* uses indirect addressing		*/
      dest_of( op ) <= 7		&&	/* references registers R0 - R7		*/
      (op & 0xFF)   < 2			)	/* displacement is 0 or 1		*/
    {
      if (has_indirect_side_effects( op ))
	{
	  int32	reg = indirect_addr_reg( op );

	  
	  /* check to see if side effects might affect other instruction */
	  
	  if (dest_of( other_op )           == real_addr_reg( reg ) ||
	      indirect_addr_reg( other_op ) == reg                   )
	    {
	      return 0;
	    }
	}

      return 1;
    }

  return 0;
      
} /* is_potential_parallel_op */

/*}}}*/

#ifdef NOT_USED
/*{{{  is_suitable_triadic() */

/*
 * returns TRUE if the op code is a triadic
 * operation that might be turned into a parallel
 * operation.  Returns FALSE otherwise
 */

static bool 
is_suitable_triadic( int32 op )
{
  if (((op & (B_111   << 29)) == (B_001 << 29))	&&	/* currently triadic                   */
      ((op & (1       << 28)) == 0)		&&	/* uses type 1 addressing              */
      ((op & (B_11    << 21)) == (B_10  << 21))	&&	/* uses register / indirect addressing */
      dest_of( op ) <= 7			&&	/* dst  uses register R0 - R7		*/
      ((op & (B_11111 <<  8)) <= (7     <<  8))	 )	/* src1 uses register R0 - R7          */
    {
      return TRUE;
    }

  return FALSE;
  
} /* is_suitable_triadic */

/*}}}*/
#endif /* NOT_USED */

/*{{{  combine_instrs() */

  
/*
 * try to combine the current instruction with the
 * previous instuction in the peephole buffer.
 * returns TRUE if successful, FALSE otherwise
 */

#define SWAP_OPS					\
  *pcurr_op                 = prev_op; 			\
  *preads                   = peep_buf[ prev ].reads;	\
  *pwrites                  = peep_buf[ prev ].writes;	\
   peep_buf[ prev ].op_code = prev_op = curr_op;	\
   peep_buf[ prev ].reads   = reads;			\
   peep_buf[ prev ].writes  = writes;			\
   curr_op                  = *pcurr_op;		\
   reads                    = *preads;			\
   writes                   = *pwrites;			\
   swapped                  = TRUE


static bool
combine_instrs(
	      int32 *	pcurr_op,
	      int32 *	preads,
	      int32 *	pwrites )
{
  signed int	prev;
  int32		prev_op;
  int32		curr_op = *pcurr_op;
  int32		reads   = *preads;
  int32		writes  = *pwrites;
  int		prev_store;
  int		prev_load;
  int		curr_store;
  int		curr_load;
  bool		swapped = FALSE;
  

  /* see if we can peephole */

  /*
   * note that we disable this kind of peepholing if we are
   * generating memory access checks, because several of our
   * assumptions are no longer valid.  For example the compiler
   * will cache seperate read and write pointers to data areas
   * and could try to store via one pointer and then load via
   * another pointer, in which case it is imperative that these
   * instructions are not combined.
   */
  
  if (no_peepholing || memory_access_checks)
    return FALSE;
  
  /* see if there are two instructions to compare */
  
  if (peep_end == peep_start)
    return FALSE;

  /* find the previous instruction */
  
  prev = previous_( peep_end );

  /* only peephole ordinary instructions */
  
  if (peep_buf[ prev ].type != OUT_INSTR)
    return FALSE;
  
  /* get the op code of the previous instruction */
  
  prev_op = peep_buf[ prev ].op_code;

  /* check to see if the two instructions are the same */

  if (prev_op == curr_op)
    {
      int32	type = (curr_op >> 29) & 0x7;


      /* see if they have any side effects */
      
      if (type == 0)
	{
	  if (!is_mode( curr_op, ADDR_MODE_INDIRECT ) ||  /* if it does not use indirect addressing       */
	      ((curr_op >> 13) & 1) == 0)		  /* or the indirect addressing does not auto inc */
	    {
	      switch (curr_op >> 23)
		{
		case OP_ADDI:
		  if (is_mode( curr_op, ADDR_MODE_IMMEDIATE ))
		    {
		      int32	val = mask_and_sign_extend_word( curr_op, 0xFFFF ) * 2;


		      if (fits_in_16_bits_signed( val ))
			{
			  peep_buf[ prev ].op_code = ((curr_op & 0xFFFF0000U) | (val & 0xFFFFU));

			  peepf( "combined multiple additions" );

			  ++peep_eliminated;
			  
			  return TRUE;
			}
		    }
		break;

		case OP_LDI:
		  peepf( "eliminated multiple identical LDIs" );
		  ++peep_eliminated;
		  return TRUE;
		  
		case OP_LDA:
		  peepf( "eliminated multiple identical LDAs" );
		  ++peep_eliminated;
		  return TRUE;

		case OP_LDF:
		  peepf( "eliminated multiple identical LDFs" );
		  ++peep_eliminated;
		  return TRUE;
		  
		case OP_AND:
		  /*
		   * eg:-   while ((c = *a) != '\0') *a++ = c + 1;
		   */
		  
		  peepf( "eliminated multiple identical ANDs" );
		  ++peep_eliminated;
		  return TRUE;
		  
		default:
		  peepf( "failed to combine multiple identical op codes (%lx)", curr_op );
		  break;
		}
	    }
	}
    }
  /*
   * check for an indirect post incr 
   * followed by an addition to same register
   * or a STIK indirect followed by an addition to the register
   *
   * eg turns:
   *	LDI	*AR6++(1), R0
   *	ADDI	1, AR6
   * into:
   *	LDI	*AR6++(2), R0
   */

  else if (
      (is_op( curr_op, OP_ADDI ))
      &&
      (
       (/*is_op( prev_op, OP_LDI  ) &&*/ is_mode( prev_op, ADDR_MODE_INDIRECT  ))
       ||
       (is_op( prev_op, OP_STIK ) && is_mode( prev_op, ADDR_MODE_IMMEDIATE ))
      )
      &&
      (is_mode( curr_op, ADDR_MODE_IMMEDIATE ))
     )
    {
      int32	dest;
      int32	amount;

      
      dest    = dest_of( curr_op );
      amount  = mask_and_sign_extend_word( curr_op, 0xffff );
      amount += prev_op & 0xff;
      
      if (
	  real_addr_reg( indirect_addr_reg( prev_op ) ) == dest && /* same register affected */
	  ((prev_op >> 11) & 0x1f) == INDIRECT_POST_INCR        && /* op was *ARx++() */
	  fits_in_8_bits_unsigned( amount )                     )  /* new addition total fits */
	{
	  peep_buf[ prev ].op_code = (prev_op & 0xffffff00U) | (amount & 0xffU);

	  peepf( "transformed indirect addition" );

	  ++peep_transformed;
	  
	  return TRUE;
	}
    }

  /*
   * There are several potential optimisations when the previous instruction
   * is a register based LDI, LDA or LDF and the current instruction is diadic
   */

  if (is_load(   prev_op )			&&	/* prev_op is one of LDI, LDA, LDF	*/
      is_mode(   prev_op, ADDR_MODE_REGISTER )	&&	/* it uses register addressing		*/
      is_diadic( curr_op )                  	)	/* current op is a diadic operation	*/
    {
      /* if they both have the same destination */

      if (dest_of( prev_op ) == dest_of( curr_op ))
	{
	  if (is_mode( curr_op, ADDR_MODE_REGISTER ))
	    {
	      unsigned long	op = OP_NOP;
		  
	      
	      /*
	       * check for loading a register
	       * followed by a register based monadic op with
	       * that register as both source and destination
	       *
	       * eg turns:
	       *	LDI	R0, R1
	       *	NEGI	R1, R1
	       * into:
	       *    	NEGI	R0, R1
	       */
	      
	      if (source_of( curr_op ) == dest_of( curr_op ) &&
		  is_monadic( curr_op ))
		{
		  /* avoid the case of LDF followed by LDI */
      
		  if (!is_op( prev_op, OP_LDF) || !is_op( curr_op, OP_LDI ))
		    {
		      peep_buf[ prev ].op_code = (curr_op & 0xff800000U) | (prev_op & 0x007fffffU);
	  
		      peepf( "merged load and diadic op into diadic op" );

		      ++peep_eliminated;
		      
		      return TRUE;
		    }
		}
		  
	      /*
	       * check for moving one register into another followed
	       * by a register based diadic op on that register
	       *
	       * eg turns:
	       *	LDI	R1, R0
	       *	ADDI	R2, R0
	       * into:
	       *	ADDI3	R1, R2, R0
	       *
	       * and:
	       * 	LDF	R5, R7
	       *	MPYF	R5, R7
	       * into:
	       * 	MPYF3	R5, R5, R7
	       */
	  
	      switch ((curr_op >> 23) & 0x3f)
		{
		case OP_ADDC:	  op = OP_ADDC3;	  break;
		case OP_ADDF:	  op = OP_ADDF3;	  break;
		case OP_ADDI:	  op = OP_ADDI3;	  break;
		case OP_AND:	  op = OP_AND3;		  break;
		case OP_ANDN:	  op = OP_ANDN3;	  break;
		case OP_ASH:	  op = OP_ASH3;		  break;
		case OP_LSH:	  op = OP_LSH3;		  break;
		case OP_MPYF:	  op = OP_MPYF3;	  break;
		case OP_MPYI:	  op = OP_MPYI3;	  break;
		case OP_OR:	  op = OP_OR3;		  break;
		case OP_SUBB:	  op = OP_SUBB3;	  break;
		case OP_SUBF:	  op = OP_SUBF3;	  break;
		case OP_SUBI:	  op = OP_SUBI3;	  break;
		case OP_XOR:	  op = OP_XOR3;		  break;
		default:
		  break;
		}      
	      
	      if (op != OP_NOP)
		{
		  peep_buf[ prev ].op_code = (B_0010 << 28) | op << 23 | 0x0U << 21 |
		    prev_op & 0x001f0000U | (prev_op & 0x1f) << 8 |
		      (source_of( curr_op ) == dest_of( prev_op ) ?
			source_of( prev_op ) :
			source_of( curr_op ) );
		  
		  peep_buf[ prev ].writes |= writes;
		  peep_buf[ prev ].reads  |= reads;
		  
		  peepf( "merged load and register op into triadic op" );

		  ++peep_eliminated;

		  return TRUE;
		}
	    }
	  else if (is_mode( curr_op, ADDR_MODE_IMMEDIATE ))
	    {
	      /*
	       * check for moving one register into another followed
	       * by an immediate diadic op on that register
	       *
	       * eg turns:
	       *	LDI	R1, R0
	       *	ADDI	1,  R0
	       * into:
	       *	ADDI3	1, R1, R0
	       */

	      unsigned long	op        = OP_NOP;
	      bool		is_signed = TRUE;
	      
	      
	      switch ((curr_op >> 23) & 0x3f)
		{
		case OP_ADDC:	  op = OP_ADDC3;	  		break;
		case OP_ADDI:	  op = OP_ADDI3;	  		break;
		case OP_AND:	  op = OP_AND3;	 is_signed = FALSE;	break;
		case OP_ANDN:	  op = OP_ANDN3; is_signed = FALSE;	break;
		case OP_ASH:	  op = OP_ASH3;  is_signed = FALSE;	break;
		case OP_LSH:	  op = OP_LSH3;  is_signed = FALSE;	break;
		case OP_MPYI:	  op = OP_MPYI3;	  		break;
		case OP_OR:	  op = OP_OR3;	 is_signed = FALSE;	break;
		case OP_SUBB:	  op = OP_SUBB3;	  		break;
		case OP_SUBI:	  op = OP_SUBI3;	  		break;
		case OP_XOR:	  op = OP_XOR3;	 is_signed = FALSE;	break;
		default:
		  break;
		}      
#ifndef TRIADIC_BINARY_OPS_ARE_UNSIGNED
	      is_signed = TRUE;
#endif
	      /* check that immediate value will fit in triadic operation */
	      
	      if (is_signed)
		{
		  if (!fits_in_8_bits_signed( mask_and_sign_extend_word( curr_op, 0xFFFF )))
		    op = OP_NOP;
		}
	      else
		{
		  if (curr_op & 0xFF00 != 0)
		    op = OP_NOP;
		}
	      
	      if (op != OP_NOP)
		{
		  peep_buf[ prev ].op_code = (B_0011 << 28) | op << 23 | 0x0U << 21 |
		    prev_op & 0x001f0000U | (prev_op & 0x1f) << 8 | (curr_op & 0xff);
		  
		  peep_buf[ prev ].writes |= writes;
		  peep_buf[ prev ].reads  |= reads;
		  
		  peepf( "merged load and immediate op into triadic op" );

		  ++peep_eliminated;
		  
		  return TRUE;
		}
	    }
	  else if (is_op( curr_op, OP_STI )		 &&	/* current op is a store	*/
		   is_mode( curr_op, ADDR_MODE_INDIRECT ) )	/* current op is NOT a STIK 	*/
	    {
	      /*
	       * transforms:
	       *    LDI	R3, R1
	       *    STI	R1, *--AR6(1)
	       * into:
	       *    STI R3, *--AR6(1)
	       *    LDI R3, R1
	       */

	      reads  &= ~regbit( real_register( dest_of(   curr_op ) ) );
	      reads  |=  regbit( real_register( source_of( prev_op ) ) );
	      
	      curr_op = (curr_op & 0xFFE0FFFFU) | ((prev_op << 16) & 0x001F0000U);
	      
	      SWAP_OPS;
	      
	      peepf( "transformed and swapped register store" );

	      ++peep_transformed;
	      ++peep_swapped;
	      
	      if (death & regbit( real_register( dest_of( curr_op ) ) ) )
		{
		  /*
		   * the load into the other register (R1 in the above example)
		   * was uncessary, and  so we can eliminate it!
		   */
		  
		  peepf( "eliminated load after swapped store" );

		  ++peep_eliminated;

		  return TRUE;
		}
	    }
	}
      
      if (is_load( prev_op )                          &&	/* make sure our knowledge of prev is OK     */
	  is_load( curr_op )                          &&	/* current operation is a load               */
	  ((curr_op >> 23) == (prev_op >> 23)	     ||		 /* both loads are the same kind    	     */
	   !(is_op( curr_op, OP_LDF )		     ||		 /* or neither of them is a 		     */
	     is_op( prev_op, OP_LDF ) ) )             &&	 /* load float				     */
	  is_mode( curr_op, ADDR_MODE_REGISTER )      &&	/* current operation is register based       */	
	  dest_of( prev_op ) == source_of( curr_op  )  )	/* previous op's dest is current op's source */
	{
	  if (dest_of( curr_op ) == source_of( prev_op ))	/* current  op's dest is prev    op's source */
	    {
	      /*
	       * check for two register to register loads
	       * with the source and destinations swapped
	       *
	       * eg turns:
	       * 	LDI	R0, R1
	       * 	LDI	R1, R0
	       * into:
	       * 	LDI	R0, R1
	       */
      
	      peepf( "eliminated unnecessary register load" );

	      ++peep_eliminated;

	      if (death & regbit( real_register( dest_of( prev_op ) ) ) )
		{
		  /*
		   * The destination of the previous load (R1 in the above example)
		   * is dead at the end of the load just eliminated.  Therefore
		   * we do not need to do the previous load either!  This can happend
		   * when we are eliminating multiple LDFs, eg:-
		   *
		   * double f1( void )
		   * { extern double f2( double );
		   *   return f2( 2.0 );
		   * }
		   */

		  peepf( "eliminated previous register load as well !" );

		  ++peep_eliminated;

		  peep_end = prev;
		}
	      
	      return TRUE;
	    }
	  else
	    {
	      /*
	       * check for two register to register loads
	       * with the source and destinations matches
	       *
	       * eg turns:
	       *	LDI	R0, R1
	       *	LDI	R1, R2
	       * into:
	       *   	LDI	R0, R2
	       *   	LDI	R0, R1		<-- note swap
	       *
	       * Why bother ?  Because it is very likly that
	       * I will be able to eliminate the second load
	       * later on
	       */

	      reads &= ~regbit( real_register( source_of( curr_op ) ) );
	      reads |=  regbit( real_register( source_of( prev_op ) ) );
	      
	      curr_op = (curr_op & 0xFFFF0000U) | source_of( prev_op );
	      
	      SWAP_OPS;
	      
	      peepf( "swapped and transformed register loads" );

	      ++peep_transformed;
	      ++peep_swapped;
	    }
	}

      if (!swapped					&&	/* if instructions are still OK	*/
	  is_mode( curr_op, ADDR_MODE_REGISTER )	&&	/* and both use reg addressing	*/
	  dest_of( prev_op ) == source_of( curr_op )	&&	/* destination is trasmitted    */
	  !is_op(  curr_op, OP_CMPI )			&&	/* do not swap instructions	*/
	  !is_op(  curr_op, OP_CMPF )			&&	/* that especially set the	*/
	  !is_op(  curr_op, OP_TSTB )			&&	/* status register		*/
	  !is_op(  curr_op, OP_LDF  )			&&	/* do not swap LDI/LDF		*/
	  !is_op(  curr_op, OP_RCPF )			&&	/* do not swap LDI/RCPF		*/
	  !is_op(  curr_op, OP_SUBF )			&&	/* do not swap LDI/SUBF		*/
	  !is_op(  curr_op, OP_SUBRF)			&&	/* do not swap LDI/SUBRF	*/
	  !is_op(  curr_op, OP_ADDF )			&&	/* do not swap LDI/ADDF		*/
	  !is_op(  curr_op, OP_MPYF )			&&	/* do not swap LDI/MPYF		*/
	  !is_op(  curr_op, OP_RND  )			&&	/* do not swap LDI/RND		*/
	  !is_op(  curr_op, OP_NEGF )			)	/* do not swap LDI/NEGF		*/
	{
	  /*
	   * transforms:
	   *    LDI	R3, R1
	   *    ADDI	R1, R2
	   * into:
	   *    ADDI	R3, R2
	   *    LDI	R3, R1
	   */

	  reads  &= ~regbit( real_register( source_of( curr_op ) ) );
	  reads  |=  regbit( real_register( source_of( prev_op ) ) );
	      
	  curr_op = (curr_op & 0xFFFF0000U) | (prev_op & 0x0000FFFFU);

	  if (dest_of( curr_op ) == source_of( prev_op ))
	    {
	      /*
	       * transform:
	       *   LDI  R3, R1
	       *   ADDI R1, R3
	       * into:
	       *   LDI  R3, R1
	       *   ADDI R3, R3
	       * but NO swapping
	       */
	       
	      peepf( "transformed register diadic" );
	    }
	  else
	    {
	      SWAP_OPS;
	      
	      peepf( "transformed and swapped register diadic" );

	      ++peep_swapped;
	    }

	  ++peep_transformed;
	}
    }
  
  /*
   * check for loading a register
   * followed by a triadic op which writes the same
   * register, and does not use the loaded register
   *
   * eg turns:
   *	LDI	R0, R1
   *	MPYI3	5,  R2, R1
   * into:
   *    MPYI3	5,  R2, R1
   *
   * and
   *	LDI	R0, R1
   *	MPYI3	5,  R1, R1
   * into:
   *	MPYI3	5,  R0, R1
   */
  
  if (is_load( prev_op )	 		&&	/* prev_op is one of LDI, LDA, LDF	*/
      is_mode(   prev_op, ADDR_MODE_REGISTER )	&&	/* prev_op uses register addressing	*/
      is_triadic( curr_op )                  	&&	/* current op is a triadic		*/
      dest_of(   prev_op ) == dest_of( curr_op ) )	/* they share the same destination	*/
    {
      bool	src1_is_prev 	= FALSE;
      bool	src2_is_prev 	= FALSE;
      bool	uses_prev	= FALSE;
      int32	T    		= (curr_op >> 21) & 3;
      int32	dest 		= dest_of( prev_op );
      int32	src1 		= (curr_op >> 8) & 0xFF;
      int32	src2 		= curr_op & 0xFF;
      

      /* check to see if triadic op uses dest of previous load */

      if ((curr_op >> 28) & 1)
	{
	  switch (T)
	    {
	    case B_00:
	      if (src1 == dest)
		src1_is_prev = TRUE;
	      break;
	      
	    case B_01:
	      if (src1 == dest)
		src1_is_prev = TRUE;

	      if (real_addr_reg( src2 & 0x7 ) == dest)
		uses_prev = TRUE;
	      break;
	      
	    case B_10:
	      if (real_addr_reg( src1 & 0x7 ) == dest)
		uses_prev = TRUE;
	      break;
	      
	    case B_11:
	      if ((real_addr_reg( src1 & 0x7 ) == dest) ||
		  (real_addr_reg( src2 & 0x7 ) == dest)  )
		uses_prev = TRUE;
	      break;
	    }
	}
      else
	{
	  switch (T)
	    {
	    case B_00:
	      if (src1 == dest)
		src1_is_prev = TRUE;
	      
	      if (src2 == dest)
		src2_is_prev = TRUE;
	      
	      break;
	      
	    case B_01:
	      if (src2 == dest)
		src2_is_prev = TRUE;
	      
	      if (((src1 & 0x7) + 0x08) == dest)
		uses_prev = TRUE;
	      break;
	      
	    case B_10:
	      if (src1 == dest)
		src1_is_prev = TRUE;
	      
	      if (((src2 & 0x7) + 0x08) == dest)
		uses_prev = TRUE;
	      break;
	      
	    case B_11:	      
	      if (((src1 & 0x7) + 0x08) == dest ||
		  ((src2 & 0x7) + 0x08) == dest)
		uses_prev = TRUE;
	      break;
	    }
	}

      if (!uses_prev)
	{
	  peepf( "eliminated unnecessary register load before triadic" );

	  ++peep_eliminated;
	  
	  peep_buf[ prev ].reads   = reads;
	  peep_buf[ prev ].writes  = writes;
	  
	  if (src1_is_prev || src2_is_prev)
	    {
	      /* move source of previous load into triadic */
	      
	      if (src1_is_prev)
		{
		  curr_op = curr_op & 0xFFFF00FFU;
		  curr_op = curr_op | (source_of( prev_op ) << 8);
		}

	      if (src2_is_prev)
		{
		  curr_op = curr_op & 0xFFFFFF00U;
		  curr_op = curr_op | source_of( prev_op );
		}
	    }
	  
	  peep_buf[ prev ].op_code = curr_op;

	  return TRUE;
	}
    }
  
#if 0	/* This is a bad and bogus peephole as it does not set R2 to correct value */
  /*
   * check for a register based diadic
   * followed by a load from the dest back into the source
   *
   * eg turns:
   *	NEGI	R1, R2
   *	LDI	R2, R1
   * into:
   *    NEGI	R1, R1
   *
   * turns:
   *    ADDI	R1, R2
   *	LDI	R2, R1
   * into:
   *    ADDI	R2, R1
   */

  if (is_diadic( prev_op )				&&	/* prev_op is a diadic			*/
      is_mode(   prev_op, ADDR_MODE_REGISTER )		&&	/* which uses register addressing	*/
      is_load(   curr_op )				&&	/* curr_op is one of LDI, LDA, LDF	*/
      is_mode(   curr_op, ADDR_MODE_REGISTER )		&&	/* and also uses register addressing	*/
      dest_of(   prev_op ) == source_of( curr_op )	&&	/* and source and destination of the	*/
      dest_of(   curr_op ) == source_of( prev_op )	)	/* two instructions are reversed	*/
    {
      switch (prev_op >> 23)
	{
	case OP_ABSF:
	case OP_ABSI:
	case OP_FIX:
	case OP_FLOAT:
	case OP_NEGF:
	case OP_NEGI:
	case OP_NOT:
	  peep_buf[ prev ].writes &= ~regbit( real_register( dest_of( prev_op ) ) );
	  peep_buf[ prev ].writes |=  regbit( real_register( dest_of( curr_op ) ) );
	  peep_buf[ prev ].op_code = (prev_op & 0xFFE0FFFFU) | (curr_op & 0x001F0000U);
	  
	  peepf( "removed unnecessary register load after monadic" );

	  ++peep_eliminated;
	  
	  return TRUE;

	case OP_AND:
	case OP_ADDF:
	case OP_ADDI:
	case OP_MPYF:
	case OP_MPYI:
	case OP_OR:
	case OP_TSTB:
	case OP_XOR:
	  peep_buf[ prev ].reads   = reads;
	  peep_buf[ prev ].writes  = writes;
	  peep_buf[ prev ].op_code = (prev_op & 0xFFE00000U) | (curr_op & 0x001FFFFFU);
	  
	  peepf( "removed unnecessary register load after diadic" );

	  ++peep_eliminated;
	  
	  return TRUE;
	}
    }
#endif /* NEVER */
  
  /*
   * check for a store followed by a load to the same register
   *
   * eg turns:
   *	STI	R0, *AR6
   *	LDI	*AR6, R0
   * into:
   *    STI	R0, *AR6
   */

  if (is_op(   prev_op, OP_STI )             	   &&	/* previous operation is an integer store        */
      is_op(   curr_op, OP_LDI )                   &&	/* current  operation is an integer load         */
      is_mode( prev_op, ADDR_MODE_INDIRECT ) 	   &&	/* previous operation uses indirect addressing   */
      is_mode( curr_op, ADDR_MODE_INDIRECT ) 	   &&	/* current  operation uses indirect addressing   */
      source_of( prev_op ) == source_of( curr_op ) &&	/* the have the same form of indirect addressing */
      dest_of( prev_op )   == dest_of( curr_op  )  && 	/* previous op's dest is current op's dest       */
      ! has_indirect_side_effects( curr_op )        )	/* the indirect addressing has no side effects   */
    {
      peepf( "eliminated unnecessary indirect load" );

      ++peep_eliminated;

      return TRUE;
    }

  /*
   * check for a load constant
   */

  if ( (prev_op >> 28)         == OP_LDIc       &&	/* previous of is a load (un)conditional	*/
      ((prev_op >> 23) & 0x1F) == C_U		&&	/* load is unconditional			*/
      is_mode( prev_op, ADDR_MODE_IMMEDIATE ) 	 )	/* previous operation uses immediate addressing	*/
    {
      if (is_op(   curr_op, OP_LDI )		      && /* current op is a load			 */
	  is_mode( curr_op, ADDR_MODE_REGISTER  )     && /* current  operation uses register  addressing */
          dest_of( prev_op ) == source_of( curr_op  ) )	 /* previous op's dest is current op's source	*/
	{
	  /*
	   * check for a load constant followed by a move register
	   *
	   * eg turns:
	   *    LDIu	0,  R5
	   *	LDI	R5, R0
	   * into:
	   *    LDIu	0, R5
	   *    LDIu	0, R0
	   *
	   * Why bother, I hear you ask.  The answer is that I hope
	   * at a later date, to be able to eliminate the first load
	   */

	  *pcurr_op = curr_op = (prev_op & 0xFFE0FFFFU) | (curr_op & 0x001F0000U);

	  *preads = reads = 0;
      
	  peepf( "transformed load register into load immeadiate" );

	  ++peep_transformed;
	}
#ifdef STIK_NOW_WORKS_ON_HARDWARE
      else if (is_op(   curr_op, OP_STI )	        &&	/* we have a store instruction    */
	       is_mode( curr_op, ADDR_MODE_INDIRECT )   &&	/* which uses indirect addressing */
	       dest_of( curr_op ) == dest_of( prev_op )  )	/* and they share the same destination */
	{
	  int32		val;

	  
	  /*
	   * check for a load followed by a store.
	   *
	   * turns:
	   *	LDIu	0,	R1
	   *	STI	R1,	*AR6
	   * into:
	   *	STIK	0,	*AR6
	   *	LDIu	0,	R1
	   */

	  val = mask_and_sign_extend_word( prev_op, 0xFFFF );
	  
	  if (fits_in_5_bits_signed( val ))
	    {
	      reads  &= ~regbit( real_register( dest_of( curr_op ) ) );	/* no longer reads destination */
	      
	      curr_op = (curr_op & 0xFF80FFFFU) | (ADDR_MODE_IMMEDIATE << 21) | ((val & 0x1F) << 16);
	      
	      SWAP_OPS;

	      if (regbit( real_register( dest_of( curr_op ) ) ) & death)
		{
		  peepf( "eliminated register load and transformed register store" );

		  ++peep_eliminated;
		  
		  return TRUE;
		}
	      else
		{
		  peepf( "transformed store register into store immeadiate" );

		  ++peep_transformed;
		}
	    }
	}
#endif /* STIK_NOW_WORKS_ON_HARDWARE */
    }
      
  /*
   * check for two successive ANDs
   *
   * eg turns:
   *	AND	0xff, R0
   *	AND	0x7f, R0
   * into:
   *    AND	0x7f, R0
   */

  if (is_op(   prev_op, OP_AND )             	  &&	/* previous operation is AND        */
      is_op(   curr_op, OP_AND )                  &&	/* current  operation is AND         */
      is_mode( prev_op, ADDR_MODE_IMMEDIATE ) 	  &&	/* previous operation uses immediate addressing   */
      is_mode( curr_op, ADDR_MODE_IMMEDIATE ) 	  &&	/* current  operation uses immediate addressing   */
      dest_of( prev_op )   == dest_of( curr_op  ) ) 	/* previous op's dest is current op's dest       */
    {
      peep_buf[ prev ].op_code = (prev_op & 0xFFFF0000U) | (prev_op & 0xFFFF & curr_op);

      ++peep_eliminated;

      peepf( "eliminated unnecessary immediate AND" );

      return TRUE;
    }
  
  /* See if both instructions alter the same register (except the status register) */

  if (writes & (peep_buf[ prev ].writes) & (~(1 << RR_ST)))
    return FALSE;

  /*
   * check for two load or store operations in succession
   */

  prev_load  = is_potential_parallel_op( prev_op, OP_LDI, curr_op );
  prev_store = is_potential_parallel_op( prev_op, OP_STI, curr_op );
  curr_load  = is_potential_parallel_op( curr_op, OP_LDI, prev_op );
  curr_store = is_potential_parallel_op( curr_op, OP_STI, prev_op );

  if (prev_load + prev_store + curr_load + curr_store >= 2)
    {
      if (   (curr_op & 0xFF)   == 0	  /* displacement is zero   */
	  && (curr_op & 0xc000) == 0)	  /* displacement is "used" */
	{
	  /* change mode to straight relative */
	  
	  curr_op = (curr_op & 0xFFFF07FFUL) | (INDIRECT_REL << 11);
	}
	      
      if (   (prev_op & 0xFF)   == 0	  /* displacement is zero   */
	  && (prev_op & 0xc000) == 0)	  /* displacement is "used" */
	{
	  /* change mode to straight relative */
	  
	  prev_op = (prev_op & 0xFFFF07FFUL) | (INDIRECT_REL << 11);
	}
	      
      if (prev_load)
	{
	  if (peep_buf[ prev ].writes & regbit( RR_PST ))
	    {
	      /*
	       * oh oh, the compiler has already used the fact that the
	       * load instruction writes to the ST register, so we cannot
	       * combine it with another instruction, preventing it from
	       * writing to ST.
	       *   See comment in peep_sets_status_reg()
	       */

	      return FALSE;	      
	    }
	  
	  if (curr_load)
	    {
	      /* previous and current LDI */
	      
	      /* check that the destination of the loads */
	      
	      if (dest_of( prev_op ) == dest_of( curr_op ))
		{
		  peepf( "WARNING: two successive loads with the same destination" );
		  
		  /* ignore first load */
		  
		  peep_buf[ prev ].op_code = curr_op;
		  peep_buf[ prev ].reads   = reads;
		  peep_buf[ prev ].writes  = writes;
		  
		  return TRUE;
		}
	      
	      peep_buf[ prev ].op_code = (B_11 << 30)                           |
	                     	         (OP_LDI_LDI << 25)                     |
		                         (prev_op & (B_111 << 16)) << (22 - 16) |
		                         (curr_op & (B_111 << 16)) << (19 - 16) |
		                         (curr_op & (0xFF  <<  8)) << ( 8 -  8) |
			                 (prev_op & (0xFF  <<  8)) >> ( 8 -  0);

	      peep_buf[ prev ].reads  |= reads;
	      peep_buf[ prev ].writes |= writes;
	      peep_buf[ prev ].writes &= ~regbit( RR_ST );
	      
	      peepf( "transformed LDI, LDI into LDI || LDI" );

	      ++peep_eliminated;
	      ++peep_transformed;
	      
	      return TRUE;
	    }
	  else
	    {
	      /* previous LDI, current STI */
	      
	      /* check that the destination of the load is not the source of the store */
	      
	      if (dest_of( prev_op ) != dest_of( curr_op ))
		{
		  peep_buf[ prev ].op_code = (B_11 << 30)                           |
		                             (OP_LDI_STI << 25)                     |
		                             (prev_op & (B_111 << 16)) << (22 - 16) |
			                     (curr_op & (B_111 << 16)) << (16 - 16) |
			                     (curr_op & (0xFF  <<  8)) << ( 8 -  8) |
			                     (prev_op & (0xFF  <<  8)) >> ( 8 -  0);
		  
		  peep_buf[ prev ].reads  |= reads;
		  peep_buf[ prev ].writes |= writes;
		  peep_buf[ prev ].writes &= ~regbit( RR_ST );
		  
		  peepf( "transformed LDI, STI into LDI || STI" );

		  ++peep_eliminated;
		  ++peep_transformed;
		  
		  return TRUE;
		}
	    }
	}
      else /* prev op is STI */
	{
	  if (curr_load)
	    {
	      if (writes & regbit( RR_PST ))
		{
		  /*
		   * See comment in peep_sets_status_reg()
		   */

		  return FALSE;	      
		}
	  
	      /* previous STI, current LDI */
	      
	      if (source_of( prev_op ) == source_of( curr_op ))
		{
		  /* weird - a store followed by a load from the same place ! */
		  
		  if (dest_of( prev_op ) == dest_of( curr_op ))
		    {
		      peepf( "eliminated register load" );

		      ++peep_eliminated;
		      
		      return TRUE;
		    }
		  else
		    {
		      /* transform load into register move */
		      
		      *preads   = regbit( dest_of( prev_op ) );
		      *pwrites  = regbit( dest_of( curr_op ) ) | regbit( RR_ST );
		      *pcurr_op = build_op( OP_LDI, ADDR_MODE_REGISTER,
					   dest_of( curr_op ), dest_of( prev_op ) );
		      
		      peepf( "transformed memory access into register transfer" );

		      ++peep_transformed;
		      
		      return FALSE;
		    }
		}

#if 0
	      /*
	       * XXX - NC
	       *
	       * We cannot safely generate this instruction because
	       * the load will take place before the store, and the
	       * destination of the store might be an alias for the
	       * source of the load.  *sigh*
	       *
	       * For example see the code for removing spaces from
	       * a command line argument in ExecuteList() in c.c
	       * This generats just such an example
	       */
	      
	      peep_buf[ prev ].op_code = (B_11 << 30)                           |
		                         (OP_LDI_STI << 25)                     |
		                         (curr_op & (B_111 << 16)) << (22 - 16) |
		                         (prev_op & (B_111 << 16)) << (16 - 16) |
		                         (prev_op & (0xFF  <<  8)) << ( 8 -  8) |
			                 (curr_op & (0xFF  <<  8)) >> ( 8 -  0);
	      
	      peep_buf[ prev ].reads  |= reads;
	      peep_buf[ prev ].writes |= writes;
	      peep_buf[ prev ].writes &= ~regbit( RR_ST );
	      
	      peepf( "transformed STI, LDI into LDI || STI" );

	      ++peep_transformed;
	      ++peep_eliminated;
	      
	      return TRUE;
#endif
	    }
#ifdef STI_STI_NOW_WORKS_ON_HARDWARE
	  else
	    {
	      /* previous and current STI */
	      
	      if (source_of( prev_op ) == source_of( curr_op ))
		{
		  /* weird two stores to the same place ! */
		  
		  peepf( "eliminating duplicate store!" );

		  ++peep_eliminated;
		  
		  /* eliminate the first store */
		  
		  SWAP_OPS;

		  return TRUE;
		}
	      
	      peep_buf[ prev ].op_code = (B_11 << 30)                           |
		                         (OP_STI_STI << 25)                     |
		                         (prev_op & (B_111 << 16)) << (22 - 16) |
		                         (curr_op & (B_111 << 16)) << (16 - 16) |
		                         (curr_op & (0xFF  <<  8)) << ( 8 -  8) |
			                 (prev_op & (0xFF  <<  8)) >> ( 8 -  0);
	      
	      peep_buf[ prev ].reads  |= reads;
	      peep_buf[ prev ].writes |= writes;
	      peep_buf[ prev ].writes &= ~regbit( RR_ST );
	      
	      peepf( "transformed STI, STI into STI || STI" );

	      ++peep_eliminated;
	      ++peep_transformed;
	      
	      return TRUE;
	    }
#endif /* STI_STI_NOW_WORKS_ON_HARDWARE */
	}
    }

  return FALSE;
  
} /* combine_instrs */

/*}}}*/
/*{{{  peep_swap() */

  
  
static void
peep_swap(
	  signed int	a,
	  signed int	b,
	  char *	reason )
{
  peep_data	tmp;

  
  tmp           = peep_buf[ a ];
  peep_buf[ a ] = peep_buf[ b ];
  peep_buf[ b ] = tmp;

  if (asmstream != NULL && annotations)
    {
      asmf( "; peepholer: swapped " );

      decode_instruction( peep_buf[ a ].op_code, FALSE );

      asmf( " with " );

      decode_instruction( peep_buf[ b ].op_code, FALSE );
  
      asmf( " because: %s\n", reason );
    }

  ++peep_swapped;
  
  return;
  
} /* peep_swap */

/*}}}*/
/*{{{  emit_from_peep() */

/*
 * remove an instruction from the peephole buffer
 * performing any peep hole optimisations that you can
 */

static void
emit_from_peep( void )
{
  peep_data *	instruction;
  int32		offset;
  int32		op;


  DBG( if (peep_start == peep_end) syserr( peep_peepholer_empty ); )
  
  /*
   * possible futher optimisations :-
   *
   *   parallel load / store & diadic / triadic op
   *
   *   elimination of compare then link-and-jump with a conditional link-and-jump
   *
   */

  /* now send out an instruction */

  instruction = &(peep_buf[ peep_start ]);

  op = instruction->op_code;

  switch (instruction->type)
    {
    default:
      syserr( peep_unknown_type, instruction->type );
      break;
      
    case OUT_XREF:
      codexrefs = (CodeXref *) global_list3( SU_Other, codexrefs,
					    instruction->reftype + codebase + codep,
					    instruction->symbol );
      
      outcodewordaux( op, LIT_RELADDR, instruction->symbol );
      
      break;

    case OUT_DELLABREF:
    case OUT_LABREF:
      if (lab_isset_( instruction->label ))
	{
	  /*
	   * This can happen when we generate an unconditional backwards reference,
	   * but we do not know the offset, because the instruction might be moved
	   * by the peepholer.
	   */

	  switch (instruction->reftype)
	    {
	    case LABREF_OFF24:
	      offset = instruction->label->u.defn & 0x7fffffffU;
	      offset = (offset - codep) / sizeof_int;
	      if ((op >> 24) & 1)
		offset -= 3; /* delayed version */
	      else
		offset -= 1;	      
	      op = (op & 0xFF000000U) | (offset & 0x00FFFFFFU);
	      break;
	      
	    default:
	      syserr( peep_unexpected_back_ref );
	      break;
	    }
	}
      else
	{
	  addfref_( instruction->label, codep | instruction->reftype );
	}
      
      /* drop through */

    case OUT_DELAYED:
      /* drop through */
      
    case OUT_NULL:
      /* drop through */

    case OUT_INSTR:
      outcodeword( op, LIT_OPCODE );
      break;

    case OUT_DELSYMXREF:
      /* drop through */
      
    case OUT_SYMXREF:
      codexrefs = (CodeXref *) global_list3( SU_Other, codexrefs,
					    instruction->reftype + codebase + codep,
					    instruction->symbol );
      /* drop through */

    case OUT_SYMREF:
      outcodewordaux( op, LIT_RELADDR, instruction->symbol );
      break;

    case OUT_DELSYMREF:
      offset = mask_and_sign_extend_word( op, 0x00ffffffU );
      
      if (offset != 0)
	{
	  /* subtract codep from offset */
	  
	  op = (op & 0xff000000U) |
	    ((offset - (codep / sizeof_int)) & 0x00ffffffU);
	}
				   
      outcodewordaux( op, LIT_RELADDR, instruction->symbol );
      
      break;
    }

  peep_start = successor_( peep_start );
  
  return;
   
} /* emit_from_peep */

/*}}}*/
/*{{{  peep_xref() */

/*
 * this function indicates that the next instruction to be appended
 * to the peepholer's buffer is a cross reference type instruction, rather than
 * an ordinary instruction
 */
  
void
peep_xref(
	  int32		reftype,
	  Symstr *	symbol )
{
  xref_pending    = TRUE;
  pending_reftype = reftype;
  pending_symbol  = symbol;


  return;
  
} /* peep_xref */

/*}}}*/
/*{{{  peep_symref() */

  
/*
 * this function indicates that the next instruction to be
 * appended to the peepholer's buffer references a symbol
 */
  
void
peep_symref( Symstr * symbol )
{
  xref_pending    = TRUE;
  pending_reftype = LABREF_NONE;
  pending_symbol  = symbol;

  return;
  
} /* peep_symref */

/*}}}*/
/*{{{  peep_fref() */

/*
 * this function indicates that the next instruction to be
 * appended to the peepholer's buffer has a forward
 * reference to a label
 */
  
void
peep_fref(
	  LabelNumber *	label,
	  int32		reftype )
{
  fref_pending    = TRUE;
  pending_label   = label;
  pending_reftype = reftype;

  return;
  
} /* peep_fref */

/*}}}*/
/*{{{  peep_shift_back() */

/*
 * Try to push the latest instruction in the peepholer back
 * by the number of instructions indicated.  Returns the
 * actual number of instructions skipped.
 */

int
peep_shift_back( int n )
{
  signed int	prev;
  signed int	last;
  int		done;
  
  
  /* check that we can move the instructions */
  
  if (xref_pending || fref_pending || peep_end == peep_start || n < 1)
    {
      return 0;
    }

  /* get last instruction */
  
  last = prev = previous_( peep_end );

  /* initialise count of successful swaps */
  
  done = 0;
  
  /* examine previous instructions */
  
  while (prev != peep_start && n--)
    {
      prev = previous_( prev );

      if (can_swap( last, prev ))
	{
	  /* swap the two instructions */

	  peep_swap( last, prev, "backwards pipeline conflict avoidance" );
	  
	  /* update pointer to the "last" instruction */
	  
	  last = prev;

	  /* increment count */
	  
	  done++;
	}
      else
	{
#if 0
	  asmf( "; peepholer: could not swap " );

	  decode_instruction(  peep_buf[ last ].op_code, FALSE );

	  asmf( " with " );
	  
	  decode_instruction( peep_buf[ prev ].op_code, FALSE );

	  asmf( "\n" );
#endif	  
	  return done;
	}
    }

  return done;
  
} /* peep_shift_back */

/*}}}*/
/*{{{  append_peep() */

void
append_peep(
	    peep_type		type,
	    int32		op_code,
	    int32		reads,
	    int32		writes,
	    Symstr *		symbol,
	    LabelNumber *	label,
	    int32		reftype )
{
  signed int	next;


  if (!xref_pending && !fref_pending)
    {
      /* see if we have a redundant op-code */

      if (type == OUT_NULL && eliminate_null_op())
	{
	  return;
	}

      /* see if we can combine two ops into one */
  
      if (type == OUT_INSTR && combine_instrs( &op_code, &reads, &writes ))
	{
	  return;
	}
    }

  if (peep_protect_pc > 0)
    {
      reads |= regbit( RR_PC );

      --peep_protect_pc;
    }
  
  /* insert the instruction into the table */
  
  peep_buf[ peep_end ].type    = type;
  peep_buf[ peep_end ].op_code = op_code;
  peep_buf[ peep_end ].reads   = reads;
  peep_buf[ peep_end ].writes  = writes;
  peep_buf[ peep_end ].symbol  = symbol;
  peep_buf[ peep_end ].reftype = reftype;
  peep_buf[ peep_end ].label   = label;

  /* check to see if this instruction was a cross reference or a forward reference */

  if (xref_pending)
    {
      if (fref_pending)
	{
	  syserr( peep_fwd_and_cross_ref );
	}
      
      if (type == OUT_INSTR)
	{
	  if (pending_reftype == LABREF_NONE)
	    {
	      peep_buf[ peep_end ].type    = OUT_SYMREF;
	      peep_buf[ peep_end ].symbol  = pending_symbol;
	    }
	  else
	    {
	      peep_buf[ peep_end ].type    = OUT_XREF;
	      peep_buf[ peep_end ].symbol  = pending_symbol;
	      peep_buf[ peep_end ].reftype = pending_reftype;
	    }
	}
      else if (type == OUT_SYMREF)
	{
	  peep_buf[ peep_end ].type    = OUT_SYMXREF;
	  peep_buf[ peep_end ].reftype = pending_reftype;
	}
      else if (type == OUT_DELSYMREF)
	{
	  peep_buf[ peep_end ].type    = OUT_DELSYMXREF;
	  peep_buf[ peep_end ].reftype = pending_reftype;
	}
      else
	{
	  syserr( peep_special_pending_cross );
	}

      xref_pending = FALSE;
    }
  else if (fref_pending)
    {
      if (type == OUT_INSTR)
	{
	  peep_buf[ peep_end ].type    = OUT_LABREF;
	  peep_buf[ peep_end ].label   = pending_label;
	  peep_buf[ peep_end ].reftype = pending_reftype;
	}
      else
	{
	  syserr( peep_special_pending_fwd );
	}

      fref_pending = FALSE;
    }

  /* adjust pointer to indicate next free slot */

  next = successor_( peep_end );

  if (next == peep_start)
    {
      /* table is full */

      emit_from_peep();
    }

  peep_end = next;

  /*
   * try to avoid pipeline conflicts when using (real) address registers
   */

  if ((new_stubs || !in_stubs) &&	/* do not alter stub generation */
      !no_peepholing)			/* only operate if allowed to	*/
    {
      signed int	back1 = previous_( peep_end );
      signed int	back2 = previous_( back1 );
      int32		prev  = 0;


      if (num_in_peep_() > 1)
	prev = peep_buf[ back2 ].op_code;
      
      if (peep_pending_swaps > 0)
	{
#if 0
	  /*
	   * XXX - NC 28/03/94
	   *
	   * Beware!  We cannot use this optimisation.
	   * Poiinter aliasing can make this change invalid.  For example
	   * in kernel/gslice.c/Dispatch() we have the following code
	   * fragment:
	   * 	pList->Tail->Next = pNode;
	   *	pList->Tail = pNode;
	   *	if (pList->Head != &(pList->Earth)) ...
	   *
	   * which produces (amoungst other bits) the following code
	   *
	   * LDI	*AR1, R2             ; R2  = *(list->Tail)     [C pointer]
	   * LSH3	-2 , R2 , AR2        ; AR2 = *(pList->Tail)    [Machine pointer]
	   * STI	AR0, *++AR2(IR0)     ; (AR0 = pNode)           [C pointer]
	   * STI	AR0, *AR1            ; (AR1 = &(pList->Tail))  [Machine pointer]
	   * LDI	*--AR1(2), R0        ; R0  = *(pList->Head)    [C pointer]
	   * CMPI	R1 , R0              ; (R1  = &(pList->earth)) [C pointer]
	   *
	   * after the optimisation below, however, this becomes
	   *
	   * LDI	*AR1, R2             
	   * LSH3	-2 , R2 , AR2        
	   * STI	AR0, *AR1            <-- moved from here
	   * LDI	*--AR1(2), R0        
	   * STI	AR0, *++AR2(IR0)     <-- moved to here
	   * CMPI	R1 , R0              
	   *
	   * (This has moved the STI AR0, *++AR2(IR0) away from the LSH3 -2, R2, AR2)
	   *
	   * Unhappily, with an initial empty list, *(pList->Tail) == &(pList->Head),
	   * so that the LDI *--AR1(2), R0 above is reading in the value that is
	   * written by the STI AR0, *++AR2(IR0), except that these are now out of
	   * sequence.
	   * 	This kind of pointer aliasing cannot be detected or avoided here,
	   * so we must disable the optimisation.
	   */
	   
	  if (num_in_peep_() > 1 	&& /* if there are instructions to swap */
	      is_normal( op_code )      && /* which are not parallel instructions */
	      is_normal( prev )         &&
	      can_swap( back1, back2 )   ) /* and we are allowed to swap them   */
	    {
	      peep_swap( back1, back2, "forwards pipeline conflict avoidance" );
	      
	      --peep_pending_swaps;
	    }
	  else
#endif
	    {
	      if (asmstream != NULL && num_in_peep_() > 1 && annotations)
		{
		  asmf( "; could not swap op " );
		  
		  decode_instruction( op_code, FALSE );
		  
		  asmf( " with op " );
		  
		  decode_instruction( prev, FALSE );
		  
		  asmf( "\n" );
		}
	      
	      peepf( "%d forward swaps left undone", peep_pending_swaps );
	      
	      /* stop trying to swap instrs */

	      peep_pending_swaps = 0;
	    }
	}
      else if (
	       is_normal( op_code )                    	         && /* is diadic or triadic		*/
	       dest_of( op_code ) == hardware_register( R_ATMP ) && /* and its destination is R_ATMP	*/
	       !is_op( op_code, OP_STI  )              	         && /* and it is not a store		*/
	       !is_op( op_code, OP_STIK )              	     	 && /* of any kind			*/
	       !is_op( op_code, OP_STF  )                         )
	{
	  (void) peep_shift_back( 2 );
	}
      else if (
	       num_in_peep_() > 1			&&	/* there is a previous op to check	*/
	       is_normal( prev ) 			&&	/* diadic or triadic 			*/
	       is_normal( op_code ) 			&&	/* diadic or triadic 			*/
	       is_mode( op_code, ADDR_MODE_INDIRECT ) 	&&	/* current op uses indirect addressing	*/
	       dest_of( prev ) == real_addr_reg( indirect_addr_reg( op_code ) ) )
	{
	  /*
	   * indicate that we would like the current instruction swapped forwards
	   */
      
	  if (is_op( prev, OP_LDPK ))
	    peep_pending_swaps = 0;
	  else if (is_op( prev, OP_LDA))
	    peep_pending_swaps = 1;
	  else
	    peep_pending_swaps = 2;
	}
    }
    
  return;
  
} /* append_peep */

/*}}}*/
/*{{{  peep_reg_transfer_to() */

/*
 * returns TRUE if the last instruction in the peepholer
 * is a register based LDI whoes destination is the register
 * given
 */

bool
peep_reg_transfer_to( RealRegister r )
{
  int32		op;
  

  op = peep_buf[ previous_( peep_end ) ].op_code;
  
  if (peep_end != peep_start            &&	/* if there are any instructions in the peepholer */
      is_op( op, OP_LDI )               &&	/* and the last instruction is a load instruction */
      is_mode( op, ADDR_MODE_REGISTER ) &&	/* and it is a transfer between two registers     */
      dest_of( op ) == hardware_register( r ) )	/* and the destination register is the given reg  */
    {
      return TRUE;
    }
  
  return FALSE;
  
} /* peep_reg_transfer_to */

/*}}}*/
/*{{{  peep_sets_status_reg() */

  
/*
 * returns TRUE if the last instruction in the peepholer
 * which sets the status register will set the condition
 * codes based on the contents of register 'r'
 */

bool
peep_sets_status_reg( RealRegister r )
{
  signed int	prev;


  if (peep_start == peep_end)
    return FALSE;					/* nothing in the peepholer */

  prev = peep_end;
  
  do
    {
      int32	writes;

      
      prev = previous_( prev );

      writes = peep_buf[ prev ].writes;
      
      if (writes & regbit( RR_ST ))			/* the instruction sets the status register	*/
	{
	  int32		op;

	  
	  if (writes & regbit( r ))			/* the instruction writes to register 'r'	*/
	    {
	      /*
	       * XXX - NC - 11/6/92
	       *
	       * The following is to catch a bug whereby a load instruction
	       * is detected as setting the status register, and so a
	       * compare instruction is not issued.  Then the load instruction
	       * is turned into a combined load/store instruction which does
	       * not set the ST register.
	       *   Strictly speaking, I should not turn a load followed by a
	       * store in a combined instruction, but this would prevent a
	       * useful optimisation.  Instead, I claim that instruction writes
	       * to a pseudo ST register, which the peepholer can detect when
	       * it wants to combine the two instructions.
	       */
	      
	      peep_buf[ prev ].writes |= regbit( RR_PST );
	      
	      return TRUE;
	    }	  

	  op = peep_buf[ prev ].op_code;
      
	  if (is_op(   op, OP_LDI ) 		    &&	/* or the instruction is a load			*/
	      is_mode( op, ADDR_MODE_REGISTER )     &&	/* and it is a register to register transfer	*/
	      (peep_buf[ prev ].reads & regbit( r )) )	/* and the instruction reads from register 'r'	*/
	    {
	      peep_buf[ prev ].writes |= regbit( RR_PST );
	      
	      return TRUE;
	    }	  

	  return FALSE;
	}
      else if (writes & regbit( r ))
	{
	  return FALSE;					/* the instruction alters 'r' but not ST	*/
	}
            
      /* continue until we run out of instructions */
    }
  while (prev != peep_start);

  return FALSE;
  
} /* peep_sets_status_reg */

/*}}}*/
/*{{{  peep_refs_label() */

  
/*
 * returns TRUE if any of the instructions in the peeholer
 * reference the given label.  Returns FALSE otherwise.
 */

bool
peep_refs_label( LabelNumber * l )
{
  signed int	i;


  for (i = peep_start; i != peep_end; i = successor_( i ))
    {
      if ((peep_buf[ i ].type == OUT_LABREF || peep_buf[ i ].type == OUT_DELLABREF) &&
	  peep_buf[ i ].label == l)
	{
	  return TRUE;
	}
    }

  return FALSE;
  
} /* peep_refs_label */

/*}}}*/
/*{{{  peep_eliminate_reg_transfer() */

  
/*
 * returns the source register of the register transfer
 * instruction that is the last instruction in the peepholer
 */

RealRegister
peep_eliminate_reg_transfer( RealRegister r )
{
  int32		op;
  

  if (xref_pending || fref_pending)
    {
      syserr( peep_elim_clash );
    }

  op = peep_buf[ previous_( peep_end ) ].op_code;
  
  if (peep_end != peep_start            &&
      is_op( op, OP_LDI )               &&
      is_mode( op, ADDR_MODE_REGISTER ) &&
      dest_of( op ) == hardware_register( r ) )
    {
      peep_end = previous_( peep_end );

      return real_register( source_of( op ) );
    }
  
  syserr( peep_non_existant_xfer );

  return GAP;
  
} /* peep_eliminate_reg_transfer */

/*}}}*/
/*{{{  flush_peepholer( DBG() */

  
/*
 * empty the peephole buffer
 */

void
flush_peepholer( DBG( const char * reason ) )
{
  flush_pending_pushes();
  
  if (xref_pending)
    {
      syserr( peep_cross_ref_pending );
    }

  if (fref_pending)
    {
      syserr( peep_fwd_ref_pending );
    }

  while (peep_start != peep_end)
    {
      emit_from_peep();      
    }

  DBG( peepf( "flushed: %s ", reason ); )

  if (num_in_peep_() > 0)
    syserr( peep_failed_reset_count );
  
  return;
  
} /* flush_peepholer */

/*}}}*/

/*}}}*/
/*{{{  Initialisation */

/*{{{  peep_init() */

void
peep_init( void )
{
  /*
   * NB/ This is necessary because J_LABEL is called before J_ENTER
   * and J_LABEL calls peep_corrupt_all_addr_regs() which does a
   * walklist() on the addr regs list.
   */

  init_list( &addr_regs );

  node_count = 0;
  
  init_list( &pending_pushes );
  
  init_list( &free_pending_pushes );
  
  return;

} /* peep_init */

/*}}}*/
/*{{{  peep_tidy() */

void
peep_tidy( void )
{
  return;

} /* peep_tidy */

/*}}}*/

/*}}}*/

/*}}}*/

/* end of peep.c */
