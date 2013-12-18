/************************************************************************/
/************************************************************************/
/*               Microsoft Windows Libraries for Helios                 */
/*                                                                      */
/*       Copyright (C) 1990-1993,  Perihelion Software Limited          */
/*                         All Rights Reserved                          */
/*                                                                      */
/*   windefs.h    to build windows.lib                                  */
/*                header file for win.c and ddeml.c                     */
/************************************************************************/
/************************************************************************/

#ifndef __WINDEFS_H
#define __WINDEFS_H

#ifndef __DDEML_H
#include "ddeml.h"
#endif

/***** local structure typedefs *****************************************/

typedef struct DeviceInfo {
                                ObjInfo obj;
                          } DeviceInfo;

typedef struct Classnode {
                                Node    node;
                                LPCSTR  lpszClassName;
                                WNDPROC Callback;
                                int     nWindows;
                         } Classnode;

typedef struct Windownode {
                                Node      node;
                                HWND      hWnd;
                                Classnode *classnode;
                          } Windownode;

typedef struct DdeInstancenode {
                                Node      node;
                                DWORD     idInst;
                                Port      port;
                                Semaphore sem;
                          } DdeInstancenode;

typedef struct DdeHData {
                                Node     node;
                                HDDEDATA hData;
                                BYTE     *lpData;
                        } DdeHData;


/***** local defines ****************************************************/
#define max_words               255
#define MSGBUFSIZE              10
#define Graph_Message           0x67770000L
#define WM_HOLD                 (WM_USER+1)
#define WM_RELEASE              (WM_USER+2)

