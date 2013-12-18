/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- tfmmisc.c								--
--                                                                      --
--	This module of the Taskforce Manager contains various utility	--
--	routines.							--
--                                                                      --
--	Author:  BLV 4/9/90						--
--                                                                      --
------------------------------------------------------------------------*/
/*$Header: /hsrc/network/RCS/tfmmisc.c,v 1.7 1994/03/01 12:39:58 nickc Exp $*/

/*{{{  headers */
#include <stdio.h>
#include <syslib.h>
#include <servlib.h>
#include <sem.h>
#include <codes.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <posix.h>
#include <ctype.h>
#include <nonansi.h>
#include <attrib.h>
#include <pwd.h>
#include <signal.h>
#include <module.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "exports.h"
#include "private.h"
#include "netutils.h"
#include "rmlib.h"
#include "tfmaux.h"
/*}}}*/
/*{{{  statics and module initialisation */
void	InitMisc(void)
{
}
/*}}}*/
/*{{{  FullRead() */
/**
*** A little utility routine to cope with the fact that pipe reads do
*** not necessarily return the amount of data requested.
**/
word FullRead(Stream *pipe, BYTE *buffer, word amount, word timeout)
{ word	read = 0;
  word	temp;

  forever  
  { temp = Read(pipe, &(buffer[read]), amount - read, timeout);
    if (temp < 0)
     return((read eq 0) ? temp : read);
    read += temp;
    if (read >= amount) return(read);
    if (timeout ne -1) return(read);
  }
}
/*}}}*/
/*{{{  BuildName() */
/**
*** Build the full processor name into the specified buffer
**/
char *BuildName(char *buffer, RmProcessor Processor)
{ const char *name;

  if (Processor eq (RmProcessor) RmRootNetwork(Processor))
   { 
     if (NetworkName[0] eq '\0')	/* cope with SingleProcessor system */
      return(buffer);
     name = NetworkName;
     *buffer++ = '/';
     for ( ; *name ne '\0'; ) *buffer++ = *name++;
     return(buffer);
   }
  else
   { 
     buffer = BuildName(buffer, (RmProcessor) RmParentNetwork(Processor));
     *buffer++ = '/';
     name = (RmIsNetwork(Processor)) ? RmGetNetworkId((RmNetwork) Processor) :
     			(const char *) Processor->ObjNode.Name;
     for ( ; *name ne '\0'; ) *buffer++ = *name++;
     *buffer = '\0';
     return(buffer);
   }
}
/*}}}*/
/*{{{  MatchProcessor() */

/**
*** Match a processor with a template. This can get quite complicated.
*** 1) Exclusive access has to be checked. The application may require
***    exclusive access, or some other application may already have it
*** 2) purpose must be checked. If the processor is currently native it
***    is not useful unless the template specifies a native processor.
***    If the template specifies native then the processor must support it.
*** 3) if the template has any attributes then the processor must
***    have the same attributes, but not vice versa
*** 4) various combinations of processor types may or may not match
*** 5) the real processor must have at least the amount of memory requested
**/

bool	MatchProcessor(RmProcessor real, RmProcessor Template)
{ int		attribute_count = RmCountProcessorAttributes(Template);
  int		template_ptype;
  int		real_ptype;

  unless (RmGetProcessorState(real) & RmS_Running)
   return(FALSE);

  if ((Template->AllocationFlags & RmF_Exclusive) ||
      (real->AllocationFlags & RmF_Exclusive))
   { DomainEntry	*domain_entry;
     domain_entry = GetDomainEntry(real);
     if (domain_entry->NumberUsers > 0)
      return(FALSE);
   }

  if (Template->Purpose eq RmP_Native)
   { unless(RmGetProcessorControl(real) & RmC_Native) return(FALSE);
   }
  elif (real->Purpose eq RmP_Native)
   return(FALSE);

  if (attribute_count > 0)
   { char	*attribs[10];
     char	**real_attribs;
     int	i;

	/* very simple test, to start with */
     if (attribute_count > RmCountProcessorAttributes(real)) return(FALSE);

     if (attribute_count > 10)
      { real_attribs = (char **) Malloc((word) attribute_count * sizeof(char *));
        if (real_attribs eq Null(char *)) return(FALSE);
      }
     else
      real_attribs = attribs;
     if (RmListProcessorAttributes(Template, real_attribs) ne RmE_Success)
      { if (attribute_count > 10) Free(real_attribs);
        return(FALSE);
      }
     for (i = 0; i < attribute_count; i++)
      unless(RmTestProcessorAttribute(real, real_attribs[i]) eq RmE_Success)
       { if (attribute_count > 10) Free(real_attribs);
         return(FALSE);
       }
    if (attribute_count > 10) Free(real_attribs); 
   }

  if (RmGetProcessorMemory(real) < RmGetProcessorMemory(Template))
   return(FALSE);

  template_ptype	= RmGetProcessorType(Template);
  real_ptype		= RmGetProcessorType(real);
  if (template_ptype ne RmT_Default)
   { if (template_ptype ne real_ptype)
     return(FALSE);
   }

  	/* I have no way of working out the exact requirement */
  return(TRUE);
}

