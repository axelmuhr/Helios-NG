/* chanio.h:	Stand-Alone C header			*/
/* $Id: chanio.h,v 1.1 1990/11/21 18:46:42 nick Exp $ */

#include <asm.h>

#define LinkVector	((Channel *)MinInt)
#define OutLink(n)	(&LinkVector[(n)])
#define InLink(n)	(&LinkVector[(n)+4])

#define chan_out_byte(c,b)	{ char __x = b; out_(1,c,&__x); }
#define chan_out_word(c,w)	{  int __x = w; out_(4,c,&__x); }
#define chan_out_data(c,d,s)	out_(s,c,d)
#define chan_out_struct(c,d)	out_(sizeof(d),c,&d)

#define chan_in_byte(c,b) 	in_(1,c,&b)
#define chan_in_word(c,w) 	in_(4,c,&w)
#define chan_in_data(c,d,s) 	in_(s,c,d)
#define chan_in_struct(c,d) 	in_(sizeof(d),c,&d)

#define link_out_byte(l,b)	{ char __x = b; out_(1,OutLink(l),&__x); }
#define link_out_word(l,w)	{  int __x = w; out_(4,OutLink(l),&__x); }
#define link_out_data(l,d,s)	out_(s,OutLink(l),d)
#define link_out_struct(l,d)	out_(sizeof(d),OutLink(l),&d)

#define link_in_byte(l,b) 	in_(1,InLink(l),&b)
#define link_in_word(l,w) 	in_(4,InLink(l),&w)
#define link_in_data(l,d,s) 	in_(s,InLink(l),d)
#define link_in_struct(l,d) 	in_(sizeof(d),InLink(l),&d)

extern int alt(int timeout, int nchans, Channel **chans);

extern void boot(int link);

