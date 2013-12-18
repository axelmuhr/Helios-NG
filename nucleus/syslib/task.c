/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--            Copyright (C) 1987,1990, Perihelion Software Ltd.         --
--                        All Rights Reserved.                          --
--                                                                      --
-- syslib/task.c							--
--                                                                      --
--	Routines related to Task creation and manipulation.		--
--									--
--                                                                      --
--	Author:  NHG 16/8/87						--
--		 NHG 03/8/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId:	 %W%	%G% Copyright (C) 1987, Perihelion Software Ltd.*/
/* $Id: task.c,v 1.19 1993/07/09 13:36:09 nickc Exp $ */

#define _in_task

#include "sys.h"

/*--------------------------------------------------------
-- Load							--
--							--
-- Load an image into the processor or TaskForceManager --
-- described by the loader object. If this argument is  --
-- NULL send request direct to the local Loader         --
-- 							--
--------------------------------------------------------*/

PUBLIC Object *Load(Object *loader, Object *obj)
{
	Object *o;
	bool local = loader == Null(Object);
	word isize = sizeof(LoadInfo) + (word)strlen(obj->Name) + 1;
	word e;
	
	LoadInfo *info;

#ifdef SYSDEB
	SysDebug(process)("Load(%O,%O)",loader,obj);
#endif

	if( (e = CheckObject(obj,C_Locate)) < 0 ) 
	{
		if( !local ) loader->Result2 = e;
		return NULL;
	}

	if( local ) loader = Locate(Null(Object),"/loader");
	elif( CheckObject(loader,C_Locate) != Err_Null ) return NULL;	

	info = (LoadInfo *)Malloc(isize);

	if( info == NULL )
	{
		obj->Result2 = EC_Error|SS_SysLib|EG_NoMemory;
		if( local ) Close((Stream *)loader);
		else loader->Result2 = obj->Result2;
		return NULL;
	}
	
	info->Cap = obj->Access;
	info->Matrix = -1;
	if( (closebits_(obj->Flags) & closebits_(Flags_Stream)) != 0 )
	{
		info->Pos = ((Stream *)obj)->Pos;
	}
	else info->Pos = 0;
	
	strcpy(info->Name,obj->Name);

	/* Ensure Special Function field set in FnRc  */
	/* This defines the particular type of Create */	
	loader->FnMod |= FF_LoadOnly;

	/* Set timeout to load timeout		      */
	loader->Timeout = LoadTimeout;
	
	o = Create(loader,NULL,0,isize,(byte *)info);

	if( o != Null(Object) ) o->Result2 = loader->Result2;

	if( local )
	{
		obj->Result2 = loader->Result2;
		Close((Stream *)loader);
	}
	
	Free(info);

	loader->FnMod &= ~FF_Mask;	/* clear subfunction	*/

	return o;
}

/*--------------------------------------------------------
-- Execute						--
--							--
-- Execute a task on the processor controlled by the    --
-- given Processor Manager. Or send Execute to the      --
-- the given TaskForceManager. If the control field is  --
-- NULL, send Execute tp the local Processor Manager.	--
-- 							--
--------------------------------------------------------*/

PUBLIC Object *Execute(Object *controller, Object *prog)
{
	Object *o;
	bool local = (controller == Null(Object));
	word namelen = (word)strlen(prog->Name) + 1;
	word isize = sizeof(TaskInfo) + namelen;
	word rc = Err_Null;
	TaskInfo *info;

#ifdef SYSDEB
	SysDebug(process)("Execute(%O,%O)",controller,prog);
#endif
	if( (rc = CheckObject(prog,C_Locate)) < 0 ) 
	{
#ifdef SYSDEB
	SysDebug(process)("Execute program %O invalid",prog);
#endif
		if( !local ) controller->Result2 = rc;
		return NULL;
	}

	if( local ) controller = Locate(Null(Object),"/tasks");
	elif( CheckObject(controller,C_Locate) != Err_Null ) return NULL;

	info = (TaskInfo *)Malloc(isize);

	if( info == NULL )
	{
		prog->Result2 = EC_Error|SS_SysLib|EG_NoMemory;
		if( local ) Close((Stream *)controller);
		else controller->Result2 = prog->Result2;
		return NULL;
	}

	info->Name = sizeof(TaskInfo);
	info->Cap = prog->Access;
	info->Matrix = -1;
	
	strcpy((char *)(info+1),prog->Name);

	/* Ensure Special Function field set in FnRc  */
	/* This defines the particular type of Create */	
	controller->FnMod |= FF_Execute;

	/* Set timeout to load timeout		      */
	controller->Timeout = LoadTimeout;

	o = Create(controller,NULL,Type_Task,isize,(byte *)info);

	rc = controller->Result2;
		/* BLV - used to be in wrong place */
	prog->Result2 = controller->Result2;
	
	if( local ) 
	{
		Close((Stream *)controller);
	}

	Free(info);

	controller->FnMod &= ~FF_Mask;		/* clear subfunction */

#ifdef SYSDEB
	SysDebug(process)("Execute: %x %O",rc, o);
#endif

	return o;
}

