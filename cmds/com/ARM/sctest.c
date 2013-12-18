#include <abcARM/ABClib.h>
#include <stdio.h>

word			screen_start;
int			ind, yind;
word			screen_stride;
int 			screen_width;
int			screen_height;
char *			screen;

void dotest(char a, char b)
{
	for (yind = 50; yind--;)	
	{
		for (ind = 128; ind--;)
			*(screen++) = (((int) screen) % 2)?a:0;
		for (ind = 128; ind--;)
			*(screen++) = (((int) screen) % 2)?b:0;
	}
}

int main()
{

	DisplayInfo(&screen_start, &screen_stride, &screen_width, &screen_height);

	screen = (char *)screen_start;

	printf("screen         %p\n", (char *)screen_start);
	printf("screen_stride  %p\n", (char *)screen_stride);
	printf("screen_width   %d\n", screen_width);
	printf("screen_height  %d\n", screen_height);
	
	printf("\nHit any key...");
	
	fflush(stdout);

	getchar();
	
	printf("%c",12);
	
	fflush(stdout);
	
	*screen = 0xff;
	screen += 0x100;

	dotest(0xff, 0xff);
	dotest(0x01, 0x00);
	dotest(0xff, 0xff);
	dotest(0x00, 0x01);
	dotest(0xff, 0xff);
	dotest(0x01, 0x00);
	dotest(0x80, 0x00);
	
	return 1;

}


