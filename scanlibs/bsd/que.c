/* $Id: que.c,v 1.1 1990/09/05 13:38:57 nick Exp $ */
#include <sys/types.h>

extern void insque(struct qelem *elem, struct qelem *pred)
{
	elem->q_back         = pred;
	elem->q_forw         = pred->q_forw;
	pred->q_forw->q_back = elem;
	pred->q_forw         = elem;
}

extern void remque(struct qelem *elem)
{
	elem->q_back->q_forw = elem->q_forw;
	elem->q_forw->q_back = elem->q_back;
}

