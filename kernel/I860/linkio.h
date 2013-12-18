#ifndef _linkio_h
#define _linkio_h

#include <link.h>
#include "adapter.h"
#include "exec.h"

word RxInterruptHandler(adapter *l);
word TxInterruptHandler(adapter *l);
void Link_Int_Disable(void);
SaveState *AbortLinkTx(LinkInfo *link);
SaveState *AbortLinkRx(LinkInfo *link);

#endif
