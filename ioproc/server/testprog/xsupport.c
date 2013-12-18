/******************************************************************************
***              X window support test program                              ***
***                                                                         ***
***  Author : BLV, 16.11.88                                                 ***
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <syslib.h>
#include <ioevents.h>
#include <attrib.h>
#include <sem.h>
#include <gsp.h>
#include <nonansi.h>
#define ne !=
#define eq ==

void wait_for_user(void);
void output_text(char *text);
void main_menu(void);
#define clear_screen() putchar('\f')
Stream *mouse_stream = Null(Stream);
Stream *keyboard_stream = Null(Stream);

int main(void)
{ Object *root_obj = Locate(NULL, "/");
  Object *mouse_obj;
  Object *keyboard_obj;
  Attributes attr;
  WORD result;
  
  setvbuf(stdin, NULL, _IONBF, 0);
  if ((result = GetAttributes(Heliosno(stdin), &attr)) < 0)
   { printf("Failed to get stdin attributes : %x. Exiting.\n", result);
     exit(result);
   }

  AddAttribute(&attr, ConsoleRawOutput);
  AddAttribute(&attr, ConsoleRawInput);
  RemoveAttribute(&attr, ConsolePause);
  RemoveAttribute(&attr, ConsoleEcho);
  
  if ((result = SetAttributes(Heliosno(stdin), &attr)) < 0)
   { printf("Failed to set stdin attributes : %x. Exiting.\n", result);
     exit(result);
   }
    
  printf("\f\t\tX Support Test\r\n");
  printf(  "\t\t==============\r\n\n\n");

  mouse_obj = Locate(root_obj, "/mouse");
  if (mouse_obj eq Null(Object))
    { printf("Failed to find a mouse device : %x.\r\n", Result2(root_obj));
      exit(Result2(root_obj));
    }
    
  printf("Found a mouse server : %s.\r\n", mouse_obj->Name);
  mouse_stream = Open(mouse_obj, NULL, O_ReadOnly);
  if (mouse_stream eq Null(Stream))
   { printf("Failed to open stream : %x.\n", Result2(mouse_obj));
     exit(Result2(mouse_obj));
   }
  Close(mouse_obj);
  
  keyboard_obj = Locate(root_obj, "/keyboard");
  if (keyboard_obj eq Null(Object))
    { printf("Failed to find a keyboard device : %x.\r\n", Result2(root_obj));
      exit(Result2(root_obj));
    }
    
  printf("Found a keyboard server : %s.\r\n", keyboard_obj->Name);
  keyboard_stream = Open(keyboard_obj, NULL, O_ReadOnly);
  if (keyboard_stream eq Null(Stream))
   { printf("Failed to open stream : %x.\n", Result2(keyboard_obj));
     exit(Result2(keyboard_obj));
   }
  Close(keyboard_obj);

  Close(root_obj);   
  wait_for_user();
  main_menu();
  Close(mouse_stream);
  Close(keyboard_stream);
  printf("\r\n\n\n");
  return(0);
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

void mouse_movement(void);
void keyboard_input(void);

#define menu_fill { Null(char), (VoidFnPtr) NULL }

menu menu_list[] =
{	{	{ "Mouse movement",	&mouse_movement },
		{ "Keyboard input",	&keyboard_input },
		menu_fill,
		menu_fill,
		menu_fill,
		menu_fill,
		menu_fill,
		menu_fill,
		menu_fill,
		menu_fill
	}
};
#define Max_Menu 1

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

Semaphore mysem;
WORD mouse_must_exit = 0;
WORD mouse_head, mouse_tail;
#define mouse_max	256
typedef struct my_mouse_event {
	WORD	id;
	WORD	buttons;
	WORD	x;
	WORD	y;
} my_mouse_event;
my_mouse_event	mouse_table[mouse_max];

void Mouseclear_tables(void)
{ mouse_must_exit = 0;
  InitSemaphore(&mysem, 1);
  mouse_head = 0; mouse_tail = 0;
}

void Mousetidy_tables(void)
{ mouse_must_exit = 1;
}

void MouseReaderProcess(WORD id, Port port)
{ PRIVATE BYTE data[IOCDataMax];
  MCB	  message;
  int     i;
  IOEvent *current_event;
    
  message.Data = &(data[0]);
  
  forever
   { message.MsgHdr.Dest = port;
     message.Timeout     = -1;

     if (GetMsg(&message) < 0)
      return;

     current_event = (IOEvent *) data;
     for (i = 0; i < (message.MsgHdr.DataSize / Mouse_EventSize); i++)
      { my_mouse_event *the_event;
        Wait(&mysem);
        the_event = &(mouse_table[mouse_head]);
        mouse_head = (mouse_head + 1) & (mouse_max - 1);
        the_event->id = id;
        the_event->x  = (WORD) current_event->Device.Mouse.X;
        the_event->y  = (WORD) current_event->Device.Mouse.Y; 
        the_event->buttons = current_event->Device.Mouse.Buttons;
	current_event++;
	Signal(&mysem);
      }
   }          
}

void MouseDisplayProcess(void)
{ forever
   { if (mouse_must_exit) return;
     Wait(&mysem);
     while (mouse_tail ne mouse_head)
      { my_mouse_event *the_event = &(mouse_table[mouse_tail]);
        char *text;

        mouse_tail = (mouse_tail + 1) & (mouse_max - 1);
        
        switch (the_event->buttons)
         { case Buttons_Unchanged	: text = "buttons unchanged"; break;
           case Buttons_Button0_Down	: text = "left button down"; break;
           case Buttons_Button0_Up	: text = "left button up"; break;
           case Buttons_Button1_Down	: text = "right button down"; break;
           case Buttons_Button1_Up	: text = "right button up"; break;
           case Buttons_Button2_Down	: text = "middle button down"; break;
           case Buttons_Button2_Up	: text = "middle button up"; break;
           default			: text = "unknown button combination";
           				  break;
         }
        printf("%d) x = %d, y %d, %s\r\n", the_event->id, the_event->x,
        	the_event->y, text);
        Signal(&mysem);
        Wait(&mysem);
      }
     Signal(&mysem);
     Delay(OneSec / 5);
   }
}

void mouse_movement(void)
{ char *text = "\
This routine just does an enable events on the mouse stream, accepts\r\n\
events for 30 seconds, and clears the event.\r\n\
";
  Port myport;
  
  output_text(text);
  
  Mouseclear_tables();
  myport = EnableEvents(mouse_stream, Event_Mouse);
  if (myport eq NullPort)
   { printf("Failed to get port for events.\r\n");
     return;
   }
  Fork(5000, MouseReaderProcess, 8, 1, myport);
  Fork(5000, MouseDisplayProcess, 0);
  Delay(30 * OneSec);
  (void) EnableEvents(mouse_stream, 0);
  Delay(2 * OneSec);
  Mousetidy_tables();
  FreePort(myport);
}

/**
*** Something similar for the keyboard
**/
WORD keyboard_must_exit = 0;
WORD keyboard_head, keyboard_tail;
#define keyboard_max	256
typedef struct my_keyboard_event {
	WORD	id;
	WORD	scancode;
	WORD	updown;
} my_keyboard_event;
my_keyboard_event keyboard_table[keyboard_max];

