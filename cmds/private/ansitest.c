/******************************************************************************
***              ANSI terminal emulation test program                       ***
***                                                                         ***
***  Author : BLV, 4.11.88                                                  ***
******************************************************************************/
#ifdef __TRAN
static char *rcsid = "$Header: /users/nickc/RTNucleus/cmds/private/RCS/ansitest.c,v 1.5 1994/03/08 12:17:35 nickc Exp $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <syslib.h>
#include <attrib.h>
#include <nonansi.h>
#define ne !=
#define eq ==

void wait_for_user(void);
void main_menu(void);
void move_to(int y, int x);
void output_text(char *text);
#define clear_screen() putchar('\f')

int rows, cols;

int main(void)
{ Attributes attr;
  WORD result;
  
  printf("\f\t\tANSI Terminal Emulation Test\n");
  printf(  "\t\t============================\n\n\n");
  
  setvbuf(stdin, NULL, _IONBF, 0);
  if ((result = GetAttributes(Heliosno(stdout), &attr)) < 0)
   { printf("Failed to get stdout attributes : %lx. Exiting.\n", result);
     exit((int)result);
   }

  printf("Window size : %d rows, %d columns.\n\n", attr.Min, attr.Time);
  rows = attr.Min; cols = attr.Time;
  AddAttribute(&attr, ConsoleRawOutput);
  RemoveAttribute(&attr, ConsolePause);
  RemoveAttribute(&attr, ConsoleEcho);
  
  if ((result = SetAttributes(Heliosno(stdout), &attr)) < 0)
   { printf("Failed to set stdout attributes : %lx. Exiting.\n", result);
     exit((int)result);
   }
   
  if ((result = GetAttributes(Heliosno(stdin), &attr)) < 0)
   { printf("Failed to get stdin attributes : %lx. Exiting.\n", result);
     exit((int)result);
   }

  AddAttribute(&attr, ConsoleRawInput);
  RemoveAttribute(&attr, ConsolePause);
  RemoveAttribute(&attr, ConsoleEcho);
  
  if ((result = SetAttributes(Heliosno(stdin), &attr)) < 0)
   { printf("Failed to set stdin attributes : %lx. Exiting.\n", result);
     exit((int) result);
   }
   
  wait_for_user();
  main_menu();
  printf("\r\n\n\n");
}

/**
*** Some utility routines.
**/
void wait_for_user(void)
{ printf("\r\nPress any key to continue...");
  fflush(stdout);
  (void) (getchar)();
  printf("\r\n");
}

void output_text(char *text)
{ clear_screen();
  puts(text);
  wait_for_user();
}

/**
*** The main menu system.
**/

int current_menu = 1;
typedef struct menu_item
{ STRING    choice;
  VoidFnPtr routine;
} menu_item;

typedef menu_item menu[10];

void fill_screen(void);
void ring_bell(void);
void overflow_line(void);
void line_feeds(void);
void carriage_returns(void);
void move_tos(void);
void backspace(void);
void vertical_tab(void);
void form_feed(void);
void horizontal_tab(void);

void insert_chars(void);
void cursor_up(void);
void cursor_down(void);
void cursor_left(void);
void cursor_right(void);
void cursor_nextline(void);
void cursor_preceeding(void);
void erase_display(void);
void erase_line(void);
void insert_line(void);

void delete_line(void);
void delete_char(void);
void scroll_up(void);
void scroll_down(void);
void rendition(void);

#define menu_fill { Null(char), (VoidFnPtr) NULL }

menu menu_list[] =
{	{	{ "Fill screen",	&fill_screen },
		{ "Ring bell",		&ring_bell },
 		{ "Line overflow",	&overflow_line },
		{ "Line Feeds",		&line_feeds },
		{ "Carriage returns",	&carriage_returns },
		{ "Horizontal tab",	&horizontal_tab },
		{ "Backspace",		&backspace },
		{ "Vertical_tab",	&vertical_tab },
		{ "Form feed",		&form_feed },
		menu_fill
	},
	{	{ "Move To's",		&move_tos },
		{ "Insert chars",	&insert_chars },
		{ "Cursor up",		&cursor_up },
		{ "Cursor_down",	&cursor_down },
		{ "Cursor_left",	&cursor_left },
		{ "Cursor_right",	&cursor_right },
		{ "Cursor next line",	&cursor_nextline },
		{ "Cursor previous line",	&cursor_preceeding },
		{ "Insert line",		&insert_line },
		menu_fill
	},
	{	{ "Delete line",	&delete_line },
		{ "Erase to end of screen",	&erase_display },
		{ "Erase to end of line",	&erase_line	},
		{ "Delete char",	&delete_char },
		{ "Scroll up",		&scroll_up },
		{ "Scroll down",	&scroll_down },
		{ "Set rendition",	&rendition },
		menu_fill,
		menu_fill,
		menu_fill
	}
};
#define Max_Menu 3

