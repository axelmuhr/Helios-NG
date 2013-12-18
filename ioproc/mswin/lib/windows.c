/************************************************************************/
/************************************************************************/
/*               Microsoft Windows Libraries for Helios                 */
/*                                                                      */
/*       Copyright (C) 1990-1993,  Perihelion Software Limited          */
/*                         All Rights Reserved                          */
/*                                                                      */
/*   windows.c    to build windows.lib                                  */
/************************************************************************/
/************************************************************************/

#include <syslib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <codes.h>
#include <nonansi.h>
#include <unistd.h>
#include <signal.h>
#include "windows.h"                /* include the std API header file */

#include "windefs.h"

/***** static variables required by the library *************************/

static List Class_List;                   /* list of registered classes */
static List Window_List;                      /* list of opened windows */
static List hdata_list;                    /* list of HDDEDATA accesses */
static List DdeInstance_List;          /* list of initialized instances */
static Stream *MsWin=NULL;   /* the port to /graphics on the I/O server */
static MCB  mcb;                               /* the mcb for all sends */
static word buf_size;                           /* the send buffer size */
static HINSTANCE hInst;                      /* the app instance handle */
static char appname[13];               /* the assigned application name */
static LPSTR cmdline=NULL;                   /* a malloc'd command line */
static Semaphore SendSem;      /* to single-thread sending to /graphics */
static Semaphore AccessSem;   /* for gaining access to the message list */
static Semaphore MsgSem;                           /* message semaphore */
static Semaphore BufferSem;                    /* buffer full semaphore */
static MSG MsgList[MSGBUFSIZE];                 /* will this be enough? */
static int nBuffer;                       /* msg buffer insertion point */
static Port CallbackPort;        /* to which all incoming msgs are sent */
static int app_argc;
static char **app_argv;

/***** internal function prototypes *************************************/
PRIVATE int init_server(void);
PRIVATE void term_handler(void);
PUBLIC  void end_server(void);
PRIVATE void get_cmdline(int argc, char **argv);
PRIVATE Port register_port(void);
PRIVATE void receive_messages(Port);
PRIVATE BOOL add_class(LPCSTR, WNDPROC);
PRIVATE BOOL add_window(Classnode *, HWND);
PRIVATE Classnode *find_class(LPCSTR);
PRIVATE Windownode *find_window(HWND);
PRIVATE BOOL remove_class(Classnode *);
PRIVATE void remove_window(HWND);
PRIVATE void clear_class(void);
PRIVATE void clear_window(void);
PRIVATE void clear_instance(void);
#ifdef never
PRIVATE BOOL IsDeferredMsg(UINT);
#endif
PRIVATE BOOL Ack(word);
PRIVATE BOOL NegAck(word);
PRIVATE HGLOBAL RegisterBuffer(word);
PRIVATE BOOL AppendBuffer(HGLOBAL, word, BYTE *, word);
PRIVATE word GetBuffer(HGLOBAL, word, BYTE *, word);

/***** Start of actual functions ****************************************/

/* This is main() which is supplied by the library - when writing       */
/* windows programs, the user supplies a WinMain function for startup.  */

/* The main routine basically locates the /graphics server running on   */
/* the windows I/O server, allocates a port to which all callback       */
/* messages will be posted, spawns a routine to receive those messages, */
/* and registers the port with the /graphics server with a call to      */
/* IO_RegisterPort (function 0 of the graphics server).  It also        */
/* installs a Control-Break handler for tidying up when a program is    */
/* terminated.  The WinMain routine is called, and when control returns */
/* to main, will perform the tidying of the application.                */

int main(int argc, char **argv)
{
    int   FnRc;

    FnRc = init_server();
    if (FnRc == 0)
    {
        /* now register port */
        if ((CallbackPort = register_port()) == NullPort)
        {
            FnRc = 3;
            end_server();
        }
        if (FnRc == 0)
        {
            /* Spawn receiving routine */
            if (!Fork(2000, receive_messages, sizeof(Port), CallbackPort))
            {
            FnRc = 4;
            FreePort(CallbackPort);
            end_server();
            }
        }
    }

    if (FnRc != 0)
    {
    /* unable to initialise server, so return error */
    switch (FnRc)
    {
        case 1:
        printf("This program requires the Helios Microsoft Windows I/O Server.\n");
        break;

        default:
        printf("Initialisation error %d\n", FnRc);
        break;
    }

    return FnRc;
    }

    get_cmdline(argc, argv);
    app_argc = argc;
    app_argv = argv;
    FnRc = WinMain(hInst, (HINSTANCE)NULL, cmdline, SW_NORMAL);
    end_server();
    return FnRc;
}


/************************************************************************/
/* init_server                                                          */
/*                                                                      */
/* Parameters                                                           */
/*      none                                                            */
/*                                                                      */
/* Returns                                                              */
/*      0 if successful, else an error code (1 = server not found,      */
/*      2 = server error)                                               */
/*                                                                      */
/* Side-effects                                                         */
/*      GrPort is allocated a port number and the send buffer is        */
/*      allocated.                                                      */
/************************************************************************/

