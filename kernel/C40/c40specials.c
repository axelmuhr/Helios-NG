/*
 * File:	c40specials.c
 * Subsystem:	c40 Helios executive
 * Author:	B. Veer
 * Date:	September 92
 *
 * Description: implements "special" links over and above the normal 6
 *		and provides a function to return the default hardware
 *		config word passed to bootstraps.
 *
 * RcsId: $Id: c40specials.c,v 1.9 1993/04/01 16:55:24 nick Exp $
 *
 * (C) Copyright 1992 Perihelion Software Ltd.
 * 
 */

/* Include Files: */

#include "../kernel.h"
#include <root.h>
#include <link.h>

/*{{{  DSP1 link specials */

#if 1
/* DOESN'T WORK DUE TO A BUG IN THE C40 C COMPILER GetExecRoot SUBSTITUTION */
/* i.e. GetExecRoot()->KernelRoot->Links doesn't work. */

/*
 * On the Hema DSP1 board a transputer link interface is used, with link
 * 3 of the C40 being an input from the transputer link and link 0 being
 * the output. The implementation is fairly trivial, all that is needed is
 * calling the executive's link tx/rx/abort functions again with the
 * real link number.
 */
static word DSP1_LinkRx(LinkTransferInfo *info)
{ word		 size	= info->size;
  MPtr		 buf	= info->buf;
  LinkInfo	*link3	= GetRoot()->Links[3];

  if (size == 0)
   return((word)AbortLinkRx(link3));
  else
   { MP_LinkRx(size, link3, buf);
     return(0);
   }
}

static word DSP1_LinkTx(LinkTransferInfo *info)
{ word		 size	= info->size;
  MPtr		 buf	= info->buf;
  LinkInfo	*link0	= GetRoot()->Links[0];

  if (size == 0)
   return((word)AbortLinkTx(link0));
  else
   { MP_LinkTx(size, link0, buf);
     return(0);
   }
}
#else
/* re-organise code to get around bug */
static word DSP1_LinkRx(LinkTransferInfo *info)
{
	RootStruct	*r = GetRoot();

	if (info->size == 0)
		return((word)AbortLinkRx(r->Links[3]));
	else {
		word	size	= info->size;
		MPtr	buf	= info->buf;

		MP_LinkRx(size, r->Links[3], buf);
		return(0);
	}
}

static word DSP1_LinkTx(LinkTransferInfo *info)
{
	RootStruct	*r = GetRoot();

	if (info->size == 0)
		return((word)AbortLinkTx(r->Links[0]));
	else {
		word	size	= info->size;
		MPtr	buf	= info->buf;

		MP_LinkTx(size, r->Links[0], buf);
		return(0);
	}
}
#endif
/*}}}*/
/*{{{  Shared Memory Links */

extern void InitSMLLink(LinkInfo *link);

/*}}}*/

void LinkInitSpecial(LinkInfo *link)
{

  if( (link->State&Link_State_Mask) == Link_State_SML ) InitSMLLink(link);
  else switch(link->State)
   { case Link_State_DSP1 :
		link->TxFunction	= (WordFnPtr) &DSP1_LinkTx;
		link->RxFunction	= (WordFnPtr) &DSP1_LinkRx;
		break;

	/* If the link type is not recognised, set the link to unusable */
     default:	link->Mode	= Link_Mode_Null;
		link->State	= Link_State_Null;
		return;
   }

	/* If the special link type is known, change the link mode and	*/
	/* state from special to running. Flags should have been set	*/
	/* already.							*/
  link->Mode	= Link_Mode_Intelligent;
  link->State	= Link_State_Running;
}


/* Returns the hardware config word that was sent to our bootstrap.
 * This is used by the networking software as the default hardware config
 * word when booting other processors.
 */
word GetHWConfig(void) {
	return GetExecRoot()->HWConfig;
}
