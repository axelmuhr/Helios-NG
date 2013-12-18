/* $Header: drawText.c,v 1.1 91/01/31 13:52:17 charles Locked $ */
/* $Source: /server/usr/users/b/charles/world/microlink/RCS/source/drawText.c,v $ */

/*----------------------------------------------------------------------*/
/*                                                   source/drawText.c  */
/*----------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#include "drawp/public.h"
#include "drawText.h"

/* drawText.c:                                                          */
/*                                                                      */
/* Draw some text on the LCD screen using Charlie's blitter             */
/*                                                                      */
/* John Grogan, Active Book Company, 3/12/90.                           */

/*----------------------------------------------------------------------*/
/*                                                              Statics */
/*----------------------------------------------------------------------*/

DpPixmap_t                   *charSet;

/*-----------------------------------------------------------------*/
/*                                                       writeText */
/*-----------------------------------------------------------------*/

void writeText(char *string, int line)
{  drawString(5, line*LINEINC+10, string);  }

/*-----------------------------------------------------------------*/
/*                                                      drawString */
/*-----------------------------------------------------------------*/

void drawString(int x, int y, char *string)
{  while (*string)
   {  drawChar(x, y, *string++);
      x+=8;
}  }

/*-----------------------------------------------------------------*/
/*                                                        drawChar */
/*-----------------------------------------------------------------*/

void drawChar(int x, int y, char ch)
{
  dpCopyArea(charSet, screen, gc, 0, (ch-32)*8, 8, 8, x, y);
}

/*----------------------------------------------------------------------*/

/* The rest of the code is by Pete Cockerell & turns the input 
   character set file into a suitable pixmap 
*/

/* ************ */
/* ** Pixmap ** */
/* ************ */
/* Create a 1bpp pixmap from the data file given */
/* The file is in X bitmap format, ie: */
/*	#define left_ptr_width 16 */
/*	#define left_ptr_height 16 */
/*	#define left_ptr_x_hot 3 */
/*	#define left_ptr_y_hot 1 */
/*	static char left_ptr_bits[] = { */
/*	   0x00, 0x00, 0x08, 0x00, 0x18, 0x00, 0x38, 0x00, 0x78, 0x00, 0xf8, 0x00, */
/*	   0xf8, 0x01, 0xf8, 0x03, 0xf8, 0x07, 0xf8, 0x00, 0xd8, 0x00, 0x88, 0x01, */
/*	   0x80, 0x01, 0x00, 0x03, 0x00, 0x03, 0x00, 0x00}; */

DpPixmap_t *readFile(const char *Name)
{
	char buff[160], leafName[20], *dstData,
		*ptr, *srcData, *sd, *str;
	DpPixmap_t *rep;
	int i, x, y, rowLen, rowLenB, rowLenW, len;
	int HotX, HotY,	width, height, lastSlash;
	FILE *f;

/*Init return vals in case of error (and HotX/Y are
only optional anyway). */

	width = height = 0;
	HotX = HotY = -1;

	f = fopen(Name, "r");
	if (f==NULL)
	{
		return NULL;
	}

/*Get just the leaf name*/
	lastSlash = -1;
	for (i = 0; Name[i]; i++)
		if (Name[i] == '/')
			lastSlash = i;
	strcpy(leafName, Name+lastSlash+1);
	for (i=0; leafName[i]; i++)
		if (leafName[i] == '.')
		{
			leafName[i] = 0;
			break;
		}

/*How many chars to skip after the #define line*/
	len = strlen("#define ") + strlen(leafName) + 1;

/*Skip until we get to the 'static' line*/
	while (fgets(buff, 160, f)[0] != 's')
	{
/*Is it a #define?*/
		if (buff[0] == '#')		
			switch (buff[len])
			{
				case 'w': width =  atoi(buff+len+5); break;
				case 'h': height = atoi(buff+len+6); break;
				case 'x': HotX =   atoi(buff+len+5); break;
				case 'y': HotY =   atoi(buff+len+5); break;
			}
	}

/*Didn't get the right info*/
	if (width == 0 || height == 0)
	{
		fclose(f);
		return NULL;
	}

/*How many bytes per row*/
	rowLen = (width+7)/8;
	sd = srcData = calloc(rowLen*height, sizeof(char));
	ptr = srcData;

/*Scan them in from the file*/
	fgets(buff, 160, f);
	str = buff;
	for (y=0; y<height; y++)
	{
		for (x=0; x<rowLen; x++)
		{
			while (*str != '0' && *str)
				str++;
			if (!*str)
			{
				fgets(buff, 160, f);
				str = buff;
				while (*str != '0')
					str++;
			}
			*ptr++ = strtol(str, &str, 0);
		}
	}	

	fclose(f);

	rep = dpConstructPixmap(NULL, width, height, 1);

/*Get start address of rep bitmap (Skip fudge word at start)*/
	dstData = (char*)rep->rawBase + 4;

/*Find out input data rowLen in bytes*/
	rowLenB = (width+7) / 8;
/*Blitter bit images are word-oriented*/
	rowLenW = rep->wpv*4;
	for (y=0; y<height; y++)
	{
		for (x = 0; x<rowLenB; x++)
			dstData[x] = srcData[x];
		dstData += rowLenW;
		srcData += rowLenB;
	}

	free(sd);
	return rep;
}
