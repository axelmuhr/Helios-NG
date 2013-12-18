/* $Header: test1.c,v 1.1 90/12/21 20:52:46 charles Locked $ */
/* $Source: /server/usr/users/b/charles/world/microlink/RCS/source/test/test1.c,v $ */

/*------------------------------------------------------------------------*/
/*                                         microlink/source/test/test1.c  */
/*------------------------------------------------------------------------*/

/* This test program to do things to the microlink server                 */

/*------------------------------------------------------------------------*/
/*                                                          Header files  */
/*------------------------------------------------------------------------*/

#include <helios.h>
#include <string.h>
#include <codes.h>
#include <stddef.h>
#include <stdlib.h>
#include <syslib.h>
#include <servlib.h>
#include <task.h>
#include <message.h>
#include <attrib.h>
#include <stdio.h>
#include <process.h>
#include <ioevents.h>

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/*------------------------------------------------------------------------*/
/*                                                             main(...)  */
/*------------------------------------------------------------------------*/

int main()
{  Object *dgo;
   Stream *dgs;
   char    buf[10];

   dgo = Locate(CurrentDir,"/microlink/digitiser");
   if(dgo==NULL)
   {  printf("Failed to locate digitiser object\n");
      return 1;
   }
   
   dgs = Open(dgo,NULL,O_ReadOnly);
   if(dgs==NULL)
   {  printf("Failed to open digitiser object\n");
      return 1;
   }
   
   for(;;)
   {  printf
      ("Trying to read a single character from the digitiser stream ...\n");
      Read(dgs,buf,1,-1);
      printf("Read a character : Hex code 0x%2X : ",buf[0]);
      if(isprint(buf[0])) printf("'%c'\n",buf[0]);
      else                printf("unprintable\n");
   }
}

