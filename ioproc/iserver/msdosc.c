
/*
 --   ---------------------------------------------------------------------------
 --
 --      ISERVER  -  INMOS standard file server
 --
 --      msdosc.c
 --
 --      MS-DOS specific extensions providing PC AFserver compatability
 --
 --      Copyright (c) INMOS Ltd., 1988.
 --      All Rights Reserved.
 --
 --   ---------------------------------------------------------------------------
*/



#include <stdio.h>
#include <dos.h>

#include "inmos.h"
#include "iserver.h"

 
EXTERN BOOL CocoPops;							   /*  for DEBUG  */
EXTERN BOOL VerboseSwitch;

EXTERN BYTE Tbuf[TRANSACTION_BUFFER_SIZE];




PRIVATE VOID DosUnknown()
{
   int PacketLength;
   
   INFO(("[Encountered unknown msdos command tag (%d)]\n", Tbuf[3]));
   Tbuf[2] = SP_UNIMPLEMENTED;
   PacketLength = 1;
   Tbuf[0] = PacketLength % 256; Tbuf[1] = PacketLength >> 8;
}


PRIVATE VOID DosSendBlock()
{
   int PacketLength;
   unsigned long int Address;
   register char huge *Ptr;    
   unsigned int Length;          
   register unsigned int i;          
   unsigned char *c;
   
   DEBUG(("DOS.SENDBLOCK",0));
   c = (unsigned char *)&Address; *c++ = Tbuf[4]; *c++ = Tbuf[5]; *c++ = Tbuf[6]; *c = Tbuf[7];
   Ptr = (char huge *)Address;
   c = (unsigned char *)&Length; *c++ = Tbuf[8]; *c = Tbuf[9];

   for ( i=0; i < Length; i++)
      *Ptr++ = Tbuf[10+i];

   Tbuf[2] = SP_SUCCESS;
   Tbuf[3] = Tbuf[8]; Tbuf[4] = Tbuf[9];
   PacketLength = 3; Tbuf[0] = PacketLength % 256; Tbuf[1] = PacketLength >> 8;
}


PRIVATE VOID DosReceiveBlock()
{
   int PacketLength;
   unsigned long int Address;
   register char huge *Ptr;
   unsigned int Length;
   unsigned register int i;                   
   unsigned char *c;

   DEBUG(("DOS.RECEIVEBLOCK",0));
   c =(unsigned char *) &Address; *c++ = Tbuf[4]; *c++ = Tbuf[5]; *c++ = Tbuf[6]; *c = Tbuf[7];
   Ptr = (char huge *)Address;
   c = (unsigned char *)&Length; *c++ = Tbuf[8]; *c = Tbuf[9];
   Tbuf[2] = SP_SUCCESS;
   Tbuf[3] = Tbuf[8]; 
   Tbuf[4] = Tbuf[9];    

   for (i = 0 ; i < Length; i++)
      Tbuf[5+i] = *Ptr++;

   PacketLength = 3+Length; Tbuf[0] = PacketLength % 256; Tbuf[1] = PacketLength >> 8;
}




