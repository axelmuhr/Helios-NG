/* -> getdir/h
 * Title:               GET and associated directives
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
#ifndef getdir_h
#define getdir_h

void GetDir(char *line, CARDINAL *lineIndex);

void WendDir(void);

void CheckStack(void);
/*Check that END, LNK, GET etc. don't occur inside conditionals*/

BOOLEAN MexitDir(void);

BOOLEAN MendDir(void);

void MnoteDir(char *line, CARDINAL *lineIndex);

#endif

/* End getdir/h */
