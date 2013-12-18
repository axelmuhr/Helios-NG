/*------------------------------------------------------------------------
--                                                                      --
--                   H E L I O S   I / O   S E R V E R                  --
--                   ---------------------------------                  --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
--      linkio.c                                                        --
--                                                                      --
--               Portable link I/O code                                 --
--                                                                      --
--  Author:  BLV 5/2/90                                                 --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: linkio.c,v 1.12 1994/07/06 10:44:59 mgun Exp $ */
/* Copyright (C) 1988, Perihelion Software Ltd.    			*/

#include "helios.h"

#if Use_linkio

/**
*** This code implements suitable link I/O routines for most hardware.
*** It is implemented in terms of a small number of machine-dependent
*** routines, as follows:
***
*** void resetlnk(void) : this is an initialisation routine. It may be
*** called many times, between every reboot for example, just in case
*** some special hardware needs resetting every time. Normally it sets
*** a static flag during the first call, to avoid wasting time. Its
*** main job is to initialise function pointers for the routines below.
***
*** int rdrdy(void) : does the current link have a byte waiting ? Returns
*** true or false
***
*** int wrrdy(void) : is the current link ready to receive a byte ? Returns
*** true or false. If the hardware does not support this request return
*** true, on the assumption that the hardware is always ready.
***
*** int byte_to_link(int value) : send a single byte down the link adapter.
*** Return 1 for failure, i.e. a timeout of typically 500Msec, 0 for
*** success. This return value indicates the amount of data failed to send
*** down the link. The timeout should not be much smaller than 500Msec, and no
*** larger than 5 seconds.
***
*** int byte_from_link(char *) : read a single byte from the link, putting
*** it at the specified address. Return 1 for failure, i.e. a timeout of
*** about 500Msec, 0 for success, i.e. the amount of data which did not
*** arrive in time.
***
*** int fetch_block(int count, char *data, int timeout) : fetch count bytes
*** of data from the link, storing the data in the specified buffer. The
*** timeout is a fairly arbitrary number, 1 indicates a millisecond or so,
*** whereas 30000 indicates a second. The timeouts can be ignored completely
*** if desired. The routine returns the amount of data which could not be
*** read in time, which means that 0 indicates complete success.
***
*** int send_block(int count, char *data, int timeout) : this is similar to
*** fetch_block() but in the opposite direction.
**/

#if !SOLARIS

int  (*rdrdy_fn)();
int  (*wrrdy_fn)();
int  (*byte_to_link_fn)();
int  (*byte_from_link_fn)();
int  (*send_block_fn)();
int  (*fetch_block_fn)();
void (*reset_fn)();
void (*analyse_fn)();

#else

/*
 * typedefs defined in structs.h
 */
WordFnPtr  		rdrdy_fn;
WordFnPtr  		wrrdy_fn;
WordIntFnPtr		byte_to_link_fn;
WordUbyteFnPtr  	byte_from_link_fn;
WordIntplusFnPtr  	send_block_fn;
WordIntplusFnPtr  	fetch_block_fn;
VoidFnPtr	  	reset_fn;
VoidFnPtr	  	analyse_fn;

#endif  /* !SOLARIS */

#define rdrdy             (*rdrdy_fn)
#define wrrdy             (*wrrdy_fn)
#define byte_to_link      (*byte_to_link_fn)
#define byte_from_link    (*byte_from_link_fn)
#define send_block        (*send_block_fn)
#define fetch_block       (*fetch_block_fn)
#define reset_processor   (*reset_fn)
#define analyse_processor (*analyse_fn)

word fn( xprdint, (void));
word fn( xpwrint, (word data));