#define IO_RegisterPort                 0      /* internal */
#define IO_RegisterClass                1
#define IO_CreateWindow                 2
#define IO_DefWindowProc                3
#define IO_PostQuitMessage              4      /* defunct */
#define IO_PostMessage                  5
#define IO_DestroyWindow                6
#define IO_ShowWindow                   7
#define IO_UnregisterClass              8
#define IO_GetWindowRect                9
#define IO_GetClientRect                10
#define IO_CreatePen                    11
#define IO_CreateSolidBrush             12
#define IO_CreateFont                   13
#define IO_SelectObject                 14
#define IO_GetDC                        15
#define IO_ReleaseDC                    16
#define IO_BeginPaint                   17
#define IO_EndPaint                     18
#define IO_GetStockObject               19
#define IO_DPtoLP                       20
#define IO_LPtoDP                       21
#define IO_ClientToScreen               22
#define IO_ScreenToClient               23
#define IO_MoveTo                       24
#define IO_LineTo                       25
#define IO_FillRect                     26
#define IO_TextOut                      27
#define IO_GetTextMetrics               28
#define IO_GetTextAlign                 29
#define IO_SetTextAlign                 30
#define IO_GetTextExtent                31
#define IO_Ack                          32     /* internal */
#define IO_NegAck                       33     /* internal */
#define IO_DeleteObject                 34
#define IO_MoveWindow                   35
#define IO_AdjustWindowRect             36
#define IO_EnableWindow                 37
#define IO_GetActiveWindow              38
#define IO_GetClassLong                 39
#define IO_GetClassWord                 40
#define IO_GetDesktopWindow             41
#define IO_GetFocus                     42
#define IO_GetNextWindow                43
#define IO_GetParent                    44
#define IO_GetTopWindow                 45
#define IO_GetWindow                    46
#define IO_GetWindowLong                47
#define IO_GetWindowWord                48
#define IO_IsChild                      49
#define IO_IsWindow                     50
#define IO_IsWindowEnabled              51
#define IO_IsWindowVisible              52
#define IO_SetClassLong                 53
#define IO_SetClassWord                 54
#define IO_SetWindowLong                55
#define IO_SetWindowWord                56
#define IO_SetCapture                   57
#define IO_ReleaseCapture               58
#define IO_SetCursor                    59
#define IO_LoadCursor                   60
#define IO_GetBkColor                   61
#define IO_GetBkMode                    62
#define IO_GetDeviceCaps                63
#define IO_GetMapMode                   64
#define IO_GetSystemMetrics             65
#define IO_GetTextColor                 66
#define IO_SetBkColor                   67
#define IO_SetBkMode                    68
#define IO_SetMapMode                   69
#define IO_SetTextColor                 70
#define IO_Arc                          71
#define IO_Chord                        72
#define IO_CreateHatchBrush             73
#define IO_CreatePatternBrush           74
#define IO_Ellipse                      75
#define IO_FloodFill                    76
#define IO_GetCurrentPosition           77
#define IO_GetNearestColor              78
#define IO_GetPixel                     79
#define IO_GetPolyFillMode              80
#define IO_GetROP2                      81
#define IO_InvalidateRect               82
#define IO_InvertRect                   83
#define IO_Pie                          84
#define IO_Polygon                      85
#define IO_Polyline                     86
#define IO_Rectangle                    87
#define IO_SetPixel                     88
#define IO_SetPolyFillMode              89
#define IO_SetROP2                      90
#define IO_UnrealizeObject              91
#define IO_UpdateWindow                 92
#define IO_ValidateRect                 93
#define IO_CreatePalette                94
#define IO_GetPaletteEntries            95
#define IO_GetNearestPaletteIndex       96
#define IO_RealizePalette               97
#define IO_SelectPalette                98
#define IO_SetPaletteEntries            99
#define IO_BitBlt                       100
#define IO_CreateBitmap                 101
#define IO_CreateCompatibleBitmap       102
#define IO_CreateDIBitmap               103
#define IO_GetBitmapBits                104
#define IO_GetDIBits                    105
#define IO_SetBitmapBits                106
#define IO_SetDIBits                    107
#define IO_SetDIBitsToDevice            108
#define IO_CreateCompatibleDC           109
#define IO_RegisterBuffer               110    /* internal */
#define IO_AppendBuffer                 111    /* internal */
#define IO_DeleteBuffer                 112    /* internal */
#define IO_AppendMenu                   113
#define IO_CheckMenuItem                114
#define IO_CreatePopupMenu              115
#define IO_CreateMenu                   116
#define IO_DeleteMenu                   117
#define IO_DestroyMenu                  118
#define IO_DrawMenuBar                  119
#define IO_EnableMenuItem               120
#define IO_GetMenu                      121
#define IO_GetMenuItemCount             122
#define IO_GetMenuItemID                123
#define IO_GetMenuState                 124
#define IO_GetMenuString                125
#define IO_GetSubMenu                   126
#define IO_GetSystemMenu                127
#define IO_InsertMenu                   128
#define IO_RemoveMenu                   129
#define IO_SetMenu                      130
#define IO_FindWindow                   131
#define IO_RegisterIOMenu               132    /* internal */
#define IO_DeleteDC                     133
#define IO_GetUpdateRect                134
#define IO_SendMessage                  135
#define IO_GetObject                    136
#define IO_DdeInitialize                137
#define IO_DdeUninitialize              138
#define IO_DdeConnectList               139
#define IO_DdeQueryNextServer           140
#define IO_DdeDisconnectList            141
#define IO_DdeConnect                   142
#define IO_DdeDisconnect                143
#define IO_DdeReconnect                 144
#define IO_DdeQueryConvInfo             145
#define IO_DdeSetUserHandle             146
#define IO_DdeAbandonTransaction        147
#define IO_DdePostAdvise                148
#define IO_DdeEnableCallback            149
#define IO_DdeNameService               150
#define IO_DdeClientTransaction         151
#define IO_DdeCreateDataHandle          152
#define IO_DdeAddData                   153
#define IO_DdeGetData                   154
#define IO_DdeAccessData                155
#define IO_DdeUnaccessData              156
#define IO_DdeFreeDataHandle            157
#define IO_DdeGetLastError              158
#define IO_DdeCreateStringHandle        159
#define IO_DdeQueryString               160
#define IO_DdeFreeStringHandle          161
#define IO_DdeKeepStringHandle          162
#define IO_DdeCmpStringHandles          163
#define IO_DdeReturnResult              164    /* internal */
#define IO_DdeReturnAdvise              165    /* internal */
#define IO_GetBuffer                    166    /* internal */
#define IO_SetScrollRange		167
#define IO_GetScrollRange		168
#define IO_SetScrollPos			169
#define IO_GetScrollPos			170
#define IO_ShowScrollBar		171
#define IO_EnableScrollBar		172

/***** internal function prototypes *************************************/
BOOL send_message(int, int, int, word *, BYTE *);
BOOL send_fn(int, int, int, word *, BYTE *);
word send_1_parameter(word, word);
word send_2_parameters(word, word, word);
word send_3_parameters(word, word, word, word);
word send_4_parameters(word, word, word, word, word);
word send_5_parameters(word, word, word, word, word, word);
word send_6_parameters(word, word, word, word, word, word, word);
HGLOBAL SendBuffer(BYTE *, word);
word    ReceiveBuffer(HGLOBAL, BYTE *, word);
BOOL DeleteBuffer(HGLOBAL);

/***** DDE functions ****************************************************/
DdeInstancenode *DdeAddInstance(DWORD, Port);
DdeInstancenode *DdeFindInstance(DWORD);
BOOL DdeRemoveInstance(DdeInstancenode *);

BOOL DdeAddHData(HDDEDATA, BYTE *);
BOOL DdeRemoveHData(HDDEDATA);

#endif    /* __WINDEFS_H */
