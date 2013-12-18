#include <stdio.h>
#include "graph.h"

int main()
{
	struct videoconfig vc;
	_setvideomode(_ERESCOLOR);
	_getvideoconfig(&vc);
	printf("pixel width = %d\n",vc.numxpixels);
	printf("pixel height = %d\n",vc.numypixels);
	printf("text width = %d\n",vc.numtextcols);
	printf("text height = %d\n",vc.numtextrows);
	printf("number of colours = %d\n",vc.numcolors);
	printf("bits per pixel = %d\n",vc.bitsperpixel);
	printf("number of video pages = %d\n",vc.numvideopages);
	printf("mode = %d\n",vc.mode);
	printf("adapter = %d\n",vc.adapter);
	printf("monitor = %d\n",vc.monitor);
	printf("memory = %d\n",vc.memory);
	return 0;
}
