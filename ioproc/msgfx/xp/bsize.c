#include <stdio.h>

int bsize(int x,int y,int bpp)
{
#ifdef never
	return 4+((x*bpp+7)/8)*y;
#endif
	return _imagesize(0,0,x-1,y-1);
}

int main()
{
	printf("TEXTBW40 - %d\n", bsize(40,25,4));
	printf("TEXTC40 - %d\n", bsize(40,25,4));
	printf("TEXTBW80 - %d\n", bsize(80,25,4));
	printf("TEXTC40 - %d\n", bsize(80,25,4));
	printf("MRES4COLOR - %d\n", bsize(320,200,2));
	printf("MRESNOCOLOR - %d\n", bsize(320,200,2));
	printf("HRESBW - %d\n", bsize(640,200,1));
	printf("TEXTMONO - %d\n", bsize(80,25,1));
	printf("MRES16COLOR - %d\n", bsize(320,200,4));
	printf("HRES16COLOR - %d\n", bsize(640,200,4));
	printf("ERESNOCOLOR - %d\n", bsize(320,350,1));
	printf("ERESCOLOR - %d\n", bsize(340,350,6));
	printf("VRES2COLOR - %d\n", bsize(640,480,1));
	printf("VRES16COLOR - %d\n", bsize(640,480,4));
	printf("MRES256COLOR - %d\n", bsize(320,200,8));
}