PRIVATE int init_server(void)
{
    Object           *graph_object;
    Object           *test_object;
    Object           *window_server;
    int              app, i;
    struct sigaction sig;
    Object           **objv;
    Environ          *env = getenviron();
    char             buffer[50];
    char             *pos;

    if (env == (Environ *)NULL)
        return 1;

    objv = env->Objv;
    for (i=0; i <= OV_CServer; i++)
        if (objv[i] == (Object *)NULL)
            return 1;

    window_server = env->Objv[OV_CServer];
    if (window_server == (Object *)MinInt)
        return 1;

    strcpy(buffer, window_server->Name);
    pos = strrchr(buffer, '/');
    *pos = '\0';
    strcat(buffer, "/graphics");
    graph_object = Locate(NULL, buffer);

    if (graph_object == (Object *)NULL)
        return 1;

    /* get a unique name */
    for (app=1; ;app++)
    {
        sprintf(appname, "winapp.%03d", app);
        test_object = Locate(graph_object, appname);
        if (test_object == (Object *)NULL)
            break;
        Close(test_object);
    }

    MsWin = Open(graph_object, appname, O_ReadWrite | O_Create);
    if (MsWin == (Stream *)NULL)
        return 2;

    InitSemaphore(&SendSem, 1);
    InitSemaphore(&AccessSem, 1);
    InitSemaphore(&MsgSem, 0);
    nBuffer = 0;
    InitSemaphore(&BufferSem, MSGBUFSIZE);
    InitList(&Class_List);
    InitList(&Window_List);
    InitList(&hdata_list);
    InitList(&DdeInstance_List);

    /* install an asynchronous SIGTERM handler */
    if (sigaction(SIGTERM, Null(struct sigaction), &sig) != 0)
    {
        /* cannot access signal facilities */
        end_server();
        return 5;
    }
    sig.sa_handler = &term_handler;
    sig.sa_flags |= SA_ASYNC;
    if (sigaction(SIGTERM, &sig, Null(struct sigaction)) != 0)
    {
        /* did not install */
        end_server();
        return 6;
    }

    return 0;
}

PRIVATE void term_handler()
{
    end_server();
    _exit(SIGTERM);
}

/************************************************************************/
/* end_server                                                           */
/*                                                                      */
/* Side-effects                                                         */
/*      This does any tidying to terminate the server connection,       */
/*      that is deletes the object.                                     */
/************************************************************************/

PUBLIC void end_server(void)
{
    Object *graph_object = Locate(NULL, "/graphics");

    Signal(&AccessSem);

    if (cmdline != (LPSTR)NULL)
        free(cmdline);
    if (MsWin != (Stream *)NULL)
        Close(MsWin);

    if (graph_object == (Object *)NULL)
        return;
    Delete(graph_object, appname);

    /* remove allocated data (class, window and ddeinstance lists) */
    clear_class();
    clear_window();
    clear_instance();

    return;
}


/************************************************************************/
/* get_cmdline                                                          */
/************************************************************************/

PRIVATE void get_cmdline(int argc, char **argv)
{
    int i, length = 0;

    if (argc < 2)
        return;

    for (i=1; i<argc; i++)
    length += strlen(argv[i]) + 1;
    length++;
    cmdline = (LPSTR)malloc(length);
    cmdline[0] = '\0';
    for (i=1; i<argc; i++)
    {
        strcat(cmdline, argv[i]);
        strcat(cmdline, " ");
    }
}


/************************************************************************/
/* register_port                                                        */
/*                                                                      */
/* Parameters                                                           */
/*      none                                                            */
/*                                                                      */
/* Returns                                                              */
/*      The allocated port, or NULL if an error occurred                */
/************************************************************************/

PRIVATE Port register_port(void)
{
	Port	new_port = NewPort();
	word	control[2];
	
	if (new_port != NullPort)
	{
		Wait(&SendSem);
		InitMCB(&mcb, MsgHdr_Flags_preserve, MsWin->Server,
			 new_port, IO_RegisterPort | Graph_Message);
		mcb.Timeout	= 60 * OneSec;
		mcb.Control	= control;
		if (PutMsg(&mcb) != Err_Null)
		{
			FreePort(new_port);
			new_port	= NullPort;
		}
		else
		{
			mcb.MsgHdr.Dest	= new_port;
			if (GetMsg(&mcb) != Err_Null)
			{
				FreePort(new_port);
				new_port	= NullPort;
			}
			else
			{
				buf_size	= control[0];
				hInst		= (HINSTANCE) control[1];
			}
		}
		Signal(&SendSem);
	}

    return new_port;
}

/************************************************************************/
/* send_message                                                         */
/*                                                                      */
/* Parameters                                                           */
/*      FnRc     - The requested IO_XXXX function number                */
/*      ContSize - The control vector size                              */
/*      DataSize - The data vector size                                 */
/*                                                                      */
/* Returns                                                              */
/*      TRUE if successful, otherwise FALSE                             */
/*                                                                      */
/* Side-effects                                                         */
/*      Any returns will be in the control or data vectors              */
/************************************************************************/

