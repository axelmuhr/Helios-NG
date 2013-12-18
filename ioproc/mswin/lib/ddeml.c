/************************************************************************/
/************************************************************************/
/*               Microsoft Windows Libraries for Helios                 */
/*                                                                      */
/*       Copyright (C) 1990-1993,  Perihelion Software Limited          */
/*                         All Rights Reserved                          */
/*                                                                      */
/*   ddeml.c - The dde management library link                          */
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
#include "windows.h"
#include "ddeml.h"

#include "windefs.h"


/***** private function prototypes **************************************/

PRIVATE void receive_dde(Port, Semaphore *, PFNCALLBACK);
PRIVATE void WINAPI DdeReturnResult(HCONV, HDDEDATA);
PRIVATE void WINAPI DdeReturnAdvise(HCONV, HDDEDATA);

/***** private function implementation **********************************/

PRIVATE void receive_dde(Port port, Semaphore *pSem, PFNCALLBACK pfnCallback)
{
    MCB      mcb;
    word     Control[8];
    HDDEDATA hRet;
    DWORD    idInst = (DWORD) 0;

    while (!TestSemaphore(pSem))
    {
        InitMCB(&mcb, MsgHdr_Flags_preserve, port, NullPort, 0);
        mcb.Data = NULL;
        mcb.MsgHdr.DataSize = 0;
        mcb.Control = &Control[0];
        mcb.MsgHdr.ContSize = 8;
        mcb.Timeout = OneSec*10;
        if (GetMsg(&mcb) == Err_Null)
        {
            if ((Control[0] == XTYP_REGISTER) || (Control[0] == XTYP_UNREGISTER))
            {
                idInst = Control[6];
                Control[6] = 0;
            }

            hRet = pfnCallback((UINT)Control[0], (UINT)Control[1], Control[2], Control[3],
                               Control[4], Control[5], Control[6], Control[7]);
            switch(Control[0])
            {
                case XTYP_REGISTER:
                case XTYP_UNREGISTER:
                    DdeFreeStringHandle(idInst, Control[3]);
                    DdeFreeStringHandle(idInst, Control[4]);
                    break;

                case XTYP_ADVDATA:
                case XTYP_XACT_COMPLETE:
                case XTYP_EXECUTE:
                case XTYP_POKE:
                    DdeFreeDataHandle(Control[5]);
                    DdeReturnResult(Control[2], hRet);
                    break;

                case XTYP_ADVSTART:
                case XTYP_ADVSTOP:
                case XTYP_REQUEST:
                    DdeReturnResult(Control[2], hRet);
                    break;

                case XTYP_ADVREQ:
                    DdeReturnAdvise(Control[2], hRet);
                    break;
            }
        }
    }
}


/***** public functions for the libarary ********************************/

UINT WINAPI DdeInitialize(DWORD *pidInst, PFNCALLBACK pfnCallback,
            DWORD afCmd, DWORD ulRes)
{
    DdeInstancenode *instance;
    Port             port = NewPort();
    word             Control[4];

    /* make sure that we get connect_confirm and disconnect */
    afCmd &= ~((CBF_SKIP_CONNECT_CONFIRMS) | (CBF_SKIP_DISCONNECTS));

    /* first make the call */
    Control[0] = *pidInst;
    Control[1] = afCmd;
    Control[2] = ulRes;
    Control[3] = port;
    
    if (!send_message(IO_DdeInitialize, 4, 0, &Control[0], NULL))
        Control[0] = DMLERR_SYS_ERROR;
    else
        if (Control[0] == DMLERR_NO_ERROR)  /* no error */
            *pidInst = Control[1];

    if (Control[0] != DMLERR_NO_ERROR)
    {
        FreePort(port);
        return (UINT)Control[0];
    }

    /* spawn receiver */
    instance = DdeAddInstance(*pidInst, port);
    if (instance == (DdeInstancenode *)NULL)
    {
        DdeUninitialize(*pidInst);
        FreePort(port);
        return DMLERR_SYS_ERROR;
    }
    if (!Fork(2000, receive_dde, sizeof(Port)+sizeof(Semaphore *)+sizeof(PFNCALLBACK),
                port, &instance->sem, pfnCallback))
    {
        /* unsuccessful - return false */
        DdeUninitialize(*pidInst);
        return DMLERR_SYS_ERROR;
    }
    return DMLERR_NO_ERROR;
}


BOOL WINAPI DdeUninitialize(DWORD idInst)
{
    DdeInstancenode *instance = DdeFindInstance(idInst);
    if (instance != (DdeInstancenode *)NULL)
    {
        Signal(&instance->sem);
        FreePort(instance->port);
        DdeRemoveInstance(instance);
    }

    return (BOOL)send_1_parameter(IO_DdeUninitialize, idInst);
}


HCONVLIST WINAPI DdeConnectList(DWORD idInst, HSZ hszService, HSZ hszTopic,
            HCONVLIST hConvList, CONVCONTEXT *pCC)
{
    word Control[4];
    Control[0] = idInst;
    Control[1] = hszService;
    Control[2] = hszTopic;
    Control[3] = hConvList;
    if (!send_message(IO_DdeConnectList, 4, sizeof(CONVCONTEXT), &Control[0], (BYTE *)pCC))
        return 0;
    return (HCONVLIST)Control[0];
}


