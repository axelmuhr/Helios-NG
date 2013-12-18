/*> ARMshape.c <*/
/*---------------------------------------------------------------------------*/
/* Provide ARM disassembler.
 * Note: The "disassembly" function prints direct to "stdout".
 */
/*---------------------------------------------------------------------------*/
/* RcsId: $Id: ARMshape.c,v 1.5 1993/08/11 17:01:34 nickc Exp $ */

#include <stdio.h>
#include <string.h>

#include "ARMshape.h"

/*---------------------------------------------------------------------------*/

int pad(int tab,int col)
{
 while (col < tab)
  {
   putchar(' ') ;
   col++ ;
  }
 return(col) ;
}

/*---------------------------------------------------------------------------*/

static int dreg(int regnum)
{
	if (pcsregs) {
		switch (regnum)	{
			case 0x00 : return(printf("a1")) ;
			case 0x01 : return(printf("a2")) ;
			case 0x02 : return(printf("a3")) ;
			case 0x03 : return(printf("a4")) ;
			case 0x04 : return(printf("v1")) ;
			case 0x05 : return(printf("v2")) ;
			case 0x06 : return(printf("v3")) ;
			case 0x07 : return(printf("v4")) ;
			case 0x08 : return(printf("v5")) ;
			case 0x09 : return(printf("mt")) ;
			case 0x0A : return(printf("use")) ;
			case 0x0B : return(printf("fp")) ;
			case 0x0C : return(printf("tmp")) ;
			case 0x0D : return(printf("usp")) ;
			case 0x0E : return(printf("lr")) ;
			case 0x0F : return(printf("pc")) ;
		}
	} else {
		switch (regnum)	{
			case 0x00 : return(printf("r0")) ;
			case 0x01 : return(printf("r1")) ;
			case 0x02 : return(printf("r2")) ;
			case 0x03 : return(printf("r3")) ;
			case 0x04 : return(printf("r4")) ;
			case 0x05 : return(printf("r5")) ;
			case 0x06 : return(printf("r6")) ;
			case 0x07 : return(printf("r7")) ;
			case 0x08 : return(printf("r8")) ;
			case 0x09 : return(printf("r9")) ;
			case 0x0A : return(printf("r10")) ;
			case 0x0B : return(printf("r11")) ;
			case 0x0C : return(printf("r12")) ;
			case 0x0D : return(printf("r13")) ;
			case 0x0E : return(printf("r14")) ;
			case 0x0F : return(printf("pc")) ;
		}
	}

	return(printf("r%d",regnum)) ;
}

/*---------------------------------------------------------------------------*/

static int dprocins(uint instruction,char *cname)
{
 int   opcode = ((instruction >> dp_shift) & 0xF) ;
 char *op = (3 * opcode) + "ANDEORSUBRSBADDADCSBCRSCTSTTEQCMPCMNORRMOVBICMVN" ;
 int   iscmp = ((opcode >= dp_tst) && (opcode <= dp_cmn)) ;
 int   width = printf("%.3s%.2s",op,cname) ;
 width += printf("%s",((instruction & scc) ? "S" : "")) ;
 width += printf("%s",((iscmp && (Rd(instruction) == 0xF)) ? "P" : "")) ;
 width = pad(8,width) ;

 if (!iscmp)
  width += dreg(Rd(instruction)) + printf(",") ;

 if ((((instruction & dp_mask) >> dp_shift) != dp_mov) && (((instruction & dp_mask) >> dp_shift) != dp_mvn))
  width += dreg(Rn(instruction)) + printf(",") ;

 return(width) ;
}

/*---------------------------------------------------------------------------*/