/*--------------------------------------------------------
-- SendEnv						--
-- GetEnv						--
--							--	
-- Environment/argument exchange protocol. Used by a	--
-- parent process to pass arguments and environment to	--
-- a child.						--
-- 							--
--------------------------------------------------------*/

PUBLIC word SendEnv(Port dest, Environ *env)
{
	MCB *m = Null(MCB);
	word argc = 0, envc = 0, objc = 0, strc = 0;
	word args = 0, envs = 0, objs = 0, strs = 0;
	string *argp;
	string *envp;
	Object **objp;
	Stream **strp;
	Port reply = NewPort();
	word e = Err_Null;
	word csize, dsize;
	Port dataport;
	bool newenv = true;

#ifdef SYSDEB
	SysDebug(process)("SendEnv(%x,%P)",dest,env);
#endif

	/* calculate size of argument data */
	argp = env->Argv;
	while( *argp != Null(char) )
	{
		argc++;
		args += (word)strlen(*argp) + 1;
		argp++;
	}
	args = wordlen(args);

	/* calculate size of environment data */
	envp = env->Envv;
	while( *envp != Null(char) )
	{
		envc++;
		envs += (word)strlen(*envp) + 1;
		envp++;
	}
	envs = wordlen(envs);

	/* and size of object data */
	objp = env->Objv;
	while( *objp != Null(Object) )
	{
		objc++;
		if( *objp != (Object *)MinInt )
		{
			objs += sizeof(ObjDesc) + (word)strlen((*objp)->Name);
			objs = wordlen(objs);
		}
		objp++;
	}

	/* and size of stream data */
	strp = env->Strv;
	while( *strp != Null(Stream) )
	{
		strc++;
		if( *strp != (Stream *)MinInt ) 
		{
			strs += sizeof(StrDesc) + (word)strlen((*strp)->Name);
			strs = wordlen(strs);
		}
		strp++;
	}

	/* we now know how much space we need for the message, allocate */
	/* a buffer							*/
	
	csize = argc+envc+objc+strc+4;
	dsize = args+envs+objs+strs;

	m = (MCB *)Malloc(sizeof(MCB)+csize*sizeof(word)+dsize);

	if( m == NULL ) 
	{
		e = EC_Error+SS_SysLib+EG_NoMemory+EO_Message;
		goto done;
	}

	m->Control = (word *)((word)m + sizeof(MCB));
	m->Data = (byte *)(m->Control+csize);

retry:

	/* New environment protocol is indicated by lsb of subfunction */
	InitMCB(m,MsgHdr_Flags_preserve,dest,reply,FC_Private|FG_SendEnv|1);

	MarshalWord(m,csize);
	MarshalWord(m,dsize);

	/* send off initial message of protocol & get ack	*/

	if((e = XchMsg(m,0)) < Err_Null ) goto done;

	/* Only use new environment if GetEnv set lsb of reply	*/
	/* This allows us to be compatible with old TFMs	*/
	
	newenv = e & 1;
	
	dataport = m->MsgHdr.Reply;

#ifdef SYSDEB
	SysDebug(process)("SendEnv: dataport %x newenv %d",dataport,newenv);
#endif

	InitMCB(m,0,dataport,reply,0);

	/* marshal the argument vector */
	argp = env->Argv;
	while( *argp != NULL )
	{
		/* MarshalString special cases empty strings	*/
		/* we do not want to do this here		*/
		if( **argp != '\0' ) MarshalString(m,*argp);
		else
		{
			word zero = 0;
			MarshalOffset(m);
			MarshalData(m,4,(byte *)&zero);
		}
		argp++;
		/* The following allows us to continue using the standard */
		/* marshalling routines on a control vector > 256	  */
		if( m->MsgHdr.ContSize > 200 )
		{ m->Control += m->MsgHdr.ContSize; m->MsgHdr.ContSize = 0; }
	}
	MarshalWord(m,-1);

	/* now marshal the environment vector */
	envp = env->Envv;
	while( *envp != Null(char) )
	{
		MarshalString(m,*envp);
		envp++;
		if( m->MsgHdr.ContSize > 200 )
		{ m->Control += m->MsgHdr.ContSize; m->MsgHdr.ContSize = 0; }
	}
	MarshalWord(m,-1);

	/* and the objects */
	objp = env->Objv;
	while( *objp != Null(Object) )
	{
		if( *objp != (Object *)MinInt ) MarshalObject(m,*objp);
		else MarshalWord(m,MinInt);
		objp++;
		if( m->MsgHdr.ContSize > 200 )
		{ m->Control += m->MsgHdr.ContSize; m->MsgHdr.ContSize = 0; }
	}
	MarshalWord(m,-1);

	/* and the streams */
	strp = env->Strv;
	while( *strp != Null(Stream) )
	{
		if( *strp != (Stream *)MinInt )
		{
			Stream **p = env->Strv;
			for(; p != strp; p++ ) if( *p == *strp ) break;
			if( p == strp ) MarshalStream(m, *strp);
			else MarshalWord(m,0xFFFF0000U|(word)(p-env->Strv));
		}
		else MarshalWord(m,MinInt);
		strp++;
		if( m->MsgHdr.ContSize > 200 )
		{ m->Control += m->MsgHdr.ContSize; m->MsgHdr.ContSize = 0; }
	}
	MarshalWord(m,-1);

	if( newenv )
	{
		/* the new environment protocol puts both vectors in the */
		/* data part of the message.				 */
		m->MsgHdr.DataSize += (unsigned short)(csize * sizeof(word));
		m->MsgHdr.ContSize = 0;
		m->Data = (byte *)((word)m + sizeof(MCB));
	}
	
	/* send the environment message & get ack */

	/* BLV - used to be OneSec * strs, but this caused overflow */
	m->Timeout = IOCTimeout + OneSec*strc;

	if((e = XchMsg(m,0)) < Err_Null ) goto retry;

#ifdef SYSDEB
	if( m->MsgHdr.Reply != NullPort ) SysDebug(error)("SendEnv: Non-Null Reply port %x",m->MsgHdr.Reply);
#endif
	if( m->MsgHdr.Reply != NullPort ) FreePort(m->MsgHdr.Reply);

	/* now close any streams with CloseOnSend bits set	*/
	strp = env->Strv;
	while( *strp != Null(Stream) )
	{
		if( (*strp != (Stream *)MinInt) && 
		    ((*strp)->Flags & Flags_CloseOnSend) ) 
		    {
		    	CloseStream(*strp);
		    	(*strp)->Type = Type_Pseudo;
		    }
		strp++;
	}

done:
	if( m != Null(MCB)) Free(m);
	FreePort(reply);
	return e;
}