HCONV WINAPI DdeQueryNextServer(HCONVLIST hConvList, HCONV hConvPrev)
{
    return (HCONV)send_2_parameters(IO_DdeQueryNextServer, hConvList, hConvPrev);
}

BOOL WINAPI DdeDisconnectList(HCONVLIST hConvList)
{
    return (BOOL)send_1_parameter(IO_DdeDisconnectList, hConvList);
}

HCONV WINAPI DdeConnect(DWORD idInst, HSZ hszService, HSZ hszTopic,
            CONVCONTEXT *pCC)
{
    word Control[3];
    Control[0] = idInst;
    Control[1] = hszService;
    Control[2] = hszTopic;
    if (pCC != (CONVCONTEXT *)NULL)
    {
        if (!send_message(IO_DdeConnect, 3, sizeof(CONVCONTEXT), &Control[0], (BYTE *)pCC))
            return 0;
    }
    else
    {
        if (!send_message(IO_DdeConnect, 3, 0, &Control[0], NULL))
            return 0;
    }
    return (HCONVLIST)Control[0];
}

BOOL WINAPI DdeDisconnect(HCONV hConv)
{
    return (BOOL)send_1_parameter(IO_DdeDisconnect, hConv);
}

HCONV WINAPI DdeReconnect(HCONV hConv)
{
    return send_1_parameter(IO_DdeReconnect, hConv);
}

UINT WINAPI DdeQueryConvInfo(HCONV hConv, DWORD idTran, CONVINFO *pConvInfo)
{
    word Control[2];
    if (pConvInfo == NULL)
        return 0;
    Control[0] = hConv;
    Control[1] = idTran;
    if (!send_fn(IO_DdeQueryConvInfo, 2, sizeof(CONVINFO), &Control[0], (BYTE *)pConvInfo))
        return 0;
    return (UINT)Control[0];
}

BOOL WINAPI DdeSetUserHandle(HCONV hConv, DWORD id, DWORD hUser)
{
    return (BOOL)send_3_parameters(IO_DdeSetUserHandle, hConv, id, hUser);
}

BOOL WINAPI DdeAbandonTransaction(DWORD idInst, HCONV hConv, DWORD idTran)
{
    return (BOOL)send_3_parameters(IO_DdeAbandonTransaction, idInst, hConv, idTran);
}

BOOL WINAPI DdePostAdvise(DWORD idInst, HSZ hszTopic, HSZ hszItem)
{
    return (BOOL)send_3_parameters(IO_DdePostAdvise, idInst, hszTopic, hszItem);
}

BOOL WINAPI DdeEnableCallback(DWORD idInst, HCONV hConv, UINT wCmd)
{
    return (BOOL)send_3_parameters(IO_DdeEnableCallback, idInst, hConv, wCmd);
}

HDDEDATA WINAPI DdeNameService(DWORD idInst, HSZ hsz1, HSZ hsz2, UINT afCmd)
{
    afCmd = afCmd & ~DNS_FILTEROFF | DNS_FILTERON;
    return send_4_parameters(IO_DdeNameService, idInst, hsz1, hsz2, afCmd);
}

HDDEDATA WINAPI DdeClientTransaction(void *pData, DWORD cbData, HCONV hConv,
            HSZ hszItem, UINT wFmt, UINT wType, DWORD dwTimeout,
            DWORD *pdwResult)
{
    word    Control[7];
    HGLOBAL hMem = NULL;
    if (cbData == -1)
        memcpy(&Control[0], &pData, sizeof(HDDEDATA));
    else
    if ((wType == XTYP_EXECUTE) || (wType == XTYP_POKE) || (wType == XTYP_XACT_COMPLETE))
    {
        if (pData == NULL)
            return NULL;
        else
        {
            hMem=SendBuffer((BYTE *)pData, cbData);
            if (hMem == NULL)
                return NULL;
            Control[0] = hMem;
        }
    }
    else
    {
        hMem = NULL;
        Control[0] = NULL;
        cbData = 0;
    }

    Control[1] = cbData;
    Control[2] = hConv;
    Control[3] = hszItem;
    Control[4] = wFmt;
    Control[5] = wType;
    Control[6] = dwTimeout;
    if (!send_message(IO_DdeClientTransaction, 7, 0, &Control[0], NULL))
    {
        if (pdwResult != NULL)
            *pdwResult = 0;
        DeleteBuffer(hMem);
        return 0;
    }
    if (pdwResult != NULL)
        *pdwResult = Control[1];
    if (hMem)
        DeleteBuffer(hMem);
    return (HDDEDATA)Control[0];
}

