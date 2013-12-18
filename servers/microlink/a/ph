/* $Header: phone.c,v 1.4 91/01/14 17:20:24 charles Locked $ */
/* $Source: /server/usr/users/b/charles/world/microlink/RCS/source/phone.c,v $ */

/*------------------------------------------------------------------------*/
/*                                              microlink/source/phone.c  */
/*------------------------------------------------------------------------*/

/* These "phone-line" interface functions are to be included in some      */
/*    abc-standard library to provide a facility for controlling the      */
/*    telephone on/off hook and dial operations.                          */

/*------------------------------------------------------------------------*/
/*                                                          Header Files  */
/*------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <syslib.h>
#include <codes.h>
#include <ctype.h>

#include "microlink/phone.h"
#include "microlink/general.h"

/*------------------------------------------------------------------------*/
/*                                                Phone_OpenChannel(...)  */
/*------------------------------------------------------------------------*/

Phone_Channel *Phone_OpenChannel(char *name,word *err)
{  static Phone_Channel Stub;
   return &Stub;
}

# if 0
{  Phone_Channel *hdl;
   Object *obj;
   Stream *stm;

      hdl = (Phone_Channel*)calloc(1,sizeof(Phone_Channel));
   if(hdl==NULL)
   {  if(err) *err = EC_Error|EG_NoMemory|EO_Object;
      return NULL;
   }
   if(name==NULL||*name==0) name = "/microlink/general";
   obj = Locate(NULL,name);
   if(obj==NULL)
   {  if(err) *err = EC_Error|EG_Unknown|EO_Object;
      free(hdl);
      return NULL;
   }
   stm = Open(obj,NULL,O_ReadWrite);
   if(stm==NULL)
   {  if(err) *err = EC_Error|EG_Broken|EO_Object;
      Close(obj);
      free(hdl);
      return NULL;
   }
   hdl->stm = stm;
   Close(obj);
   return hdl;
}
#endif

/*------------------------------------------------------------------------*/
/*                                      Phone_RestartDeadmanTimeout(...)  */
/*------------------------------------------------------------------------*/

word Phone_RestartDeadmanTimeout(Phone_Channel *hdl)
{  /* Send the appropriate message here */
   return Phone_Ok;
}

/*------------------------------------------------------------------------*/
/*                                      Phone_RefreshDeadmanTimeout(...)  */
/*------------------------------------------------------------------------*/

word Phone_RefreshDeadmanTimeout(Phone_Channel *hdl)
{  /* Send the appropriate message here */
   return Phone_Ok;
}

/*------------------------------------------------------------------------*/
/*                                                  Phone_IsRinging(...)  */
/*------------------------------------------------------------------------*/

word Phone_IsRinging(Phone_Channel *hdl)
{  /* Send the appropriate message here, get reply */
   return 0;
}

/*------------------------------------------------------------------------*/
/*                                                   Phone_IsOnHook(...)  */
/*------------------------------------------------------------------------*/

word Phone_IsOnHook(Phone_Channel *hdl)
{  /* Get status here */
   return 0;
}

/*------------------------------------------------------------------------*/
/*                                                 Phone_IsDialling(...)  */
/*------------------------------------------------------------------------*/

word Phone_IsDialling(Phone_Channel *hdl)
{  /* Get status here */
    return 0;
}

/*------------------------------------------------------------------------*/
/*                                                  Phone_AwaitRing(...)  */
/*------------------------------------------------------------------------*/

word Phone_AwaitRing(Phone_Channel *hdl, word timeout)
{  /* Wait ringing here */
   /* Use events and GetMsg() with timeout */
   return Phone_Ok;
}

/*------------------------------------------------------------------------*/
/*                                                Phone_TakeOffHook(...)  */
/*------------------------------------------------------------------------*/

word Phone_TakeOffHook(Phone_Channel *hdl)
{  /* Send 'take-off-hook' message here and get reply */
   /* Return status */
   return Phone_Ok;
}

/*------------------------------------------------------------------------*/
/*                                                 Phone_DialNumber(...)  */
/*------------------------------------------------------------------------*/

word Phone_DialNumber(Phone_Channel *hdl,char *num)
{  int pd,sc;
   char *ptr;

   /* Verify number is valid format */
   if(*num=='P') pd=1; else if(*num=='T') pd=0; else return Phone_BadNum;
   for(sc=0,ptr=num+1;*ptr;ptr++)
   {  if(isdigit(*ptr)) { sc=1; continue; }
      if(*ptr=='(') continue;
      if(*ptr==')') continue;
      if(*ptr=='-') continue;
      if(!pd) break;
      if(*ptr=='*') { sc=1; continue; }
      if(*ptr=='#') { sc=1; continue; }
      break;
   }
   if(*ptr!=0) return Phone_BadNum;
   if(!sc)     return Phone_BadNum; /* No numbers or # or * in number */

   /* Now to send number digit by digit down 'phone line                  */
   /* Let microcontroller validate 'phone off hook, dead man's handle etc */

   /* Send '[' character here */
   for(ptr=num+1;*ptr;ptr++)
   {  /* Send '*ptr' character here */
      if(0/*Number not accepted*/) break;
   }

   if(*ptr)
   {  /* Send any required dialling-reset codes here */
      return Phone_RejectedNum;
   }

   /* Send ']' character here */
   if(0/* ']' not accepted for some reason */)
   {  /* Reset dialling if required */
      return Phone_RejectedNum;
   }
   
   return Phone_Ok;
}

/*------------------------------------------------------------------------*/
/*                                          Phone_AwaitDialComplete(...)  */
/*------------------------------------------------------------------------*/

word Phone_AwaitDialComplete(Phone_Channel *hdl, word timeout)
{  /* Usual thing */
   return Phone_Ok;
}

/*------------------------------------------------------------------------*/
/*                                             Phone_UnblockChannel(...)  */
/*------------------------------------------------------------------------*/

word Phone_UnblockChannel(Phone_Channel *hdl)
{  /* Abort appropriate message ports */
   /* Set appropriate status          */
   return Phone_Ok;
}

/*------------------------------------------------------------------------*/
/*                                               Phone_CloseChannel(...)  */
/*------------------------------------------------------------------------*/

word Phone_CloseChannel(Phone_Channel *hdl)
{  return Phone_Ok;  }

#if 0
{  Close(hdl->stm);
   return Phone_Ok;
}
#endif
