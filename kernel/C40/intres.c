/* test code for interrupt resume function. */

#define in_kernel 1
#include "gexec.h"
#include <root.h>



/*
 * LinkTxRxResume
 *
 * Add a thread to the scheduling list, enabling the scheduler
 * to execute the thread again.
 *
 * Resume is only called at high priority, but must be made interrupt
 * resilient as it is called from within interrupt handlers that use the
 * HardenedSignal() kernel function.
 */

/* txrx = tx = 1, rx = 2 */
void LinkTxRxResume(int comchan, int txrx)
{
	ExecRoot	*xroot = GetExecRoot();
	RootStruct	*nroot = GetRoot();

	SaveState	*ss = nroot->Links[comchan]->TxThread;
	ThreadQ		*q = &xroot->Queues[0];

	if (txrx == 3)
		return;	/* spurious interrupt */

	if (txrx == 1)
		ss = nroot->Links[comchan]->TxThread;
	else
		ss = nroot->Links[comchan]->RxThread;

	/* LinkTx/Rx is always high priority */
	xroot->HighestAvailPri = 0;

	/* add thread to end of run Q for its priority */
	ss->next = NULL;
	q->tail = q->tail->next = ss;
}
