/*------------------------------------------------------------------------
--                                                                      --
--     H E L I O S   M E I K O R T E  L I N K  I / O   S Y S T E M      --
--     -----------------------------------------------------------      --
--                                                                      --
--             Copyright (C) 1989, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
--      linklib.c                                                       --
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: 3.8 28/3/90\ Copyright (C) 1989, Perihelion Software Ltd.        */


#define Linklib_Module
#include "../helios.h"

/**
*** The hardware-independent link IO routines.
**/


#define FAIL 1
#define SUCCESS 0

PRIVATE int  fn( byte_from_link,     (int *));
PRIVATE int  fn( byte_to_link,       (int));
PRIVATE int  fn( fetch_block,        (int, BYTE *, int));
PRIVATE int  fn( send_block,         (int, BYTE *, int));
PRIVATE int  fn( rdrdy,              (void));
PRIVATE int  fn( wrrdy,              (void));
PRIVATE void fn( reset_transputer,   (void));
PRIVATE void fn( analyse_transputer, (void));
PRIVATE void fn( init_link,          (void));

WORD fn( xprdint, (void));
WORD fn( xpwrint, (int));

WORD PutMsg(mcb)
MCB *mcb;
{ 
          /* protocol byte */
  if (byte_to_link(2) eq FAIL) return(FALSE);

          /* message header */
  if (send_block(16, (BYTE *) &(mcb->MsgHdr), 1000))
   return(FALSE);

          /* control vector */
  if (mcb->MsgHdr.ContSize ne 0)
   if (send_block(4 * mcb->MsgHdr.ContSize, (BYTE *) mcb->Control, 1000))
    return(FALSE);

          /* check for overflow */ 
  if ((word) mcb->MsgHdr.DataSize > maxdata)
   { output("***\r\n*** Serious : the server has overflowed its");
     output("message buffer\r\n***\r\n");
   }

          /* send the data vector */
  if (mcb->MsgHdr.DataSize ne 0)
   if (send_block(mcb->MsgHdr.DataSize, mcb->Data, 1000))
    return(FALSE);

  return(TRUE);
}


word GetMsg(mcb)
MCB *mcb;
{ int temp = 0;
  static word probe_value = 0L;

  if(byte_from_link(&temp) eq FAIL)
    return(FALSE);

  if (temp ne Proto_Msg)               /* first byte of message is always a 2 */
   { switch(temp)
      { case  Proto_Write :               /* transputer trying to probe link */
                  (void) fetch_block(4, (BYTE *) &temp, 1000);   /* address */
                  (void) fetch_block(4, (BYTE *) &probe_value, 1000);
                  probe_value = swap(probe_value);
                  return(0);

        case  Proto_Read :                /* transputer reading back probe */
                  (void) fetch_block(4, (BYTE *) &temp, 1000);  /* address */
                  (void) send_block(4, (BYTE *) &probe_value, 1000);
                  return(0);

        case  Proto_Null :               /* these should not happen */
        case  Proto_SecurityCheck :
        case  Proto_Reset :
                  return(0);

        case Proto_Term :                /* processor has terminated */
                  return(Proto_Term);

        case Proto_Reconfigure:          /* link has been reconfigured ??? */
                  return(Proto_Reconfigure);

        case Proto_Info :                /* info request between link guardians */
                  return(Proto_Info);

        default :
                  return(0);
      }
   }

  if (fetch_block(16, &(mcb->MsgHdr), 1000))
   { ServerDebug("Failed to receive message header");
     return(FALSE);
   }


  if (mcb->MsgHdr.ContSize ne 0)
   if(fetch_block(mcb->MsgHdr.ContSize * 4, mcb->Control, 1000))
    { ServerDebug("Failed to receive control vector");
      return(FALSE);
    }


  if (mcb->MsgHdr.DataSize ne 0)
   { if ((word)mcb->MsgHdr.DataSize > maxdata)
      { WORD x, y;
        output("***\r\n*** Serious : the transputer sent an illegal");
        output("amount of data.\r\n***\r\n");
        (void) fetch_block(maxdata, mcb->Data, 1000);
                for (x = maxdata; x < (word)mcb->MsgHdr.DataSize; x++)
         (void) byte_from_link(&y);
        return(FALSE);
      }
 
    if(fetch_block(mcb->MsgHdr.DataSize, mcb->Data, 1000))
      return(FALSE);
   }
  return(TRUE);
}

word xpwrbyte(b)
word b;
{ return( (word) !byte_to_link((int) b) );
}

word xpwrrdy()
{ return((word) !wrrdy());
}

word xprdbyte()
{ int temp=0;
  if (!byte_from_link(&temp))
    return((word) temp);
  else
    return(FALSE);
}

word xprdrdy()
{ return((word) !rdrdy());
}

