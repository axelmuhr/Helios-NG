/* Placed into the public domain by Daniel J. Bernstein. */

/* This is a bit-output library; sorry for the lack of documentation. */
/* Feel free to use it in other programs requiring fast bit output. */
/* Outside #defines: BITBUFSIZE, TYPE, BRAINDAMAGED. */

#ifndef BITOUT_H
#define BITOUT_H

#ifndef BITBUFSIZE
#define BITBUFSIZE (1000)
#endif

#ifndef TYPE
#define TYPE short
#endif

typedef TYPE bitnum; /* must be signed */
typedef unsigned TYPE bitword;

extern bitword bit_wbuf[];
extern bitnum bit_bbuf[];
extern int bit_bufsize;
extern int bit_printbuf();
extern int bit_flushbuf();
extern long bit_numout;
extern int bit_fillflush();

#define bits_out(n,b) \
( (bit_wbuf[bit_bufsize] = n), (bit_bbuf[bit_bufsize] = 8 - b), \
  (bit_bufsize++), ((bit_bufsize == BITBUFSIZE) && bit_printbuf()) )

#endif