BOOL send_message(int FnRc, int ContSize, int DataSize,
                  word *Control, BYTE *Data)
{
    Port Reply;
    BOOL bFreePort = TRUE;

    Wait(&SendSem);
    Reply = NewPort();
    if (Reply == NullPort)
    {
        Reply = MsWin->Reply;
        bFreePort = FALSE;
    }
    InitMCB(&mcb, MsgHdr_Flags_preserve, MsWin->Server, Reply,
            Graph_Message | FnRc);

    mcb.Timeout = OneSec * 60;
    mcb.Control = Control;
    mcb.MsgHdr.ContSize = ContSize;
    mcb.Data = Data;
    mcb.MsgHdr.DataSize = DataSize;

    if (PutMsg(&mcb) != Err_Null)
    {
        if (bFreePort)
            FreePort(Reply);
        Signal(&SendSem);
        return FALSE;
    }

    mcb.MsgHdr.Dest = Reply;
    if (GetMsg(&mcb) != Err_Null)
    {
        if (bFreePort)
            FreePort(Reply);
        Signal(&SendSem);
        return FALSE;
    }
    if (bFreePort)
        FreePort(Reply);

    if ((DataSize != mcb.MsgHdr.DataSize) && (mcb.MsgHdr.DataSize != 0))
    {
        fprintf(stderr, "msg ERROR: DataSize - expecting %d, received %d", DataSize, mcb.MsgHdr.DataSize);
        fprintf(stderr, ",FnRc = %d\n", FnRc);
    }

    Signal(&SendSem);
    return TRUE;
}

/************************************************************************/
/* send_fn                                                              */
/*                                                                      */
/* Purpose                                                              */
/*      This function is the same as above (parameters etc), except it  */
/*      is called when the data is in the Control buffer and the return */
/*      results are expected in the Data buffer.                        */
/************************************************************************/

BOOL send_fn(int FnRc, int ContSize, int DataSize,
                 word *Control, BYTE *Data)
{
    Port Reply;
    BOOL bFreePort = TRUE;

    Wait(&SendSem);
    Reply = NewPort();
    if (Reply == NullPort)
    {
        Reply = MsWin->Reply;
        bFreePort = FALSE;
    }
    InitMCB(&mcb, MsgHdr_Flags_preserve, MsWin->Server, Reply,
            Graph_Message | FnRc);

    mcb.Timeout = OneSec * 60;
    mcb.Control = Control;
    mcb.MsgHdr.ContSize = ContSize;
    mcb.Data = NULL;
    mcb.MsgHdr.DataSize = 0;

    if (PutMsg(&mcb) != Err_Null)
    {
        if (bFreePort)
            FreePort(Reply);
        Signal(&SendSem);
        return FALSE;
    }

    mcb.MsgHdr.Dest = Reply;
    mcb.Control = Control;
    mcb.MsgHdr.ContSize = ContSize;
    mcb.Data = Data;
    mcb.MsgHdr.DataSize = DataSize;
    if (GetMsg(&mcb) != Err_Null)
    {
        if (bFreePort)
            FreePort(Reply);
        Signal(&SendSem);
        return FALSE;
    }
    if (bFreePort)
        FreePort(Reply);

/*    if (DataSize != mcb.MsgHdr.DataSize)
    {
        fprintf(stderr, "fn ERROR: DataSize - expecting %d, received %d", DataSize, mcb.MsgHdr.DataSize);
        fprintf(stderr, ",FnRc = %d\n", FnRc);
    }
*/

    Signal(&SendSem);
    return TRUE;
}

/************************************************************************/
/* receive_messages                                                     */
/*                                                                      */
/* This function is spawned off during main() to receive the messages   */
/* sent on the Port passed as a parameter.  These are messages from     */
/* Windows (like the WM_XXX messages).                                  */
/*                                                                      */
/* Parameters                                                           */
/*      port    - The port on which the messages arrive                 */
/************************************************************************/

