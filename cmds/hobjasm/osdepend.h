/* -> osdepend/h
 * Title:               Encapsulate machine dependant bits in one module
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */

#ifndef osdependent_h
#define osdependent_h

#include "constant.h"
#include <stdio.h>

BOOLEAN PollEscape(void);

void Init_osd(void);

#endif

/* End osdepend/h */