PRIVATE VOID DosCallInterrupt()
{
   int PacketLength;
   int Interrupt;
   int RegisterValue;
   unsigned char *c;
   int i;
   struct SREGS _SegRegs, *SegRegs = &_SegRegs;
   union REGS _InRegs, *InRegs = &_InRegs, _OutRegs, *OutRegs = &_OutRegs;
    
  
   DEBUG(("DOS.CALLINTERRUPT",0));
   c = (unsigned char *)&Interrupt; *c++ = Tbuf[4]; *c = Tbuf[5];

   c = (unsigned char *)&RegisterValue ; *c++ = Tbuf[ 6] ; *c++ = Tbuf[ 7];
   InRegs->x.ax = RegisterValue;
   c = (unsigned char *)&RegisterValue ; *c++ = Tbuf[10] ; *c++ = Tbuf[11];
   InRegs->x.bx = RegisterValue;
   c = (unsigned char *)&RegisterValue ; *c++ = Tbuf[14] ; *c++ = Tbuf[15];
   InRegs->x.cx = RegisterValue;
   c = (unsigned char *)&RegisterValue ; *c++ = Tbuf[18] ; *c++ = Tbuf[19];
   InRegs->x.dx = RegisterValue;
   c = (unsigned char *)&RegisterValue ; *c++ = Tbuf[22] ; *c++ = Tbuf[23];
   InRegs->x.di = RegisterValue;
   c = (unsigned char *)&RegisterValue ; *c++ = Tbuf[26] ; *c++ = Tbuf[27];
   InRegs->x.si = RegisterValue;
   c = (unsigned char *)&RegisterValue ; *c++ = Tbuf[30] ; *c++ = Tbuf[31];
   SegRegs->cs = RegisterValue;
   c = (unsigned char *)&RegisterValue ; *c++ = Tbuf[34] ; *c++ = Tbuf[35];
   SegRegs->ds = RegisterValue;
   c = (unsigned char *)&RegisterValue ; *c++ = Tbuf[38] ; *c++ = Tbuf[39];
   SegRegs->es = RegisterValue;
   c = (unsigned char *)&RegisterValue ; *c++ = Tbuf[42] ; *c++ = Tbuf[43];
   SegRegs->ss = RegisterValue;
            
   int86x( Interrupt, InRegs, OutRegs, SegRegs );
      
   Tbuf[2] = SP_SUCCESS;
   Tbuf[3] = OutRegs->x.cflag;
   for ( i=6; i < 44; i++ )
      Tbuf[i] = 0;
      
   Tbuf[ 4] = OutRegs->x.ax % 256; Tbuf[ 5] = OutRegs->x.ax >> 8;
   Tbuf[ 8] = OutRegs->x.bx % 256; Tbuf[ 9] = OutRegs->x.bx >> 8;
   Tbuf[12] = OutRegs->x.cx % 256; Tbuf[13] = OutRegs->x.cx >> 8;
   Tbuf[16] = OutRegs->x.dx % 256; Tbuf[17] = OutRegs->x.dx >> 8;
   Tbuf[20] = OutRegs->x.di % 256; Tbuf[21] = OutRegs->x.di >> 8;
   Tbuf[24] = OutRegs->x.si % 256; Tbuf[25] = OutRegs->x.si >> 8;
   Tbuf[28] = SegRegs->cs % 256; Tbuf[29] = SegRegs->cs >> 8;
   Tbuf[32] = SegRegs->ds % 256; Tbuf[33] = SegRegs->ds >> 8;
   Tbuf[36] = SegRegs->es % 256; Tbuf[37] = SegRegs->es >> 8;
   Tbuf[40] = SegRegs->ss % 256; Tbuf[41] = SegRegs->ss >> 8;
      
   PacketLength = 42; Tbuf[0] = PacketLength % 256; Tbuf[1] = PacketLength >> 8;
}




PRIVATE VOID DosReadRegisters()
{
   int PacketLength;
   int i;
   struct SREGS _SegRegs, *SegRegs = &_SegRegs;
   
   DEBUG(("DOS.READREGISTERS",0));
   segread( SegRegs );
   Tbuf[2] = SP_SUCCESS;
   for ( i=5; i < 19; i++ )
      Tbuf[i] = 0;
   Tbuf[ 3] = SegRegs->cs % 256; Tbuf[ 4] = SegRegs->cs >> 8;
   Tbuf[ 7] = SegRegs->ds % 256; Tbuf[ 8] = SegRegs->ds >> 8;
   Tbuf[11] = SegRegs->es % 256; Tbuf[12] = SegRegs->es >> 8;
   Tbuf[15] = SegRegs->ss % 256; Tbuf[16] = SegRegs->ss >> 8;
   PacketLength = 17; Tbuf[0] = PacketLength % 256; Tbuf[1] = PacketLength >> 8;
}




PRIVATE VOID DosPortWrite()
{
   int PacketLength;
   int Port;
   BYTE Value;
   char *c;
   
   DEBUG(("DOS.PORTWRITE",0));
   c = (unsigned char *)&Port; *c++ = Tbuf[4]; *c = Tbuf[5];
   Value = Tbuf[6];
   outp( Port, Value );
   Tbuf[2] = SP_SUCCESS;
   PacketLength = 1;
   Tbuf[0] = PacketLength % 256; Tbuf[1] = PacketLength >> 8;
}




PRIVATE VOID DosPortRead()
{
   int PacketLength;
   int Port;
   BYTE Value;
   char *c;
   
   DEBUG(("DOS.PORTREAD",0));
   c = (unsigned char *)&Port; *c++ = Tbuf[4]; *c = Tbuf[5];
   Value = inp( Port );
   Tbuf[2] = SP_SUCCESS;
   Tbuf[3] = Value;
   PacketLength = 2; Tbuf[0] = PacketLength % 256; Tbuf[1] = PacketLength >> 8;
}




PUBLIC VOID SpMsdos()
{
   DEBUG(("Oh no, not an msdos tag...",0));
   switch ( (int)Tbuf[3] )
      {
         case 0 : DosSendBlock(); break;
         case 1 : DosReceiveBlock(); break;
         case 2 : DosCallInterrupt(); break;
         case 3 : DosReadRegisters(); break;
         case 4 : DosPortWrite(); break;
         case 5 : DosPortRead(); break;
         default: DosUnknown();
      }
}



/*
 *  Eof
 */