static int decodeRm(uint instruction)
{
 int imm = ((instruction & simm_mask) >> 0x7) ;
 int width = dreg(Rm(instruction)) ;

 switch ((instruction >> 4) & 0x7)
  {
   case 0 : if (imm != 0)
             width += printf(" LSL %d",imm) ;
            return(width) ;

   case 2 : width += printf(" LSR %d",((imm == 0) ? 32 : imm)) ;
            return(width) ;

   case 4 : width += printf(" ASR %d",((imm == 0) ? 32 : imm)) ;
            return(width) ;

   case 6 : if (imm == 0)
             width += printf(" RRX") ;
            else
             width += printf(" ROR %d",imm) ;
            return(width) ;

   case 1 : width += printf(" LSL ") ;
            break ;

   case 3 : width += printf(" LSR ") ;
            break ;

   case 5 : width += printf(" ASR ") ;
            break ;

   case 7 : width += printf(" ROR ") ;
            break ;
  }
 return(width + dreg(imm >> 1)) ;
}

static int DTDecodeRm(uint instruction)
{
 int imm = ((instruction & simm_mask) >> 0x7) ;
 int width = dreg(Rm(instruction)) ;

 switch ((instruction >> 5) & 0x3)
  {
   case 0 : if (imm != 0)
             width += printf(" LSL %d",imm) ;
            return(width) ;

   case 2 : width += printf(" LSR %d",((imm == 0) ? 32 : imm)) ;
            return(width) ;

   case 4 : width += printf(" ASR %d",((imm == 0) ? 32 : imm)) ;
            return(width) ;

   case 6 : if (imm == 0)
             width += printf(" RRX") ;
            else
             width += printf(" ROR %d",imm) ;
            return(width) ;
   }

  return 0;
}

/*---------------------------------------------------------------------------*/

static int decodeldrstr(uint instruction,char *cname,int postp)
{
 int width = printf("%.3s",((instruction & lsb) ? "LDR" : "STR")) ;
 width += printf("%.2s%s",cname,((instruction & bwb) ? "B" : "")) ;
 width += printf("%s",((postp && (instruction & wbb)) ? "T" : "")) ;
 width = pad(8,width) ;
 width += dreg(Rd(instruction)) + printf(",[") ;
 return(width + dreg(Rn(instruction))) ;
}

/*---------------------------------------------------------------------------*/

static int decodeldcstc(uint instruction,char *cname)
{
 int width = printf("%.3s",((instruction & lsb) ? "LDC" : "STC")) ;
 width += printf("%.2s%s",cname,((instruction & bwb) ? "L" : "")) ;
 width = pad(8,width) ;
 width += printf("%d",cpnum(instruction)) ;
 width += printf(",cr%d,[",Rd(instruction)) ;
 return(width + dreg(Rn(instruction))) ;
}

/*---------------------------------------------------------------------------*/
/* disassemble:
 *
 * Output format:
 * "aaaaaaaa : cccc : iiiiiiii : dddddddd"
 *           aaaaaaaa -> 32bit address
 *           cccc     -> ASCII representation (lo-byte first)
 *           iiiiiiii -> 32bit instruction value
 *           dddddddd -> disassembly
 *
 * Options:
 *      C register naming       : ARM register naming
 *      ASCII and HEX displayed : No ASCII or HEX displayed
 */