void main_menu(void)
{ menu_item *current_list;
  int i, choice;
   
  forever
   { clear_screen(); 
     current_list = &((menu_list[current_menu-1])[0]);
     for (i = 0; current_list[i].choice ne Null(char); i++)
      printf("\t%2d)\t%s\r\n",i+1, current_list[i].choice);

     printf("\n\tP for previous menu, N for next, Q to exit ? ");
     fflush(stdout);
     choice = getchar();      
     switch (choice)
      { case 'Q' :
        case 'q' : return;
        
        case 'P' :
        case 'p' : if (current_menu > 1) current_menu--;
        	   continue;

	case 'N' :
	case 'n' : if (current_menu < Max_Menu) current_menu++;
		   continue;

	case '1' : case '2' : case '3' : case '4' : case '5' :
	case '6' : case '7' : case '8' : case '9' :
		   choice -= '1';
		   if (current_list[choice].choice ne Null(char))
		     { printf("\r\n");
		       (*(current_list[choice].routine))();
		       wait_for_user();
		     }
		     
	default  :
		   continue;
      }		    
   }
}

/**
*** Here are some useful routines for the actual tests.
**/
void fill_entire_screen()
{ int x, y;

  clear_screen();
  for (y = 0; y < rows; y++)
   { for (x = 0; x < cols; x++)
      putchar( ((x + y) % 10) + '0');
     if (y < (rows - 1))
      { putchar('\n'); putchar('\r'); }
   }
  fflush(stdout);
}

void fill_line()
{ int x;

  clear_screen();
  for (x = 0; x < cols; x++)
    putchar((x % 10) + '0');
  putchar('\r');
  fflush(stdout);
}

void move_to(int y, int x)
{ printf("%c%d;%dH", 0x009B, y+1, x+1);
  fflush(stdout);
}

/**
*** The individual test routines.
**/

void fill_screen()
{ PRIVATE char *text ="\
This routine should fill the entire screen with numbers. The numbers\r\n\
allow a quick check that no rows or columns are missed out.\r\n\
It is particularly important that the bottom-right corner is filled\r\n\
without scrolling. Check that the top-left corner is 0, i.e. that the\r\n\
screen has not scrolled.\
";
  
  output_text(text);
  fill_entire_screen();
  Delay(5 * OneSec);
}

void ring_bell()
{ PRIVATE char *text = "\
This routine should ring the bell 5 times, at 2 second intervals. It should\r\n\
be apparent to the user either as a sound or as a flashing screen.\r\n\
";
  int x;
  
  output_text(text);
  for (x = 0; x < 5; x++)
   { putchar(0x07); fflush(stdout); Delay(2 * OneSec); }
   
}

void overflow_line(void)
{ PRIVATE char *text = "\
This routine attempts to write 20 characters too many on the current\r\n\
line, without issuing a carriage return or linefeed. The extra characters\r\n\
should be put on the next line automatically, i.e. there is implicit\r\n\
wrapping.The first character on the new line should be a 0.\r\n\
";
   int x;
   
   output_text(text);
   putchar('\r'); putchar('\n');
   for (x = 0; x < cols; x++) putchar('*');
   for (x = 0; x < 20; x++) putchar((x % 10) + '0');
   fflush(stdout);
}

  
void line_feeds()
{ PRIVATE char *text = "\
This routine should fill the screen as in test 1. It should then draw a\r\n\
diagonal line of spaces from the top left corner, by outputting spaces\r\n\
followed by linefeeds. On reaching the bottom it will pause for 5 seconds\r\n\
without issuing another linefeed, and then continue with another 9 \r\n\
linefeed/char pairs, which should cause the screen to scroll up leaving\r\n\
a 9 in the top left corner.\r\n\
";
  int x;
  
  output_text(text);
  fill_entire_screen();
  move_to(0,0);
  for (x = 0; x < rows; x++)
   { putchar(' '); if (x < (rows-1)) putchar('\n'); }
  fflush(stdout);
  Delay(5 * OneSec);
  for (x = 0; x < 9; x++)
    { putchar('\n'); putchar('*'); }
  fflush(stdout);
  Delay(5 * OneSec);
}

