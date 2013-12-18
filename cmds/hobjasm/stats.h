/* -> stats/h
 * Title:               Statistics gathering
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */

#ifndef stats_h
#define stats_h

CARDINAL ReadAccesses(void);

CARDINAL ReadTries(void);

void AddAccess(CARDINAL tries);

void Init(void);

#endif

/* End stats/h */
