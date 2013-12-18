#include <stdio.h>
#include <graph.h>


main()
{	int x;
	_setvideomode(_MRES4COLOR);
	_ellipse(_GFILLINTERIOR, 80, 50,240,150 );
	x = _getcolor();
/*	_floodfill(81,50,_getcolor()); */
	while(getchar() != '\n');
	_setvideomode(_DEFAULTMODE);
	printf("Current color = %d\n",x);
}