PRIVATE void receive_messages(Port port)
{
    MCB mcb;
    word sequence = 0L;
    word Control[5];
    word *msg_sequence = &Control[0];
    word *hwnd = &Control[1];
    word *message = &Control[2];
    word *wParam = &Control[3];
    word *lParam = &Control[4];

    forever
    {
        Wait(&BufferSem);
        InitMCB(&mcb, MsgHdr_Flags_preserve, port, NullPort, 0);
        mcb.Data = NULL;
        mcb.MsgHdr.DataSize = 0;
        mcb.Control = msg_sequence;
        mcb.MsgHdr.ContSize = 5;
        mcb.Timeout = OneSec / 2;
        if (GetMsg(&mcb) == Err_Null)
        {
            /* data received so check WM_QUIT, sequence no */
            if (*message == WM_QUIT)
            {
                Wait(&AccessSem);
                MsgList[nBuffer].hwnd = (HWND)*hwnd;
                MsgList[nBuffer].message = (UINT)*message;
                MsgList[nBuffer].wParam = (WPARAM)*wParam;
                MsgList[nBuffer].lParam = (LPARAM)*lParam;
                nBuffer++;
                Signal(&MsgSem);
                Signal(&AccessSem);
            }
            else if (*msg_sequence == sequence)
            {
                /* correct sequence number, ack if modulo 32 */
                Wait(&AccessSem);
                MsgList[nBuffer].hwnd = (HWND)*hwnd;
                MsgList[nBuffer].message = (UINT)*message;
                MsgList[nBuffer].wParam = (WPARAM)*wParam;
                MsgList[nBuffer].lParam = (LPARAM)*lParam;
                nBuffer++;
                Signal(&MsgSem);
                Signal(&AccessSem);
                if ((sequence % 32) == 0)
                        Ack(sequence);
                sequence++;
            }
            else if (*msg_sequence > sequence)
            {
            /* wrong one received, so negative ackowledge */
                NegAck(sequence);
            }
            /* if < expected number, discard */
        }
        else
        {
            Signal(&BufferSem);
            Signal(&MsgSem);    /* release GetMsg to check for msgs */
        } /* if GetMsg */
    } /* forever */
}


/************************************************************************/
/* add_class                                                            */
/*                                                                      */
/* Parameters                                                           */
/*      name    - The name of the class being added                     */
/*      Callback- The callback function called for that class of window */
/*                                                                      */
/* Returns                                                              */
/*      TRUE if successful, otherwise FALSE                             */
/************************************************************************/

PRIVATE BOOL add_class(LPCSTR name, WNDPROC Callback)
{
    Classnode *classnode = (Classnode *)malloc(sizeof(Classnode));
    if (classnode == (Classnode *)NULL)
            return FALSE;
    classnode->lpszClassName = (LPCSTR)malloc(strlen(name)+1);
    classnode->Callback = Callback;
    classnode->nWindows = 0;
    strcpy((char *)&(classnode->lpszClassName[0]), (char *)&name[0]);
    AddTail(&Class_List, &(classnode->node));
    return TRUE;
}


/************************************************************************/
/* add_window                                                           */
/*                                                                      */
/* Parameters                                                           */
/*      class - A pointer to a class node to which this window belongs  */
/*      hWnd  - The window handle of that particular class              */
/*                                                                      */
/* Returns                                                              */
/*      TRUE if successful, otherwise FALSE                             */
/************************************************************************/

PRIVATE BOOL add_window(Classnode *classnode, HWND hWnd)
{
    Windownode *window = (Windownode *)malloc(sizeof(Windownode));
    if (window == (Windownode *)NULL)
    return FALSE;
    window->hWnd = hWnd;
    window->classnode = classnode;
    classnode->nWindows++;
    AddTail(&Window_List, &(window->node));
    return TRUE;
}


/************************************************************************/
/* find_class                                                           */
/*                                                                      */
/* Parameters                                                           */
/*      name    - Name of class to find                                 */
/*                                                                      */
/* Returns                                                              */
/*      A pointer to the class or NULL if it does not exist             */
/************************************************************************/

PRIVATE Classnode *find_class(LPCSTR name)
{
    Classnode *classnode;
    BOOL      bNotFound = TRUE;

    for (classnode = (Classnode *)Class_List.Head;
    (bNotFound = strcmp((char *)(name),
                              (char *)(classnode->lpszClassName))) &&
               (classnode->node.Next != (Node *)NULL);
    classnode = (Classnode *)classnode->node.Next);

    if (bNotFound)
        return (Classnode *)NULL;

    return classnode;
}


/************************************************************************/
/* find_window                                                          */
/*                                                                      */
/* Parameters                                                           */
/*      hWnd    - A handle to the window to find                        */
/*                                                                      */
/* Returns                                                              */
/*      A pointer to the Windownode or NULL if not found                */
/************************************************************************/

PRIVATE Windownode *find_window(HWND hWnd)
{
    Windownode *window;
    BOOL       bNotFound = TRUE;

    for (window = (Windownode *)Window_List.Head;
    (bNotFound = (window->hWnd != hWnd)) &&
               (window->node.Next != (Node *)NULL);
    window = (Windownode *)window->node.Next);

    if (bNotFound)
    return (Windownode *)NULL;

    return window;
}


/************************************************************************/
/* remove_class                                                         */
/*                                                                      */
/* Parameters                                                           */
/*      class   - Pointer to the Classnode to remove                    */
/*                                                                      */
/* Returns                                                              */
/*      TRUE if successful, otherwise FALSE                             */
/************************************************************************/

PRIVATE BOOL remove_class(Classnode *classnode)
{
    if (classnode->nWindows != 0)
        return FALSE;
    free((void *)classnode->lpszClassName);
    free(Remove(&(classnode->node)));
    return TRUE;
}


/************************************************************************/
/* remove_window                                                        */
/*                                                                      */
/* Parameters                                                           */
/*      hWnd   - The handle of the window to remove                     */
/************************************************************************/