/**
*** PutMsg are local versions of the Helios message-passing primitives. They
*** use the same data structures as on the transputer, but NB they return
*** different values!
***
*** PutMsg() is used to send Helios messages to the transputer. It just sends
*** the data block by block, starting with a byte 2 which indicates a Helios
*** message coming across a link. Note that this module is designed to run
*** on an 8086 which has the same byte-ordering as the transputer. On a
*** 68000 you have to swap the words in the message header and in the control
*** vector, and you have to take particular care about the first word in the
*** header which contains packed bytes and an integer. The header file
*** IOhelios.h contains two versions of the message header. There are two
*** versions of PutMsg() : one for Unix boxes tries to minimise the number
*** of link transfers, because of the process switching overheads when
*** interacting with device drivers; the other does things the boring way.
***
*** GetMsg() is used to check for data coming down the link. There are three
*** types of data I have to support. First, and most common, there is a
*** Helios message which is always introduced by a single byte 2. Second
*** there are poke and peek requests, introduced by a byte 0 or 1. These are
*** extremely important because they are used by (I think) the network server
*** to decide the state of each node and whether or not to reset and reboot
*** each node. The peeks and pokes always refer to the same address, so that
*** can be ignored. The value peeked must be the inverted form of the last
*** value poked, as an indication that the node is alive and well.
*** The final form of message is a raw (occam) message introduced by a byte
*** F0, and this type of message must be handled at a higher level, i.e. in
*** module tload.c, by returning F0.
**/

#if (UNIX && !MEIKORTE)
     /* This version is optimised to use a minimum number of transfers */
word fast_PutMsg(mcb)
MCB *mcb;
{ word          amount;
  PRIVATE BYTE  big_buf[4096];
  BYTE         *buf_ptr;
  
	/* Protocol byte/word	*/
  if (target_processor eq Processor_C40)
   { *((int *)big_buf) = swap(0x02);
     buf_ptr = big_buf;
   }
  else 
   { big_buf[3] = 2;
     buf_ptr = &(big_buf[3]);
   }

   	/* Message header and control vector	*/
#if swapping_needed
  { register word *p, *p2, i;
    p2 = (word *) &(big_buf[4]); p = (word *) &(mcb->MsgHdr);
    for (i = 0; i < 4; i++)
     *p2++ = swap(*p++);

    for (i = 0; i < mcb->MsgHdr.ContSize; i++)
     *p2++ = swap(mcb->Control[i]);
  }
#else
  memcpy(&(big_buf[4]), &(mcb->MsgHdr), 16);
  if (mcb->MsgHdr.ContSize > 0)
   memcpy(&(big_buf[20]), mcb->Control, 4 * mcb->MsgHdr.ContSize);
#endif

  if (target_processor eq Processor_C40)
   amount = 20 + (mcb->MsgHdr.ContSize * 4);
  else
   amount = 17 + (4 * mcb->MsgHdr.ContSize);

  if (mcb->MsgHdr.DataSize eq 0)
   { if (send_block(amount, buf_ptr, 1000))
      return(false);
     else
      return(true);
   }
 
  if ((word) mcb->MsgHdr.DataSize > maxdata)
   { output("***\r\n*** Serious : the server has overflowed its");
     output("message buffer\r\n***\r\n");
   }

  if (amount + mcb->MsgHdr.DataSize >= 4090)
   { if (send_block(amount, buf_ptr, 1000)) return(false);
     amount = mcb->MsgHdr.DataSize;
     if (target_processor eq Processor_C40)
      amount = (amount + 3) & ~3;
     if (send_block(amount, mcb->Data, 1000)) return(false);
     return(true);
   }

  if (target_processor eq Processor_C40)
   { memcpy(&(big_buf[amount]), mcb->Data, mcb->MsgHdr.DataSize);
     amount = (amount + mcb->MsgHdr.DataSize + 3) & ~3;
     if (send_block(amount, big_buf, 1000))
      return(false);
   }
  else
   { memcpy(&(big_buf[3 + amount]), mcb->Data, mcb->MsgHdr.DataSize);
     if (send_block(amount + mcb->MsgHdr.DataSize, &(big_buf[3]), 1000))
      return(false);
   }

  return(true);
}
     
