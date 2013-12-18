/* $Source: /server/usr/users/b/charles/world/microlink/RCS/source/test/test5.c,v $ */
/* $Header: test5.c,v 1.1 91/01/31 13:52:35 charles Locked $ */

/*------------------------------------------------------------------------*/
/*                                         source/microlink/test/test5.c  */
/*------------------------------------------------------------------------*/

/* Test to see about SetInfo().                                           */

/*------------------------------------------------------------------------*/
/*                                                          Header Files  */
/*------------------------------------------------------------------------*/

#include <stddef.h>
#include <stdlib.h>
#include <syslib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <process.h>
#include <codes.h>
#include <fault.h>

#include <time.h>

#include "microlink/digitiser.h"

/*------------------------------------------------------------------------*/
/*                                                             main(...)  */
/*------------------------------------------------------------------------*/

int main()
{  Object *obj;
   Stream *stm;
   DigitiserInfo info;

   obj = Locate(CurrentDir,"/microlink/digitiser");
   if(obj==NULL)
   {  printf("Failed to locate digitiser object\n");
      return 1;
   }
   
   stm = Open(obj,NULL,O_ReadOnly);
   if(stm==NULL)
   {  printf("Failed to open digitiser object\n");
      return 1;
   }
   
   GetInfo(stm,(void*)&info);

   printf("GetInfo() called ...\n");
   printf("( %+9d %+9d %+9d )\n",info.cXX,info.cXY,info.cX1);
   printf("( %+9d %+9d %+9d )\n",info.cYX,info.cYY,info.cY1);
   printf("denom = %+9d\n",info.denom);
   printf("( %+9d , %+9d ) Filters\n",info.xFilter,info.yFilter);
   printf("( %+9d , %+9d ) Glitch \n",info.xGlitch,info.yGlitch);
   printf("( %+9d , %+9d ) min raw X\n",info.minRawX,info.minRawY);
   printf("( %+9d , %+9d ) max raw Y\n",info.maxRawY,info.maxRawY);

   info.cXX = 1;
   info.cXY = 0;
   info.cX1 = 0;
   info.cYX = 0;
   info.cYY = 1;
   info.cY1 = 0;
   info.denom = 1;
   
   SetInfo(stm,(void*)&info,sizeof(DigitiserInfo));

   printf("SetInfo() called ...\n");
   printf("( %+9d %+9d %+9d )\n",info.cXX,info.cXY,info.cX1);
   printf("( %+9d %+9d %+9d )\n",info.cYX,info.cYY,info.cY1);
   printf("denom = %+9d\n",info.denom);
   printf("( %+9d , %+9d ) Filters\n",info.xFilter,info.yFilter);
   printf("( %+9d , %+9d ) Glitch \n",info.xGlitch,info.yGlitch);
   printf("( %+9d , %+9d ) min raw X\n",info.minRawX,info.minRawY);
   printf("( %+9d , %+9d ) max raw Y\n",info.maxRawY,info.maxRawY);

   GetInfo(stm,(void*)&info);

   printf("GetInfo() called again ...\n");
   printf("( %+9d %+9d %+9d )\n",info.cXX,info.cXY,info.cX1);
   printf("( %+9d %+9d %+9d )\n",info.cYX,info.cYY,info.cY1);
   printf("denom = %+9d\n",info.denom);
   printf("( %+9d , %+9d ) Filters\n",info.xFilter,info.yFilter);
   printf("( %+9d , %+9d ) Glitch \n",info.xGlitch,info.yGlitch);
   printf("( %+9d , %+9d ) min raw X\n",info.minRawX,info.minRawY);
   printf("( %+9d , %+9d ) max raw Y\n",info.maxRawY,info.maxRawY);

   return 0;
}
