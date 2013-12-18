
#include <asm.h>
#include <trace.h>
#include <salib.h>
#include <sysinfo.h>

/*--------------------------------------------------------
-- Debugging						--
--							--
--							--
--------------------------------------------------------*/

static word *TraceVec = NULL;

#define TVSize		(4*1024)	/* 4k trace vector (must be 2^x)*/
#define	TVMask		(TVSize-4)	/* mask for offset bits		*/
#define TVEnable	1		/* trace enable bit		*/

static void debug( word *tv, word value );

extern void _TraceInit(void)
{
	if( TraceVec == NULL ) 
	{
		if( _SYSINFO->TraceVec == NULL ) memtop();
		TraceVec = _SYSINFO->TraceVec;
	}
	TraceVec[0] = 4;
}

extern void _Mark(void)
{
	word *tv = TraceVec;
	
	if( (*tv & TVEnable) == 0 )
	{
		debug(tv,0x22222222);
		debug(tv,ldtimer_());
		debug(tv,ldpri_()|ldlp_(7));
		debug(tv,ldl_(4));
	}
}

extern void _Trace(word x,...)
{
	word *tv = TraceVec;

	if( (*tv & TVEnable) == 0 )
	{
		debug(tv,0x11111111);
		debug(tv,ldtimer_());
		debug(tv,ldpri_()|ldlp_(7));
		debug(tv,ldl_(4));
		debug(tv,ldl_(6));
		debug(tv,ldl_(7));
		debug(tv,ldl_(8));				
	}
	return; x=x;
}

extern void _Halt()
{
	_Mark();
	stopp_();
}

static void debug(word *tv,word value)
{
	word offset = (tv[0] & TVMask)/4;
	
	tv[offset] = value;
	
	/* if that was the last word in the vector, inc by 8 so wrap-round */
	/* skips over control word					   */
	if( (offset & (TVMask/4)) == (TVMask/4) ) tv[0] += 8;
	else tv[0] += 4;
	
}

