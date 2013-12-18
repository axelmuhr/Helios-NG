/* This convolution program is based on the example given in 
 * "The Helios parallel programming tutorial". 
 *  Publisher: D.S.L
 *  Order code: H09012
 */  
#include <stdio.h>
#include <posix.h>
#include <stdlib.h>
#include <string.h>
#include <helios.h>
#include <sem.h>
#include <nonansi.h>
#include <attrib.h>
#include <X11/Xlib.h>


#define eq ==
#define ne !=


BYTE 		**display;
int 		height;
int 		width;
int 		number_of_slaves;
Display   	*dpy;
Window     	win;
XImage    	*image;
int        	screen;

void initialise_console(void);
void initialise_display(char *,int *,int *);
void initialise_slaves(void);
void display_picture(char *);
void draw_pix(void);
int  menu(void);
void do_convolution(int);

int main(int argc, char **argv)
{   

  int choice;

  if (argc != 3)
  { 
    fprintf(stderr, "Invalid number of arguments to master component.\n");
    exit(1);
  }

  number_of_slaves = atoi(argv[2]);
 
  initialise_console();

  initialise_display(argv[1],&height,&width);
  initialise_slaves();
  display_picture(argv[0]);
  forever
  { 
     choice = menu();
     do_convolution(choice);
     draw_pix();
  }
  XDestroyImage(image);
  XDestroyWindow(dpy, win);
  XCloseDisplay(dpy);
  return(0);   
}


  
void initialise_display(char *filename, int *height, int *width)
{ 
  FILE *data;
  int val;
  char *cur;
  int i, len, size;
  
  data = fopen(filename, "rb");
  if (data == Null(FILE))
  { 
    fprintf(stderr, "Cannot open %s\n", filename);
    exit(1);
  }
  
  fread((void *) &i, sizeof(int), 1, data);
  *height = (i >> 16) & 0x0FFFF;
  *width  = i & 0x0FFFF;
  
  size = *height * *width;
  
  if ((display[0]= malloc(size)) eq Null(char))
  {
    fprintf(stderr, "Failed to allocate memory for display, cannot allocate %d bytes\n",size); 
    exit(1);
  }
  
  cur = display[0];
  for (val = fgetc(data); val != EOF; val = fgetc(data))
  { 
    if (val == 0x00FF)
    { 
      len =256 * fgetc(data);
      len += fgetc(data);
      val = fgetc(data);
      for (i=0 ; i < len ; i++ )
        *cur++ = val;
    }
    else
      *cur++ = val;
  }
  fclose(data);
  for ( i = 1; i < *height; i++)
   display[i] = display[i-1] + *width;
   
}

typedef struct slave {
	Semaphore	wait_for_job;
	BYTE		*picture;
	BYTE		*top_bit;
	BYTE		*bottom_bit;
	int		input_stream;
	int		output_stream;
} slave;

typedef struct slave_info {
	WORD		height;
	WORD		width;
} slave_info;

slave      *slave_array;
slave_info slaveinfo;

void interact_with_a_slave(int slaveno);

Semaphore wait_for_slaves;		
int  slave_error = FALSE;
int  current_fn;

void initialise_slaves(void)
{ 
  int i;

  slaveinfo.width  = width;
  slaveinfo.height = height / number_of_slaves;
  
  slave_array = (slave *) malloc(number_of_slaves * sizeof(slave));
  if (slave_array == Null(slave))
  { 
    fprintf(stderr, "Failed to start up slaves.\n");
    exit(1);
  }

  InitSemaphore(&wait_for_slaves, 0);
  for (i = 0; i < number_of_slaves; i++)
  if (Fork(5000, interact_with_a_slave, sizeof(int), i) == 0)
  { 
    fprintf(stderr, "Failed to start up slaves.\n");
    exit(1);
  }

  for (i = 0; i < number_of_slaves; i++)
    Wait(&wait_for_slaves);     

  if (slave_error)
  { 
    fprintf(stderr, "Failed to communicate with slave %d.\n", slave_error - 1);
    exit(1);
  }
}


void interact_with_a_slave(int slave_no)
{ slave *myslave = &(slave_array[slave_no]);
  int   result, temp;
  Delay(OneSec);

#define slave_fail(a) { slave_error = a; Signal(&wait_for_slaves); return; }
  
  InitSemaphore(&(myslave->wait_for_job), 0);
  myslave->input_stream  = 4 + (2 * slave_no);
  myslave->output_stream = 5 + (2 * slave_no);
  myslave->picture       = display[slave_no * slaveinfo.height];

  myslave->top_bit       = (slave_no == 0) ? display[0] : 
                           display[slave_no * slaveinfo.height - 4];

  myslave->bottom_bit    = (slave_no == number_of_slaves - 1) ?
                           display[height - 4] :
                           display[(slave_no + 1) * slaveinfo.height];

  result = write(myslave->output_stream, (char *) &slaveinfo, sizeof(slave_info));

  if (result < sizeof(slave_info))
    slave_fail(slave_no + 1);

  result = read(myslave->input_stream, (char *) &temp, sizeof(int));

  if (result < sizeof(int))
    slave_fail(slave_no + 1);
   
  if (!temp)
    slave_fail(slave_no + 1);

  result = write(myslave->output_stream, myslave->picture,
                 slaveinfo.height * slaveinfo.width);

  if (result < slaveinfo.height * slaveinfo.width)
    slave_fail(slave_no + 1);

  Signal(&wait_for_slaves);
  
  forever
  { 
    Wait(&myslave->wait_for_job);

    result = write(myslave->output_stream, (char *) &current_fn, sizeof(int));
    if (result < sizeof(int))
      slave_fail(slave_no + 1);

    result = write(myslave->output_stream, myslave->top_bit, 4 * width);
    if (result < (4 * width))
      slave_fail(slave_no + 1);

    result = write(myslave->output_stream, myslave->bottom_bit, 4 * width);
    if (result < (4 * width))
      slave_fail(slave_no + 1);

    result = read(myslave->input_stream, myslave->picture,
                   (slaveinfo.height * width));
    if (result < (slaveinfo.height * width))
      slave_fail(slave_no + 1);
      
    Signal(&wait_for_slaves);
  }

#undef slave_fail
}

