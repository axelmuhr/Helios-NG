/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1988, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- init.c								--
--                                                                      --
--	A typical initialisation program				--
--                                                                      --
--	Author:  BLV 4/9/88						--
--               (based on code by NHG)                                 --
------------------------------------------------------------------------*/
/* $Id: oldinit.c,v 1.1 1991/02/28 16:06:49 paul Exp $ */

/**
*** This is a sample init program, which is placed at the end of a Helios
*** system image. Its main job is to start up the window manager, and run
*** a shell in a new window. This is not suitable for all applications, e.g.
*** for process control you would want to modify this so that it boots up the
*** Helios network and runs various programs in the nodes of the network.
***
*** Note that the entire system image, including this program, will be booted
*** into every Helios node. Hence it is a good idea to keep this program
*** fairly small. You will note that the program checks whether this node has
*** been booted from an IO processor, i.e. whether it is the first node in the
*** network, and only performs certain initialisations if so.
***
*** Also note that when this program is started only the main system libraries
*** are available. In particular, you cannot use any Posix or C library routines
*** in this program.
***
*** If you want to start up the network server and the task force manager inside
*** this program, you should #define TFM.
**/

#include <syslib.h>
#include <gsp.h>
#include <servlib.h>
#include <root.h>
#include <string.h>

#define LOGIN 1 

Object *root;

char *SccsId = "@(#)init.c	2.9	17/3/89 Copyright Perihelion Software Ltd.\n";

#ifdef DEMO
char *Release = 
"\fHelios V1.0 Serial 07494203\n"
"(c) Copyright 1988 Perihelion Software Ltd.\n"
"All Rights Reserved.\n\n\n"
"\n\n\n\n\n\t\t\tDemonstration Version\n"
"\n\n\n\tThis Operating System will self-destruct in 30 minutes\n\n";
#else
char *Release = 
"\fHelios V1.1 (Beta3) Serial 07494203\n"
"(c) Copyright 1988 Perihelion Software Ltd.\n"
"All Rights Reserved.\n";
#endif

void NewName(string name,word type,word flags,Port port,Matrix matrix);
int run(Object *w, string program, string *argv, bool wait);

/* The Logerror File for the Network Control System */

#define LogerrorServer "/logger"

#ifdef NETWORK
string nsargs[] = {
	"net_serv",
	"-r",
	"/helios/etc/default.map",
	NULL
};

#endif

string shellargs[] = {
	"-",
	"-i",
	NULL
};

#ifdef LOGIN
string loginargs[] = {
	"-",
	NULL
};
#endif

int main()
{
	Object *window_manager=Null(Object), *shell_window=Null(Object);
	Stream *window_stream=Null(Stream);
	LinkInfo *parent = NULL;
	int i;
	RootStruct *Root = GetRoot();
		
	/* locate my parent link */
	for( i=0; i < GetConfig()->Nlinks; i++ )
		if( Root->Links[i]->Flags & Link_Flags_parent )
			parent = Root->Links[i];
			
	root = Locate(NULL,"/");

	/* install stub names for servers which will only be loaded	*/
	/* when they are accessed. Eventually this will be the job of	*/
	/* the NetServer.						*/

	NewName("fifo",Type_Name,Flags_StripName,NullPort,DefDirMatrix);
	NewName("ram",Type_Name,Flags_StripName,NullPort,DefDirMatrix);
	NewName("null",Type_Name,Flags_StripName,NullPort,DefDirMatrix);
	NewName("pipe",Type_Name,Flags_StripName,NullPort,DefDirMatrix);	

#ifdef SYNCBOOT
	/* acknowledge startup to procman				*/
	{
		MCB m;
		word e;
		InitMCB(&m,0,MyTask->Parent,NullPort,0x456);
		e = PutMsg(&m);
	}
#endif


	/* only create Network Server or (shell & window manager) */
	/* if this is first processor to be booted in the network */
	if(parent == NULL || parent->Flags & Link_Flags_ioproc )
	{
#ifndef NETWORK
		window_manager = Locate(NULL, "/IO/window");
		if (window_manager == Null(Object))
		{
			NewName("window",Type_Name,Flags_StripName,NullPort,DefDirMatrix);
			window_manager = Locate(NULL, "/window");
		}

		if( window_manager != Null(Object) )
			shell_window = Create(window_manager,"Shell",Type_File,NULL,NULL);

		if( shell_window != Null(Object) )
			window_stream = Open(shell_window,NULL,O_WriteOnly);	/* to keep window in existance */

		if( window_stream == Null(Stream) )
		{
			IOdebug("Cannot find Window Server");
			for(;;);
		}

		Write(window_stream,Release,strlen(Release),-1);
		

#ifdef LOGIN
		if( !run(shell_window,"/helios/bin/login",&(loginargs[0]),false) )
#endif
		if( !run(shell_window,"/helios/bin/shell",&(shellargs[0]),false) )
		{
			IOdebug("Cannot execute shell!!");
			for(;;);
		}

		Close(shell_window);
		Close(window_manager);
		Close(window_stream);
#else
		{
		Object *Console = Null(Object);

		window_manager = Locate(NULL, "/IO/window");
		if (window_manager != Null(Object))
		{
			Console = Create(window_manager,"Console",Type_File,NULL,NULL);
		}
		else
		{
			Console = Locate(Null(Object),LogerrorServer);

		}

		/* Just run the Network Server It will do the rest */

		run(Console,"/helios/lib/net_serv",&(nsargs[0]),false); 
		}

#endif
	}

	Exit(0);

	return 0;
}

void NewName(string name,word type,word flags,Port port,Matrix matrix)
{
	NameInfo info;

	info.Port = port;
	info.Flags = flags;
	info.Matrix = matrix;
	info.LoadData = NULL;

	Create(root,name,type,sizeof(NameInfo),(byte *)&info);
}

int run(Object *w,string program, string *argv, bool wait)
{
	Object *code;
	Object *prog, *objv[2];
	Stream *s, *strv[4];
        char   *dummy = Null(char);
	word e;
	Environ env;
	char mcname[50];

	code = Locate(NULL,program);

	prog = Execute(NULL,code);

	if( prog == Null(Object)) return false;

	s = Open(prog,NULL,O_WriteOnly);

	if( s == Null(Stream) ) return false;

	MachineName(mcname);

	objv[0] = Locate(NULL,"/helios");
	objv[1] = Null(Object);

	strv[0] = Open(w,NULL,O_ReadOnly);
	strv[1] = Open(w,NULL,O_WriteOnly);
	strv[2] = Open(w,NULL,O_WriteOnly);
	strv[3] = Null(Stream);

	env.Argv = argv;
	env.Envv = &dummy; 
	env.Objv = &objv[0];
	env.Strv = &strv[0];
	
	e = SendEnv(s->Server,&env);

	if( wait )
	{
		MCB m;

		InitMCB(&m,0,prog->Reply,NullPort,0);
		m.Timeout = MaxInt;
		while((e = GetMsg(&m)) == EK_Timeout);
	}

	Close(code);
	Close(prog);
	Close(s);
	Close(objv[0]);
	Close(strv[0]);
	Close(strv[1]);
	Close(strv[2]);
	
	return true;
}