void carriage_returns()
{ PRIVATE char *text = "\
This routine will fill the entire screen. It then moves to the top line,\r\n\
and outputs lots of text which should stay on that line. Then it moves\r\n\
to the bottom line and does the same.\r\n\
";

  output_text(text);
  fill_entire_screen();
  move_to(0, 0);
  printf("This is line 1.\r");
  printf("This is line 2.\r");
  printf("This is line 3.\r");
  printf("This is line 4.\r");
  printf("This is line 5.\r");
  printf("This is line 6.\r");
  printf("This is line 7.\r");
  printf("This is line 8.\r");
  printf("This is line 9.\r");
  printf("This is line 10, and should be entirely on the top line.      ");
  move_to(rows - 1, 0);
  printf("This is line 1.\r");
  printf("This is line 2.\r");
  printf("This is line 3.\r");
  printf("This is line 4.\r");
  printf("This is line 5.\r");
  printf("This is line 6.\r");
  printf("This is line 7.\r");
  printf("This is line 8.\r");
  printf("This is line 9.\r");
  printf("This is line 10, and should be entirely on the bottom line.      ");
  fflush(stdout);
  Delay(5 * OneSec);
}

void backspace(void)
{ PRIVATE char *text = "\
This routine fills the top line. It then moves to the centre of the line,\r\n\
outputs an 'a', does a backspace, outputs a 'b' which should overwrite\r\n\
the 'a', does two backspaces, etc. Eventually the backspaces should reach\r\n\
the left-hand side, where they should stop.\r\n\
";
  int x, y;
  
  output_text(text);
  fill_line();
  move_to(0, (cols / 2));
  for (x = 1; x <= 26; x++)
   { putchar('a' + x - 1);
     for (y = 0; y < x; y++) putchar('\b');
   }
  putchar('\n');
  fflush(stdout);
}

void vertical_tab(void)
{ PRIVATE char *text = "\
This routine moves to the cursor to the bottom left hand corner. It then\r\n\
outputs pairs of asterixes and vertical tabs, which should cause a\r\n\
diagonal trail. On reaching the top of the screen the vertical tabs should\r\n\
have no effect, and the asterixes will be displayed horizontally.\r\n\
Some of these may overflow on the next line, in which case the top left\r\n\
corner should be blank.\r\n\
";
  int x;

  output_text(text);
  clear_screen();
  move_to(rows-1, 0);
  for (x = 0; x < 100; x++)
   { putchar('*'); putchar('\v'); }
  fflush(stdout);
}

void form_feed(void)
{ PRIVATE char *text = "\
This routine should just fill the entire screen, and then issue a form\r\n\
feed character. The form feed should clear the entire screen leaving the\r\n\
cursor in the top-left corner, where a single asterix should appear.\r\n\
";

  output_text(text);
  fill_entire_screen();
  putchar('\f');
  putchar('*');
  fflush(stdout);
}

void horizontal_tab(void)
{ PRIVATE char *text = "\
This routine displays a ruler line at the top of the screen, and then\r\n\
outputs characters + tabs on the next line. Then, on the next line, it\r\n\
outputs some asterixes, a very large number of tabs and some more\r\n\
asterixes. The last lot should appear on the next line, because tabs\r\n\
wrap around just like text.\r\n\
";

  output_text(text);
  fill_line();
  putchar('\n');
  printf("*\t**\t***\t****\t*****\t******\t*******\t********\t*********\t");
  printf("\r\n***\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t***");
  fflush(stdout);
}

void move_tos()
{ PRIVATE char *text = "\
This routine takes two stages. First it puts an asterix in all four\r\n\
corners, and waits for the user. Then it tries to move beyond these\r\n\
corners, illegally. The terminal should adjust the Goto's to the four\r\n\
corners, i.e. the display should be the same.\r\n\
";

  output_text(text);
  clear_screen();
  move_to(0, 0);
  putchar('*');
  move_to(0, cols - 1);
  putchar('*');
  move_to(rows - 1, 0);
  putchar('*');
  move_to(rows - 1, cols - 1);
  putchar('*');
  move_to(rows / 2, 0);
  wait_for_user();
  clear_screen();
  move_to(-1, -1);
  putchar('+');
  move_to(-1, 0);
  putchar('-');
  move_to(0, -1);
  putchar('*');
  move_to(-1, cols + 20);
  putchar('+');
  move_to(-1, cols);
  putchar('-');
  move_to(0, cols + 20);
  putchar('*');
  move_to(rows, -1);
  putchar('+');
  move_to(rows-1, -1);
  putchar('-');
  move_to(rows, 0);
  putchar('*');
  move_to(rows, cols+10);
  putchar('+');
  move_to(rows-1, cols+10);
  putchar('-');
  move_to(rows, cols-1);
  putchar('*');
  move_to(rows/2, 0);
}