PUBLIC word GetEnv(Port port, Environ *env)
{
	MCB  m;
	word envsize[2];
	Port reply;
	word *control;
	byte *data;
	word e = Err_Null;
	bool newenv = true;

#ifdef SYSDEB
	SysDebug(process)("GetEnv(%x)",env);
#endif

retry:
	m.MsgHdr.Dest = port;
	m.Control = envsize;
	m.Timeout = -1;

	if((e = GetMsg(&m)) < Err_Null ) goto done;

	newenv = e & 1;
	reply = m.MsgHdr.Reply;

#ifdef SYSDEB
	SysDebug(process)("GetEnv 1: replyport %x sizes %d %d newenv %d",reply,envsize[0],envsize[1],newenv);
#endif
	control = (word *)Malloc(envsize[0]*sizeof(word) + envsize[1]);

	if( control == Null(word) ) 
		e = EC_Error+SS_SysLib+EG_NoMemory+EO_Message;
	else e = 1;
	
	InitMCB(&m,0,reply,port,e);

	if((e = PutMsg(&m)) < Err_Null) goto done;

	if( control == Null(word) ) goto done;
		
	/* we are now ready for the environment message */

	m.MsgHdr.Dest = port;
	m.Control = control;
	data = (byte *)(control + envsize[0]);

	if( newenv ) m.Data = (byte *)control;
	else 	     m.Data = data;

	if((e = GetMsg(&m)) < Err_Null ) goto retry;

	reply = m.MsgHdr.Reply;
#ifdef SYSDEB
	SysDebug(process)("GetEnv 2: replyport %x",reply);
#endif

	/* now we can un-marshal the environment data */

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
				int ix = (int)(*control & 0x0000FFFF);
				*control = (word)(env->Strv[ix]);
			}
			else
			{
				StrDesc *s = (StrDesc *)&data[*control];
				*control = (word)NewStream(s->Name,&s->Cap,s->Mode);
				((Stream *)(*control))->Pos = s->Pos;
			}
		}
		control++;
	}
	*control++ = NULL;

	/* now reply to the sender */

	InitMCB(&m,0,reply,NullPort,e);

	PutMsg(&m);