PRIVATE void remove_window(HWND hWnd)
{
    Windownode *window = find_window(hWnd);
    if (window == (Windownode *)NULL)
        return;
    window->classnode->nWindows--;
    free(Remove(&(window->node)));
}


/************************************************************************/
/* clear_class                                                          */
/*                                                                      */
/* Purpose                                                              */
/*      Remove all class nodes                                          */
/************************************************************************/

PRIVATE void clear_class(void)
{
    Classnode *classnode, *next;

    classnode = (Classnode *)Class_List.Head;
    while (classnode->node.Next != (Node *)NULL)
    {
        next = (Classnode *)classnode->node.Next;
        free(classnode);
        classnode = next;
    }
}

/************************************************************************/
/* clear_window                                                         */
/*                                                                      */
/* Purpose                                                              */
/*      Remove all window nodes                                         */
/************************************************************************/

PRIVATE void clear_window(void)
{
    Windownode *window, *next;

    window = (Windownode *)Window_List.Head;
    while (window->node.Next != (Node *)NULL)
    {
        next = (Windownode *)window->node.Next;
        free(window);
        window = next;
    }
}

/************************************************************************/
/* clear_instance                                                       */
/*                                                                      */
/* Purpose                                                              */
/*      Remove all dde instance nodes                                   */
/************************************************************************/

PRIVATE void clear_instance(void)
{
    DdeInstancenode *instance, *next;

    instance = (DdeInstancenode *)DdeInstance_List.Head;
    while (instance->node.Next != (Node *)NULL)
    {
        FreePort(instance->port);
        Signal(&instance->sem);
        next = (DdeInstancenode *)instance->node.Next;
        free(instance);
        instance = next;
    }
}

#ifdef never
/************************************************************************/
/* IsDeferredMsg                                                        */
/*                                                                      */
/* Parameters                                                           */
/*      msg - The message number to test                                */
/************************************************************************/

PRIVATE BOOL IsDeferredMsg(UINT msg)
{
    switch (msg)
    {
    default:
            return FALSE;
    }
}
#endif

/************************************************************************/
/* Ack                                                                  */
/*                                                                      */
/* Purpose                                                              */
/*      To acknowledge the receipt of a particular received message     */
/************************************************************************/

PRIVATE BOOL Ack(word ack_no)
{
    if (!send_message(IO_Ack, 1, 0, &ack_no, NULL))
        return FALSE;
    return TRUE;
}


/************************************************************************/
/* NegAck                                                               */
/*                                                                      */
/* Purpose                                                              */
/*      To negative ackowledge a particular message                     */
/************************************************************************/

PRIVATE BOOL NegAck(word ack_no)
{
    if (!send_message(IO_NegAck, 1, 0, &ack_no, NULL))
        return FALSE;
    return TRUE;
}


/************************************************************************/
/* RegisterBuffer                                                       */
/*                                                                      */
/* Purpose                                                              */
/*      To register a buffer larger than the msg buffer size for data   */
/*      transmission.                                                   */
/************************************************************************/

PRIVATE HGLOBAL RegisterBuffer(word size)
{
    word Control = size;
    if (!send_message(IO_RegisterBuffer, 1, 0, &Control, NULL))
        return 0;
    return (HGLOBAL)Control;
}


/************************************************************************/
/* AppendBuffer                                                         */
/*                                                                      */
/* Purpose                                                              */
/*      To append data to the buffer on the I/O server at the specified */
/*      point in the buffer.                                            */
/************************************************************************/

PRIVATE BOOL AppendBuffer(HGLOBAL handle, word offset, BYTE *buffer, word size)
{
    word Control[3];
    Control[0] = handle;
    Control[1] = offset;
    Control[2] = size;
    if (!send_message(IO_AppendBuffer, 3, (int)size, &Control[0], buffer))
        return FALSE;
    return (BOOL)Control[0];
}


/************************************************************************/
/* GetBuffer                                                            */
/*                                                                      */
/* Purpose                                                              */
/*      To get the specified section of data from the globally alloc'd  */
/*      buffer into the Helios buffer.                                  */
/************************************************************************/

PRIVATE word GetBuffer(HGLOBAL handle, word offset, BYTE *buffer, word size)
{
    word Control[3];
    Control[0] = handle;
    Control[1] = offset;
    Control[2] = size;
    if (!send_fn(IO_GetBuffer, 3, (int)size, &Control[0], buffer))
        return 0;
    return Control[0];
}


/************************************************************************/
/* DeleteBuffer                                                         */
/*                                                                      */
/* Purpose                                                              */
/*      Deallocate the buffer on the I/O server.                        */
/************************************************************************/

BOOL DeleteBuffer(HGLOBAL handle)
{
    word Control = handle;
    if (!send_message(IO_DeleteBuffer, 1, 0, &Control, NULL))
        return FALSE;
    return (BOOL)Control;
}


/************************************************************************/
/* SendBuffer                                                           */
/*                                                                      */
/* Purpose                                                              */
/*      Sends the relevant buffer in bits                               */
/************************************************************************/

