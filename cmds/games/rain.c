#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <posix.h>
#include <helios.h>
#include <syslib.h>

#define	eq	==
#define	ne	!=

static	int	x_coords[5];
static	int	y_coords[5];

#define	Screen_Height	25
#define Screen_Width	80

static void draw(int);
static void undraw(int);
static void outputch(int);
static void outputstr(char *);
static void moveto(int, int);
static void flush(void);

int main(void)
{ int i;

  srand(time(NULL));

  outputch('\f');
    
  for (i = 0; i < 5; i++)
   x_coords[i] = y_coords[i] = -1;
   
  for (;;)
   { for (i = 4; i >= 0; i--)
      { if (x_coords[i] ne -1) undraw(i);
	x_coords[i] = x_coords[i-1];
	y_coords[i] = y_coords[i-1];
	if (i eq 4) continue;
	if (i eq 0)
	 { x_coords[0] = (rand() % (Screen_Width - 6)) + 3;
           y_coords[0] = (rand() % (Screen_Height - 6)) + 3;
	 }
	if (x_coords[i+1] ne -1) draw(i+1);
      }
     draw(0);
     moveto(Screen_Height, 1);
     flush();
     Delay(OneSec / 10);
   }
}

static void draw(int code)
{ switch(code)
   { case 0 :	moveto(y_coords[0], x_coords[0]);
		outputch('.');
              	break;
     case 1 :	moveto(y_coords[1], x_coords[1]);
     		outputch('o');
     		break;
     case 2 :	moveto(y_coords[2], x_coords[2]);
     		outputch('O');
     		break;
     case 3 :	moveto(y_coords[3]-1, x_coords[3]);
     		outputch('_');
     		moveto(y_coords[3], x_coords[3] - 1);
     		outputstr("|.|");
     		moveto(y_coords[3] + 1, x_coords[3]);
     		outputch('-');
     		break;
     case 4 :	moveto(y_coords[4] - 2, x_coords[4]);
     		outputch('_');
     		moveto(y_coords[4] - 1, x_coords[4] - 1);
     		outputstr("/ \\");
     		moveto(y_coords[4], x_coords[4] - 2);
     		outputstr("| O |");
     		moveto(y_coords[4] + 1, x_coords[4] - 1);
     		outputstr("\\ /");
     		moveto(y_coords[4] + 2, x_coords[4]);
     		outputch('-');
     		break;
   }
}

static void undraw(int code)
{ switch(code)
   { case 0 :	moveto(y_coords[0], x_coords[0]);
		outputch(' ');
              	break;
     case 1 :	moveto(y_coords[1], x_coords[1]);
     		outputch(' ');
     		break;
     case 2 :	moveto(y_coords[2], x_coords[2]);
     		outputch(' ');
     		break;
     case 3 :	moveto(y_coords[3]-1, x_coords[3]);
     		outputch(' ');
     		moveto(y_coords[3], x_coords[3] - 1);
     		outputstr("   ");
     		moveto(y_coords[3] + 1, x_coords[3]);
     		outputch(' ');
     		break;
     case 4 :	moveto(y_coords[4] - 2, x_coords[4]);
     		outputch(' ');
     		moveto(y_coords[4] - 1, x_coords[4] - 1);
     		outputstr("   ");
     		moveto(y_coords[4], x_coords[4] - 2);
     		outputstr("     ");
     		moveto(y_coords[4] + 1, x_coords[4] - 1);
     		outputstr("   ");
     		moveto(y_coords[4] + 2, x_coords[4]);
     		outputch(' ');
     		break;
   }
}

static	char	buffer[256];
static	int	index = 0;

static void outputch(int x)
{ buffer[index++] = x;
}

static void outputstr(char *s)
{ while (*s ne '\0') buffer[index++] = *s++;
}

static void moveto(int y, int x)
{ char	buf[16];
  sprintf(buf, "%c%d;%dH", 0x9B, y, x);
  outputstr(buf);
}

static void flush()
{ write(1, buffer, index);
  index = 0;
}