word xpwrword(d)
word d;
{
  return( (word) !send_block(4, &d, 1000));
}

word xprdword()
{ word temp;
  if (!fetch_block(4, &temp, 1000))
   return(temp);
  else
   return(FALSE);
}

word xpwrint(d)                 /* this is byteswapped */
word d;
{
#if swapping_needed
  d=swap(d);
#endif
  return( (word) !send_block(4, &d, 1000));
}

word xprdint()
{ word temp;
  if (fetch_block(4, &temp, 1000))
    return(FALSE);
  return(swap(temp));
}

word xpwrdata(buf, size)
byte *buf;
word size;
{ 
  return((word) !send_block((int) size, buf, 20000));
}

word xprddata(buf, size)
byte *buf;
word size;
{ return( (word) !fetch_block((int) size, buf, 1000));
}

void resetlnk()
{
  init_link();
}

void xpreset()
{ 
  reset_transputer();
}

void xpanalyse()
{
  analyse_transputer();
}

word dbwrword(address, data)
word address, data;
{ 
  address = swap(address); 
  if(byte_to_link(0)) return(FALSE);
  if(send_block(4, &address, 1000)) return(FALSE);
  if(send_block(4, &data, 1000)) return(FALSE);
  return(TRUE);
}

word dbrdword(address)
word address;
{ word temp;
  address = swap(address);
  if(byte_to_link(1)) return(FALSE);
  if(send_block(4, &address, 1000)) return(FALSE);
  if(fetch_block(4, &temp, 1000)) return(FALSE);
  return(temp);
}

word dbrdint(address)
word address;
{ word temp;
  address = swap(address);
  if(byte_to_link(1)) return(FALSE);
  if(send_block(4, &address, 1000)) return(FALSE);
  if(fetch_block(4, &temp, 1000)) return(FALSE);
    return(swap(temp));
}

word dbwrint(address, data)
word address, data;
{ address = swap(address);
  data = swap(data);
  if(byte_to_link(0)) return(FALSE);
  if(send_block(4, &address, 1000)) return(FALSE);
  if(send_block(4, &data, 1000)) return(FALSE);
  return(TRUE);
}

/**
*** RTE : these have to be written. Once you get them working the code
*** should be incorporated into the above routines for efficiency, and
*** these routines should be scrapped.
**/

PRIVATE CHAN *in_chan ,*out_chan;
PRIVATE int root_proc;

PRIVATE WORD byte_from_link(addr)
int *addr;
{ BYTE junk[1];

  if (cread(in_chan, (void *) &(junk[0]), 1) eq 0)
   { *addr = junk[0];
     return(SUCCESS);
   } 
 return(FAIL);
}

PRIVATE WORD byte_to_link(value)
int value;
{ BYTE junk[1];

  junk[0] = (BYTE) value;
  if (cwrite(out_chan, (void *) &(junk[0]), 1) eq 0)
   return(SUCCESS);
  return(FAIL);
}

PRIVATE WORD send_block(amount, buf, timeout)
int  amount;
BYTE *buf;
int  timeout;
{ if (cwrite(out_chan, (void *) buf, amount) eq 0)
   return(SUCCESS);
  return(FAIL);
}

PRIVATE WORD fetch_block(amount, buf, timeout)
int  amount;
BYTE *buf;
int  timeout;
{ if (cread(in_chan, (void *) buf, amount) eq 0)
   return(SUCCESS);
  return(FAIL);
}

PRIVATE WORD rdrdy()
{ int guards[1];
  guards[0] = 1;
  
  if (timalt(1, 1, in_chan, guards) eq 0)
   return(SUCCESS);
   
  return(FAIL);
}

PRIVATE WORD wrrdy()
{ return(SUCCESS);
}

PRIVATE void init_link()
{ WORD link_to_use = get_int_config("rte_link");
  WORD first_proc  = get_int_config("root_processor_id");
  
  if (link_to_use eq Invalid_config) link_to_use = 0;
  if (first_proc eq Invalid_config)
   root_proc = 1;
  else
   root_proc = (int) first_proc;
   
  switch(link_to_use)
   { case 0 : in_chan = LINK0IN; out_chan = LINK0OUT; break;
     case 1 : in_chan = LINK1IN; out_chan = LINK1OUT; break;
     case 2 : in_chan = LINK2IN; out_chan = LINK2OUT; break;
     case 3 : in_chan = LINK3IN; out_chan = LINK3OUT; break;
     default : printf("Invalid link id %d\r\n", link_to_use);
               longjmp(exit_jmpbuf, 1);
   }
    
   *in_chan = MINT; *out_chan = MINT;
}

PRIVATE void reset_transputer()
{ superReset(root_proc);
}

PRIVATE void analyse_transputer()
{ reset_transputer();
}

void tidy_link()
{
}