HGLOBAL SendBuffer(BYTE *pData, word Size)
{
    HGLOBAL hBuffer;
    int i, times, left, to_send;
    int offset;

    if (Size == 0)
        return NULL;     /* don't waste time */

    hBuffer = RegisterBuffer(Size);
    if (hBuffer != NULL)
    {
        times = (int)(Size / buf_size);
        if ((Size % buf_size) != 0)
                times++;
        left = (int)Size;
        offset = 0;

        for (i=0; i<times; i++)
        {
            to_send = (left <= (int)buf_size) ? left : (int)buf_size;
            left -= to_send;
            if (!AppendBuffer(hBuffer, offset, &pData[offset], to_send))
            {
                /* some error - abort */
                DeleteBuffer(hBuffer);
                return NULL;
            }
            offset += to_send;
        }
    }

    return hBuffer;
}


/************************************************************************/
/* ReceiveBuffer                                                        */
/*                                                                      */
/* Purpose                                                              */
/*      Receives the buffer indicated in the handle into the local      */
/*      buffer                                                          */
/************************************************************************/

word ReceiveBuffer(HGLOBAL hBuffer, BYTE *pData, word Size)
{
    int i, times, left, to_get;
    int offset;

    if (Size == 0)
        return 0;     /* don't waste time */

    offset = 0;
    
    if (hBuffer != NULL)
    {
        times = (int)(Size / buf_size);
        if ((Size % buf_size) != 0)
                times++;
        left = (int)Size;

        for (i=0; i<times; i++)
        {
            to_get = (left <= (int)buf_size) ? left : (int)buf_size;
            left -= to_get;
            if (!GetBuffer(hBuffer, offset, &pData[offset], to_get))
            {
                /* some error - abort */
                return NULL;
            }
            offset += to_get;
        }
    }

    return offset;
}


word send_1_parameter(word fn, word one)
{
    word Control = one;
    if (!send_message((int)fn, 1, 0, &Control, NULL))
        return 0;
    return Control;
}


word send_2_parameters(word fn, word one, word two)
{
    word Control[2];
    Control[0] = one;
    Control[1] = two;
    if (!send_message((int)fn, 2, 0, &Control[0], NULL))
        return 0;
    return Control[0];
}


word send_3_parameters(word fn, word one, word two, word three)
{
    word Control[3];
    Control[0] = one;
    Control[1] = two;
    Control[2] = three;
    if (!send_message((int)fn, 3, 0, &Control[0], NULL))
        return 0;
    return Control[0];
}


word send_4_parameters(word fn, word one, word two, word three, word four)
{
    word Control[4];
    Control[0] = one;
    Control[1] = two;
    Control[2] = three;
    Control[3] = four;
    if (!send_message((int)fn, 4, 0, &Control[0], NULL))
        return 0;
    return Control[0];
}


word send_5_parameters(word fn, word one, word two, word three, word four, word five)
{
    word Control[5];
    Control[0] = one;
    Control[1] = two;
    Control[2] = three;
    Control[3] = four;
    Control[4] = five;
    if (!send_message((int)fn, 5, 0, &Control[0], NULL))
        return 0;
    return Control[0];
}


word send_6_parameters(word fn, word one, word two, word three, word four, word five, word six)
{
    word Control[6];
    Control[0] = one;
    Control[1] = two;
    Control[2] = three;
    Control[3] = four;
    Control[4] = five;
    Control[5] = six;
    if (!send_message((int)fn, 6, 0, &Control[0], NULL))
        return 0;
    return Control[0];
}


/************************************************************************/
/************************************************************************/
/* This is the start of the library functions which are the equivalent  */
/* to the Windows functions of the same name.  These therefore provide  */
/* the functionality which will all compilation under Helios.           */
/************************************************************************/
/************************************************************************/

/***** First, non-standard functions ************************************/

void WINAPI GetArgcArgv(int *pArgc, char ***pArgv)
{
    *pArgc = app_argc;
    *pArgv = app_argv;
}

/***** standard messages ************************************************/


BOOL WINAPI GetMessage(LPMSG msg, HWND hWnd, UINT msgMin, UINT msgMax)
{
    int  i;
    BOOL bRet = TRUE;
    BOOL bFound = FALSE;

    Wait(&MsgSem);
    Wait(&AccessSem);

    if (msgMax == 0)
    msgMax = 0xffff;

    for (i=0; i<nBuffer; i++)
        if (MsgList[i].message == WM_QUIT)
        {
            bFound = TRUE;
            bRet = FALSE;
            break;
        }

    if (!bFound)
        for (i=0; i<nBuffer; i++)
        {
            if ((MsgList[i].message >= msgMin) &&
            (MsgList[i].message <= msgMax))
            {
                if ((hWnd == (HWND)NULL) || (hWnd == MsgList[i].hwnd))
                {
                    /* this is our message */
                    bFound = TRUE;
                    memcpy(msg, &MsgList[i], sizeof(MSG));
                    memmove(&MsgList[i], &MsgList[i+1], ((--nBuffer)-i)*sizeof(MSG));
                    Signal(&BufferSem);

                    if (msg->message == WM_PAINT)
                    {
                        HWND hWndTest = msg->hwnd;
                        int j = i;
                        /* swallow up any other paint messages */
                        while (j<nBuffer)
                            if ((MsgList[j].message == WM_PAINT) &&
                            (MsgList[j].hwnd == hWndTest))
                            {
                                memmove(&MsgList[j], &MsgList[j+1], ((--nBuffer)-j)*sizeof(MSG));
                                Signal(&BufferSem);
                            }
                            else
                                j++;
                    }
                    break;
                }
            }
        }

    if (!bFound)
        msg->message = WM_NULL;                    /* nothing to find */

    Signal(&AccessSem);
    return bRet;
}


