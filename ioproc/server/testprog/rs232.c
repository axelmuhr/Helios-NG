/******************************************************************************
***                     RS232 device test program                           ***
***                                                                         ***
***  Author : BLV, 4.11.88                                                  ***
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <syslib.h>
#include <attrib.h>
#include <codes.h>
#include <nonansi.h>
#include <ioevents.h>
#define ne !=
#define eq ==
#define puts(a) fputs(a, stdout)
#define MaxSend	500

Stream		*rs232_stream;
Attributes	attr;
BYTE		buffer[MaxSend];

void init_console(void);
void wait_for_user(void);
void main_menu(void);
void output_text(char *text);
#define clear_screen() putchar('\f')

int main(int argc, char **argv)
{ WORD		result;
  char		*rs232_name;
  Object	*rs232_obj;
  
  init_console();
  
  printf("\f\t\tRS232 device Test\n");
  printf(  "\t\t=================\n\n\n");
  
  if (argc eq 1)
    rs232_name = "/rs232/default";
  else
    rs232_name = *(++argv);
    
  rs232_obj = Locate(NULL, rs232_name);
  if (rs232_obj eq Null(Object))
   { printf("Unable to locate %s\n", rs232_name);
     exit(1);
   }
 
  if ((rs232_stream = Open(rs232_obj, NULL, O_ReadWrite)) eq Null(Stream))
   { printf("Unable to Open stream to %s : %x\n", rs232_name,
            Result2(rs232_obj));
     Close(rs232_obj);
     exit(1);
   }

  Close(rs232_obj);
  printf("Opened stream to %s\n", rs232_name);
  
  if ((result = GetAttributes(rs232_stream, &attr)) < Err_Null)
   { printf("Failed to get attributes : %x\n", result);
     Close(rs232_stream);
     exit(1);
   }
     
  wait_for_user();
  main_menu();

  Close(rs232_stream);
  printf("\r\n\n\n");
}

void init_console(void)
{ Attributes	attr;
  WORD		result;
  
  setvbuf(stdin, NULL, _IONBF, 0);
  if ((result = GetAttributes(Heliosno(stdin), &attr)) < 0)
   { printf("Failed to get stdin attributes : %x. Exiting.\n", result);
     exit(result);
   }

  AddAttribute(&attr, ConsoleRawInput);
  
  if ((result = SetAttributes(Heliosno(stdin), &attr)) < 0)
   { printf("Failed to set stdin attributes : %x. Exiting.\n", result);
     exit(result);
   }
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

void transmit(void);
void receive(void);
void five_bits(void);
void six_bits(void);
void seven_bits(void);
void eight_bits(void);

void baud_19200(void);
void baud_9600(void);
void baud_4800(void);
void baud_2400(void);
void baud_1200(void);
void baud_300(void);

void xon_xoff(void);
void clocal(void);

void detect_break(void);
void cause_break(void);
void set_hupcl(void);
void clear_hupcl(void);
void reopen_rs232(void);
void detect_modem(void);

void parity_enable(void);
void parity_disable(void);
void parity_even(void);
void parity_odd(void);
void strip_on(void);
void strip_off(void);

void inpck_on(void);
void inpck_off(void);
void ignpar_on(void);
void ignpar_off(void);
void parmrk_on(void);
void parmrk_off(void);

#define menu_fill { Null(char), (VoidFnPtr) NULL }
#define Max_Menu 5

menu menu_list[Max_Menu] =
{	{	{ "Transmit data",		&transmit },
		{ "Receive data",		&receive },
 		{ "Set 5 bits",			&five_bits },
		{ "Set 6 bits",			&six_bits },
		{ "Set 7 bits",			&seven_bits },
		{ "Set 8 bits",			&eight_bits },
		{ "Enable Xon/Xoff",		&xon_xoff },
		{ "Enable hardware handshakes",	&clocal },
		menu_fill,
		menu_fill
	},
	{	{ "Transmit data",		&transmit },
		{ "Receive data",		&receive },
		{ "19200 baud",			&baud_19200 },
		{ "9600 baud",			&baud_9600 },
		{ "4800 baud",			&baud_4800 },
		{ "2400 baud",			&baud_2400 },
		{ "1200 baud",			&baud_1200 },
		{ "300 baud",			&baud_300 },
		menu_fill,
		menu_fill
	},
	{	{ "Transmit data",		&transmit },
		{ "Receive data",		&receive },
		{ "Parity on",			&parity_enable },
		{ "Parity off",			&parity_disable },
		{ "Even parity",		&parity_even },
		{ "Odd parity",			&parity_odd },
		{ "Enable strip",		&strip_on },
		{ "Disable strip",		&strip_off },
		menu_fill,
		menu_fill
	},
	{	{ "Transmit data",		&transmit },
		{ "Receive data",		&receive },
		{ "InPck on",			&inpck_on },
		{ "InPck off",			&inpck_off },
		{ "IgnPar on",			&ignpar_on },
		{ "IgnPar off",			&ignpar_off },
		{ "ParMrk on",			&parmrk_on },
		{ "ParMrk off",			&parmrk_off },
		menu_fill,
		menu_fill
	},		
	{	{ "Test for break",		&detect_break },
		{ "Cause break",		&cause_break },
		{ "Reopen rs232 stream",	&reopen_rs232 },
		{ "Set HupCl",			&set_hupcl },
		{ "Clear HupCl",		&clear_hupcl },
		{ "Test for modem",		&detect_modem },
		menu_fill,
		menu_fill,
		menu_fill,
		menu_fill
	}
};

void display_stats(void);

void main_menu(void)
{ menu_item *current_list;
  int i, choice;
   
  forever
   { clear_screen(); 
     display_stats();
     
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

void transmit(void)
{ int i;
  WORD result;
  
  for (i = 0; i < MaxSend; i++)
   buffer[i] = (i % 200) + 32;
    

  result = Write(rs232_stream, buffer, MaxSend, 20 * OneSec);

  printf("Written %d, result2 is %x\n", result, Result2(rs232_stream));
}

void check_data(BYTE *, int);

void receive(void)
{ WORD result = Read(rs232_stream, buffer, MaxSend, 20 * OneSec);

  printf("Read %d, result2 is %x\n", result, Result2(rs232_stream));
  check_data(buffer, result);
}

void do_attr(void)
{ WORD result;

  if ((result = SetAttributes(rs232_stream, &attr)) < Err_Null)
   { printf("Failed to set attributes : %x\n", result);
     return;
   }
  if ((result = GetAttributes(rs232_stream, &attr)) < Err_Null)
   { printf("Failed to get attributes : %x\n", result);
     return;
   }
}

void clear_bits(void)
{ if (IsAnAttribute(&attr, RS232_Csize_5))
   RemoveAttribute(&attr, RS232_Csize_5);
  if (IsAnAttribute(&attr, RS232_Csize_6))
   RemoveAttribute(&attr, RS232_Csize_6);
  if (IsAnAttribute(&attr, RS232_Csize_7))
   RemoveAttribute(&attr, RS232_Csize_7);
  if (IsAnAttribute(&attr, RS232_Csize_8))
   RemoveAttribute(&attr, RS232_Csize_8);
}

void five_bits(void)
{ clear_bits();
  AddAttribute(&attr, RS232_Csize_5);
  do_attr();
}

void six_bits(void)
{ clear_bits();
  AddAttribute(&attr, RS232_Csize_6);
  do_attr();
}

void seven_bits(void)
{ clear_bits();
  AddAttribute(&attr, RS232_Csize_7);
  do_attr();
}

void eight_bits(void)
{ clear_bits();
  AddAttribute(&attr, RS232_Csize_8);
  do_attr();
}

void baud_19200(void)
{ SetInputSpeed(&attr, RS232_B19200);
  do_attr();
}

void baud_9600(void)
{ SetInputSpeed(&attr, RS232_B9600);
  do_attr();
}

void baud_4800(void)
{ SetInputSpeed(&attr, RS232_B4800);
  do_attr();
}

void baud_2400(void)
{ SetInputSpeed(&attr, RS232_B2400);
  do_attr();
}

void baud_1200(void)
{ SetInputSpeed(&attr, RS232_B1200);
  do_attr();
}

void baud_300(void)
{ SetInputSpeed(&attr, RS232_B300);
  do_attr();
}

void xon_xoff(void)
{ AddAttribute(&attr, RS232_IXON);
  AddAttribute(&attr, RS232_IXOFF);
  AddAttribute(&attr, RS232_CLocal);
  do_attr();
}

void clocal(void)
{ RemoveAttribute(&attr, RS232_IXON);
  RemoveAttribute(&attr, RS232_IXOFF);
  RemoveAttribute(&attr, RS232_CLocal);
  do_attr();
}

void parity_enable(void)
{ AddAttribute(&attr, RS232_ParEnb);
  do_attr();
}

void parity_disable(void)
{ RemoveAttribute(&attr, RS232_ParEnb);
  do_attr();
}

void parity_even(void)
{ RemoveAttribute(&attr, RS232_ParOdd);
  do_attr();
}

void parity_odd(void)
{ AddAttribute(&attr, RS232_ParOdd);
  do_attr();
}

void strip_on(void)
{ AddAttribute(&attr, RS232_Istrip);
  do_attr();
}

void strip_off(void)
{ RemoveAttribute(&attr, RS232_Istrip);
  do_attr();
}

void inpck_on(void)
{ AddAttribute(&attr, RS232_InPck);
  do_attr();
}

void inpck_off(void)
{ RemoveAttribute(&attr, RS232_InPck);
  do_attr();
}

void ignpar_on(void)
{ AddAttribute(&attr, RS232_IgnPar);
  do_attr();
}

void ignpar_off(void)
{ RemoveAttribute(&attr, RS232_IgnPar);
  do_attr();
}

void parmrk_on(void)
{ AddAttribute(&attr, RS232_ParMrk);
  do_attr();
}

void parmrk_off(void)
{ RemoveAttribute(&attr, RS232_ParMrk);
  do_attr();
}


void break_wait(Port port)
{ MCB mcb;
  BYTE tempbuf[512];
  WORD result;
  
  forever
  { mcb.MsgHdr.Dest = port;
    mcb.Data = tempbuf;
    mcb.Timeout = -1;
    result = GetMsg(&mcb);
    printf("\rBreak wait : result is %x\n", result);
    if (result < 0) return;
  }
}

void detect_break(void)
{ Port temp_port = EnableEvents(rs232_stream, Event_RS232Break);
  if (temp_port eq NullPort)
   { printf("Failed to enable the event.\n");
     return;
   }
 
  (void) Fork(5000, break_wait, 4, temp_port);
  printf("Waiting for an event.\n");
  wait_for_user();
  (void) EnableEvents(rs232_stream, 0);
  FreePort(temp_port);
  Delay(OneSec);
}

void cause_break(void)
{ SetInputSpeed(&attr, RS232_B0);
  do_attr();
}

void set_hupcl(void)
{ AddAttribute(&attr, RS232_HupCl);
  do_attr();
}

void clear_hupcl(void)
{ RemoveAttribute(&attr, RS232_HupCl);
  do_attr();
}

void reopen_rs232(void)
{ char   name[128];
  Object *rs232_obj;
  
  strcpy(name, rs232_stream->Name);
  if ((rs232_obj = Locate(NULL, name)) eq Null(Object))
   { printf("Unable to locate %s\n", name);
     return;
   }
  Close(rs232_stream);
  if ((rs232_stream = Open(rs232_obj, NULL, O_ReadWrite)) == Null(Stream))
   { printf("Unable to reopen %s\n", name);
     Close(rs232_obj);
     return;
   }
  Close(rs232_obj);
}

void modem_wait(Port port)
{ MCB mcb;
  BYTE tempbuf[512];
  WORD result;
  
  forever
  { mcb.MsgHdr.Dest = port;
    mcb.Data = tempbuf;
    mcb.Timeout = -1;
    result = GetMsg(&mcb);
    printf("\rModem wait : result is %x\n", result);
    if (result < 0) return;
  }
}

void detect_modem(void)
{ Port temp_port = EnableEvents(rs232_stream, Event_ModemRing);
  if (temp_port eq NullPort)
   { printf("Failed to enable the event.\n");
     return;
   }
 
  (void) Fork(5000, modem_wait, 4, temp_port);
  printf("Waiting for an event.\n");
  wait_for_user();
  (void) EnableEvents(rs232_stream, 0);
  FreePort(temp_port);
  Delay(OneSec);
}

void display_stats(void)
{ char *temp;
	
   puts("Current attributes are :\nSpeed ");
   switch(GetInputSpeed(&attr))
    { case RS232_B38400 : temp = "38400"; break;
      case RS232_B19200 : temp = "19200"; break;
      case RS232_B9600  : temp = "9600";  break;
      case RS232_B4800  : temp = "4800";  break;
      case RS232_B2400  : temp = "2400";  break;
      case RS232_B1800  : temp = "1800";  break;
      case RS232_B1200  : temp = "1200";  break;
      case RS232_B600   : temp = "600";   break;
      case RS232_B300   : temp = "300";   break;
      case RS232_B200   : temp = "200";   break;
      case RS232_B150   : temp = "150";   break;
      case RS232_B134   : temp = "134";   break;
      case RS232_B110   : temp = "110";   break;
      case RS232_B75    : temp = "75";    break;
      case RS232_B50    : temp = "50";    break;
      default		: temp = "unknown";
    }

  puts(temp);
  puts(", no of bits = ");
  if (IsAnAttribute(&attr, RS232_Csize_5)) putchar('5');  
  if (IsAnAttribute(&attr, RS232_Csize_6)) putchar('6');  
  if (IsAnAttribute(&attr, RS232_Csize_7)) putchar('7');  
  if (IsAnAttribute(&attr, RS232_Csize_8)) putchar('8');  
  puts(", stop bits = ");
  putchar( IsAnAttribute(&attr, RS232_Cstopb) ? '2' : '1');
  puts(".\nFlow control : xon/xoff on input ");
  puts(IsAnAttribute(&attr, RS232_IXOFF) ? "enabled" : "disabled");
  puts(", on output ");
  puts(IsAnAttribute(&attr, RS232_IXON) ? "enabled" : "disabled");
  puts(", handshakes ");
  puts(IsAnAttribute(&attr, RS232_CLocal) ? "ignored" : "used");

  puts(".\nParity : ");
  puts(IsAnAttribute(&attr, RS232_ParEnb) ? "on, " : "off, ");
  puts(IsAnAttribute(&attr, RS232_ParOdd) ? "odd" : "even");
  puts(", strip ");
  puts(IsAnAttribute(&attr, RS232_Istrip) ? "on" : "off");
  puts(", InPck ");
  puts(IsAnAttribute(&attr, RS232_InPck) ? "on" : "off");
  puts(", IgnPar ");
  puts(IsAnAttribute(&attr, RS232_IgnPar) ? "on" : "off");
  puts(", ParMrk ");
  puts(IsAnAttribute(&attr, RS232_ParMrk) ? "on" : "off");
  puts(".\nBreaks : ");
  puts(IsAnAttribute(&attr, RS232_IgnoreBreak) ? "ignored" : "treated");
  puts(", break events ");
  if (!IsAnAttribute(&attr, RS232_BreakInterrupt)) puts("not");
  puts("generated, HupCl ");
  puts(IsAnAttribute(&attr, RS232_HupCl) ? "on" : "off");
  puts(".\n\n");
}


void check_data(BYTE *buffer, int amount)
{ buffer = buffer; amount = amount;
}

