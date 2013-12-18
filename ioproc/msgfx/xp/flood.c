#include <stdio.h>
#include "graph.h"


main()
{
	struct videoconfig vc;
	int i;
	int boxwidth;
	int boxheight;
	int boxesperline;
	int gmode;
	int nlines;

	_getvideoconfig(&vc);
	switch(vc.adapter)
	{
	case _CGA:
		printf("Adapter is CGA\n");
		boxesperline = 2;
		boxwidth = 320/boxesperline;
		boxheight = 200/boxesperline;
		nlines = 2;
		gmode = _MRES4COLOR;
		break;
	case _EGA:
		printf("Adapter is EGA\n");
		boxesperline = 8;
		boxwidth = 640/boxesperline;
		boxheight = 350/boxesperline;
		nlines = 2;
		gmode = _ERESCOLOR;
		break;
	case _MCGA:
	case _VGA:
		printf("Adapter is VGA or MCGA\n");
		boxesperline = 16;
		boxwidth = 320/boxesperline;
		boxheight = 200/boxesperline;
		nlines = 16;
		gmode = _MRES256COLOR;
		break;
	default:
		printf("Unknown video adapter - %d\n",vc.adapter);
		exit(1);
	}

	_setvideomode(gmode);
	
	for(i=0; i<nlines; i++)
	{	int j;
		for(j=0;j<boxesperline;j++)
		{	_setcolor(boxesperline*i+j);
			_rectangle(_GFILLINTERIOR,
					j*boxwidth, i*boxheight,
					(j+1)*boxwidth-1, (i+1)*boxheight-1);
		}
	}
	while(getchar() != '\n');
#if 0
	_setvideomode(_DEFAULTMODE);
#endif
}