done:

#ifdef SYSDEB
	SysDebug(process)("GetEnv: %E",e);
#endif

	return e;
}


/*--------------------------------------------------------
-- Exit							--
--							--
-- Terminate the task. Close streams, free resources,   --
-- and quit.						--
-- 							--
--------------------------------------------------------*/

PUBLIC void
Exit( WORD code )
{
	MCB m;

#ifdef SYSDEB
	SysDebug(process)("Exit(%x)",code);
#endif
	TidyUp();

	InitMCB( &m, MsgHdr_Flags_preserve, MyTask->IOCPort, NullPort,
		EC_Error | SS_SysLib | EG_Exception | EE_Kill );

	m.Control         = &code;
	m.MsgHdr.ContSize = 1;

	(void) PutMsg( &m );

	StopProcess();
}

/*--------------------------------------------------------
-- TidyUp						--
--							--
-- Close all object and stream structures.		--
-- 							--
--------------------------------------------------------*/

void TidyUp()
{
	bool	old_terminating;

	Wait(&StreamLock);	/* convenient semaphore	*/
	old_terminating	= Terminating;
	Terminating = true;
	Signal(&StreamLock);

	if (old_terminating) StopProcess();
	
#ifdef SYSDEB
	SysDebug(process)("TidyUp Objects");
#endif
	WalkList(&Objects,Close);
	
#ifdef SYSDEB
	SysDebug(process)("TidyUp Streams");
#endif	

	WalkList(&Streams,Close);

#ifdef SYSDEB
	SysDebug(process)("TidyUp Done");
#endif

}


/*--------------------------------------------------------
-- Alarm						--
--							--
-- Ask IOC to send us an alarm signal in the given	--
-- number of seconds.					--
--							--
--------------------------------------------------------*/

PUBLIC WORD Alarm(word secs)
{
	word rc = Err_Null;
	MCB *mcb;
	Port reply = NewPort();

	mcb = NewMsgBuf(0);

#ifdef SYSDEB
	SysDebug(process)("Alarm(%d)",secs);
#endif

	InitMCB(mcb,MsgHdr_Flags_preserve,
		MyTask->IOCPort,reply,FC_Private|FG_Alarm);

	MarshalWord(mcb,secs);

	if( (rc = XchMsg(mcb,0)) >= Err_Null ) rc = mcb->Control[0];

	FreePort(reply);
#ifdef SYSDEB
	SysDebug(process)("Alarm: %d secs left",rc);
	if( mcb->MsgHdr.Reply != NullPort ) SysDebug(error)("Alarm: Non-Null Reply port %x",mcb->MsgHdr.Reply);
#endif
	if( mcb->MsgHdr.Reply != NullPort ) FreePort(mcb->MsgHdr.Reply);

	FreeMsgBuf(mcb);

	return rc;
}

/*--------------------------------------------------------
-- SetSignalPort					--
--							--
-- Install port to which signals will be delivered as	--
-- messages.						--
--							--
--------------------------------------------------------*/