void insert_chars(void)
{ PRIVATE char *text = "\
This routine fills the entire screen. Then on the second line, a couple\r\n\
of columns in, it inserts a single character which should manifest\r\n\
itself as a space, with the rest of the line shifted along and the end\r\n\
lost. Two lines down it inserts 10 spaces, and two lines down it inserts\r\n\
a 1000 which should delete the rest of the line.\r\n\
";

  output_text(text);
  fill_entire_screen();
  move_to(1, 5);
  printf("%c@\n\n%c10@\n\n%c1000@", 0x009B, 0x009B, 0x009B);
  fflush(stdout);
}

void cursor_up(void)
{ PRIVATE char *text = "\
This routine moves to the bottom left corner, outputs an asterix,\r\n\
moves the cursor up 1, outputs another asterix, moves the cursor\r\n\
up 2, etc. When the top of the screen is reached the cursor should\r\n\
stick.\r\n\
";
  int x;
  
  output_text(text);
  clear_screen();
  move_to(rows-1, 0);
  for (x = 1; x < 20; x++)
   printf("*%c%dA", 0x009B, x);
  fflush(stdout);
}

void cursor_down(void)
{ PRIVATE char *text = "\
This routine draws an asterix in the top left corner, moves the cursor\r\n\
down 1, outputs another asterix, moves the cursor down 2, etc. When the\r\n\
bottom of the screen is reached the cursor should stick.\r\n\
";
  int x;
  
  output_text(text);
  clear_screen();
  for (x = 1; x < 20; x++)
    printf("*%c%dB", 0x009B, x);
  fflush(stdout);
  Delay(5 * OneSec);
}

void cursor_left(void)
{ PRIVATE char *text = "\
This routine should move the cursor to the right-hand side of the screen,\r\n\
output an 'a', move left 1, output a 'b' which should overwrite the 'a',\r\n\
move left 2, etc. When the cursor reaches the left-hand edge it should\r\n\
stick.\r\n\
";
  int x;
  
  output_text(text);
  clear_screen();
  move_to(rows / 2, cols - 1);
  for (x = 1; x < 20; x++)
   { putchar('a' + x - 1);
     printf("%c%dD", 0x009B, x);
   }
  fflush(stdout);
  
}

void cursor_right(void)
{ PRIVATE char *text = "\
This routine should move the cursor to the left-hand side of the screen,\r\n\
output a 'a', move right 1, output a 'b', move right 2, etc. On reaching\r\n\
the right-hand side, moving the cursor right any further will mean moving\r\n\
the cursor to the right-most place, possibly moving it back in a manner of\r\n\
speaking.\r\n\
";
  int x;
  
  output_text(text);
  clear_screen();
  move_to(rows / 2, 0);
  for (x = 1; x < 20; x++)
   { putchar('a' + x - 1);
     printf("%c%dC", 0x009B, x);
   }
  fflush(stdout);
}

void cursor_nextline(void)
{ PRIVATE char *text = "\
This routine outputs some text, moves the cursor to the next line, \r\n\
outputs some more text, moves the cursor down two lines, etc. On reaching\r\n\
the bottom the screen should scroll.\r\n\
";
  int x;
  
  output_text(text);
  clear_screen();
  for (x = 1; x < 10; x++)
   { printf("Here is some text%c%dE", 0x009B, x);
     fflush(stdout);
     Delay(OneSec / 2);
   }
}

void cursor_preceeding(void)
{ PRIVATE char *text = "\
This routine moves the cursor to the bottom left corner, outputs some\r\n\
text, moves to the preceeding line, outputs more text, moves two\r\n\
preceeding lines, outputs more text, etc. At the top of the screen\r\n\
the text should stick.\r\n\
";
  int x;
  
  output_text(text);
  clear_screen();
  move_to(rows - 1, 0);
  for (x = 1; x < 10; x++)
   printf("Some silly text%c%dF", 0x009B, x);
  fflush(stdout);
  Delay(5 * OneSec);
}

