/* $Header: digCalibrate.c,v 1.1 91/01/31 13:52:11 charles Locked $ */
/* $Source: /server/usr/users/b/charles/world/microlink/RCS/source/digCalibrate.c,v $ */

/*----------------------------------------------------------------------*/
/*                                               source/digCalibrate.c  */
/*----------------------------------------------------------------------*/

/* digCalibrate:                                                        */
/*                                                                      */
/* Digitiser / microlink calibration program                            */
/*                                                                      */
/* Author: John Grogan, Active Book Company, 23/1/91                    */

/* This program compiles to single program which can run when the       */
/*   microlink server has been installed. It uses the stylus-digitiser */
/*   stylus input by opening a stream to "/microlink/digitiser" and     */
/*   asks the user to touch the stylus against various points on the    */
/*   screen. It uses the data to compute an optimal linear              */
/*   transformation of the stylus co-ordinates to the screen            */
/*   co-ordinates. It programs this transformation into the microlink   */
/*   stylus-digitiser server using a SetInfo() call on the open         */
/*   digitiser stream. It then closes the stream and exits: The         */
/*   transformation remains programmed into the server so that if       */
/*   another application (e.g. Smalltalk) comes along and reads         */
/*   digitiser co-ordinates, it will receive the linearly transformed   */
/*   co-ordinates suitably scaled to the screen.                        */

/*----------------------------------------------------------------------*/
/*                                                        Header Files  */
/*----------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <nonansi.h>
#include <attrib.h>
#include <syslib.h>
#include <helios.h>
#include <math.h>
#include <ioevents.h>
#include <gsp.h>
#include <ctype.h>

#include "drawText.h"
#include "microlink/digitiser.h"
#include "drawp/public.h"

/*----------------------------------------------------------------------*/
/*                                                               Macros */
/*----------------------------------------------------------------------*/

#define MICROLINK_DEVICE "/microlink/digitiser"

#define CAL_DENOM    13      /* The larger this number, the nearer the */
			     /* screen edges the calibration crosses   */
			     /* will be.                               */

#define NUM_CAL_SAMPLES 20   /* Read in this many smaples at each      */
                             /* calibration cross & average them.      */

#define DEFAULTGLITCH 1000   /* Default glitch-elimination threshold     */
#define DEFAULTFILTER 0.0    /* Default jitter filtering parameter       */

#define INFO_DENOM   4096    /* 'denom' field in DigitiserInfo structure */

/*----------------------------------------------------------------------*/
/*                                                 Function prototypes  */
/*----------------------------------------------------------------------*/

double     readNum(char *prompt);
void       calibrate(void);
void       calReadCoord(char *prompt, int xpos, int ypos, int *digX, int *digY);
int        openMicrolink(void);
void       cls(void);
void       box(void);
void       writeText(char *string, int line);
void       drawString(int x, int y, char *string);
void       drawChar(int x, int y, char ch);
DpPixmap_t *readFile(const char *Name);

/*----------------------------------------------------------------------*/
/*                                                    Global variables  */
/*----------------------------------------------------------------------*/

Stream *microlink;
Port eventPort;

DpPixmap_t *screen, *charSet;
DpGraphicsContext_t *gc;

int cX1, cXX, cXY;
int cY1, cYX, cYY;
int xGlitch, yGlitch;
int xFilter, yFilter;

int vopt;

/*----------------------------------------------------------------------*/
/*                                                           main(...)  */
/*----------------------------------------------------------------------*/