#endif

word PutMsg(mcb)
MCB *mcb;
{ word i;
  bool ready = false;

#if (UNIX && !MEIKORTE)
	/* On Unix boxes it is usually better to use a fast version of PutMsg()	*/
	/* which copies most or all of the message into a single buffer and	*/
	/* transfers it with a single write. Memory copies within a process	*/
	/* address space are significantly faster than context switches when	*/
	/* calling into the system with write(). However, this will not work	*/
	/* on shared memory systems where reads and writes should be completely	*/
	/* synchronous.								*/
  if (target_processor ne Processor_C40)
   return(fast_PutMsg(mcb));
  else
   { static	int	shared_memory = -1;

     if (shared_memory eq -1)
     {	/* first time through	*/
       char *box = get_config("box");
       if (!mystrcmp(box, "vc40"))	/* or any other shared memory system	*/
        shared_memory = 1;
       else
        shared_memory = 0;
     }

     if (shared_memory eq 0)
      return(fast_PutMsg(mcb));
   }
#endif

  for (i=0L; (i < mcb->Timeout) && (!ready); i++)
   if (wrrdy())
    ready = true;
  unless(ready) return(false);

  if (target_processor eq Processor_C40)
   { if (!xpwrint(2)) return(false); }
  else
   { if (byte_to_link(2) ne 0) return(false); }

#if swapping_needed
  { word *x = (word *) mcb; int y;
    for (y = 0; y < 4; y++)
     if (!xpwrint(*x++)) return(false);
    x = mcb->Control;
    for (y = 0; y < mcb->MsgHdr.ContSize; y++)
     if (!xpwrint(*x++)) return(false);
  }
#else
  if(send_block(16, (byte *) mcb, 300)) return(false);
  if (mcb->MsgHdr.ContSize ne 0)
   if(send_block(mcb->MsgHdr.ContSize * 4, (byte *) mcb->Control, 300))
     return(false);
#endif

  if (mcb->MsgHdr.DataSize ne 0)
   { word size = (word) mcb->MsgHdr.DataSize;

     if (size > maxdata)
      { output("***\r\n*** Serious : the server has overflowed its ");
        output("message buffer\r\n***\r\n");
      }
     if (target_processor eq Processor_C40)
	size = (size + 3) & ~3L;	/* round up to word multiple */
       
     if(send_block((int) size, mcb->Data, 300))
      return(false);
   }

  return(true);
}

