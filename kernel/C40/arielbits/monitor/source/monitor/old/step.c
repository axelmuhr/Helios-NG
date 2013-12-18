#include "hydra.h"

extern unsigned long breaks[];
extern unsigned long brk_addrs[];


void step( reg_set *call_set )
{
	unsigned long break1, break2;
	unsigned long break1adr, break2adr=NO_BREAK2;
	unsigned long instruction = *(unsigned long *)call_set->ret_add;
	int reg_mode=FALSE;
	int i;
	int is_at_break=MAX_BREAKS;


	for( i=0 ; i < MAX_BREAKS ; i++ )
		if( call_set->ret_add == brk_addrs[i] )
		{
			is_at_break = i;
			*(unsigned long *)brk_addrs[i] = breaks[i];
			break;
		}

	if( ((instruction & BcondAF_MASK) == BcondAF)
	 || ((instruction & BcondAT_MASK) == BcondAT)
	 || ((instruction & BcondD_MASK) == BcondD)
	 || ((instruction & DBcondD_MASK) == DBcondD)
    || ((instruction & LAJcond_MASK) == LAJcond)
	 )
	{
		break1adr = call_set->ret_add + 4;
		break1 = *(unsigned long *)break1adr;
		*(unsigned long *)break1adr = BREAK_TRAP;

		if( instruction & 0x02000000 ) /* Check for PC-Relative branch */
		{
			break2adr = call_set->ret_add + (int)(instruction & 0xFFFF) + 3;
			break2 = *(unsigned long *)break2adr;
			*(unsigned long *)break2adr = BREAK_TRAP;
		}
		else /* Register mode */
		{
			reg_mode = TRUE;
		}
	}
	else if( ((instruction & BRD_MASK) == BRD)
			|| ((instruction & LAJ_MASK) == LAJ)
		  )
	{
		break1adr = call_set->ret_add + (int)(instruction & 0xFFFFFF) + 3;
		break1 = *(unsigned long *)break1adr;
		*(unsigned long *)break1adr = BREAK_TRAP;
	}
	else if( ((instruction & Bcond_MASK) == Bcond)
			|| ((instruction & CALLcond_MASK) == CALLcond)
			|| ((instruction & DBcond_MASK) == DBcond)
			)
	{
   	break1adr = call_set->ret_add + 1;
		break1 = *(unsigned long *)break1adr;
		*(unsigned long *)(call_set->ret_add + 4) = BREAK_TRAP;

		if( instruction & 0x02000000 ) /* Check for PC-Relative branch */
		{
			break2adr = call_set->ret_add + (int)(instruction & 0xFFFF) + 1;
			break2 = *(unsigned long *)break2adr;
			*(unsigned long *)break2adr = BREAK_TRAP;
		}
		else /* Register mode */
		{
			reg_mode = TRUE;
		}
	}
	else if( ((instruction & CALL_MASK) == CALL)
			|| ((instruction & BR_MASK) == BR)
			)
	{
		break1adr = call_set->ret_add + (int)(instruction & 0xFFFFFF) + 1;
		break1 = *(unsigned long *)break1adr;
		*(unsigned long *)break1adr = BREAK_TRAP;
	}
	else if( ((instruction & RETIcondD_MASK) == RETIcondD) )
	{
   	break1adr = call_set->ret_add + 4;
		break1 = *(unsigned long *)break1adr;
		*(unsigned long *)break1adr = BREAK_TRAP;

		break2adr = *(unsigned long *)call_set->DSP_SP;
		break2 = *(unsigned long *)break2adr;
		*(unsigned long *)break2adr = BREAK_TRAP;
	}
   else if( ((instruction & RETIcond_MASK) == RETIcond)
			|| ((instruction & RETScond_MASK) == RETScond)
			)
	{
		break1adr = call_set->ret_add + 1;
		break1 = *(unsigned long *)break1adr;
		*(unsigned long *)break1adr = BREAK_TRAP;

		break2adr = *(unsigned long *)call_set->DSP_SP;
		break2 = *(unsigned long *)break2adr;
		*(unsigned long *)break2adr = BREAK_TRAP;
	}
	else if( ((instruction & RPTBreg_MASK) == RPTBreg)
			|| ((instruction & RPTBDreg_MASK) == RPTBDreg)
			)
	{
		switch( instruction & 0x1F )
		{
			case C40_R0 :
				break1adr = call_set->DSP_ir0 + 1;
				break1 = *(unsigned long *)break1adr;
				*(unsigned long *)break1adr = BREAK_TRAP;
				break;
			case C40_R1 :
				break1adr = call_set->DSP_ir1 + 1;
				break1 = *(unsigned long *)break1adr;
				*(unsigned long *)break1adr = BREAK_TRAP;
				break;
			case C40_R2 :
				break1adr = call_set->DSP_ir2 + 1;
				break1 = *(unsigned long *)break1adr;
				*(unsigned long *)break1adr = BREAK_TRAP;
				break;
			case C40_R3 :
				break1adr = call_set->DSP_ir3 + 1;
				break1 = *(unsigned long *)break1adr;
				*(unsigned long *)break1adr = BREAK_TRAP;
				break;
			case C40_R4 :
				break1adr = call_set->DSP_ir4 + 1;
				break1 = *(unsigned long *)break1adr;
				*(unsigned long *)break1adr = BREAK_TRAP;
				break;
			case C40_R5 :
				break1adr = call_set->DSP_ir5 + 1;
				break1 = *(unsigned long *)break1adr;
				*(unsigned long *)break1adr = BREAK_TRAP;
				break;
			case C40_R6 :
				break1adr = call_set->DSP_ir6 + 1;
				break1 = *(unsigned long *)break1adr;
				*(unsigned long *)break1adr = BREAK_TRAP;
				break;
			case C40_R7 :
				break1adr = call_set->DSP_ir7 + 1;
				break1 = *(unsigned long *)break1adr;
				*(unsigned long *)break1adr = BREAK_TRAP;
				break;
			case C40_R8 :
				break1adr = call_set->DSP_ir8 + 1;
				break1 = *(unsigned long *)break1adr;
				*(unsigned long *)break1adr = BREAK_TRAP;
				break;
			case C40_R9 :
				break1adr = call_set->DSP_ir9 + 1;
				break1 = *(unsigned long *)break1adr;
				*(unsigned long *)break1adr = BREAK_TRAP;
				break;
			case C40_R10 :
				break1adr = call_set->DSP_ir10 + 1;
				break1 = *(unsigned long *)break1adr;
				*(unsigned long *)break1adr = BREAK_TRAP;
				break;
			case C40_R11 :
				break1adr = call_set->DSP_ir11 + 1;
				break1 = *(unsigned long *)break1adr;
				*(unsigned long *)break1adr = BREAK_TRAP;
				break;
			case C40_AR0 :
				break1adr = call_set->DSP_ar0 + 1;
				break1 = *(unsigned long *)break1adr;
				*(unsigned long *)break1adr = BREAK_TRAP;
				break;
			case C40_AR1 :
				break1adr = call_set->DSP_ar1 + 1;
				break1 = *(unsigned long *)break1adr;
				*(unsigned long *)break1adr = BREAK_TRAP;
				break;
			case C40_AR2 :
				break1adr = call_set->DSP_ar2 + 1;
				break1 = *(unsigned long *)break1adr;
				*(unsigned long *)break1adr = BREAK_TRAP;
				break;
			case C40_AR3 :
				break1adr = call_set->DSP_ar3 + 1;
				break1 = *(unsigned long *)break1adr;
				*(unsigned long *)break1adr = BREAK_TRAP;
				break;
			case C40_AR4 :
				break1adr = call_set->DSP_ar4 + 1;
				break1 = *(unsigned long *)break1adr;
				*(unsigned long *)break1adr = BREAK_TRAP;
				break;
			case C40_AR5 :
				break1adr = call_set->DSP_ar5 + 1;
				break1 = *(unsigned long *)break1adr;
				*(unsigned long *)break1adr = BREAK_TRAP;
				break;
			case C40_AR6 :
				break1adr = call_set->DSP_ar6 + 1;
				break1 = *(unsigned long *)break1adr;
				*(unsigned long *)break1adr = BREAK_TRAP;
				break;
			case C40_AR7 :
				break1adr = call_set->DSP_ar7 + 1;
				break1 = *(unsigned long *)break1adr;
				*(unsigned long *)break1adr = BREAK_TRAP;
				break;
			case C40_DP :
				break1adr = call_set->DSP_DP + 1;
				break1 = *(unsigned long *)break1adr;
				*(unsigned long *)break1adr = BREAK_TRAP;
				break;
			case C40_IR0 :
				break1adr = call_set->DSP_IR0 + 1;
				break1 = *(unsigned long *)break1adr;
				*(unsigned long *)break1adr = BREAK_TRAP;
				break;
			case C40_IR1 :
				break1adr = call_set->DSP_IR1 + 1;
				break1 = *(unsigned long *)break1adr;
				*(unsigned long *)break1adr = BREAK_TRAP;
				break;
			case C40_SP :
				break1adr = call_set->DSP_SP + 1;
				break1 = *(unsigned long *)break1adr;
				*(unsigned long *)break1adr = BREAK_TRAP;
				break;
			case C40_ST :
				break1adr = call_set->DSP_ST + 1;
				break1 = *(unsigned long *)break1adr;
				*(unsigned long *)break1adr = BREAK_TRAP;
				break;
			case C40_DIE :
				break1adr = call_set->DSP_DIE + 1;
				break1 = *(unsigned long *)break1adr;
				*(unsigned long *)break1adr = BREAK_TRAP;
				break;
			case C40_IIE :
				break1adr = call_set->DSP_IIE + 1;
				break1 = *(unsigned long *)break1adr;
				*(unsigned long *)break1adr = BREAK_TRAP;
				break;
			case C40_IIF :
				break1adr = call_set->DSP_IIF + 1;
				break1 = *(unsigned long *)break1adr;
				*(unsigned long *)break1adr = BREAK_TRAP;
				break;
		}
	}
	else if( ((instruction & RPTBim_MASK) == RPTBim) )
	{
		break1adr = call_set->ret_add + (instruction&0x00FFFFFF) + 2;
		break1 = *(unsigned long *)break1adr;
		*(unsigned long *)break1adr = BREAK_TRAP;
	}
	else if( ((instruction & RPTS_MASK) == RPTS) )
	{
		break1adr = call_set->ret_add + 2;
		break1 = *(unsigned long *)break1adr;
		*(unsigned long *)break1adr = BREAK_TRAP;
	}
	else if( ((instruction & RPTBDim_MASK) == RPTBDim) )
	{
		break1adr = call_set->ret_add + (instruction&0x00FFFFFF) + 4;
		break1 = *(unsigned long *)break1adr;
		*(unsigned long *)break1adr = BREAK_TRAP;
	}
	else if( (instruction & TRAPcond_MASK) == TRAPcond )
	{
		break1adr = call_set->ret_add + 1;
		break1 = *(unsigned long *)break1adr;
		*(unsigned long *)break1adr = BREAK_TRAP;

		break2adr = *(unsigned long *)(call_set->DSP_TVTP + (unsigned long)(instruction & 0xFF));
		break2 = *(unsigned long *)break2adr;
		*(unsigned long *)break2adr = BREAK_TRAP;
	}
	else if( (instruction & LATcond_MASK) == LATcond )
	{
		break1adr = call_set->ret_add + 4;
		break1 = *(unsigned long *)break1adr;
		*(unsigned long *)break1adr = BREAK_TRAP;

		break2adr = *(unsigned long *)(call_set->DSP_TVTP + (unsigned long)(instruction & 0xFF));
		break2 = *(unsigned long *)break2adr;
		*(unsigned long *)break2adr = BREAK_TRAP;
	}
	else
	{
		break1adr = call_set->ret_add + 1;
		break1 = *(unsigned long *)break1adr;
		*(unsigned long *)break1adr = BREAK_TRAP;
	}

	if( reg_mode )
	{
		switch( instruction & 0xFF )
		{
			case C40_R0 :
         	break2adr = call_set->DSP_ir0;
				break2 = *(unsigned long *)break2adr;
				*(unsigned long *)break2adr = BREAK_TRAP;
				break;
			case C40_R1 :
         	break2adr = call_set->DSP_ir0;
				break2 = *(unsigned long *)break2adr;
				*(unsigned long *)break2adr = BREAK_TRAP;
				break;
			case C40_R2 :
				break2adr = call_set->DSP_ir0;
				break2 = *(unsigned long *)break2adr;
				*(unsigned long *)break2adr = BREAK_TRAP;
				break;
			case C40_R3 :
         	break2adr = call_set->DSP_ir0;
				break2 = *(unsigned long *)break2adr;
				*(unsigned long *)break2adr = BREAK_TRAP;
				break;
			case C40_R4 :
         	break2adr = call_set->DSP_ir0;
				break2 = *(unsigned long *)break2adr;
				*(unsigned long *)break2adr = BREAK_TRAP;
				break;
			case C40_R5 :
         	break2adr = call_set->DSP_ir0;
				break2 = *(unsigned long *)break2adr;
				*(unsigned long *)break2adr = BREAK_TRAP;
				break;
			case C40_R6 :
         	break2adr = call_set->DSP_ir0;
				break2 = *(unsigned long *)break2adr;
				*(unsigned long *)break2adr = BREAK_TRAP;
				break;
			case C40_R7 :
         	break2adr = call_set->DSP_ir0;
				break2 = *(unsigned long *)break2adr;
				*(unsigned long *)break2adr = BREAK_TRAP;
				break;
			case C40_R8 :
         	break2adr = call_set->DSP_ir0;
				break2 = *(unsigned long *)break2adr;
				*(unsigned long *)break2adr = BREAK_TRAP;
				break;
			case C40_R9 :
         	break2adr = call_set->DSP_ir0;
				break2 = *(unsigned long *)break2adr;
				*(unsigned long *)break2adr = BREAK_TRAP;
				break;
			case C40_R10 :
         	break2adr = call_set->DSP_ir0;
				break2 = *(unsigned long *)break2adr;
				*(unsigned long *)break2adr = BREAK_TRAP;
				break;
			case C40_R11 :
         	break2adr = call_set->DSP_ir0;
				break2 = *(unsigned long *)break2adr;
				*(unsigned long *)break2adr = BREAK_TRAP;
				break;
			case C40_AR0 :
         	break2adr = call_set->DSP_ir0;
				break2 = *(unsigned long *)break2adr;
				*(unsigned long *)break2adr = BREAK_TRAP;
				break;
			case C40_AR1 :
         	break2adr = call_set->DSP_ir0;
				break2 = *(unsigned long *)break2adr;
				*(unsigned long *)break2adr = BREAK_TRAP;
				break;
			case C40_AR2 :
         	break2adr = call_set->DSP_ir0;
				break2 = *(unsigned long *)break2adr;
				*(unsigned long *)break2adr = BREAK_TRAP;
				break;
			case C40_AR3 :
         	break2adr = call_set->DSP_ir0;
				break2 = *(unsigned long *)break2adr;
				*(unsigned long *)break2adr = BREAK_TRAP;
				break;
			case C40_AR4 :
         	break2adr = call_set->DSP_ir0;
				break2 = *(unsigned long *)break2adr;
				*(unsigned long *)break2adr = BREAK_TRAP;
				break;
			case C40_AR5 :
         	break2adr = call_set->DSP_ir0;
				break2 = *(unsigned long *)break2adr;
				*(unsigned long *)break2adr = BREAK_TRAP;
				break;
			case C40_AR6 :
         	break2adr = call_set->DSP_ir0;
				break2 = *(unsigned long *)break2adr;
				*(unsigned long *)break2adr = BREAK_TRAP;
				break;
			case C40_AR7 :
         	break2adr = call_set->DSP_ir0;
				break2 = *(unsigned long *)break2adr;
				*(unsigned long *)break2adr = BREAK_TRAP;
				break;
			case C40_DP :
         	break2adr = call_set->DSP_ir0;
				break2 = *(unsigned long *)break2adr;
				*(unsigned long *)break2adr = BREAK_TRAP;
				break;
			case C40_IR0 :
         	break2adr = call_set->DSP_ir0;
				break2 = *(unsigned long *)break2adr;
				*(unsigned long *)break2adr = BREAK_TRAP;
				break;
			case C40_IR1 :
         	break2adr = call_set->DSP_ir0;
				break2 = *(unsigned long *)break2adr;
				*(unsigned long *)break2adr = BREAK_TRAP;
				break;
			case C40_SP :
         	break2adr = call_set->DSP_ir0;
				break2 = *(unsigned long *)break2adr;
				*(unsigned long *)break2adr = BREAK_TRAP;
				break;
			case C40_ST :
         	break2adr = call_set->DSP_ir0;
				break2 = *(unsigned long *)break2adr;
				*(unsigned long *)break2adr = BREAK_TRAP;
				break;
			case C40_DIE :
         	break2adr = call_set->DSP_ir0;
				break2 = *(unsigned long *)break2adr;
				*(unsigned long *)break2adr = BREAK_TRAP;
				break;
			case C40_IIE :
         	break2adr = call_set->DSP_ir0;
				break2 = *(unsigned long *)break2adr;
				*(unsigned long *)break2adr = BREAK_TRAP;
				break;
			case C40_IIF :
         	break2adr = call_set->DSP_ir0;
				break2 = *(unsigned long *)break2adr;
				*(unsigned long *)break2adr = BREAK_TRAP;
				break;
		}
	}

	run( BREAK_TRAP_NUM );

	if( *(unsigned long *)(call_set->ret_add-1) == BREAK_TRAP )
		call_set->ret_add--;

	*(unsigned long *)break1adr = break1;
	if( break2adr != NO_BREAK2 )
		*(unsigned long *)break2adr = break2;

	if( is_at_break != MAX_BREAKS )
		*(unsigned long *)brk_addrs[is_at_break] = BREAK_TRAP;

	reg_dump( *call_set );
}