int main(int argc, char *argv[])
{ 
  long          filterDenom;
  DigitiserInfo info;
  char          charsetPath[256];
  double        xf, yf;
  int c;

  /* Character set file is found at path CHARSETFILE by default */
  strcpy(charsetPath, CHARSETFILE);

  vopt=0;
  for (c=0; c<argc; c++)

    /* -v (verbose) option causes glitch / filter values to be requested
       rather than the default values being used, and some diagnostic 
       information to be printed out
       */
    if ( !strcmp(argv[c], "-v") )
      vopt=1;

  /* -c <pathname> option specifies a full pathname at which to find the 
     character set file used to generate the characters printed out using the
     blitter. If it is omitted, the default value <CHARSETFILE> is used.
  */
    else if ( !strcmp(argv[c], "-c") )
      strcpy(charsetPath, argv[++c]);

    else if ( !strcmp(argv[c], "-help") )
      { fprintf(stderr, "Usage: digCalibrate [-v] [-c <charset file>]\n");
	exit(0);
      }
      	

  if (!openMicrolink())
    { fprintf(stderr, "%s; failed to open microlink\n", argv[0]);
      exit(2);
    }

  /* Set the digitiser calibration values in the microlink server back
     to their 'identity' values, so that we get raw digitiser coordinates 
     rather than ones scaled to the screen size. These are the coords that
     we need for the calibration process. 
  */
  GetInfo(microlink,(void*)&info);
  info.cXX = 1;
  info.cXY = 0;
  info.cX1 = 0;
  info.cYX = 0;
  info.cYY = 1;
  info.cY1 = 0;
  info.denom = 1;
  SetInfo(microlink,(void*)&info,sizeof(DigitiserInfo));

  if ((screen=dpMapScreen(0, 0)) == NULL)
    { fprintf(stderr, "%s: failed to open screen for graphics operations.\n",
	               argv[0]);
      exit(2);
    }

  if ( (gc=dpConstructGC(0, screen->depth)) == NULL) 
      { fprintf(stderr, "%s: failed to create a graphics context.\n", argv[0]);
	exit(2);
      }

  gc->foreground=1;

  if ((charSet=readFile(charsetPath))==NULL)
    { fprintf(stderr, "Error: coudn't read character set bitmap file '%s'\n",
	      charsetPath);
      exit(2);
    }

  calibrate();

  GetInfo(microlink,(void*)&info);
  filterDenom = 1L<<info.filterBits;
  
  if (vopt)
   {     
     printf("\f");
     printf("*** Entry of other calibration parameters:\n\n");

     xGlitch=(int)readNum("Enter x glitch value (eg. 16) : ");

     yGlitch=(int)readNum("Enter y glitch value (eg. 16) : ");

     xf=readNum("Enter x filter value (eg. 0.2) : ");
     xFilter=(int)( xf * filterDenom);

     yf=readNum("Enter y filter value (eg. 0.2) : ");
     yFilter=(int)( yf * filterDenom);

     printf("\fSetInfo parameters are:\n\n");
     printf("cXX: %+9d, cXY: %+9d, cX1: %+9d\n", cXX, cXY, cX1);
     printf("cYX: %+9d, cYY: %+9d, cY1: %+9d\n", cYX, cYY, cY1);
     printf("denom: %+9d\n", INFO_DENOM);
     printf("xGlitch: %+5d, yGlitch: %+5d\n", xGlitch, yGlitch);
     printf("xFilter: %+5d, yFilter: %+5d, filterBits: %d\n", 
	    xFilter, yFilter, info.filterBits);
   }
  else
   { 
     xGlitch=DEFAULTGLITCH;
     yGlitch=DEFAULTGLITCH;
     xFilter=(int)(DEFAULTFILTER*filterDenom);
     yFilter=xFilter;
   }

  info.cXX   = cXX;
  info.cXY   = cXY;
  info.cX1   = cX1;
  info.cYX   = cYX;
  info.cYY   = cYY;
  info.cY1   = cY1;
  info.denom = INFO_DENOM;
  info.xGlitch = xGlitch;
  info.yGlitch = yGlitch;
  info.xFilter = xFilter;
  info.yFilter = yFilter;
  
  SetInfo(microlink, (void*)&info, sizeof(DigitiserInfo));

  Close(microlink);

  return 0;
}