PUBLIC Port SetSignalPort(Port port)
{
	word rc = Err_Null;
	MCB *mcb = NewMsgBuf(0);
#ifndef __TRAN
	Port replyp = NewPort();
#endif

#ifdef SYSDEB
	SysDebug(process)("SetSignalPort(%x)",port);
#endif

#ifndef __TRAN
	InitMCB(mcb,MsgHdr_Flags_preserve,
		MyTask->IOCPort,replyp,FC_Private|FG_SetSignalPort);
	MarshalWord(mcb,port);
#else
	InitMCB(mcb,MsgHdr_Flags_preserve,
		MyTask->IOCPort,port,FC_Private|FG_SetSignalPort);
#endif

	rc = XchMsg(mcb,0);

#ifdef SYSDEB
	SysDebug(process)("SetSignalPort: %x",rc);
#endif
	port = mcb->MsgHdr.Reply;

	FreeMsgBuf(mcb);
#ifndef __TRAN
	FreePort(replyp);
#endif
	return port;
}


/*--------------------------------------------------------
-- TaskData						--
--							--
-- Get information relevant to this task.		--
-- Note: I was going to call this TaskInfo, but I 	--
-- already have a data structure with that name.	--
-- I suppose in theory this routine should apply to the --
-- IOC for these operations so we don't share memory.	--
-- 							--
--------------------------------------------------------*/

PUBLIC word TaskData(word code, void *value)
{
	word e = Err_Null;

	switch( code )
	{
	case TD_Program:
		*(MPtr *)value = MyTask->Program;
		break;

	case TD_Port:
		*(Port *)value = MyTask->Port;
		break;

	case TD_Parent:
		*(Port *)value = MyTask->Parent;
		break;

	case TD_HeapBase:
		*(byte **)value = MyTask->HeapBase;
		break;

	case TD_Flags:
		*(word *)value = MyTask->Flags;
		break;

	case TD_Pool:
		*(Pool **)value = &MyTask->MemPool;
		break;
		
	default:
		e = EC_Error|SS_SysLib|EG_Invalid;
	}

	return e;
}

/*--------------------------------------------------------
-- SetException						--
--							--
-- Set the tasks exception code and data.		--
--							--
-- In processors other than the transputer this is 	--
-- usually used for syncronous signals such as		--
-- SIGSEGV, SIGSTAK and SIGFPE. The handlers for these	--
-- basic hardware traps use CallException() to execute	--
-- the exception code setup here.			--
-- If the SignalPort has not been setup then this	--
-- exception code will also be forked to for		--
-- asyncronous signals as well.				--
-- SetException is usually called by the CLib startup	--
-- code.						--
-- 							--
--------------------------------------------------------*/

PUBLIC word SetException(WordFnPtr fn, byte *data, word datasize)
{
	byte *d = NULL;

	if (datasize > 0)
	{
		if ((d = (byte *)Malloc(datasize)) == NULL)
			return EC_Error|SS_SysLib|EG_NoMemory;
		else
			memcpy((byte *)d,data,(int)datasize);
	}

#ifdef SYSDEB
	SysDebug(process)("SetException(%x,%P,%d)",fn,data,datasize);
#endif

	MyTask->ExceptCode = (VoidFnPtr)fn;
	MyTask->ExceptData = d;

	return 0;
}

/*--------------------------------------------------------
-- SendSignal						--
--							--
-- Send an exception signal to a child task.		--
-- The stream is one returned from Opening the Object	--
-- returned from .					--
-- 							--
--------------------------------------------------------*/

PUBLIC word SendSignal(Stream *stream, word signal)
{
	word e = Err_Null;
	MCB m;
#ifdef SYSDEB
	SysDebug(process)("SendSignal(%S,%d)",stream,signal);
#endif

	if( (e = CheckStream(stream,C_ReOpen)) != Err_Null ) return e;

	Wait(&stream->Mutex);

	InitMCB(&m,MsgHdr_Flags_preserve,
		stream->Server,stream->Reply,FC_Private+FG_Signal|stream->FnMod);

	m.Control = &signal;
	m.MsgHdr.ContSize = 1;

	e = StreamMsg( &m, stream );

#ifdef SYSDEB
	SysDebug(process)("SendSignal: %E",e);
	
	if( m.MsgHdr.Reply != NullPort ) SysDebug(error)("SendSignal: Non-Null Reply port %x",m.MsgHdr.Reply);
#endif
	
 	if ( m.MsgHdr.Reply != NullPort )
		FreePort(m.MsgHdr.Reply);
	
	stream->Result2 = e;

	Signal(&stream->Mutex);

	return e;
}

