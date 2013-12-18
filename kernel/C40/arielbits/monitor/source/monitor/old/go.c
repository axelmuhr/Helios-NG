#include "hydra.h"

extern unsigned long breaks[];
extern unsigned long brk_addrs[];

void go( reg_set *call_set )
{
	int i;
	int is_at_break=MAX_BREAKS;


	for( i=0 ; i < MAX_BREAKS ; i++ )
		if( call_set->ret_add == brk_addrs[i] )
		{
			is_at_break = i;
			*(unsigned long *)brk_addrs[i] = breaks[i];
			break;
		}

	run( BREAK_TRAP_NUM );

	if( *(unsigned long *)(call_set->ret_add-1) == BREAK_TRAP )
		call_set->ret_add--;

	if( is_at_break != MAX_BREAKS )
		*(unsigned long *)brk_addrs[is_at_break] = BREAK_TRAP;

	reg_dump( *call_set );
}