/*----------------------------------------------------------------------*/
/*                                                              readNum */
/*----------------------------------------------------------------------*/

double readNum(char *prompt)
{
  char line[256];
  int c;

  while (1)
    {
      line[0]=0;
      while (!line[0])
	{ printf("%s", prompt); fflush(stdout);
	  gets(line);
	}
      for (c=0; (c<256) && line[c]; c++)
	if ( (!isalnum(line[c])) && (line[c]!='.') &&
	     (line[c]!='+') && (line[c]!='-') )
	  continue;

      if (c<256)
	break;
    }

  return atof(line);
}


/*----------------------------------------------------------------------*/
/*                                                      calibrate(...)  */
/*----------------------------------------------------------------------*/

void calibrate(void)
{
  int x0, y0, x1, y1, x2, y2, x3, y3;
  int x0d, y0d, x1d, y1d, x2d, y2d, x3d, y3d;

  double Sx, Sy, Sxy, Sxx, Syy, X1, Xx, Xy, Y1, Yx, Yy, D;
  double M[3][3];

  int n=4;   /* Number of calibration points */


  /* 1st cross in top-left corner */

  calReadCoord("Touch 1st calibration point", 1, 1, &x0, &y0);
  x0d=screen->sizeX/CAL_DENOM; 
  y0d=screen->sizeY/CAL_DENOM;

  /* 2nd cross in top-right corner */

  calReadCoord("Touch 2nd calibration point", CAL_DENOM-1, 1, &x1, &y1);
  x1d=(CAL_DENOM-1)*screen->sizeX/CAL_DENOM; 
  y1d=screen->sizeY/CAL_DENOM;

  /* 3rd cross in bottom-left corner */
  calReadCoord("Touch 3rd calibration point", 1, CAL_DENOM-1, &x2, &y2);
  x2d=screen->sizeX/CAL_DENOM; 
  y2d=(CAL_DENOM-1)*screen->sizeY/CAL_DENOM;

  /* 4th cross in bottom-right corner */
  calReadCoord("Touch 4th calibration point", 
            CAL_DENOM-1, CAL_DENOM-1, &x3, &y3);
  x3d=(CAL_DENOM-1)*screen->sizeX/CAL_DENOM; 
  y3d=(CAL_DENOM-1)*screen->sizeY/CAL_DENOM;
  
  if (vopt)
    {
      printf("\fPoints read are as follows:\n");
      printf("%20s  %20s","Screen co-ordinate","Digitiser co-ordinate\n");
      printf("       (%5d,%5d)              (%5d,%5d)\n",x0d,y0d,x0,y0);
      printf("       (%5d,%5d)              (%5d,%5d)\n",x1d,y1d,x1,y1);
      printf("       (%5d,%5d)              (%5d,%5d)\n",x2d,y2d,x2,y2);
      printf("       (%5d,%5d)              (%5d,%5d)\n",x3d,y3d,x3,y3);
    }

  /*                                                                      */
  /* Calibration coefficients calculation: See Charlie's documentation    */
  /*   for a derivation.                                                  */
  /*                                                                      */

  Sx=(double)x0+x1+x2+x3;
  Sy=(double)y0+y1+y2+y3;
  Sxy=(double)x0*y0+(double)x1*y1+(double)x2*y2+(double)x3*y3;
  Sxx=(double)x0*x0+(double)x1*x1+(double)x2*x2+(double)x3*x3;
  Syy=(double)y0*y0+(double)y1*y1+(double)y2*y2+(double)y3*y3;
  
  M[0][0]=Sxx*Syy-Sxy*Sxy;
  M[0][1]=Sxy*Sy-Sx*Syy;
  M[0][2]=Sx*Sxy-Sxx*Sy;
  M[1][0]=Sxy*Sy-Sx*Syy;
  M[1][1]=n*Syy-Sy*Sy;
  M[1][2]=Sx*Sy-n*Sxy;
  M[2][0]=Sx*Sxy-Sxx*Sy;
  M[2][1]=Sx*Sy-n*Sxy;
  M[2][2]=n*Sxx-Sx*Sx;
  
  X1=(double)x0d+x1d+x2d+x3d;
  Xx=(double)x0d*x0+(double)x1d*x1+(double)x2d*x2+(double)x3d*x3;
  Xy=(double)x0d*y0+(double)x1d*y1+(double)x2d*y2+(double)x3d*y3;
  
  Y1=(double)y0d+y1d+y2d+y3d;
  Yx=(double)y0d*x0+(double)y1d*x1+(double)y2d*x2+(double)y3d*x3;
  Yy=(double)y0d*y0+(double)y1d*y1+(double)y2d*y2+(double)y3d*y3;
  
  D=n*Sxx*Syy-n*Sxy*Sxy+2*Sx*Sy*Sxy-Sx*Sx*Syy-Sxx*Sy*Sy;
  
  cX1=(int)( (X1*M[0][0]+Xx*M[0][1]+Xy*M[0][2]) * INFO_DENOM / D );
  cXX=(int)( (X1*M[1][0]+Xx*M[1][1]+Xy*M[1][2]) * INFO_DENOM / D );
  cXY=(int)( (X1*M[2][0]+Xx*M[2][1]+Xy*M[2][2]) * INFO_DENOM / D );
  
  cY1=(int)( (Y1*M[0][0]+Yx*M[0][1]+Yy*M[0][2]) * INFO_DENOM / D );
  cYX=(int)( (Y1*M[1][0]+Yx*M[1][1]+Yy*M[1][2]) * INFO_DENOM / D );
  cYY=(int)( (Y1*M[2][0]+Yx*M[2][1]+Yy*M[2][2]) * INFO_DENOM / D );

}


