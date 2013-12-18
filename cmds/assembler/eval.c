/*
 * File:	eval.c
 * Author:	P.A.Beskeen
 * Date:	Aug '91
 *
 * Description: Eval function that evaluates binary expression trees.
 *
 * RcsId: $Id: eval.c,v 1.3 1993/07/12 16:18:30 nickc Exp $
 *
 * (C) Copyright 1991 Perihelion Software Ltd.
 * 
 * $RcsLog$
 *
 */

 
#include "gasm.h"
#include "y.tab.h"		/* for LSHIFT and RSHIFT definitions */


/*
 * Expression structs are used to hold numeric expressions. The structs are
 * linked together to form a binary evaluation tree that contains the
 * operands, operators and defines their order of evaluation (precidence).
 *
 * e.g. -42 + 3 * (52 - 1 % fred)
 *
 *					+
 *				       / \
 *				     /	   *
 *				   /	  / \
 *				 -	 3   -
 *				/ \	    / \
 *			     NULL  42     52   %
 *					      / \
 *					     1   fred
 *						
 */

/********************************************************
 * Eval							*
 *						    	*
 * The Eval function evaluates the expression held in	*
 * the binary evaluation tree that is passed as its	*
 * sole argument.					*
 *							*
 * The pc is used in pc relative calulations as our	*
 * current position.					*
 *							*
 * Returns the result of the evaluated expression	*
 *							*
 ********************************************************/

int Eval(struct Expression *e, int pc)
{
	if (e->what == E_Num)
		return e->type.number;

	if (e->what == E_Str) {
		Symbol *s;

		if ( (s = FindSymb(e->type.name)) == NULL ) {
			char Err[128];

			sprintf(Err, "\"%s\" label not defined",  e->type.name);
			Error(Err);

			return 0;
		}
		else {
			if (s->what == HT_IMPORT) {
				char Err[128];
	
				sprintf(Err, "\"%s\" is an imported label and cannot be used in an expression context",  e->type.name);
				Error(Err);
	
				return 0;
			}
			if (s->what != HT_LABEL) {
				char Err[128];
	
				sprintf(Err, "\"%s\" is not a label",  e->type.name);
				Error(Err);
	
				return 0;
			}
			return (s->type.value - pc);
		}
	}

	switch (e->type.expr.Operator) {
	case '|':
		return (Eval(e->type.expr.left, pc) | Eval(e->type.expr.right, pc));
	case '^':
		return (Eval(e->type.expr.left, pc) ^ Eval(e->type.expr.right, pc));
	case '&':
		return (Eval(e->type.expr.left, pc) & Eval(e->type.expr.right, pc));
	case LSHIFT:
		return (Eval(e->type.expr.left, pc) << Eval(e->type.expr.right, pc));
	case RSHIFT:
		return (Eval(e->type.expr.left, pc) >> Eval(e->type.expr.right, pc));
	case '*':
		return (Eval(e->type.expr.left, pc) * Eval(e->type.expr.right, pc));
	case '/':
		return (Eval(e->type.expr.left, pc) / Eval(e->type.expr.right, pc));
	case '%':
		return (Eval(e->type.expr.left, pc) % Eval(e->type.expr.right, pc));
	case '+':
		return (Eval(e->type.expr.left, pc) + Eval(e->type.expr.right, pc));
	case '-':
		if (e->type.expr.left == NULL)
			return ( - Eval(e->type.expr.right, pc));
		else
			return (Eval(e->type.expr.left, pc) - Eval(e->type.expr.right, pc));
	case '~':
		if (e->type.expr.left != NULL)
			Fatal("Binary ~ (not) operator is impossible");
		else
			return ( ~ Eval(e->type.expr.right, pc));
	default:
		Fatal("Unknown operator found during expression evaluation");

	}

	return 0;	/*happy compiler*/
}


/* eval.c */