word GetMsg(mcb)
MCB *mcb;
{ unsigned char temp[1];
  word i, x;
  static word probe_value = 0L;

  for (i=0L; i < mcb->Timeout; i++)
    if (rdrdy()) break;
  unless(rdrdy()) return(false);

  temp[0] = 0;
  if (target_processor eq Processor_C40)
   { if (fetch_block(4, (byte *) &i, 100) > 0)
      { ServerDebug("Warning : failed to read protocol word");
        return(false);
      }
#if swapping_needed
     else
      i = swap(i);
#endif
   }
  else
   { if(byte_from_link(&(temp[0])))
      { ServerDebug("Warning : failed to read protocol byte");
        return(false);
      }
     i = temp[0];
   }

  if (i ne Proto_Msg)
   switch((unsigned char) i)
    { case Proto_Write : /* poke request */
                  (void) xprdint();
                  probe_value = xprdint();
                  return(false);

      case Proto_Read : /* peek request */
                  (void) xprdint();
                  xpwrint(~probe_value);
                  return(false);

      case Proto_Null : /* Null byte */
      case Proto_SecurityCheck :
      case Proto_Reset :
                  return(false);

      case Proto_Term : /* Terminate */
                  ServerDebug("*** The root processor has terminated.");
                  return(false);

      case Proto_Reconfigure : /* Reconfigure */
            	  ServerDebug("*** The link to the root processor has been reconfigured.");
                  return(false);

      case Proto_Info :        /* info request */
                  return(Proto_Info);

      case Proto_ReSync :	/* resyncronise request */
		/* Up to 64KB Proto_ReSync bytes sent by Helios to */
		/* resyncronise with I/O server. This only happens in */
		/* *Dire* emergencies. */
		return (false);

      default :        
                ServerDebug("*** Unknown link protocol prefix 0x%lx ('%c')", i, (i > 31 && i <127)? i : '?');
                return(false);
   }

  if (C40HalfDuplex)
   {	 /* Fix to stop half duplex link blocking. */
	 /* Signal C40 that we are ready to receive rest of msg. */
     if (target_processor eq Processor_C40) {
	if (xpwrint((word)Proto_Go) eq 0)
	 {	ServerDebug("Warning : failed to send prototype word (0x%lx)", Proto_Go);
		return(false);	
	 }
     } else {
      	if (xpwrbyte((word)Proto_Go) eq 0)
  	 {	ServerDebug("Warning : failed to send prototype byte (0x%lx)", Proto_Go);
 		return(false);	
         }
     }
   }

  if ((x = fetch_block(16, (byte *) mcb, 100)) > 0) /* get the message header */
   { ServerDebug("Warning : failed to read message header (%d)", x);
     return(false);
   }

#if swapping_needed
  { register word *x = (word *) mcb; int y;
    *x = swap(*x); x++;
    *x = swap(*x); x++;
    *x = swap(*x); x++;
    *x = swap(*x);
  }
#endif

  if (mcb->MsgHdr.ContSize ne 0)   /* get the control vector */
   if ((x = fetch_block(mcb->MsgHdr.ContSize * 4, (byte *) mcb->Control, 100)) > 0)
    { ServerDebug("Warning : failed to read control vector (%d)", x);
#if 0
      ServerDebug("Warning : fl %x, cs %x, ds %x, dp %lx, rp %lx, fnrc %lx",mcb->MsgHdr.Flags,mcb->MsgHdr.ContSize, mcb->MsgHdr.DataSize, mcb->MsgHdr.Dest, mcb->MsgHdr.Reply, mcb->MsgHdr.FnRc);
#endif
      return(false);
    }

#if swapping_needed
  { register word *x = mcb->Control; int y;
    for (y = 0; y < mcb->MsgHdr.ContSize; y++)
     { *x = swap(*x); x++; }
  }
#endif

  if (mcb->MsgHdr.DataSize ne 0)
   { word size = (word) mcb->MsgHdr.DataSize;
     word bytealign = (mcb->MsgHdr.Flags & MsgHdr_Flags_bytealign);

		/* round up to word multiple */
     if (target_processor eq Processor_C40)
      size = (bytealign + size + 3L) & ~3L;

     if (size > maxdata)
      { word x;
        unsigned char y[1];
        output("***\r\n*** Serious : the root processor sent an illegal ");
        output("amount of data.\r\n***\r\n");

	if (target_processor eq Processor_C40)
	 for (x = 0; x < size; x += 4)
	  xprdword();
	else
         { (void) fetch_block((int) maxdata, mcb->Data, 100);
 	    for (x = maxdata; x < (word) mcb->MsgHdr.DataSize; x++)
             (void) byte_from_link(&(y[0]));
	 }
        return(false);
      }

     if ((x = fetch_block((int) size, mcb->Data, 100)) > 0)
      { ServerDebug("Warning : failed to read data vector (%d) (bytealign %d)", x, bytealign);
        return(false);
      }
      
     if ((target_processor eq Processor_C40) && (bytealign ne 0))
      	/* There are some dummy bytes at the start of the message */
      	/* which should be discarded.				  */
	memmove(mcb->Data, &(mcb->Data[bytealign]), (unsigned int) (size - bytealign));
   }

  return(true);
}

