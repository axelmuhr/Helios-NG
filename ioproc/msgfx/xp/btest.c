#include <stdio.h>
#include "graph.h"

int bsize(int x,int y,int bpp)
{
#if 0
	return 4+((x*bpp+7)/8)*y;
#endif
	return _imagesize(0,0,x-1,y-1);
}

int main()
{
	if( _setvideomode(_TEXTBW40) )
		printf("TEXTBW40 - %d\n", bsize(40,25,4));
	if( _setvideomode(_TEXTC40) )
		printf("TEXTC40 - %d\n", bsize(40,25,4));
	if( _setvideomode(_TEXTBW80) )
		printf("TEXTBW80 - %d\n", bsize(80,25,4));
	if( _setvideomode(_TEXTC80) )
		printf("TEXTC80 - %d\n", bsize(80,25,4));
	if( _setvideomode(_MRES4COLOR) )
		printf("MRES4COLOR - %d\n", bsize(320,200,2));
	if( _setvideomode(_MRESNOCOLOR) )
		printf("MRESNOCOLOR - %d\n", bsize(320,200,2));
	if( _setvideomode(_HRESBW) )
		printf("HRESBW - %d\n", bsize(640,200,1));
	if( _setvideomode(_TEXTMONO) )
		printf("TEXTMONO - %d\n", bsize(80,25,1));
	if( _setvideomode(_MRES16COLOR) )
		printf("MRES16COLOR - %d\n", bsize(320,200,4));
	if( _setvideomode(_HRES16COLOR) )
		printf("HRES16COLOR - %d\n", bsize(640,200,4));
	if( _setvideomode(_ERESNOCOLOR) )
		printf("ERESNOCOLOR - %d\n", bsize(320,350,1));
	if( _setvideomode(_ERESCOLOR) )
		printf("ERESCOLOR - %d\n", bsize(340,350,6));
	if( _setvideomode(_VRES2COLOR) )
		printf("VRES2COLOR - %d\n", bsize(640,480,1));
	if( _setvideomode(_VRES16COLOR) )
		printf("VRES16COLOR - %d\n", bsize(640,480,4));
	if( _setvideomode(_MRES256COLOR) )
		printf("MRES256COLOR - %d\n", bsize(320,200,8));
}