HDDEDATA WINAPI DdeCreateDataHandle(DWORD idInst, void *pSrc, DWORD cb,
            DWORD cbOff, HSZ hszItem, UINT wFmt, UINT afCmd)
{
    HGLOBAL hMem=NULL;
    word    Control[7];
    if ((cb != 0) && (pSrc != NULL))
    {
        hMem = SendBuffer((BYTE *)pSrc, cb);
        if (hMem == NULL)
            return NULL;
    }
    Control[0] = idInst;
    Control[1] = hMem;
    Control[2] = cb;
    Control[3] = cbOff;
    Control[4] = hszItem;
    Control[5] = wFmt;
    Control[6] = afCmd;
    if (!send_message(IO_DdeCreateDataHandle, 7, 0, &Control[0], NULL))
    {
        if (hMem)
            DeleteBuffer(hMem);
        return NULL;
    }
    if (hMem)
        DeleteBuffer(hMem);
    return Control[0];
}

HDDEDATA WINAPI DdeAddData(HDDEDATA hData, void *pSrc, DWORD cb, DWORD cbOff)
{
    HGLOBAL  hMem=NULL;
    HDDEDATA hRet;
    if ((cb == 0) || (pSrc == NULL))
        return NULL;
    
    hMem = SendBuffer((BYTE *)pSrc, cb);
    if (hMem == NULL)
        return NULL;
    
    hRet = send_4_parameters(IO_DdeAddData, hData, hMem, cb, cbOff);
    DeleteBuffer(hMem);
    return hRet;
}

DWORD WINAPI DdeGetData(HDDEDATA hData, void *pDst, DWORD cbMax, DWORD cbOff)
{
    word Control[3];
    word received;

    if (pDst == NULL)
        cbMax = 0;

    Control[0] = hData;
    Control[1] = cbMax;
    Control[2] = cbOff;

    if (!send_message(IO_DdeGetData, 3, 0, &Control[0], NULL))
        return 0;

    /* we now get the data into our buffer */
    if ((Control[0] != 0) && (pDst != NULL))
                        if ((received=ReceiveBuffer((HGLOBAL)Control[1], (BYTE *)pDst, (word)Control[0])) != Control[0])
            printf("DdeGetData error: received %d, expecting %d\n", (int)received, (int)Control[0]);
    
    if (Control[1] != NULL)    
        DeleteBuffer((HGLOBAL)Control[1]);

    return Control[0];
}

BYTE * WINAPI DdeAccessData(HDDEDATA hData, DWORD *pcbDataSize)
{
    BYTE *pData;
    *pcbDataSize = DdeGetData(hData, NULL, 0, 0);
    pData = (BYTE *)malloc((size_t)*pcbDataSize);
    if (pData == NULL)
        return NULL;
    DdeGetData(hData, pData, *pcbDataSize, 0);
    DdeAddHData(hData, pData);
    return pData;
}

BOOL WINAPI DdeUnaccessData(HDDEDATA hData)
{
    return DdeRemoveHData(hData);
}

BOOL WINAPI DdeFreeDataHandle(HDDEDATA hData)
{
    return (BOOL)send_1_parameter(IO_DdeFreeDataHandle, hData);
}

UINT WINAPI DdeGetLastError(DWORD idInst)
{
    return (UINT)send_1_parameter(IO_DdeGetLastError, idInst);
}

HSZ WINAPI DdeCreateStringHandle(DWORD idInst, LPCSTR psz, int iCodePage)
{
    word Control[2];
    Control[0] = idInst;
    Control[1] = iCodePage;
    if (psz != NULL)
    {
        if (!send_message(IO_DdeCreateStringHandle, 2, strlen(psz)+1, &Control[0], (BYTE *)psz))
            return NULL;
    }
    else
        if (!send_message(IO_DdeCreateStringHandle, 2, 0, &Control[0], NULL))
            return NULL;

    return Control[0];
}

DWORD WINAPI DdeQueryString(DWORD idInst, HSZ hsz, LPSTR psz, DWORD cchMax,
            int iCodePage)
{
    word Control[4];
    Control[0] = idInst;
    Control[1] = hsz;
    Control[2] = cchMax;
    Control[3] = iCodePage;
    if (psz == NULL)
        cchMax = 0;
    if (!send_fn(IO_DdeQueryString, 4, (int)cchMax, &Control[0], (BYTE *)psz))
        return 0;
    return Control[0];
}

BOOL WINAPI DdeFreeStringHandle(DWORD idInst, HSZ hsz)
{
    return (BOOL)send_2_parameters(IO_DdeFreeStringHandle, idInst, hsz);
}

BOOL WINAPI DdeKeepStringHandle(DWORD idInst, HSZ hsz)
{
    return (BOOL)send_2_parameters(IO_DdeKeepStringHandle, idInst, hsz);
}

int WINAPI DdeCmpStringHandles(HSZ hsz1, HSZ hsz2)
{
    return (int)send_2_parameters(IO_DdeCmpStringHandles, hsz1, hsz2);
}

PRIVATE void WINAPI DdeReturnResult(HCONV hConv, HDDEDATA hResult)
{
    send_2_parameters(IO_DdeReturnResult, hConv, hResult);
}

PRIVATE void WINAPI DdeReturnAdvise(HCONV hConv, HDDEDATA hResult)
{
    send_2_parameters(IO_DdeReturnAdvise, hConv, hResult);
}