/*}}}*/
/*{{{  MatchTask() */

/**
*** Almost the same code, but matching a processor and a task
**/
bool	MatchTask(RmProcessor real, RmTask Template)
{ int	attribute_count = RmCountTaskAttributes(Template);
  int	template_ptype;
  int	real_ptype;

  unless(RmGetProcessorState(real) & RmS_Running)
   return(FALSE);

  unless(RmGetProcessorPurpose(real) eq RmP_Helios)
   return(FALSE);

  if (attribute_count > 0)
   { char	*attribs[10];
     char	**real_attribs;
     int	i;

	/* very simple test, to start with */
     if (attribute_count > RmCountProcessorAttributes(real)) return(FALSE);

     if (attribute_count > 10)
      { real_attribs = (char **) Malloc((word) attribute_count * sizeof(char *));
        if (real_attribs eq Null(char *)) return(FALSE);
      }
     else
      real_attribs = attribs;
     if (RmListTaskAttributes(Template, real_attribs) ne RmE_Success)
      { if (attribute_count > 10) Free(real_attribs);
        return(FALSE);
      }
     for (i = 0; i < attribute_count; i++)
      unless(RmTestProcessorAttribute(real, real_attribs[i]) eq RmE_Success)
       { if (attribute_count > 10) Free(real_attribs);
         return(FALSE);
       }
    if (attribute_count > 10) Free(real_attribs); 
   }

  if (RmGetProcessorMemory(real) < RmGetTaskMemory(Template))
   return(FALSE);

  template_ptype	= RmGetTaskType(Template);
  real_ptype		= RmGetProcessorType(real);
  if (template_ptype ne RmT_Default)
   { if (template_ptype eq real_ptype)
      return(TRUE);
     else
      return(FALSE);
   }
  else
  	/* I have no way of determining the real processor type */
   return(TRUE);
}

/*}}}*/
/*{{{  TfmMapProcessorToObject() */
/**
*** Mapping processor to object, there is a very similar routine in netboot.c
**/
Object	*TfmMapProcessorToObject(RmProcessor Processor)
{ char		*buf = (char *) Malloc(IOCDataMax);
  Object	*result;
  Capability	*Cap;

  if (buf eq Null(char)) return(Null(Object));
  (void) BuildName(buf, Processor);

  Cap = RmGetProcessorCapability(Processor, TRUE);
  if (*((word *) Cap) eq 0)
   result = Locate(Null(Object), buf);
  else
   result = NewObject(buf, Cap);

  Free(buf);
  return(result);
}
/*}}}*/
/*{{{  tfm_GetEnv() and tfm_FreeEnv() */

