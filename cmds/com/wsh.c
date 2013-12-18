
static char *rcsid = "$Header: /hsrc/cmds/com/RCS/wsh.c,v 1.2 1990/08/23 10:40:41 james Exp $";

#include <syslib.h>
#include <gsp.h>
#include <stdio.h>
#include <string.h>

extern char *environ;

char *shell[2] = { "shell", NULL };

int main(int argc, char **argv)
{
	Object *code, *wm, *w, *pm;
	Object *prog, *objv[2];
	Stream *s, *strv[4];
	word e;
	char pmname[100];
	Environ env;

	if (argc > 2)
	{ printf("Usage: wsh [mcname]\n");
	  return 1;
	}

	wm = Locate(CurrentDir,"/window");

	if( wm == Null(Object))
	{
		printf("Cannot locate window : %x\n",Result2(CurrentDir));
		return 1;
	}

	strcpy(pmname,"Shell");
	if (argc > 1)
	{ 
		char *n = argv[1];
		char *p = n + strlen(n);
		while( p != n && *p != '/' ) p--;
		if( *p == '/' ) p++;
		strcat(pmname,".");
		strcat(pmname,p);
	}
	w = Create(wm,pmname,Type_File,NULL,NULL);

	if( w == Null(Object))
	{
		printf("Cannot create window : %x\n",Result2(wm));
		return 1;
	}


	if( argv[1] != NULL )
	{
		strcpy(pmname,"/");
		strcat(pmname,argv[1]);
		strcat(pmname,"/tasks");
		pm = Locate(NULL,pmname);
		if (pm == NULL)
		{  
			printf("Unable to locate processor %s\n",argv[1]);
			Delete(w, Null(char));
		   	return 1;
		}
	}
	else if ( (pm = Locate(NULL,"/tfm")) == Null(Object) ) 
		{pm = NULL;}

	if ( (code = Locate(NULL,"/loader/shell")) == Null(Object) )
		{ code = Locate(NULL,"/helios/bin/shell"); }



	prog = Execute(pm,code);

	if( prog == Null(Object))
	{
		printf("Cannot execute shell : %x\n",Result2(code));
		Delete(w, Null(char));
		return 1;
	}

	s = Open(prog,NULL,O_WriteOnly);

	if( s == Null(Stream) )
	{
 		printf("Cannot open %s : %x\n",&prog->Name,Result2(prog));
		Delete(w, Null(char));
		return 1;
	}

	
    objv[0] = (Object *)CurrentDir;
    objv[1] = Null(Object);

    strv[0] = Open(w,NULL,O_ReadOnly);
    strv[1] = Open(w,NULL,O_WriteOnly);
    strv[2] = Open(w,NULL,O_WriteOnly);
    strv[3] = Null(Stream);

    env.Argv = shell;
    env.Envv = (char **)environ; 
    env.Objv = &objv[0];
    env.Strv = &strv[0];

    e = SendEnv(s->Server,&env);

    return 0;
}
