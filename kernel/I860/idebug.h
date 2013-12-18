#include "adapter.h"

extern void obyte(char ch);
extern char ibyte(void);
extern int  XIOdebug(char *fmt, ...);
extern int  printunum(unsigned int n,int base);
extern int  printnum(int n,int base);
extern void oword(int w);
extern int  outstring(char *s);
extern void peekpoke(void);
extern void dumpregs(struct TrapData *td);
extern void dumpstate(struct TrapData *td);

