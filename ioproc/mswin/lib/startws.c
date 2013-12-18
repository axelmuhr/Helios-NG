/************************************************************************/
/*                                                                      */
/*           Windows I/O server Help and New Shell Support              */
/*               (C) 1993 Perihelion Software Limited                   */
/*                                                                      */
/************************************************************************/

#include <stdio.h>
#include <syslib.h>
#include <stdlib.h>
#include <nonansi.h>
#include <posix.h>
#include <string.h>
#include <servlib.h>
#include <signal.h>
#include "windows.h"
#include "ddeml.h"

#define eq ==
#define ne !=
#define IDM_NEWSH       107
#define IDM_HELP        108

/**
*** find_file() is not currently located. It searches through the command
*** search path for the specified string.
**/
extern void     find_file(char *, char *);
extern void     end_server(void);
/**
*** forward declarations.
**/
static void     term_handler(void);
static void     RunIt(char *command);
static int      FindMenu(HMENU hMenu, char *str);
static Object   *HCreateWindow(char *);
static Object   *HGetConsoleWindow(void);
static int      RunCommand(char **, Object *, bool);
static void     mysignalhandler(int);
static Object   *running_command;
HDDEDATA CALLBACK DdeCallback(UINT, UINT, HCONV, HSZ, HSZ, HDDEDATA, DWORD, DWORD);

/**
*** static global variables
**/

static HMENU hFileMenu;
static HMENU hHelpPopup;
static HMENU hMenu;
static HWND  hWnd;

static DWORD idInst;
static HSZ   hszHeliosServer;
static HSZ   hszSystemTopic;
static BOOL  bConnected = FALSE;
static BOOL  bCreateWindow;

char *newsh_command = "/helios/bin/shell";
char *help_command = "/helios/bin/help";


int WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpszCmdLine, int nCmdShow)
{
  /* this main routine is very simple, in that it modifies the I/O server */
  /* menu, registers with the I/O server (so that callbacks are processed */
  /* correctly) and then sits in the message loop until terminated.       */
  int   i;
  MSG   msg;
  struct sigaction sig;
  char *strData;

  hWnd = FindWindow("Shells", NULL);
  if (hWnd eq NULL)
  { printf("Unable to find console\n");
    return 255;
  }
  hMenu = GetMenu(hWnd);
  if (hMenu eq NULL)
  { printf("Unable to find menu\n");
    return 255;
  }

  /* install an asynchronous SIGTERM handler */
  if (sigaction(SIGTERM, Null(struct sigaction), &sig) != 0)
  { printf("SIGTERM error\n");
    return 255;
  }
  sig.sa_handler = (VoidFnPtr)&term_handler;
  sig.sa_flags |= SA_ASYNC;
  if (sigaction(SIGTERM, &sig, Null(struct sigaction)) != 0)
  { printf("SIGTERM error\n");
    return 255;
  }

  /* search for help - if there the program is already loaded */
  i = FindMenu(hMenu, "&Help");
  if (i ne -1)
  { printf("startws already loaded\n");
    return 255;
  }

  i = FindMenu(hMenu, "&File");
  if (i eq -1)
  { printf("File menu not found\n");
    return 255;
  }

  hFileMenu = GetSubMenu(hMenu, i);
  InsertMenu(hFileMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
  InsertMenu(hFileMenu, 0, MF_BYPOSITION | MF_STRING, IDM_NEWSH, "&New shell");
  hHelpPopup = CreatePopupMenu();
  AppendMenu(hHelpPopup, MF_STRING, IDM_HELP, "&Help");
  AppendMenu(hMenu, MF_POPUP, hHelpPopup, "&Help");
  DrawMenuBar(hWnd);

  /* now register the two commands with the I/O server */
  RegisterIOMenu(IDM_NEWSH);
  RegisterIOMenu(IDM_HELP);

  /* Now register the DDE server */
  idInst = 0;
  DdeInitialize(&idInst, DdeCallback, APPCLASS_STANDARD, 0);
  if (idInst ne 0)
  {
    hszHeliosServer = DdeCreateStringHandle(idInst, "Helios", CP_WINANSI);
    hszSystemTopic = DdeCreateStringHandle(idInst, "System", CP_WINANSI);
    DdeNameService(idInst, hszHeliosServer, NULL, DNS_REGISTER);
  }

  while (GetMessage(&msg, NULL, NULL, NULL))
  { if (msg.message eq WM_COMMAND)
    {
      if (msg.wParam eq IDM_NEWSH)
      {
        bCreateWindow = TRUE;
        strData = (char *)malloc(strlen(newsh_command)+1);
        strcpy(strData, newsh_command);
        Fork(2000, RunIt, sizeof(char *), strData);
      }
      else
        if (msg.wParam eq IDM_HELP)
        {
          bCreateWindow = TRUE;
          strData = (char *)malloc(strlen(help_command)+1);
          strcpy(strData, help_command);
          Fork(2000, RunIt, sizeof(char *), strData);
        }
    }
  }

  return 0;
}


HDDEDATA CALLBACK DdeCallback(UINT wType, UINT wFmt, HCONV hConv,
        HSZ hsz1, HSZ hsz2, HDDEDATA hData, DWORD dwData1, DWORD dwData2)
{
    char str[500];

    switch (wType)
    {
        case XTYP_CONNECT_CONFIRM:
            if (bConnected)
                DdeDisconnect(hConv);

            if (DdeCmpStringHandles(hsz1, hszSystemTopic) ||
                DdeCmpStringHandles(hsz2, hszHeliosServer))
                DdeDisconnect(hConv);

            bConnected = TRUE;
            bCreateWindow = TRUE;
            break;

        case XTYP_DISCONNECT:
            bConnected = FALSE;
            break;

        case XTYP_EXECUTE:
            {
                char *pos, *end;
                char *cmd;
                BOOL OK = TRUE;

                DdeGetData(hData, str, 500, 0);

                pos = strchr(str, '[');
                if (pos == NULL)
                    break;

                do {
                    pos++;
                    end = strchr(pos, ']');
                    if (end == NULL)
                    {
                        OK = FALSE;
                        break;
                    }
                    *end = '\0';

                    cmd = (char *)malloc(strlen(pos)+1);
                    strcpy(cmd, pos);
                    if (!strcmp(cmd, "nowindow"))
                    {
                        bCreateWindow = FALSE;
                        free(cmd);
                    }
                    else if (!strcmp(cmd, "createwindow"))
                    {
                        bCreateWindow = TRUE;
                        free(cmd);
                    }
                    else
                    {
                        Fork(2000, RunIt, sizeof(char *), cmd);
                    }

                    *end = ']';
                } while (((pos = strchr(pos, '[')) != NULL) && OK);

                if (OK)
                    return DDE_FACK;
            }
            break;
    }
    return 0;
}

static void term_handler(void)
{ int i;
  i = FindMenu(hMenu, "&File");
  if (i >= 0)
  { hFileMenu = GetSubMenu(hMenu, i);
    i = FindMenu(hFileMenu, "&New shell");
    if (i >= 0)
    { DeleteMenu(hFileMenu, i, MF_BYPOSITION);
      if ((GetMenuState(hFileMenu, 0, MF_BYPOSITION) & MF_SEPARATOR) eq MF_SEPARATOR)
        DeleteMenu(hFileMenu, 0, MF_BYPOSITION);
    }
  }

  i = FindMenu(hMenu, "&Help");
  if (i >= 0)
  { hHelpPopup = GetSubMenu(hMenu, i);
    DeleteMenu(hMenu, i, MF_BYPOSITION);
    DestroyMenu(hHelpPopup);
  }

  DrawMenuBar(hWnd);
  DdeNameService(idInst, hszHeliosServer, NULL, DNS_UNREGISTER);
  DdeFreeStringHandle(idInst, hszHeliosServer);
  DdeFreeStringHandle(idInst, hszSystemTopic);
  DdeUninitialize(idInst);

  end_server();
  _exit(SIGTERM);
}


static int FindMenu(HMENU hMenu, char *strLook)
{
  int count, i;

  count = GetMenuItemCount(hMenu);
  for (i=0; i<count; i++)
  { char str[20];
    GetMenuString(hMenu, i, str, 20, MF_BYPOSITION);
    if (!strcmp(str, strLook))
      return i;
  }

  return -1;
}

static char **BuildArgs(char *command)
{ int i, count;
  char *pos;
  char **args;

  count = 1;
  pos = strchr(command, ' ');
  while (pos)
  { count++;
    pos++;
    while ((pos[0] != '\0') && (pos[0] == ' '))
      memmove(&pos[0], &pos[1], strlen(pos));
    pos = strchr(pos, ' ');
  }

  args = (char **)malloc((count+1)*sizeof(char *));

  for (i=0; i<count; i++)
  { pos = strchr(command, ' ');
    if (pos)
      *pos = '\0';
    args[i] = command;
    command = pos+1;
  }

  args[count] = (char *)NULL;
  return args;
}

static void FreeArgs(char ***p_command_args)
{
  free(*p_command_args);
  *p_command_args = (char **)NULL;
}

static void RunIt(char *command)
{ char          **command_args;
  bool          wait_for_child = TRUE;
  Object        *window;
  int           rc;
  bool          args_created;
  static char   *default_args[] = {"shell", Null(char) };

  args_created = FALSE;
  if (command eq NULL)
    command_args = default_args;
  else
  { command_args = BuildArgs(command);
    args_created = TRUE;
  }
  if (bCreateWindow)
  {
    window = HCreateWindow(command_args[0]);
    if (window eq Null(Object)) return;
  }
  else
  {
    window = HGetConsoleWindow();
    if (window eq Null(Object)) return;
  }
  rc = RunCommand(command_args, window, wait_for_child);
  (void) Delete(window, Null(char));
  if (args_created)
    FreeArgs(&command_args);
  free(command);
  return;
}

/**
*** Creating a new window. This is done by getting an object for the
*** current console server out of the environment.
**/
static Object *HCreateWindow(char *command_name)
{ Object        *window_server;
  Object        *new_window;
  char          buffer[NameMax];
  Environ       *env = getenviron();

  if (env eq Null(Environ))
   { fputs("run: corrupt environment.\n", stderr);
     return(Null(Object));
   }

  { Object      **objv = env->Objv;
    int         i;
    for (i = 0; i <= OV_CServer; i++)
     if (objv[i] eq Null(Object))
      { fputs("run: incomplete environment.\n", stderr);
        return(Null(Object));
      }
  }

  window_server = env->Objv[OV_CServer];
  if (window_server eq (Object *) MinInt)
   { fputs("run: there is no window server in the current environment.\n",
                 stderr);
     return(Null(Object));
   }

  strncpy(buffer, objname(command_name), NameMax);
  buffer[NameMax - 1] = '\0';
  new_window = Create(window_server, buffer, Type_Stream, 0, Null(BYTE));
  if (new_window eq Null(Object))
   fprintf(stderr, "run : failed to Create window %s/%s", window_server->Name,
                buffer);
  return(new_window);
}

static Object *HGetConsoleWindow(void)
{ Environ       *env = getenviron();

  if (env eq Null(Environ))
   { fputs("run: corrupt environment.\n", stderr);
     return(Null(Object));
   }

  { Object      **objv = env->Objv;
    int         i;
    for (i = 0; i <= OV_CServer; i++)
     if (objv[i] eq Null(Object))
      { fputs("run: incomplete environment.\n", stderr);
        return(Null(Object));
      }
  }

  return env->Objv[OV_Console];
}

/**
*** This runs a command using Helios calls only. An attempt is made to
*** open the specified window. If successful the environment is built
*** up, and an attempt is made to locate the program. If successful the
*** program is loaded into memory on the same processor, executed
*** locally, and is sent its environment. Unless the detach option has
*** been given some signal handling is done, so that ctrl-C is forwarded
*** to the child process. Also, run may or may not wait for the child to
*** terminate.
**/
static int RunCommand(char **command_args, Object *window, bool wait_for_child)
{ char          command_name[IOCDataMax];
  Stream        *window_stream = Open(window, Null(char), O_ReadWrite);
  Object        *objv[OV_End + 1];
  Stream        *strv[5];
  Environ       *my_environ = getenviron();
  Environ       sending;
  Stream        *program_stream = Null(Stream);
  int           rc = (int)Err_Null;

  objv[OV_Cdir]         = my_environ->Objv[OV_Cdir];
  objv[OV_Task]         = (Object *) MinInt;
  objv[OV_Code]         = (Object *) MinInt;
  objv[OV_Source]       = (Object *) MinInt;
  objv[OV_Parent]       = my_environ->Objv[OV_Task];
  objv[OV_Home]         = my_environ->Objv[OV_Home];
  objv[OV_Console]      = window;
  objv[OV_CServer]      = my_environ->Objv[OV_CServer];
  objv[OV_Session]      = my_environ->Objv[OV_Session];
  objv[OV_TFM]          = my_environ->Objv[OV_TFM];
  objv[OV_TForce]       = (Object *) MinInt;
  objv[OV_End]          = Null(Object);

  if (window_stream eq Null(Stream))
   { fprintf(stderr, "run : failed to open window %s\n", window->Name);
     goto fail;
   }

  window_stream->Flags |= Flags_OpenOnGet;
  strv[0] = window_stream;
  strv[1] = strv[2] = CopyStream(window_stream);
  if (strv[1] eq NULL)
   { fprintf(stderr, "run: out of memory\n");
     goto fail;
   }
  strv[0]->Flags &= ~O_WriteOnly;
  strv[1]->Flags &= ~O_ReadOnly;
  strv[3] = my_environ->Strv[3];
  strv[4] = (Stream *) MinInt;

  sending.Strv = strv;
  sending.Objv = objv;
  sending.Envv = my_environ->Envv;
  sending.Argv = command_args;

  if (*(command_args[0]) eq '/')
   strcpy(command_name, command_args[0]);
  else
   find_file(command_name, command_args[0]);

  objv[OV_Source] = Locate(CurrentDir, command_name);
  if (objv[OV_Source] eq Null(Object))
   { fprintf(stderr, "run : failed to locate command %s\n", command_args[0]);
     goto fail;
   }

  objv[OV_Code] = (Object *) MinInt;

  if (getenv("CDL") ne Null(char))
   { Object     *tfm = my_environ->Objv[OV_TFM];
     int        i;
     for (i = 0; i < OV_TFM; i++)
      if (my_environ->Objv[i] eq Null(Object))
       { tfm = Null(Object); break; }
     if (tfm eq (Object *) MinInt) tfm = Null(Object);
     objv[OV_Task] = Execute(tfm, objv[OV_Source]);
   }
  else  /* run it locally */
   objv[OV_Task] = Execute(Null(Object), objv[OV_Source]);

  if (objv[OV_Task] eq Null(Object))
   { fprintf(stderr, "run: failed to execute command %s\n",
        objv[OV_Source]->Name);
     goto fail;
   }
  program_stream = Open(objv[OV_Task], Null(char), O_ReadWrite);
  if (program_stream eq Null(Stream))
   { fprintf(stderr, "run: failed to open task %s\n", objv[OV_Task]->Name);
     goto fail;
   }

  running_command = objv[OV_Task];

  if (wait_for_child)
   { struct sigaction   temp;
     if (sigaction(SIGINT, Null(struct sigaction), &temp) ne 0)
      { fprintf(stderr, "run: warning, failed to access signal handling facilities.\n");
        goto skip_signal;
      }
     temp.sa_handler    = &mysignalhandler;
     temp.sa_flags      |= SA_ASYNC;
     if (sigaction(SIGINT, &temp, Null(struct sigaction)) ne 0)
      fprintf(stderr, "run: warning, failed to modify signal handling facilities.\n");
   }
skip_signal:

  (void) SendEnv(program_stream->Server, &sending);

  if (wait_for_child)
   { if (InitProgramInfo(program_stream, PS_Terminate) < Err_Null)
      { fprintf(stderr, "run: failed to wait for task %s\n",
                objv[OV_Task]->Name);
        goto done;
      }
     rc = (int)GetProgramInfo(program_stream, (word*)NULL, -1);
     if (rc ne 0)
      { rc = rc >> 8;   /* ignore bottom byte */
        Delay(OneSec / 2);
      }
   }
  else
   Delay(OneSec);

done:
  Close(window_stream);
  Close(program_stream);
  Close(objv[OV_Task]);
  Close(objv[OV_Source]);
  return(rc);

fail:
  if (window_stream ne Null(Stream)) Close(window_stream);
  if (program_stream ne Null(Stream)) Close(program_stream);
  if (objv[OV_Task] ne Null(Object))
   { (void) Delete(objv[OV_Task], Null(char));
     (void) Close(objv[OV_Task]);
   }
  if (objv[OV_Source] ne Null(Object)) Close(objv[OV_Source]);
  return(EXIT_FAILURE);
}

static void mysignalhandler(int x)
{ Stream        *program_stream = PseudoStream(running_command, O_ReadWrite);
  if (program_stream ne Null(Stream))
   { SendSignal(program_stream, SIGINT);
     Close(program_stream);
   }
  x = x;
}