/**
*** send a single byte down to link, returning true or false.
**/
word xpwrbyte(b)
word b;
{
  return( (word) (byte_to_link((int) b) eq 0));
}

/**
*** is the other side of the link waiting for data ?
**/
word xpwrrdy()
{ return((word) wrrdy());
}

/**
*** get a single byte from the link, returning the byte or 0
**/
word xprdbyte()
{ unsigned char temp[1];
  if (byte_from_link(&(temp[0])) eq 0)
    return((word) temp[0]);
  else
    return(false);
}

/**
*** is the other side of the link trying to send data ?
**/
word xprdrdy()
{ return((word) rdrdy());
}

/**
*** send a 4-byte word down the link without byte-swapping
**/
word xpwrword(d)
word d;
{ 
 return( (word) !send_block(4, (byte *) &d, 1));
}

/**
*** read a word from the link without byte-swapping
**/
word xprdword()
{ word temp;
  if (!fetch_block(4, (byte *) &temp, 10))
   return(temp);
  else
   return(false);
}

/**
*** send a word down the link, byteswapping as necessary so that the integer
*** has the same value on the transputer and locally.
**/
word xpwrint(d)
word d;
{ int i;

#if swapping_needed
  d = swap(d);
#endif

  i = send_block(4, (byte *) &d, 1);
  return( (word) !i);
}

/**
*** get a word down the link, byteswapping as necessary
**/
word xprdint()
{ word temp;
  if (!fetch_block(4, (byte *) &temp, 10))
   {
#if swapping_needed
     temp = swap(temp);
#endif
     return(temp);
   }
  else
    return(false);
}

/**
*** send a number of bytes of data down the link, without any swapping
*** If the data consists of integers they must be swapped before calling
*** this routine, directly or via PutMsg.
**/
word xpwrdata(buf, size)
byte *buf;
word size;
{ int i = send_block((int)size, buf, 10);
  return((word) !i);
}

/**
*** read a number of bytes from the link
**/
word xprddata(buf, size)
byte *buf;
word size;
{ return( (word) !fetch_block((int) size, buf, 10));
}

/**
*** This is meant to reset the root transputer, by fair means or foul.
**/
void xpreset()
{
  reset_processor();
  link_table[current_link].state = Link_Reset;
}

/**
*** This is meant to analyse the root transputer.
*** apply.
**/
void xpanalyse()
{ analyse_processor();
  link_table[current_link].state = Link_Reset;
}

/**
*** These are used mainly by the debugger to access the transputer's memory
*** by sending peek and poke requests.
**/
word dbwrword(address, data)
word address, data;
{ if(byte_to_link(0)) return(false);
#if swapping_needed
  address = swap(address);
#endif
  if(send_block(4, (byte *) &address, 1)) return(false);
  if(send_block(4, (byte *) &data, 1)) return(false);
  return(true);
}

word dbrdword(address)
word address;
{ word temp;
  if(byte_to_link(1)) return(false);
#if swapping_needed
  address = swap(address);
#endif
  if(send_block(4, (byte *)  &address, 1)) return(false);
  if(fetch_block(4, (byte *) &temp, 1)) return(false);
  return(temp);
}

word dbrdint(address)
word address;
{ word temp;
  if(byte_to_link(1)) return(false);
#if swapping_needed
  address = swap(address);
#endif
  if(send_block(4, (byte *) &address, 1)) return(false);
  if(fetch_block(4, (byte *)&temp, 1)) return(false);
#if swapping_needed
  temp = swap(temp);
#endif
  return(temp);
}

word dbwrint(address, data)
word address, data;
{ if(byte_to_link(0)) return(false);
#if swapping_needed
  address = swap(address); data = swap(data);
#endif
  if(send_block(4, (byte *) &address, 1)) return(false);
  if(send_block(4, (byte *) &data, 1)) return(false);
  return(true);													
}

#endif   /* Use_linkio */
 
