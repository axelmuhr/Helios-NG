/* Placed into the public domain by Daniel J. Bernstein. */

#include <stdio.h>
#include "bitout.h"

static int twom1[9] = { 0,1,3,7,15,31,63,127,255 } ;

bitword bit_wbuf[BITBUFSIZE + 1];
bitnum bit_bbuf[BITBUFSIZE + 1];
int bit_bufsize = 0;

long bit_numout = 0;

static int savecurbyte = 0;
static bitnum savecurbb8 = 8;

int bit_printbuf()
{
 register int curbyte = savecurbyte;
 register bitnum curbb8 = savecurbb8;
 register bitword curw;
 register int i;

 /* XXX: should use registers & curbyte & so on better */

 for (i = 0;i < bit_bufsize;i++)
  {
   /* assumes bit_bbuf[i] <= 0 */
   curw = bit_wbuf[i];
   curbyte += ((curw & twom1[curbb8]) << (8 - curbb8));
#ifdef BRAINDAMAGED
   if (curbyte == 255)
     (void) putchar((char) curbyte);
   else
#endif
   if (putchar((char) curbyte) == EOF)
     return EOF;
   bit_numout++;
   curw >>= curbb8;
   curbb8 += bit_bbuf[i];

   /* just in case bit_bbuf[i] < -7 */
   while (curbb8 <= 0)
    {
#ifdef BRAINDAMAGED
     if (curw & 255 == 255)
       (void) putchar((char) (curw & 255));
     else
#endif
     if (putchar((char) (curw & 255)) == EOF)
       return EOF;
     bit_numout++;
     curbb8 += 8;
     curw >>= 8;
    }
   curbyte = (int) curw; /* does not lose accuracy */
  }
 savecurbyte = curbyte; /* ah, the joy of coroutines */
 savecurbb8 = curbb8; /* better this way (double vars) for efficiency */
 bit_bufsize = 0;
 return 0;
}

int bit_flushbuf()
{
 if (bit_printbuf() == EOF)
   return EOF;
 if (savecurbb8 < 8)
  {
#ifdef BRAINDAMAGED
   if (savecurbyte == 255)
     (void) putchar((char) savecurbyte);
   else
#endif
   if (putchar((char) savecurbyte) == EOF)
     return EOF;
   bit_numout++;
  }
 savecurbyte = 0;
 savecurbb8 = 8;
 return 0;
}

int bit_fillflush(x)
int x;
{
 if (bit_printbuf() == EOF)
   return EOF;
 savecurbyte += (x << 8 - savecurbb8) & 255;
#ifdef BRAINDAMAGED
 if (savecurbyte == 255)
   (void) putchar((char) savecurbyte);
 else
#endif
 if (putchar((char) savecurbyte) == EOF)
   return EOF;
 bit_numout++;
 savecurbyte = 0;
 savecurbb8 = 8;
 return 0;
}
