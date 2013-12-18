/************************************************************************/
/************************************************************************/
/*                       Helios Windows I/O Server                      */
/*              Copyright (C) Perihelion Software Limited               */
/*                                                                      */
/* graphdef.h - defines and types local to graph.c and hel_dde.c        */
/************************************************************************/
/************************************************************************/

#ifndef __graphdef_h
#define __graphdef_h

#define MAX_MESSAGES    160
#define GraphicsDebug(a)   Debug(Graphics_Flag, a)
#define DdeDebug(a)        Debug(DDE_Flag, a)

typedef struct Menunode {
                                Node node;
                                UINT ID;
                        } Menunode;

typedef struct Classnode {
                                Node      node;   /* linked list stuff    */
                                int       usage;
                                UINT      style;
                                int       cbClsExtra;
                                int       cbWndExtra;
                                HINSTANCE hInstance;
                                HCURSOR   hCursor;
                                HBRUSH    hbrBackground;
                                char      szClassName[30];
                                ATOM      atomClass;
                         } Classnode;

typedef struct Classlink {
                                Node      node;
                                Classnode *class;
                         } Classlink;

typedef struct MSGnode {
                                MSG       msg;
                                word      sequence_no;
                       } MSGnode;

/* Graphnode is the structure that maintains all the required information */
/* on a particular graphic window. The Graphnode structures associated    */
/* with each open graphics window are kept in a linked list, whose header */
/* is Graph_List.                                                         */

typedef struct Graphnode { DirEntryNode node;      /* Linked list stuff  */
                           Port         msg_port;  /* window msg port    */
                           List         class_list;
                           List         window_list;
                           List         menu_list;
                           List         dde_list;
                           int          num_classes;
                           int          num_windows;
                           int          app_id;
                           int          stream_count;
                           Attributes   attr;
                           MSGnode      msg_buffer[MAX_MESSAGES];
                           int          msg_head;
                           int          msg_tail;
                           word         send_sequence;
                         } Graphnode;


typedef struct Windownode {
                                Node node;      /* linked list stuff */
                                int  num;       /* number of deferred msgs */
                                HWND hwnd;      /* handle for lookup */
                                RECT rectInvalid;
                                Classnode *class;
                                Graphnode *app;
                          } Windownode;


typedef struct MCBnode {
                                Node node;
                                MCB mcb;
                       } MCBnode;

#ifndef BIG_HANDLE
DECLARE_HANDLE32(BIG_HANDLE);
#endif

typedef struct DdeInstancenode {
                                Node       node;
                                DWORD      idInst;
                                Port       port;
                                Graphnode *app;
                                BOOL       bAdviseFlag;
                                UINT       nAdviseCount;
                                UINT       nAdviseTotal;
                                BIG_HANDLE hszTopic;
                                BIG_HANDLE hszItem;
                               } DdeInstancenode;

typedef struct DdeConvnode {
                                DdeInstancenode *dde;
                                BOOL             bIsBlocked;
                                BIG_HANDLE       hData;
                                BIG_HANDLE       hAdvData;
                           } DdeConvnode;

typedef struct DdeServicenode {
                                BIG_HANDLE       server;
                                DdeInstancenode *dde;
                              } DdeServicenode;

#define num_routines  173 /* Number of routines providing graphics interface */

/***** Some shared functions ********************************************/

Graphnode *get_first_app(void);

BOOL add_instance(Graphnode FAR *, DWORD, Port);
DdeInstancenode FAR *find_instance(Graphnode FAR *, DWORD);
BOOL delete_instance(Graphnode FAR *, DWORD);

BOOL add_server(BIG_HANDLE, DdeInstancenode FAR *);
BOOL delete_server(BIG_HANDLE, DWORD);
BOOL remove_servers(DWORD);
DdeInstancenode FAR *find_server(BIG_HANDLE);

BOOL add_conv(BIG_HANDLE, DdeInstancenode FAR *);
BOOL delete_conv(BIG_HANDLE);
BOOL remove_convs(DWORD);
DdeConvnode FAR *find_conv(BIG_HANDLE);
DdeInstancenode FAR *find_conv_dde(BIG_HANDLE);

BOOL add_pending(BIG_HANDLE, BIG_HANDLE, DdeInstancenode FAR *);
DdeInstancenode FAR *find_pending(BIG_HANDLE, BIG_HANDLE);

#endif  /* __graphdef_h */