void erase_display(void)
{ PRIVATE char *text = "\
This routine fills the entire screen. Then it moves to near the bottom\r\n\
corner, erases to the end of screen, moves up and left a bit, erases\r\n\
the rest of screen again, etc. Each time I output a single character\r\n\
to check that the cursor position has not moved. When the top of the\r\n\
screen is reached there is a delay, then the cursor is moved to the\r\n\
top left and there is another erase_display.\r\n\
";
  int x;
  
  output_text(text);
  fill_entire_screen();
  for (x = rows - 3; x >= 0; x -= 1)
   { move_to(x, cols + x - rows);
     printf("%cJ*", 0x009B);
     fflush(stdout);
     Delay(OneSec / 2);
   }
  Delay(3 * OneSec);
  move_to(0, 0);
  printf("%cJ*", 0x009B);
  fflush(stdout);
}

void erase_line(void)
{ PRIVATE char *text = "\
This routine fills the entire screen and moves to the bottom right\r\n\
corner. It deletes to the end of line, which should erase the\r\n\
character there. Then it moves diagonally upwards, deleting to the\r\n\
end of line each time and outputting a single character. Finally\r\n\
the cursor is moved to the bottom left corner, and another delete\r\n\
line should erase the entire bottom line.\r\n\
";
   int x;
   output_text(text);
   fill_entire_screen();
   move_to(rows-1, cols-1);
   printf("%cK", 0x009B);
   for (x = 2; x <= rows; x++)
    { move_to(rows - x, cols - x);
      printf("%cK*", 0x009B);
    }
   fflush(stdout);
   Delay(OneSec);
   move_to(rows-1, 0);
   printf("%cK", 0x009B);
   fflush(stdout);
   Delay(5 * OneSec);
}

void insert_line(void)
{ PRIVATE char *text = "\
This routine fills the entire screen, and moves the cursor to the middle\r\n\
line. It then inserts a line, outputs some asterixes and a line feed,\r\n\
waits a bit, and repeats. At the bottom of the screen insert line\r\n\
should just delete the line, and the linefeed causes scrolling.\r\n\
";
  int x;
  
  output_text(text);
  fill_entire_screen();
  move_to(rows / 2, 0);
  for (x = 0; x < 20; x++)
   { printf("%cL***\n", 0x009B); fflush(stdout); Delay(OneSec); }
   
}

void delete_line(void)
{ PRIVATE char *text = "\
This routine fills the entire screen, and moves the cursor to the middle\r\n\
line. It then deletes a single line, outputs some chars and issues a\r\n\
linefeed, waits a bit, and repeats.\r\n\
";
  int x;
  
  output_text(text);
  fill_entire_screen();
  move_to(rows / 2, 0);
  for (x = 1; x < 10; x++)
   { printf("%cM***\n", 0x009B); fflush(stdout); Delay(2 * OneSec); }
}

void delete_char(void)
{ PRIVATE char *text = "\
This routine fills the entire screen, and moves the cursor to the top\r\n\
left corner. It deletes a single character, issues a linefeed, deletes\r\n\
two characters, issues a linefeed, deletes FOUR characters, etc.\r\n\
The spaces should appear at the right-hand side.\r\n\
";
  int x;
  
  output_text(text);
  fill_entire_screen();
  move_to(0, 0);
  for (x = 1; x < 300; x *= 2)
   printf("%c%dP\n", 0x009B, x); 
  fflush(stdout);
}

void scroll_up(void)
{ PRIVATE char *text = "\
This routine fills the entire screen, moves the cursor to the top left,\r\n\
scrolls up one line, outputs a character, issues a linefeed, scrolls up\r\n\
two lines, outputs a character, issues two linefeeds, etc. Eventually the\r\n\
entire screen will be wiped by the scrolling.\r\n\
";
   int x;
   
   output_text(text);
   fill_entire_screen();
   move_to(0, 0);
   for (x = 1; x <= (rows + 5); x++)
    { printf("%c%dS*\n\n", 0x009B, x);
      fflush(stdout);
      Delay(OneSec);
    }
}

void scroll_down(void)
{ PRIVATE char *text = "\
This routine fills the entire screen, moves the cursor to the top left,\r\n\
scrolls down one line, outputs a character, issues a linefeed, scrolls down\r\n\
two lines, outputs a character, issues two linefeeds, etc. Eventually the\r\n\
entire screen will be wiped by the scrolling.\r\n\
";
   int x;
   
   output_text(text);
   fill_entire_screen();
   move_to(0, 0);
   for (x = 1; x <= (rows + 5); x++)
    { printf("%c%dT*\n\n", 0x009B, x);
      fflush(stdout);
      Delay(OneSec);
    }
}

void rendition(void)
{ PRIVATE char *text = "\
Sorry. Currently unimplemented.\r\n\
";

  output_text(text);
}