void do_convolution(int choice)
{ 
  int i;

  printf("\nDoing convolution %d.\n", choice);
  
  current_fn = choice;  
  for (i = 0; i < number_of_slaves; i++)
    Signal(&(slave_array[i].wait_for_job));

  for (i = 0; i < number_of_slaves; i++)
    Wait(&wait_for_slaves);
   
  if (slave_error)
  { 
    fprintf(stderr, "Failed to communicate with slave %d.\n", slave_error - 1);
    exit(1);
  }
}

void initialise_console(void)
{ Attributes attr;
  if (GetAttributes(Heliosno(stdin), &attr) < 0)
  { 
    fprintf(stderr, "Failed to get console attributes.\n");
    exit(1);
  }
  AddAttribute(&attr, ConsoleRawInput);
  if (SetAttributes(Heliosno(stdin), &attr) < 0)
  { 
    fprintf(stderr, "Failed to set console attributes.\n");
    exit(1);
  }
   
  setvbuf(stdin, Null(char), _IONBF, 0);
}

int menu(void)
{ 
  int x;

  putchar('\f');
  printf("1) normal averaging\n");
  printf("2) biassed averaging\n");
  printf("3) emphasise horizontals\n");
  printf("4) emphasise verticals\n");
  printf("5) emphasise diagonals\n");
  printf("Q) to exit.\n");
  printf("\nYour Choice ? ");
  fflush(stdout);

  forever
  { x = getchar();
    if ((x == 'q') || (x == 'Q'))
      exit(0);
    if ((x < '1') || (x > '5'))
      { putchar('\b'); putchar(' '); putchar('\b'); fflush(stdout); continue; }
    return(x - '0');
   }
}

void display_picture(char *name)
{
  Colormap   	cmap;
  
  if ((dpy =XOpenDisplay(Null(char))) eq Null(Display))
  {
    fprintf(stderr, "Unable to open default display.\n");
    exit(1);
  }
  
  screen = DefaultScreen(dpy);
  
  if ((win = XCreateSimpleWindow( dpy, RootWindow(dpy, screen),
      0, 0, width, height, 1, BlackPixel(dpy, screen),
      BlackPixel(dpy, screen))) eq NULL)
      {
      XCloseDisplay(dpy); free(display[0]);
      fprintf(stderr, "Unable to open window.\n");
      exit(1);
      }
   
      {
      XSizeHints hints;
  
      hints.width      = width;
      hints.min_width  = width;
      hints.max_width  = width;
      hints.height     = height;
      hints.min_height = height;
      hints.max_height = height;
      hints.flags      = PSize + PMinSize + PMaxSize;
    
      XSetNormalHints(dpy, win, &hints);
      }
  
      XSelectInput(dpy, win, StructureNotifyMask);

      XStoreName(dpy, win, name);

  
      {
      int i;
      XColor color;
      XVisualInfo vinfo, *pvinfo;
    
      vinfo.screen = screen;
      vinfo.class  = PseudoColor;
    
      pvinfo = XGetVisualInfo(dpy, VisualScreenMask | VisualClassMask,         
      &vinfo, &i);
              
      cmap = XCreateColormap(dpy, RootWindow(dpy, screen),
             pvinfo[0].visual, AllocAll);
      XFree ((char *)pvinfo);
    
      for (i = 0; i < DisplayCells(dpy, screen); i++)
      {
        color.pixel = i;
        color.red = color.green = color.blue = i << 8;
        color.flags = DoRed | DoGreen | DoBlue;
        XStoreColor(dpy, cmap, &color);
      }
     
      XSetWindowColormap(dpy, win, cmap);
      }
      XMapWindow(dpy, win);
  
      {
        XEvent e;
        while (XNextEvent( dpy, &e), e.type != MapNotify);
      }
 
      draw_pix();
 
} 

void draw_pix()
{
  image = XCreateImage(dpy, DefaultVisual(dpy, screen), 8,
              ZPixmap, 0, display[0], width, height, BitmapPad(dpy),0);
           
  XPutImage(dpy, win ,DefaultGC(dpy, screen), image,
           0,0,0,0, width, height);
           
  XSync(dpy, FALSE);
}