/**
*** Get an environment. The system library's GetEnv cannot be used
*** because the initial message has been received already. This has been
*** copied from the system library with some modifications.
**/
Port tfm_GetEnv(Port port, MCB *m, Environ *env)
{
	Port reply;
	word *control;
	word *buffer;
	byte *data;
	word e = Err_Null;
	word	*envsize = m->Control;
	bool	newenv = TRUE;
	bool	failed = FALSE;
	
	env->Argv = Null(char *);
	env->Envv = Null(char *);
	env->Objv = Null(Object *);
	env->Strv = Null(Stream *);
	
	newenv = m->MsgHdr.FnRc & 1;

	reply = m->MsgHdr.Reply;
	
	buffer = control = (word *)Malloc(envsize[0]*sizeof(word) + envsize[1]);

	if( control == Null(word) ) 
		e = EC_Error+SS_SysLib+EG_NoMemory+EO_Message;
	else
		e = newenv;
			
	InitMCB(m,0,reply,port,e);
        reply = NullPort;
        
	if((e = PutMsg(m)) < Err_Null) goto done;

	if( control == Null(word) ) goto done;
		
	/* we are now ready for the environment message */

	m->MsgHdr.Dest	= port;
	m->Control	= control;
	data		= (byte *)(control + envsize[0]);

	if (newenv) m->Data = (byte *) control;
	else	    m->Data = data;
	
	m->Timeout	= 30 * OneSec;

	if((e = GetMsg(m)) < Err_Null ) goto done;

	reply = m->MsgHdr.Reply;

	/* now we can un-marshal the environ ment data */
	/* argv first */
	env->Argv = (string *)control;
	while( *control != -1 ) 
	{
		*control = (word)&data[*control];
		control++;
	}
	*control++ = NULL;

	/* now envv */
	env->Envv = (string *)control;
	while( *control != -1 )
	{
		*control = (word)&data[*control];
		control++;
	}
	*control++ = NULL;

	/* now objects */
	env->Objv = (Object **)control;
	while( *control != -1 )
	{
		if( *control != MinInt )
		{	
			ObjDesc *o = (ObjDesc *)&data[*control];
			*control = (word)NewObject(o->Name,&o->Cap);
			if (*control == NULL)
			 { failed = TRUE; goto done; }
		}
		control++;
	}
	*control++ = NULL;
	
	/* and finally streams */
	env->Strv = (Stream **)control;
	while( *control != -1 )
	{
		if( *control != MinInt )
		{
			if( (*control&0xFFFF0000) == 0xFFFF0000 )
			{
				word ix = *control & 0x0000FFFF;
				*control = (word)(env->Strv[ix]);
			}
			else
			{
				StrDesc *s = (StrDesc *)&data[*control];
				word	openonget = (s->Mode & Flags_OpenOnGet);
				s->Mode &= ~Flags_OpenOnGet;
				*control = (word)NewStream(s->Name,&s->Cap,s->Mode);
				if (*control == NULL)
				 { failed = TRUE; goto done; }
				((Stream *)(*control))->Pos = s->Pos;
				if (openonget ne 0)
				 ((Stream *)(*control))->Flags |= Flags_OpenOnGet;
			}
		}
		control++;
	}
	*control++ = NULL;

  if (DebugOptions & dbg_Environ)
   { int i;
     report("received environment");
     for (i = 0; env->Argv[i] ne Null(char); i++)
      report("argument %d is %s", i, env->Argv[i]);
     for (i = 0; env->Envv[i] ne Null(char); i++)
      report("environment string %d is %s", i, env->Envv[i]);
     for (i = 0; env->Strv[i] ne Null(Stream); i++)
      if (env->Strv[i] ne (Stream *) MinInt)
       report("stream %d is %S", i, env->Strv[i]);
     for (i = 0; env->Objv[i] ne Null(Object); i++)
      if (env->Objv[i] ne (Object *) MinInt)
       report("object %d is %O", i, env->Objv[i]);
   }
   
done:
  if ((failed) && (buffer != Null(WORD)))
   { int i;
     Debug(dbg_Environ, ("error receiving environment"));
     if (env->Objv != Null(Object *))
      { for (i = 0; env->Objv[i] != Null(Object); i++)
         if (env->Objv[i] != (Object *) MinInt)
          Close(env->Objv[i]);
      }
     if (env->Strv != Null(Stream *))
      { for (i = 0; env->Strv[i] != Null(Stream); i++)
         if (env->Strv[i] != (Stream *) MinInt)
          Close(env->Strv[i]);
      }
     Free(buffer);
     InitMCB(m, 0, reply, NullPort, EC_Error + EG_NoMemory + EO_Message);
     PutMsg(m);
   }
  return(reply);
}

void tfm_FreeEnv(Environ *received)
{ Stream	**streams, **streams2;
  Object	**objects;

  for (streams = received->Strv; *streams ne Null(Stream); streams++)
   if (*streams ne (Stream *) MinInt)
    { Close(*streams);
		/* With the new environment, the same stream object may */
		/* be present more than once. It should be closed once	*/
		/* only.						*/
      for (streams2 = &(streams[1]); *streams2 ne Null(Stream); streams2++)
       if (*streams2 eq *streams)
        *streams2 = (Stream *) MinInt;
       *streams = (Stream *) MinInt;          
    }

  for (objects = received->Objv; *objects ne Null(Object); objects++)
   if (*objects ne (Object *) MinInt)
    { Close(*objects); *objects = (Object *) MinInt; }

  Free(received->Argv);	/* Environment info starts here */
}

/*}}}*/



