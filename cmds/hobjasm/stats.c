/* -> stats/c
 * Title:               Statistics gathering
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */


#include "constant.h"
#include "stats.h"

CARDINAL accesses =0,
         tries =0;

CARDINAL ReadAccesses(void)
{
  return accesses;
} /* End ReadAccesses */

CARDINAL ReadTries(void)
{
  return tries;
} /* End ReadTries */

void AddAccess(CARDINAL a)
{
  accesses++;
  tries += a;
} /* End AddAccess */

void Init(void)
{
 accesses = 0 ;
 tries = 0 ;
} /* End Init */

/* End stats/c */
