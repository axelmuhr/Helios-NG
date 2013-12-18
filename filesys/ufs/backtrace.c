/*
 * mybacktrace.c - code to display a back trace of program execution
 *
 * This code is copyright (C) 1991 by Perihelion Software Ltd.
 * All rights are reserved.
 *
 * Version: 	1.0
 * Date:	10/7/91
 * Authors:	N Clifton, B Veer, N Garnet
 */

/*
 * This code will only work in a Helios environment.
 * The code only works properly if function names have been left in the
 * object code.
 *
 * The code assumes that the program's execution stack is laid out as follows:
 *
 *                                 --------------
 *                                 | Module Table ...
 *                                 --------------
 *                                        ^
 *                                        |
 *                                 ---------------------- - -
 *                                 | module table ptr |
 *                                 ---------------------- - - 
 *                                        ^
 *                                        |
 *   - - - ------------------------------------------------------ - - -
 *          local vars | return addr | display ptr | arg1 | arg2 ...
 *   - - - ------------------------------------------------------- - - -
 *          <-- decreasing addrs             increasing addrs -->
 * 
 *               |_______________________________________|
 *                                   |
 *                    a function's stack frame
 *
 */
#if 0 
#include <module.h>
#include <stdio.h>
#include <helios.h>
#endif
/*
 * Function:
 *	back_trace
 *
 * Arguments:
 *	none (well strictly speaking one, but it is unused)
 *
 * Description:
 *	prints out a trace of all the functions on the stack
 *	starting with the parent of the function calling this function
 *
 *	NB/ externally this function is seen as:
 *
 *		extern void mybacktrace( void );
 *
 *	whereas internally it is:
 *
 *		extern void mybacktrace( int );
 *
 * Returns:
 *	nothing
 */
 
void
my_back_trace( int arg )
{
	int		T_Mo = T_Module & 0xFFFF0000;
	int		dule = T_Module & 0x0000FFFF;
	int		T_P  = T_Proc   & 0xFFFF0000;
	int		roc  = T_Proc   & 0x0000FFFF;
	int *		caller;
	int *		stack_ptr;
	int *		last_stack;
	int		vector_stack;
	int		last_vector_stack;
	char *		name;
	int		mod_table;
	extern int	strcmp( char *, char * );


	/*
	 * NB/ we do not check for equality with
	 * magic constants directly as we do not want
	 * the constant to appear in our code, and
	 * hence accidently be confused with the
	 * start of a function or module!
	 */

	/* find address of stack frame */
	
	stack_ptr = (int *)&arg;

	/* get display pointer */
	
	--stack_ptr;
	
	/*
	 * get address of module table (treated as integer)
	 * this is constant for every program, but can vary
	 * between programs
	 */
	 
	mod_table = *((int *)(*stack_ptr));
	last_vector_stack = ((int *)(*stack_ptr))[1];
	
	/* pointer to return address (to our caller) */
	
	stack_ptr--;
	
	/* get return address off stack */
		
	caller = (int *)(*stack_ptr);	
	
	/* word align address */
		
	caller = (int *)((int)caller & ~3);

	/* scan backwards for start of procedure */
	
	while (*--caller != (T_P|roc))
		;

	/* extract the name of out caller */
	
	name = ((Proc *)caller)->Name;
			
	/* then for every parent function ... */

	last_stack = stack_ptr;	
	forever
	{
		int *	file;
						
	
		/* skip past current frame */
				
		stack_ptr += 2;
		
		/*
		 * search back through stack looking for previous frame
		 * skip address which point directly to the module table
		 * skip address which do not point to a display
		 */
		
		while (++stack_ptr, (*stack_ptr) == mod_table ||
			*((int *)(*stack_ptr)) != mod_table)
			;
		
		vector_stack = ((int *)(*stack_ptr))[1];
		
		/* point to return address */
		
		--stack_ptr;

		/* get return address off stack */
		
		caller = (int *)(*stack_ptr);	
		
		/* word align address */
		
		caller = (int *)((int)caller & ~3);

		/* copy address for use later */
		
		file = caller;
		
		/* scan backwards for start of procedure */
		
		while (*--caller != (T_P|roc))
			;
		
		/* scan backwards for start of module */
		
		while (*--file != (T_Mo|dule))
			;
			
		/* announce who called us */
		IOdebug( "%s used %d stack space",((Proc *)caller)->Name,(stack_ptr - last_stack));
		last_stack = stack_ptr;		
		IOdebug( "'%s' called from '%s' in file '%s'\n",
			name,
			((Proc *)caller)->Name,
			((Module *)file)->Name );
		IOdebug( "vector stack used %d vsp %x  sp %x",last_vector_stack-vector_stack,last_vector_stack,stack_ptr);
		last_vector_stack = vector_stack;
	
		name = ((Proc *)caller)->Name;
		
		/* if we have reached main we have finished */
		
		if (strcmp( "main",  name ) == 0 ||
		    strcmp( "_main", name ) == 0  )
			break;
			
		/* otherwise carry on */
	}
			
	/* finished */
	
	return;	

} /* back_trace */