/*----------------------------------------------------------------------*/
/*                                                   calReadCoord(...)  */
/*----------------------------------------------------------------------*/

/* Print out a prompt, draw a cross on the screen, and then loop round  */
/*    reading in stylus events until we get NUM_CAL_SAMPLES coordinate  */
/*    pairs with the tip down. Return the mean of all such coordinate   */
/*    pairs in *digX and *digY                                          */

void calReadCoord(char *prompt, int xpos, int ypos, int *digX, int *digY)
{
  BYTE    data[IOCDataMax];
  IOEvent *event;
  MCB     message;
  WORD    rc, buttons;
  int     i, j, got, eventsGot, count;
  int     x, y, tipDown, inProx;


  count=0; tipDown=0; inProx=0; *digX=0; *digY=0;
  
  cls ();
  writeText(prompt, 10);
  dpDrawLine(screen, gc, 
	     screen->sizeX * xpos / CAL_DENOM, 
	     screen->sizeY * ypos / CAL_DENOM - 20, 
	     screen->sizeX * xpos / CAL_DENOM, 
	     screen->sizeY * ypos / CAL_DENOM + 20 );
  dpDrawLine(screen, gc, 
	     screen->sizeX * xpos / CAL_DENOM - 20, 
	     screen->sizeY * ypos / CAL_DENOM, 
	     screen->sizeX * xpos / CAL_DENOM + 20, 
	     screen->sizeY * ypos / CAL_DENOM );

  i=0;
  while (i<NUM_CAL_SAMPLES)
    {
      message.Data        = &data[0];
      message.MsgHdr.Dest = eventPort;
      message.Timeout     = -1;

      rc = GetMsg(&message);
      if (rc < 0)
	{
	  printf("GetMsg failed %x\n", (int)rc);
	  continue; /* Assume it was timeout! */
	}

      got = message.MsgHdr.DataSize;
      eventsGot = got/sizeof(IOEvent);
      event = (IOEvent *)data;

      for (j = 0; j < eventsGot; ++j, ++event)
	{
	  x=event->Device.Stylus.X;
	  y=event->Device.Stylus.Y;
	  buttons=event->Device.Stylus.Buttons;

	  if (buttons==Buttons_Tip_Down)
	    tipDown=1;
	  else if (buttons==Buttons_Tip_Up)
            tipDown=0;
	  else if (buttons==Buttons_Into_Prox)
            inProx=1;
	  else if (buttons==Buttons_OutOf_Prox)
            inProx=0;

	  if (vopt)
	    {
	      if (tipDown)
		writeText("tipDown   ",5);
	      else
		writeText("tipUp     ",5);
          
	      if (inProx)
		writeText("inProx    ",6);
	      else
		writeText("outOfProx ",6);
	    }
	        

	  /* Only use coordinates read in whilst the stylus is pressed down
	     & in proximity */
	  if (tipDown && inProx)
	    { static char buf[100];
	      *digX+=x;
	      *digY+=y;
	      i++;
	      if (vopt)
		{ sprintf(buf,"Read Sample No. %d",i);
		  writeText(buf,0);
		}
	    }
	}
    }

  *digX /= NUM_CAL_SAMPLES;
  *digY /= NUM_CAL_SAMPLES;
  
  cls(); box();
}


