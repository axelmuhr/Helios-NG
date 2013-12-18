#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "graph.h"

#define bsizex 70
#define bsizey 60
#define bposx  0
#define bposy  0

int main()
{
	char far *gbuff;
	int gbuffsize;
	_setvideomode(_ERESNOCOLOR);
/* Top left */
	_setcolor(2);
	_rectangle(_GFILLINTERIOR,bposx,bposy,
				bposx+bsizex/2-1,bposy+bsizey/2-1);
/* bottom right */
	_setcolor(3);
	_rectangle(_GFILLINTERIOR,bposx+bsizex/2,bposy+bsizey/2,
				bposx+bsizex-1,bposy+bsizey-1);
/* bottom left */
	_setcolor(4);
	_rectangle(_GFILLINTERIOR,bposx,bposy+bsizey/2,
				bposx+bsizex/2-1,bposy+bsizey-1);
/* top right */
	_setcolor(5);
	_rectangle(_GFILLINTERIOR,bposx+bsizex/2,bposy,
				bposx+bsizex-1,bposy+bsizey/2-1);

	gbuffsize = _imagesize(bposx,bposy,bsizex+bposx-1,bsizey+bposy-1); 
	gbuff= _fmalloc(gbuffsize);

	_getimage(bposx,bposy,bsizex+bposx-1,bsizey+bposy-1,gbuff);
	fprintf(stderr,"imagesize = %d\n",gbuffsize);

	_putimage(bposx,bsizey+bposy,gbuff,_GPSET);
	_putimage(bsizex+bposx,bposy,gbuff,_GPSET);
	_putimage(bsizex+bposx,bsizey+bposy,gbuff,_GPSET);
	_putimage(2*bsizex+bposx,bposy,gbuff,_GPSET);
	_putimage(2*bsizex+bposx,bsizey+bposy,gbuff,_GPSET);

	_setvideomode(_DEFAULTMODE);

	return 0;
}