BOOL WINAPI TranslateMessage(const MSG *msg)
{
    /* we do nothing here, because it would create too much traffic     */
    /* to send each message to Helios and then back to the I/O server   */
    /* The translation is therefore done by the I/O server before the   */
    /* message is sent.  This function is therefore useless, but is     */
    /* included for completeness.                                       */
    return FALSE;
}


LONG WINAPI DispatchMessage(const MSG *msg)
{
    LONG nRet;

    Windownode *window = find_window(msg->hwnd);
    if (window == (Windownode *)NULL)
        return NULL;
    nRet = window->classnode->Callback(msg->hwnd, msg->message, msg->wParam, msg->lParam);

    /* if user did not handle the necessary messages */
    if (nRet != 0)
        DefWindowProc(msg->hwnd, msg->message, msg->wParam, msg->lParam);

#ifdef never
    /* if a deferred message, return result */
    if (IsDeferredMsg(msg->message))
    {
        PostMessage(msg->hwnd, WM_RELEASE, msg->message, nRet);
    }
#endif

    return nRet;
}


ATOM WINAPI RegisterClass(const WNDCLASS FAR *lpWc)
{
    word Control[6];
    BYTE *Data;

    if (find_class(lpWc->lpszClassName) != NULL)
        return (ATOM)NULL;
    if (!add_class(lpWc->lpszClassName, lpWc->lpfnWndProc))
        return (ATOM)NULL;

    /* now put the stuff in the buffers and send it */
    Control[0] = (word)lpWc->style;
    Control[1] = (word)lpWc->cbClsExtra;
    Control[2] = (word)lpWc->cbWndExtra;
    Control[3] = (word)lpWc->hInstance;
    Control[4] = (word)lpWc->hCursor;
    Control[5] = (word)lpWc->hbrBackground;
    Data = (BYTE *)&(lpWc->lpszClassName[0]);
    if (!send_message(IO_RegisterClass, 6, strlen(lpWc->lpszClassName)+1, &Control[0], Data) ||
        (Control[0] == (word)NULL))
    {
        /* registration failed */
        Classnode *classnode = find_class(lpWc->lpszClassName);
        remove_class(classnode);
        return (ATOM)NULL;
    }
    return (ATOM)Control[0];
}


BOOL WINAPI UnregisterClass(LPCSTR lpszClassName, HINSTANCE hInst)
{
    word Control = (word)hInst;
    BYTE *Data = (BYTE *)&(lpszClassName[0]);
    Classnode *classnode = find_class(lpszClassName);
    if (classnode == (Classnode *)NULL)
        return FALSE;
    if (send_message(IO_UnregisterClass, 1, strlen(lpszClassName)+1, &Control, Data) &&
        (Control != (word)NULL))
    {
        /* if succeeded */
        remove_class(classnode);
        return TRUE;
    }
    return FALSE;
}


HWND WINAPI CreateWindow(LPCSTR    lpszClassName,
                         LPCSTR    lpszWindowName,
                         DWORD     style,
                         int       x,
                         int       y,
                         int       nWidth,
                         int       nHeight,
                         HWND      hWndParent,
                         HMENU     hMenu,
                         HINSTANCE hInst,
                         void FAR  *lpvParam)
{
    word Control[8];
    int  size = strlen(lpszClassName) + strlen(lpszWindowName) + 2;
    BYTE *Data = (BYTE *)malloc(size);
    Classnode *classnode = find_class(lpszClassName);

    Control[0] = (word)style;
    Control[1] = (word)x;
    Control[2] = (word)y;
    Control[3] = (word)nWidth;
    Control[4] = (word)nHeight;
    Control[5] = (word)hWndParent;
    Control[6] = (word)hMenu;
    Control[7] = (word)hInst;
    strcpy((char *)Data, (char *)lpszClassName);
    strcpy((char *)&Data[strlen(lpszClassName)+1], lpszWindowName);

    if (!send_message(IO_CreateWindow, 8, size, &Control[0], &Data[0]))
    {
        free(Data);
        return (HWND)NULL;
    }
    if ((classnode != (Classnode *)NULL) && ((HWND)Control[0] != (HWND)NULL))
        if (!add_window(classnode, (HWND)Control[0]))
        {
                /* local registration failed - remove window */
                DestroyWindow((HWND)Control[0]);
                free(Data);
                return (HWND)NULL;
        }

    free(Data);
    return (HWND)Control[0];
    lpvParam=lpvParam;
}