/*--------------------------------------------------------
-- DefaultException					--
--							--
-- Default Exception handler, called from processor	--
-- manager. Simply performs an exit.			--
--							--
--------------------------------------------------------*/

PUBLIC word DefaultException(word reason, void *data)
{
	MCB m;
	word e;
#ifdef SYSDEB
	SysDebug(process)("DefaultException(%d,%P)",reason,data);
#endif
	TidyUp();

	InitMCB(&m,MsgHdr_Flags_preserve,MyTask->IOCPort,NullPort,
		EC_Error|SS_SysLib|EG_Exception|EE_Kill);

	e = 1;
	m.Control = &e;
	m.MsgHdr.ContSize = 1;

	e = PutMsg(&m);

	return 0;
	
	reason = reason;	/* keeps compiler happy */
	data = data;
}


/*--------------------------------------------------------
-- InitProgramInfo					--
-- GetProgramInfo					--
--							--
-- Program status protocol.				--
--							--
--------------------------------------------------------*/

static word InitProgramInfo1(Stream *stream, word mask );

PUBLIC word InitProgramInfo(Stream *stream, word mask )
{
	word e = Err_Null;
		
	if( (e = CheckStream(stream,C_ReOpen)) != Err_Null ) return e;

	Wait(&stream->Mutex);

	e = InitProgramInfo1(stream,mask);
	
	Signal(&stream->Mutex);
	
	return e;
}

static word InitProgramInfo1(Stream *stream, word mask )
{
	word tfsize = 0;
	word resbuf[2];		/* just below vsp threshold */
	word e = Err_Null;
	MCB m;

#ifdef SYSDEB
	SysDebug(process)("InitProgramInfo(%S,%x)",stream,mask);
#endif

	InitMCB(&m,MsgHdr_Flags_preserve,
		stream->Server,stream->Reply,FC_GSP+FG_ProgramInfo|stream->FnMod);

	resbuf[0] = mask;
	m.Control = resbuf;
	m.MsgHdr.ContSize = 1;

	if( (e = StreamMsg( &m, stream )) >= Err_Null )
		mask = resbuf[0],tfsize = resbuf[1];

	if( e < Err_Null ) stream->Result2 = e;
	else stream->Result2 = mask;
	
#ifdef SYSDEB
	SysDebug(process)("InitProgramInfo: %d mask/error %x port %x",tfsize,stream->Result2,stream->Reply);
	if( m.MsgHdr.Reply != NullPort ) SysDebug(error)("InitProgramInfo: Non-Null Reply port %x",m.MsgHdr.Reply);
#endif
	return tfsize;
}

PUBLIC word GetProgramInfo(Stream *stream, word *status_vec, word timeout)
{
	MCB m;
	word e;
#ifdef SYSDEB
	SysDebug(process)("GetProgramInfo(%S,%P,%d)",stream,status_vec,timeout);
#endif

	if( (e = CheckStream(stream,C_ReOpen)) != Err_Null ) return e;

	Wait(&stream->Mutex);

	InitMCB(&m,0,stream->Reply,0,0);
	m.Timeout = timeout;
	m.Control = status_vec;

	while( (e = GetMsg(&m)) < Err_Null )
	{
#ifdef SYSDEB
		SysDebug(error)("GetProgramInfo: GetMsg error %x",e);
#endif
		if( e == EK_Timeout ) break;
		if( InitProgramInfo1(stream, PS_Terminate) == 0) break;

		InitMCB(&m,0,stream->Reply,0,0);
		m.Timeout = timeout;
	}

#ifdef SYSDEB
	SysDebug(process)("GetProgramInfo %S: %E",stream,e);
	if( m.MsgHdr.Reply != NullPort ) SysDebug(error)("GetProgramInfo: Non-Null Reply port %x",m.MsgHdr.Reply);
#endif
	if( m.MsgHdr.Reply != NullPort ) FreePort(m.MsgHdr.Reply);
	Signal(&stream->Mutex);
	
	return e;
}

/* end of task.c */
