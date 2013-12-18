/************************************************************************/
/************************************************************************/
/*                       Helios Windows I/O Server                      */
/*           Copyright (C) 1993 Perihelion Software Limited             */
/*                                                                      */
/* graphfn.h - IO_XXXX prototypes                                       */
/************************************************************************/
/************************************************************************/

#ifndef __graphfn_h
#define __graphfn_h

/****** Functions which handle the private protocol function calls ********/
PRIVATE void IO_RegisterPort(Graphnode *);              /* 000 */
PRIVATE void IO_RegisterClass(Graphnode *);             /* 001 */
PRIVATE void IO_CreateWindow(Graphnode *);              /* 002 */
PRIVATE void IO_DefWindowProc(Graphnode *);             /* 003 */
PRIVATE void IO_PostQuitMessage(Graphnode *);           /* 004 */
PRIVATE void IO_PostMessage(Graphnode *);               /* 005 */
PRIVATE void IO_DestroyWindow(Graphnode *);             /* 006 */
PRIVATE void IO_ShowWindow(Graphnode *);                /* 007 */
PRIVATE void IO_UnregisterClass(Graphnode *);           /* 008 */
PRIVATE void IO_GetWindowRect(Graphnode *);             /* 009 */
PRIVATE void IO_GetClientRect(Graphnode *);             /* 010 */
PRIVATE void IO_CreatePen(Graphnode *);                 /* 011 */
PRIVATE void IO_CreateSolidBrush(Graphnode *);          /* 012 */
PRIVATE void IO_CreateFont(Graphnode *);                /* 013 */
PRIVATE void IO_SelectObject(Graphnode *);              /* 014 */
PRIVATE void IO_GetDC(Graphnode *);                     /* 015 */
PRIVATE void IO_ReleaseDC(Graphnode *);                 /* 016 */
PRIVATE void IO_BeginPaint(Graphnode *);                /* 017 */
PRIVATE void IO_EndPaint(Graphnode *);                  /* 018 */
PRIVATE void IO_GetStockObject(Graphnode *);            /* 019 */
PRIVATE void IO_DPtoLP(Graphnode *);                    /* 020 */
PRIVATE void IO_LPtoDP(Graphnode *);                    /* 021 */
PRIVATE void IO_ClientToScreen(Graphnode *);            /* 022 */
PRIVATE void IO_ScreenToClient(Graphnode *);            /* 023 */
PRIVATE void IO_MoveTo(Graphnode *);                    /* 024 */
PRIVATE void IO_LineTo(Graphnode *);                    /* 025 */
PRIVATE void IO_FillRect(Graphnode *);                  /* 026 */
PRIVATE void IO_TextOut(Graphnode *);                   /* 027 */
PRIVATE void IO_GetTextMetrics(Graphnode *);            /* 028 */
PRIVATE void IO_GetTextAlign(Graphnode *);              /* 029 */
PRIVATE void IO_SetTextAlign(Graphnode *);              /* 030 */
PRIVATE void IO_GetTextExtent(Graphnode *);             /* 031 */
PRIVATE void IO_Ack(Graphnode *);                       /* 032 */
PRIVATE void IO_NegAck(Graphnode *);                    /* 033 */
PRIVATE void IO_DeleteObject(Graphnode *);              /* 034 */
PRIVATE void IO_MoveWindow(Graphnode *);                /* 035 */
PRIVATE void IO_AdjustWindowRect(Graphnode *);          /* 036 */
PRIVATE void IO_EnableWindow(Graphnode *);              /* 037 */
PRIVATE void IO_GetActiveWindow(Graphnode *);           /* 038 */
PRIVATE void IO_GetClassLong(Graphnode *);              /* 039 */
PRIVATE void IO_GetClassWord(Graphnode *);              /* 040 */
PRIVATE void IO_GetDesktopWindow(Graphnode *);          /* 041 */
PRIVATE void IO_GetFocus(Graphnode *);                  /* 042 */
PRIVATE void IO_GetNextWindow(Graphnode *);             /* 043 */
PRIVATE void IO_GetParent(Graphnode *);                 /* 044 */
PRIVATE void IO_GetTopWindow(Graphnode *);              /* 045 */
PRIVATE void IO_GetWindow(Graphnode *);                 /* 046 */
PRIVATE void IO_GetWindowLong(Graphnode *);             /* 047 */
PRIVATE void IO_GetWindowWord(Graphnode *);             /* 048 */
PRIVATE void IO_IsChild(Graphnode *);                   /* 049 */
PRIVATE void IO_IsWindow(Graphnode *);                  /* 050 */
PRIVATE void IO_IsWindowEnabled(Graphnode *);           /* 051 */
PRIVATE void IO_IsWindowVisible(Graphnode *);           /* 052 */
PRIVATE void IO_SetClassLong(Graphnode *);              /* 053 */
PRIVATE void IO_SetClassWord(Graphnode *);              /* 054 */
PRIVATE void IO_SetWindowLong(Graphnode *);             /* 055 */
PRIVATE void IO_SetWindowWord(Graphnode *);             /* 056 */
PRIVATE void IO_SetCapture(Graphnode *);                /* 057 */
PRIVATE void IO_ReleaseCapture(Graphnode *);            /* 058 */
PRIVATE void IO_SetCursor(Graphnode *);                 /* 059 */
PRIVATE void IO_LoadCursor(Graphnode *);                /* 060 */
PRIVATE void IO_GetBkColor(Graphnode *);                /* 061 */
PRIVATE void IO_GetBkMode(Graphnode *);                 /* 062 */
PRIVATE void IO_GetDeviceCaps(Graphnode *);             /* 063 */
PRIVATE void IO_GetMapMode(Graphnode *);                /* 064 */
PRIVATE void IO_GetSystemMetrics(Graphnode *);          /* 065 */
PRIVATE void IO_GetTextColor(Graphnode *);              /* 066 */
PRIVATE void IO_SetBkColor(Graphnode *);                /* 067 */
PRIVATE void IO_SetBkMode(Graphnode *);                 /* 068 */
PRIVATE void IO_SetMapMode(Graphnode *);                /* 069 */
PRIVATE void IO_SetTextColor(Graphnode *);              /* 070 */
PRIVATE void IO_Arc(Graphnode *);                       /* 071 */
PRIVATE void IO_Chord(Graphnode *);                     /* 072 */
PRIVATE void IO_CreateHatchBrush(Graphnode *);          /* 073 */
PRIVATE void IO_CreatePatternBrush(Graphnode *);        /* 074 */
PRIVATE void IO_Ellipse(Graphnode *);                   /* 075 */
PRIVATE void IO_FloodFill(Graphnode *);                 /* 076 */
PRIVATE void IO_GetCurrentPosition(Graphnode *);        /* 077 */
PRIVATE void IO_GetNearestColor(Graphnode *);           /* 078 */
PRIVATE void IO_GetPixel(Graphnode *);                  /* 079 */
PRIVATE void IO_GetPolyFillMode(Graphnode *);           /* 080 */
PRIVATE void IO_GetROP2(Graphnode *);                   /* 081 */
PRIVATE void IO_InvalidateRect(Graphnode *);            /* 082 */
PRIVATE void IO_InvertRect(Graphnode *);                /* 083 */
PRIVATE void IO_Pie(Graphnode *);                       /* 084 */
PRIVATE void IO_Polygon(Graphnode *);                   /* 085 */
PRIVATE void IO_Polyline(Graphnode *);                  /* 086 */
PRIVATE void IO_Rectangle(Graphnode *);                 /* 087 */
PRIVATE void IO_SetPixel(Graphnode *);                  /* 088 */
PRIVATE void IO_SetPolyFillMode(Graphnode *);           /* 089 */
PRIVATE void IO_SetROP2(Graphnode *);                   /* 090 */
PRIVATE void IO_UnrealizeObject(Graphnode *);           /* 091 */
PRIVATE void IO_UpdateWindow(Graphnode *);              /* 092 */
PRIVATE void IO_ValidateRect(Graphnode *);              /* 093 */
PRIVATE void IO_CreatePalette(Graphnode *);             /* 094 */
PRIVATE void IO_GetPaletteEntries(Graphnode *);         /* 095 */
PRIVATE void IO_GetNearestPaletteIndex(Graphnode *);    /* 096 */
PRIVATE void IO_RealizePalette(Graphnode *);            /* 097 */
PRIVATE void IO_SelectPalette(Graphnode *);             /* 098 */
PRIVATE void IO_SetPaletteEntries(Graphnode *);         /* 099 */
PRIVATE void IO_BitBlt(Graphnode *);                    /* 100 */
PRIVATE void IO_CreateBitmap(Graphnode *);              /* 101 */
PRIVATE void IO_CreateCompatibleBitmap(Graphnode *);    /* 102 */
PRIVATE void IO_CreateDIBitmap(Graphnode *);            /* 103 */
PRIVATE void IO_GetBitmapBits(Graphnode *);             /* 104 */
PRIVATE void IO_GetDIBits(Graphnode *);                 /* 105 */
PRIVATE void IO_SetBitmapBits(Graphnode *);             /* 106 */
PRIVATE void IO_SetDIBits(Graphnode *);                 /* 107 */
PRIVATE void IO_SetDIBitsToDevice(Graphnode *);         /* 108 */
PRIVATE void IO_CreateCompatibleDC(Graphnode *);        /* 109 */
PRIVATE void IO_RegisterBuffer(Graphnode *);            /* 110 */
PRIVATE void IO_AppendBuffer(Graphnode *);              /* 111 */
PRIVATE void IO_DeleteBuffer(Graphnode *);              /* 112 */
PRIVATE void IO_AppendMenu(Graphnode *);                /* 113 */
PRIVATE void IO_CheckMenuItem(Graphnode *);             /* 114 */
PRIVATE void IO_CreatePopupMenu(Graphnode *);           /* 115 */
PRIVATE void IO_CreateMenu(Graphnode *);                /* 116 */
PRIVATE void IO_DeleteMenu(Graphnode *);                /* 117 */
PRIVATE void IO_DestroyMenu(Graphnode *);               /* 118 */
PRIVATE void IO_DrawMenuBar(Graphnode *);               /* 119 */
PRIVATE void IO_EnableMenuItem(Graphnode *);            /* 120 */
PRIVATE void IO_GetMenu(Graphnode *);                   /* 121 */
PRIVATE void IO_GetMenuItemCount(Graphnode *);          /* 122 */
PRIVATE void IO_GetMenuItemID(Graphnode *);             /* 123 */
PRIVATE void IO_GetMenuState(Graphnode *);              /* 124 */
PRIVATE void IO_GetMenuString(Graphnode *);             /* 125 */
PRIVATE void IO_GetSubMenu(Graphnode *);                /* 126 */
PRIVATE void IO_GetSystemMenu(Graphnode *);             /* 127 */
PRIVATE void IO_InsertMenu(Graphnode *);                /* 128 */
PRIVATE void IO_RemoveMenu(Graphnode *);                /* 129 */
PRIVATE void IO_SetMenu(Graphnode *);                   /* 130 */
PRIVATE void IO_FindWindow(Graphnode *);                /* 131 */
PRIVATE void IO_RegisterIOMenu(Graphnode *);            /* 132 */
PRIVATE void IO_DeleteDC(Graphnode *);                  /* 133 */
PRIVATE void IO_GetUpdateRect(Graphnode *);             /* 134 */
PRIVATE void IO_SendMessage(Graphnode *);               /* 135 */
PRIVATE void IO_GetObject(Graphnode *);                 /* 136 */