void disassemble(uint instruction,uint address,int useR15)
{
 char *cname = "EQNECSCCMIPLVSVCHILSGELTGTLE\0\0NV"+((instruction>>27)&0x1E) ;
 int   localc = 0 ;

 switch (instruction & op_mask)
  {
   case op_regxreg1 :
   case op_regxreg2 : {
                       if ((instruction & 0x00000090) == 0x00000090)
                        {
                         /* special instruction */
                         switch (instruction & 0x0F0000F0)
                          {
                           case 0x00000090 : if ((instruction & 0x0FC00000) == 0x00000000)
                                              {
                                               localc += printf("%3s",((instruction & 0x00200000) ? "MLA" : "MUL")) ;
                                               localc += printf("%.2s%s",cname,((instruction & scc) ? "S" : "")) ;
                                               localc = pad(8,localc) ;
                                               localc += dreg(Rn(instruction)) + printf(",") ;
                                               localc += dreg(Rm(instruction)) + printf(",") ;
                                               localc += dreg(Rs(instruction)) ;
                                               if (instruction & 0x00200000)
                                                localc += printf(",") + dreg(Rd(instruction)) ;
                                              }
                                             else
                                              {
                                               localc += printf("???%.2s",cname) ;
                                               localc = pad(8,localc) ;
                                               localc += printf("<Mul> 0x%08X",instruction) ;
                                              }
                                             break ;
                           case 0x01000090 : if ((instruction & 0x00B00F00) == 0x00000000)
                                              {
                                               localc += printf("SWP%.2s%s",cname,((instruction & bwb) ? "B" : "")) ;
                                               localc = pad(8,localc) ;
                                               localc += dreg(Rd(instruction)) + printf(",") ;
                                               localc += dreg(Rm(instruction)) + printf(",[") ;
                                               localc += dreg(Rn(instruction)) + printf("]") ;
                                              }
                                             else
                                              {
#if 1
                                               localc += printf("<SWP 0x%08X>",instruction) ;
#else
                                               localc += printf("???%.2s",cname) ;
                                               localc = pad(8,localc) ;
                                               localc += printf("<Sem> 0x%08X",instruction) ;
#endif
                                              }
                                             break ;
                           default         : {
#if 1
                                              localc += printf("<UNDEFINED 0x%08X>",instruction) ;
#else
                                              localc += printf("???%.2s",cname) ;
                                              localc = pad(8,localc) ;
                                              localc += printf("<Special> 0x%08X",instruction) ;
#endif
                                             }
                          }
                        }
                       else
                        {
                         /* data-processing instruction */
                         localc = dprocins(instruction,cname) ;
                         localc += decodeRm(instruction) ;
                        }
                      }
                      break ;

   case op_regximm1 :
   case op_regximm2 : {
                       /* data processing instruction */
                       localc = dprocins(instruction,cname) ;
                       {
                        int shift = ((instruction & imm_shiftmask) >> imm_shiftshift) ;
                        int value = (instruction & imm_valuemask) ;
                        value = ((value >> shift) | (value << (32 - shift))) ;
                        localc += printf("0x%08X",value) ;
                       }
                      }
                      break ;

   case op_postimm  : {
                       localc = decodeldrstr(instruction,cname,1) ;
                       localc += printf("],") ;
                       if ((instruction & udb) == 0)
                        localc += printf("-") ;
                       localc += printf("%d",(instruction & ls_offset_mask)) ;

                       /* we should be able to decode the referenced address */
                       if (useR15 && (Rn(instruction) == 15))
                        {
                       	 int value = (instruction & ls_offset_mask) ;
                       	 value *= ((instruction & udb) ? 1 : -1) ;
                         localc += printf(" ; ") ;
                       	 localc += ((instruction & lsb) ? printf("from 0x") : printf("to 0x")) ;
                       	 localc += printf("%08X",(address + 8 + value)) ;
                        }

                      }
                      break ;

   case op_preimm   : {
                       localc = decodeldrstr(instruction,cname,0) ;
                       localc += printf(",") ;
                       if ((instruction & udb) == 0)
                        localc += printf("-") ;
                       localc += printf("%d]",(instruction & ls_offset_mask)) ;
                       if (instruction & wbb)
                        localc += printf("!") ;

                       /* we should be able to decode the referenced address */
                       if (useR15 && (Rn(instruction) == 15))
                        {
                       	 int value = (instruction & ls_offset_mask) ;
                       	 value *= ((instruction & udb) ? 1 : -1) ;
                         localc += printf(" ; ") ;
                         localc += ((instruction & lsb) ? printf("from 0x") : printf("to 0x")) ;
                         localc += printf("%08X",(address + 8 + value)) ;
                        }

                      }
                      break ;

   case op_postreg  : localc = decodeldrstr(instruction,cname,1) ;
                      localc += printf("],") ;
                      if ((instruction & udb) == 0)
                       localc += printf("-") ;
#if 1
		      localc += DTDecodeRm(instruction);
#else
                      localc += dreg(Rm(instruction)) ;
#endif
                      break ;

   case op_prereg   : localc = decodeldrstr(instruction,cname,1) ;
                      localc += printf(",") ;
                      if ((instruction & udb) == 0)
                       localc += printf("-") ;
#if 1
		      localc += DTDecodeRm(instruction);
#else
                      localc += dreg(Rm(instruction)) ;
#endif
                      localc += printf("]") ;
                      if (instruction & wbb)
                       localc += printf("!") ;

                      break ;

   case op_ldmstm1  :
   case op_ldmstm2  :
                      {
                       char *mod = "??" ;
                       switch (instruction & 0x0F900000)
                        {
                         case op_LDMDA :
                         case op_STMDA : mod = "DA" ;
                                         break ;

                         case op_LDMDB :
                         case op_STMDB : mod = "DB" ;
                                         break ;

                         case op_LDMIA :
                         case op_STMIA : mod = "IA" ;
                                         break ;

                         case op_LDMIB :
                         case op_STMIB : mod = "IB" ;
                                         break ;
                        }
                       localc += printf("%.3s%.2s%s",((instruction & lsb) ? "LDM" : "STM"),cname,mod) ;
                       localc = pad(8,localc) ;
                       localc += dreg(Rn(instruction)) ;
                       localc += printf("%s,{",((instruction & wbb) ? "!" : "")) ;
                       {
                        int r ;
                        int some = 0 ;
                        for (r=0; (r < 16); r++)
                         if (instruction & regbit(r))
                          {
                           if (some)
                            localc += printf(",") ;
                           localc += dreg(r) ;
                           some = 1 ;
                          }
                       }
                       localc += printf("}%s",((instruction & psrfu) ? "^" : "")) ;
                      }
                      break ;

   case op_b        :
   case op_bl       : {
                       int w = instruction ;
                       if (w & 0x00800000)
                        w |= 0xFF000000 ;
                       else
                        w &= 0x00FFFFFF ;
                       localc = printf("%s%.2s",(((instruction & op_mask) == op_b)? "B" : "BL"),cname) ;
                       localc = pad(8,localc) ;

      /***** at this point we could look for "w" in the symbol table *****/

                       localc += printf("0x%08X  ; -> 0x%08X", (wordsize*w)+8, (address+(wordsize*w)+8)) ;
                      }
                      break ;

   case op_cppost   : localc = decodeldcstc(instruction,cname) ;
                      /* post-index */
                      localc += printf("],%s",(((instruction & udb) == 0) ? "-" : "")) ;
                      localc += printf("%d",((instruction & cp_offset_mask) * 4)) ;
                      break ;

   case op_cppre    : localc = decodeldcstc(instruction,cname) ;
                      /* pre-index */
                      localc += printf(",%s",(((instruction & udb) == 0) ? "-" : "")) ;
                      localc += printf("%d]",((instruction & cp_offset_mask) * 4)) ;
                      if (instruction & wbb)
                       localc += printf("!") ;
                      break ;

   case op_cpop     : if (instruction & regtrans)
                       {
                        /* co-proc register transfer */
			if (instruction & lsb)
			 localc = printf("MRC%.2s",cname); /*load from coproc*/
			else
			 localc = printf("MCR%.2s",cname); /*store to coproc*/

                        localc = pad(8,localc) ;
                        localc += printf("%d,%d,",cpnum(instruction),cpopcode2(instruction)) ;
                        localc += dreg(Rd(instruction)) ;
                       }
                      else
                       {
                        /* co-proc data operation */
                        localc = printf("CDO%.2s",cname) ;
                        localc = pad(8,localc) ;
                        localc += printf("%d,%d,cr%d",cpnum(instruction),cpopcode(instruction),Rd(instruction)) ;
                       }
                      localc += printf(",cr%d,cr%d,%d",Rn(instruction),Rm(instruction),cpinfo(instruction)) ;
                      break ;

   case op_swi      : localc = printf("SWI%.2s",cname) ;
                      localc = pad(8,localc) ;

      /***** at this point we could look for the SWI in the symbol table *****/

                      localc += printf("0x%06X",(instruction & 0x00FFFFFF)) ;
                      break ;
  }

#if 0
 printf("\n") ;
#endif
 return ;
}

/*---------------------------------------------------------------------------*/
/*> EOF c.ARMshape <*/