/*----------------------------------------------------------------------*/
/*                                                  openMicrolink(...)  */
/*----------------------------------------------------------------------*/

int openMicrolink(void)
{ 
  Object *obj;


  /* Open() doesn't seem to work with a NULL context, so we have to do	*/
  /* a Locate() first.							*/

  obj = Locate(CurrentDir, MICROLINK_DEVICE);
  if (obj == NULL) 
    { fprintf(stderr,"Can't locate microlink\n");
      return 0;
    }

  microlink = Open(obj, NULL, O_ReadWrite);
  if (microlink == 0) 
    { fprintf(stderr, "Can't open microlink\n");
      return 0;
    }

  eventPort = EnableEvents(microlink, 1);
  if (eventPort == NullPort)
  {
    printf("EnableEvents on microlink stream failed\n");
    Close(microlink);
    return 0;
  }  
  return 1;
}


/*----------------------------------------------------------------------*/
/*                                                            cls(...)  */
/*----------------------------------------------------------------------*/

void cls(void)
{ int f;

  f=gc->function;
  gc->function=GXclear;
  dpFillRectangle(screen, gc, 0, 0, screen->sizeX, screen->sizeY);
  gc->function=f;

  box();
}  


/*----------------------------------------------------------------------*/
/*                                                            box(...)  */
/*----------------------------------------------------------------------*/

void box(void)
{
  /* Box the screen */
  dpDrawLine(screen, gc, 0, 0, screen->sizeX-1, 0);
  dpDrawLine(screen, gc, screen->sizeX-1, 0, screen->sizeX-1, screen->sizeY-1);
  dpDrawLine(screen, gc, screen->sizeX-1, screen->sizeY-1, 0, screen->sizeY-1);
  dpDrawLine(screen, gc, 0, screen->sizeY-1, 0, 0);

}


/*----------------------------------------------------------------------*/
/*                                                            writeText */
/*----------------------------------------------------------------------*/

void writeText(char *string, int line)
{  drawString(5, line*LINEINC+10, string);  }

/*----------------------------------------------------------------------*/
/*                                                           drawString */
/*----------------------------------------------------------------------*/

void drawString(int x, int y, char *string)
{  while (*string)
   {  drawChar(x, y, *string++);
      x+=8;
}  }

/*----------------------------------------------------------------------*/
/*                                                             drawChar */
/*----------------------------------------------------------------------*/

void drawChar(int x, int y, char ch)
{
  dpCopyArea(charSet, screen, gc, 0, (ch-32)*8, 8, 8, x, y);
}


/*----------------------------------------------------------------------*/
/*                                                             readFile */
/*----------------------------------------------------------------------*/

/* This function is by Pete Cockerell & turns the input 
   character set file into a suitable pixmap.
*/

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
			*ptr++ = (char)strtol(str, &str, 0);
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

