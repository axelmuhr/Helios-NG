/* -> osdepend/c
 * Title:               Encapsulate machine dependent bits in one module
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
/*---------------------------------------------------------------------------*/

#include "asm.h"
#include "constant.h"
#include "formatio.h"
#include "globvars.h"
#include "getline.h"
#include "osdepend.h"
#include "code.h"
#include "listing.h"
#include <stdio.h>
#include <signal.h>

/*---------------------------------------------------------------------------*/

static int escape = 0 ;

/*---------------------------------------------------------------------------*/

BOOLEAN PollEscape(void)
{
 int i = escape ;
 if (i)
  printf("User termination request\n") ;
 escape = 0 ;
 return(i) ;
}

/*---------------------------------------------------------------------------*/

static void EscapeHandler(int sig)
{
 escape = 1 ;
 signal(SIGINT,EscapeHandler) ; /* replace the handler */
 sig = sig ;
}

/*---------------------------------------------------------------------------*/

void Init_osd(void)
{
 signal(SIGINT,EscapeHandler) ; /* ctrl-C handler */
}

/*---------------------------------------------------------------------------*/
/*> EOF osdepend/c <*/
