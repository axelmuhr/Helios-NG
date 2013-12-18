#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>

#define eq ==
#define ne !=

char *read_image(char *, int *, int *);
void initialise_display(char *, int *, int *);
void display_picture(char *);

BYTE 	   **display;
int        height, width;

int main(int argc, char **argv)
{

  char      *picture;
  int       exit(int);
    
  if (argc ne 2)
   { 
   fprintf(stderr, "Usage : show <picture>\n");
   exit(1); 
   }


  initialise_display(argv[1], &height, &width);  
  picture = display[0];
  display_picture(argv[1]);
  return(0);
    
}

char *read_image(char *filename, int *height, int *width)
{ 
  FILE *data;
  int val; char *buf, *cur;
  int i, len, size;
  char *malloc(int);
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
  
  if ((buf = malloc(size)) eq Null(char))
   {
    fprintf(stderr, "Cannot allocate %d bytes\n", size);
    exit(1);
   }
   

  cur = buf;
  for (val = fgetc(data); val != EOF; val = fgetc(data) )
   { if (val == 0x00FF)
      { len = 256 * fgetc(data);
        len += fgetc(data);
        val = fgetc(data);
        for (i = 0; i < len; i++)
         *cur++ = val;
      }
     else
      *cur++ = val;
   }
  
  fclose(data);
  return(buf);
}
	
void initialise_display(char *filename, int *height, int *width)
{ 
  FILE *data;
  int val;
  char *cur;
  int i, len, size;
  char *malloc(int);
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
    fprintf(stderr, "Failed to allocate memory for display\n"); 
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
  
void display_picture(char *name)
{
  Display   *dpy;
  Window     win;
  int        screen;
  XImage    *image;
  Colormap   cmap;
  int free (char *);
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
  
      image = XCreateImage(dpy, DefaultVisual(dpy, screen), 8,
              ZPixmap, 0, display[0], width, height, BitmapPad(dpy),0);
           
      XPutImage(dpy, win ,DefaultGC(dpy, screen), image,
           0,0,0,0, width, height);
           
          
      XSync(dpy, FALSE);
                    
      Delay(20 * OneSec);
  	     
      XDestroyImage(image);
 
      XDestroyWindow(dpy, win);
 
      XCloseDisplay(dpy);
 
} 