/* functions 137 to 163 are DDEML functions */
void IO_DdeInitialize(Graphnode *);                     /* 137 */
void IO_DdeUninitialize(Graphnode *);                   /* 138 */
void IO_DdeConnectList(Graphnode *);                    /* 139 */
void IO_DdeQueryNextServer(Graphnode *);                /* 140 */
void IO_DdeDisconnectList(Graphnode *);                 /* 141 */
void IO_DdeConnect(Graphnode *);                        /* 142 */
void IO_DdeDisconnect(Graphnode *);                     /* 143 */
void IO_DdeReconnect(Graphnode *);                      /* 144 */
void IO_DdeQueryConvInfo(Graphnode *);                  /* 145 */
void IO_DdeSetUserHandle(Graphnode *);                  /* 146 */
void IO_DdeAbandonTransaction(Graphnode *);             /* 147 */
void IO_DdePostAdvise(Graphnode *);                     /* 148 */
void IO_DdeEnableCallback(Graphnode *);                 /* 149 */
void IO_DdeNameService(Graphnode *);                    /* 150 */
void IO_DdeClientTransaction(Graphnode *);              /* 151 */
void IO_DdeCreateDataHandle(Graphnode *);               /* 152 */
void IO_DdeAddData(Graphnode *);                        /* 153 */
void IO_DdeGetData(Graphnode *);                        /* 154 */
void IO_DdeAccessData(Graphnode *);                     /* 155 */
void IO_DdeUnaccessData(Graphnode *);                   /* 156 */
void IO_DdeFreeDataHandle(Graphnode *);                 /* 157 */
void IO_DdeGetLastError(Graphnode *);                   /* 158 */
void IO_DdeCreateStringHandle(Graphnode *);             /* 159 */
void IO_DdeQueryString(Graphnode *);                    /* 160 */
void IO_DdeFreeStringHandle(Graphnode *);               /* 161 */
void IO_DdeKeepStringHandle(Graphnode *);               /* 162 */
void IO_DdeCmpStringHandles(Graphnode *);               /* 163 */
void IO_DdeReturnResult(Graphnode *);                   /* 164 */
void IO_DdeReturnAdvise(Graphnode *);                   /* 165 */

PRIVATE void IO_GetBuffer(Graphnode *);                 /* 166 */
PRIVATE void IO_SetScrollRange(Graphnode *);		/* 167 */
PRIVATE void IO_GetScrollRange(Graphnode *);		/* 168 */
PRIVATE void IO_SetScrollPos(Graphnode *);		/* 169 */
PRIVATE void IO_GetScrollPos(Graphnode *);		/* 170 */
PRIVATE void IO_ShowScrollBar(Graphnode *);		/* 171 */
PRIVATE void IO_EnableScrollBar(Graphnode *);		/* 172 */

#endif   /* __graphfn_h */

