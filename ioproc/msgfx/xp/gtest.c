#include <stdio.h>
#include <stdlib.h>

#include <nonansi.h>

#include "graph.h"

#define bsizex 70
#define bsizey 60
#define bposx  0
#define bposy  0

int main()
{
	FILE *f = fopen("gfile","wb");
	char *gbuff;
	int gbuffsize;
	WORD t1;
	WORD t2;

	dsize = 10240;
	if( !_setvideomode(_MRES4COLOR) )
	{
		fprintf(stderr,"Unable to setvideomode\n");
		exit(1);
	}
	FlushMSG(WAIT);
	t1 = _cputime();
/* Top left */
	_setcolor(0);
	_rectangle(_GFILLINTERIOR,bposx,bposy,
				bposx+bsizex/2-1,bposy+bsizey/2-1);
/* bottom right */
	_setcolor(1);
	_rectangle(_GFILLINTERIOR,bposx+bsizex/2,bposy+bsizey/2,
				bposx+bsizex-1,bposy+bsizey-1);
/* bottom left */
	_setcolor(2);
	_rectangle(_GFILLINTERIOR,bposx,bposy+bsizey/2,
				bposx+bsizex/2-1,bposy+bsizey-1);
/* top right */
	_setcolor(3);
	_rectangle(_GFILLINTERIOR,bposx+bsizex/2,bposy,
				bposx+bsizex-1,bposy+bsizey/2-1);
	FlushMSG(WAIT);
	t2 = _cputime();
	fprintf(stderr,"Time to draw 4 boxes = %dms\n",(t2-t1)*10);

	gbuffsize = _imagesize(bposx,bposy,bsizex+bposx-1,bsizey+bposy-1); 
	gbuff= malloc(gbuffsize);

	t1 = _cputime();
	_getimage(bposx,bposy,bsizex+bposx-1,bsizey+bposy-1,gbuff);
	t2 = _cputime();
	fprintf(stderr,"Time to get image = %dms\n",(t2-t1)*10);
	fprintf(stderr,"imagesize = %d\n",gbuffsize);

	t1 = _cputime();
	_putimage(bposx,bsizey+bposy,gbuff,_GPSET);
	_putimage(bsizex+bposx,bposy,gbuff,_GPSET);
	_putimage(bsizex+bposx,bsizey+bposy,gbuff,_GPSET);
	_putimage(2*bsizex+bposx,bposy,gbuff,_GPSET);
	_putimage(2*bsizex+bposx,bsizey+bposy,gbuff,_GPSET);
	FlushMSG(PASS);
	t2 = _cputime();
	fprintf(stderr,"Time to draw 5 images = %dms\n",(t2-t1)*10);

	fwrite(gbuff, gbuffsize,1,f);
/*	_setvideomode(_DEFAULTMODE); */

	return 0;
}