void Keyboardclear_tables(void)
{ keyboard_must_exit = 0;
  InitSemaphore(&mysem, 1);
  keyboard_head = 0; keyboard_tail = 0;
}

void Keyboardtidy_tables(void)
{ keyboard_must_exit = 1;
}

void KeyboardReaderProcess(WORD id, Port port)
{ PRIVATE BYTE data[IOCDataMax];
  MCB	  message;
  int     i;
  IOEvent *current_event;
    
  message.Data = &(data[0]);
  
  forever
   { message.MsgHdr.Dest = port;
     message.Timeout     = -1;

     if (GetMsg(&message) < 0)
      return;

     current_event = (IOEvent *) data;
     for (i = 0; i < (message.MsgHdr.DataSize / Keyboard_EventSize); i++)
      { my_keyboard_event *the_event;
        Wait(&mysem);
        the_event = &(keyboard_table[keyboard_head]);
        keyboard_head = (keyboard_head + 1) & (keyboard_max - 1);
        the_event->id = id;
        the_event->scancode  = (WORD) current_event->Device.Keyboard.Key;
        the_event->updown    = current_event->Device.Keyboard.What;
	current_event++;
	Signal(&mysem);
      }
   }          
}

void KeyboardDisplayProcess(void)
{ forever
   { if (keyboard_must_exit) return;
     Wait(&mysem);
     while (keyboard_tail ne keyboard_head)
      { my_keyboard_event *the_event = &(keyboard_table[keyboard_tail]);

        keyboard_tail = (keyboard_tail + 1) & (keyboard_max - 1);

        printf("%d) scancode %x %s\r\n", the_event->id, the_event->scancode,
        	(the_event->updown eq Keys_KeyUp) ? "up" : "down");
        Signal(&mysem);
        Wait(&mysem);
      }
     Signal(&mysem);
     Delay(OneSec / 5);
   }
}

void keyboard_input(void)
{ char *text = "\
This routine just does an enable events on the keyboard stream, accepts\r\n\
events for 30 seconds, and clears the event.\r\n\
";
  Port myport;
  
  output_text(text);
  
  Keyboardclear_tables();
  myport = EnableEvents(keyboard_stream, Event_Keyboard);
  if (myport eq NullPort)
   { printf("Failed to get port for events.\r\n");
     return;
   }
  Fork(5000, KeyboardReaderProcess, 8, 1, myport);
  Fork(5000, KeyboardDisplayProcess, 0);
  Delay(60 * OneSec);
  (void) EnableEvents(keyboard_stream, 0);
  Delay(2 * OneSec);
  Keyboardtidy_tables();
  FreePort(myport);
}