BOOL WINAPI DestroyWindow(HWND hWnd)
{
    word Control;
    Control = (word)hWnd;
    if (!send_message(IO_DestroyWindow, 1, 0, &Control, NULL))
        return FALSE;
    remove_window(hWnd);
    return (BOOL)Control;
}


LRESULT WINAPI DefWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    word Control[4];

    if ((msg == WM_QUIT) || (msg == WM_NULL))
        return (LRESULT)NULL;

    Control[0] = (word)hWnd;
    Control[1] = (word)msg;
    Control[2] = (word)wParam;
    Control[3] = (word)lParam;
    if (!send_message(IO_DefWindowProc, 4, 0, &Control[0], NULL))
        return (LRESULT)NULL;

    return (LRESULT)Control[0];
}


void WINAPI PostQuitMessage(int nExitCode)
{
    /* This differs in that we should not have any windows open, and    */
    /* therefore cannot send any callback stuff back to a window.  We   */
    /* must therefore send the data directly to the incoming port.      */
    MCB mcb;
    word Control[5];

    do
    {
        Control[0] = 0;
        Control[1] = 0;
        Control[2] = WM_QUIT;
        Control[3] = nExitCode;
        Control[4] = 0;

        InitMCB(&mcb, MsgHdr_Flags_preserve, CallbackPort, NullPort, Err_Null);
        mcb.Timeout = OneSec*60;
        mcb.MsgHdr.ContSize = 5;
        mcb.Control = &Control[0];
        mcb.MsgHdr.DataSize = 0;
        mcb.Data = NULL;
    }
    while (PutMsg(&mcb) != Err_Null);
    return;
}


BOOL WINAPI PostMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    word Control[4];

    /* first check if it is a quit message */
    /* if programmer is using WM_QUIT to terminate another app - wrong! */
    /* the application will not terminate cleanly (WM_CLOSE is better)  */
    /* so I exclude it here.                                            */
    if (msg == WM_QUIT)
        PostQuitMessage(wParam);

    /* must send via I/O server because we no not know the sequence     */
    /* number.                                                          */
    Control[0] = (word)hWnd;
    Control[1] = (word)msg;
    Control[2] = (word)wParam;
    Control[3] = (word)lParam;
    if (!send_message(IO_PostMessage, 4, 0, &Control[0], NULL))
        return FALSE;
    return (BOOL)Control[0];
}

void WINAPI UpdateWindow(HWND hWnd)
{
    /* this must find the callback function and specifically call it */
    Windownode *window = find_window(hWnd);
    if (window != (Windownode *)NULL)
    {
        window->classnode->Callback(hWnd, WM_PAINT, 0, 0L);
        return;
    }
    send_1_parameter(IO_UpdateWindow, hWnd);
}


LRESULT WINAPI WINAPISendMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    Windownode *window = find_window(hWnd);
    if (window != (Windownode *)NULL)
        return window->classnode->Callback(hWnd, msg, wParam, lParam);

    return send_4_parameters(IO_SendMessage, hWnd, msg, wParam, lParam);
}

DdeInstancenode *DdeAddInstance(DWORD idInst, Port port)
{
    DdeInstancenode *instance = (DdeInstancenode *)malloc(sizeof(DdeInstancenode));
    if (instance == (DdeInstancenode *)NULL)
        return NULL;
    instance->idInst = idInst;
    instance->port = port;
    InitSemaphore(&instance->sem, 0);
    AddTail(&DdeInstance_List, &instance->node);
    return instance;
}

DdeInstancenode *DdeFindInstance(DWORD idInst)
{
    DdeInstancenode *instance;
    BOOL bNotFound = TRUE;

    for (instance=(DdeInstancenode *)DdeInstance_List.Head;
                (instance->node.Next != (Node *)NULL) &&
                (bNotFound=(instance->idInst != idInst));
                instance = (DdeInstancenode *)instance->node.Next);
    if (bNotFound)
        return (DdeInstancenode *)NULL;
    else
        return instance;
}

BOOL DdeRemoveInstance(DdeInstancenode *instance)
{
    if (instance != (DdeInstancenode *)NULL)
    {
        free(Remove(&(instance->node)));
        return TRUE;
    }
    return FALSE;
}

BOOL DdeAddHData(HDDEDATA hData, BYTE *lpData)
{
    DdeHData *lphData = (DdeHData *)malloc(sizeof(DdeHData));

    if (lphData == (DdeHData *)NULL)
        return FALSE;
    lphData->hData = hData;
    lphData->lpData = lpData;
    AddTail(&hdata_list, &lphData->node);
    return TRUE;
}

BOOL DdeRemoveHData(HDDEDATA hData)
{
    DdeHData *lphData;

    for (lphData = (DdeHData *)hdata_list.Head;
                lphData->node.Next != (Node *)NULL;
                lphData = (DdeHData *)lphData->node.Next)
        if (lphData->hData == hData)
        {
            free(lphData->lpData);
            free(Remove(&lphData->node));
            return TRUE;
        }
    return FALSE;
}

