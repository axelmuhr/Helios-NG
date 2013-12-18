/************************************************************************/
/************************************************************************/
/*                      Helios Windows I/O Server                       */
/*         Copyright (C) 1993 Perihelion Software Limited               */
/*                                                                      */
/* hel_dde.c - Helios DDEML function implementation                     */
/************************************************************************/
/************************************************************************/

#define DDE_Module
#include "helios.h"
#include <ddeml.h>

#include "windows\graph.h"
#include "windows\hash.h"
#include "windows\graphdef.h"
#include "windows\graphfn.h"

/***** Shared globals ***************************************************/

extern HashTable hashWindows;

/***** Private function prototypes **************************************/

PRIVATE void SendRegister(UINT, UINT, HSZ, HSZ);


/***** DDE callback function ********************************************/

HDDEDATA CALLBACK DdeCallback(UINT wType, UINT wFmt, HCONV hConv, HSZ hsz1, HSZ hsz2, 
                HDDEDATA hData, DWORD dwData1, DWORD dwData2)
{
    DdeInstancenode *dde;
    DdeConvnode *conv;

    DdeDebug(("DdeCallback(0x%04x,0x%04x,0x%lx,0x%lx,0x%lx,0x%lx,0x%lx,0x%lx)", 
        wType, wFmt, hConv, hsz1, hsz2, hData, dwData1, dwData2));

    /* first, those functions which have a null conversation handle */
    if (hConv == 0)
    {
        if (wType == XTYP_CONNECT)
        {
            /* find a registered server/item */
            if ((dde = find_server(hsz2)) != (DdeInstancenode *)NULL)
            {
                if (add_pending(hsz2, hsz1, dde))
                {
                    send_dde_to_port(dde->port, wType, wFmt, hConv, hsz1, hsz2, hData, dwData1, dwData2);
                    return 1;
                }
                else
                    return 0;
            }
            else
                return 0;
        }
        else if (wType == XTYP_WILDCONNECT)
        {
            /* this is not supported so return NULL */
            return NULL;
        }
        else
        {
            /* XTYP_REGISTER and XTYP_UNREGISTER - send to all instances */
            SendRegister(wType, wFmt, hsz1, hsz2);
            return NULL;
        }
    }
    switch (wType)
    {
        /* non-blockable callbacks */
        case XTYP_CONNECT_CONFIRM:
            /* add connection to list, then send data */
            dde = find_pending(hsz2, hsz1);
            if (dde == (DdeInstancenode *)NULL)
                DdeDisconnect(hConv);
            else if (add_conv(hConv, dde))
                send_dde_to_port(dde->port, wType, wFmt, hConv, hsz1, hsz2, hData, dwData1, dwData2);
            else
                DdeDisconnect(hConv);
            break;

        case XTYP_DISCONNECT:
            /* send data, then remove from lists */
            dde = find_conv_dde(hConv);
            if (dde != (DdeInstancenode *)NULL)
            {
                send_dde_to_port(dde->port, wType, wFmt, hConv, hsz1, hsz2, hData, dwData1, dwData2);
                delete_conv(hConv);
            }
            break;

        case XTYP_ERROR:
            /* send to conversation id */
            dde = find_conv_dde(hConv);
            if (dde != (DdeInstancenode *)NULL)
                send_dde_to_port(dde->port, wType, wFmt, hConv, hsz1, hsz2, hData, dwData1, dwData2);
            return NULL;

        case XTYP_ADVREQ:
            conv = find_conv(hConv);
            if (conv == (DdeConvnode *)NULL)
                return NULL;

            dde = conv->dde;
            if (!dde->bAdviseFlag)
            {
                /* send the data - return null because we cannot return it yet */
                DdeDebug(("XTYP_ADVREQ number %d", dde->nAdviseTotal));
                dde->nAdviseTotal++;
                send_dde_to_port(dde->port, wType, wFmt, hConv, hsz1, hsz2, hData, dwData1, dwData2);
                return NULL;
            }
            else
            {
                /* we can return the stored value */
                dde->nAdviseTotal--;
                DdeDebug(("XTYP_ADVREQ releasing number %d", dde->nAdviseTotal));
                return conv->hAdvData;
            }
            break;

        /* blockable callbacks */
        case XTYP_ADVDATA:
        case XTYP_XACT_COMPLETE:
        case XTYP_EXECUTE:
        case XTYP_POKE:
            conv = find_conv(hConv);
            if (conv != (DdeConvnode *)NULL)
            {
                dde = conv->dde;
                if (conv->bIsBlocked)
                {
                    DdeDebug(("DdeCallback releasing conv 0x%lx with 0x%lx, dde=0x%lx", hConv, conv->hData, dde));
                    conv->bIsBlocked = FALSE;
                    return conv->hData;
                }
                else
                {
                    BYTE FAR *lpData;
                    DWORD dwSize;
                    HDDEDATA hNewData;

                    DdeDebug(("DdeCallback for conv 0x%lx posted and blocked, dde = 0x%lx", hConv, dde));
                    lpData = DdeAccessData(hData, &dwSize);

                    hNewData = DdeCreateDataHandle(dde->idInst, lpData, dwSize, 0, hsz2, 0, HDATA_APPOWNED);
                    DdeUnaccessData(hData);
                    hData = hNewData;
                    AddHandle(&hashWindows, hData, HT_HDATA, (void FAR *)dde->app);

                    DdeDebug(("Data handle now 0x%lx (appowned)", hData));
                    send_dde_to_port(dde->port, wType, wFmt, hConv, hsz1, hsz2, hData, dwData1, dwData2);
                    conv->bIsBlocked = TRUE;
                    return CBR_BLOCK;
                }
            }
            else
                return (HDDEDATA)DDE_FNOTPROCESSED;
            break;

        case XTYP_ADVSTART:
        case XTYP_ADVSTOP:
        case XTYP_REQUEST:
            /* if a conv doesn't exist - not processed */
            conv = find_conv(hConv);
            if (conv != (DdeConvnode *)NULL)
            {
                dde = conv->dde;
                if (conv->bIsBlocked)
                {
                    DdeDebug(("DdeCallback releasing conv 0x%lx with 0x%lx, dde = 0x%lx", hConv, conv->hData, dde));
                    conv->bIsBlocked = FALSE;
                    return conv->hData;
                }
                else
                {
                    DdeDebug(("DdeCallback for conv 0x%lx posted and blocked, 0x%lx", hConv, dde));
                    send_dde_to_port(dde->port, wType, wFmt, hConv, hsz1, hsz2, hData, dwData1, dwData2);
                    conv->bIsBlocked = TRUE;
                    return CBR_BLOCK;
                }
            }
            else
                return (HDDEDATA)DDE_FNOTPROCESSED;

        default:
            ServerDebug("ERROR: Unknown DDE callback function 0x%04x", wType);
    }
}

/***** PRIVATE functions ************************************************/

void SendRegister(UINT wType, UINT wFmt, HSZ hsz1, HSZ hsz2)
{
    Graphnode *app;
    DdeInstancenode *dde;

    for (app=get_first_app(); 
         app->node.node.next != (Node *)NULL;
         app = (Graphnode *)app->node.node.next)
    {
        for (dde=(DdeInstancenode *)app->dde_list.head;
             dde->node.next != (Node *)NULL;
             dde = (DdeInstancenode *)dde->node.next)
        {
            send_dde_to_port(dde->port, wType, wFmt, 0, hsz1, hsz2, 0, dde->idInst, 0);
            DdeKeepStringHandle(dde->idInst, hsz1);
            DdeKeepStringHandle(dde->idInst, hsz2);
        }
    }
}


/***** DDE implementation functions *************************************/

/* 137 */
void IO_DdeInitialize(Graphnode *app)
{
    DWORD idInst = mcb->Control[0];
    DWORD afCmd  = mcb->Control[1];
    DWORD ulRes  = mcb->Control[2];
    Port  port   = mcb->Control[3];
    UINT  uRet;
    push_mcb();

    uRet = DdeInitialize(&idInst, (PFNCALLBACK)lpfnDdeCallback, afCmd, ulRes);
    if (uRet == DMLERR_NO_ERROR)
    {
        if (!add_instance(app, idInst, port))
        {
            DdeDebug(("DdeInitialize failed"));
            DdeUninitialize(idInst);
            uRet = DMLERR_LOW_MEMORY;
        }
    }
    DdeDebug(("DdeInitialize(0x%lx,0x%lx,0x%lx)=0x%x", idInst, afCmd, ulRes, uRet));
    mcb->Control[1] = idInst;
    mcb->Control[0] = uRet;
    my_Request_Return(ReplyOK, 2, 0);
}

/* 138 */
void IO_DdeUninitialize(Graphnode *app)
{
    DWORD idInst = mcb->Control[0];
    BOOL  bRet;
    push_mcb();

    bRet = DdeUninitialize(idInst);

    if (bRet)
    {
        remove_convs(idInst);
        remove_servers(idInst);
        delete_instance(app, idInst);
    }
    DdeDebug(("DdeUninitialize(0x%lx) = 0x%x", idInst, bRet));

    mcb->Control[0] = bRet;
    my_Request_Return(ReplyOK, 1, 0);
}

/* 139 */
void IO_DdeConnectList(Graphnode *app)
{
    DWORD     idInst     = mcb->Control[0];
    HSZ       hszService = mcb->Control[1];
    HSZ       hszTopic   = mcb->Control[2];
    HCONVLIST hConvList  = mcb->Control[3];
    HCONVLIST hNewList;
    HCONV     hConv=NULL;
    CONVCONTEXT FAR *pCC = (CONVCONTEXT FAR *)&mcb->Data[0];
    DdeInstancenode *dde;
    push_mcb();

    hNewList = DdeConnectList(idInst, hszService, hszTopic, hConvList, pCC);
    DdeDebug(("DdeConnectList(0x%lx,0x%lx,0x%lx,0x%lx) = 0x%lx", idInst, hszService, hszTopic, hConvList));

    /* find the instance node */
    dde = find_instance(app, idInst);

    /* for each connection - save the conversation */
    while ((hConv = DdeQueryNextServer(hNewList, hConv)) != NULL)
    {
        if (dde == (DdeInstancenode *)NULL)
        {
            DdeDisconnect(hConv);
            hConv = NULL;
        }
        else
        {
            if (!add_conv(hConv, dde))
            {
                DdeDisconnect(hConv);
                hConv = NULL;
            }
        }
    }

    my_Request_Return(ReplyOK, 1, 0);
}

/* 140 */
void IO_DdeQueryNextServer(Graphnode *app)
{
    HCONVLIST hConvList = mcb->Control[0];
    HCONV     hConv     = mcb->Control[1];
    push_mcb();

    mcb->Control[0] = DdeQueryNextServer(hConvList, hConv);
    DdeDebug(("DdeQueryNextServer(0x%lx,0x%lx) = 0x%lx", hConvList, hConv, mcb->Control[0]));

    my_Request_Return(ReplyOK, 1, 0);
}

/* 141 */
void IO_DdeDisconnectList(Graphnode *app)
{
    HCONVLIST hConvList = mcb->Control[0];
    HCONV hConv = NULL;
    push_mcb();

    /* for each connection - remove the connection data */
    while ((hConv = DdeQueryNextServer(hConvList, hConv)) != NULL)
        delete_conv(hConv);
    
    mcb->Control[0] = DdeDisconnectList(hConvList);
    DdeDebug(("DdeDisconnectList(0x%lx) = 0x%lx", hConvList, mcb->Control[0]));

    my_Request_Return(ReplyOK, 1, 0);
}

/* 142 */
void IO_DdeConnect(Graphnode *app)
{
    DWORD     idInst     = mcb->Control[0];
    HSZ       hszService = mcb->Control[1];
    HSZ       hszTopic   = mcb->Control[2];
    HCONV     hConv;
    CONVCONTEXT FAR *pCC = (CONVCONTEXT FAR *)&mcb->Data[0];
    DdeInstancenode *dde;
    push_mcb();

    if (mcb->MsgHdr.DataSize == 0)
        pCC = NULL;

    hConv = DdeConnect(idInst, hszService, hszTopic, pCC);
    DdeDebug(("DdeConnect(0x%lx,0x%lx,0x%lx) = 0x%lx", idInst, hszService, hszTopic, hConv));
    if (hConv != NULL)
    {
        dde = find_instance(app, idInst);
        if (dde == (DdeInstancenode *)NULL)
        {
            DdeDisconnect(hConv);
            hConv = NULL;
        }
        else
        {
            if (!add_conv(hConv, dde))
            {
                DdeDisconnect(hConv);
                hConv = NULL;
            }
        }
    }

    mcb->Control[0] = hConv;
    my_Request_Return(ReplyOK, 1, 0);
}

/* 143 */
void IO_DdeDisconnect(Graphnode *app)
{
    HCONV hConv = mcb->Control[0];
    push_mcb();

    mcb->Control[0] = DdeDisconnect(hConv);
    DdeDebug(("DdeDisconnect(0x%lx) = 0x%lx", hConv, mcb->Control[0]));
    delete_conv(hConv);

    my_Request_Return(ReplyOK, 1, 0);
}

/* 144 */
void IO_DdeReconnect(Graphnode *app)
{
    HCONV hConv = mcb->Control[0];
    HCONV hConvNew;
    DdeInstancenode *dde;
    push_mcb();

    hConvNew = DdeReconnect(hConv);
    DdeDebug(("DdeReconnect(0x%lx) = 0x%lx", hConv, hConvNew));
    if (hConvNew != NULL)
    {
        dde = find_conv_dde(hConv);
        if (dde == (DdeInstancenode *)NULL)
        {
            DdeDisconnect(hConvNew);
            hConvNew = NULL;
        }
        else
        {
            if (!add_conv(hConvNew, dde))
            {
                DdeDisconnect(hConvNew);
                hConvNew = NULL;
            }
        }
    }

    mcb->Control[0] = hConvNew;
    my_Request_Return(ReplyOK, 1, 0);
}

/* 145 */
void IO_DdeQueryConvInfo(Graphnode *app)
{
    HCONV hConv = mcb->Control[0];
    DWORD idTran= mcb->Control[1];
    CONVINFO FAR *pConvInfo = (CONVINFO FAR *)&mcb->Data[0];
    push_mcb();

    mcb->Control[0] = DdeQueryConvInfo(hConv, idTran, pConvInfo);
    DdeDebug(("DdeQueryConvInfo(0x%lx,0x%lx) = 0x%lx", hConv, idTran, mcb->Control[0]));

    my_Request_Return(ReplyOK, 1, sizeof(CONVINFO));
}

/* 146 */
void IO_DdeSetUserHandle(Graphnode *app)
{
    HCONV hConv = mcb->Control[0];
    DWORD id    = mcb->Control[1];
    DWORD hUser = mcb->Control[2];
    push_mcb();

    mcb->Control[0] = DdeSetUserHandle(hConv, id, hUser);
    DdeDebug(("DdeSetUserHandle(0x%lx,0x%lx,0x%lx) = 0x%lx", hConv, id, hUser, mcb->Control[0]));

    my_Request_Return(ReplyOK, 1, 0);
}

/* 147 */
void IO_DdeAbandonTransaction(Graphnode *app)
{
    DWORD idInst = mcb->Control[0];
    HCONV hConv  = mcb->Control[1];
    DWORD idTran = mcb->Control[2];
    push_mcb();

    mcb->Control[0] = DdeAbandonTransaction(idInst, hConv, idTran);
    DdeDebug(("DdeAbandonTransaction(0x%lx,0x%lx,0x%lx) = 0x%lx", idInst, hConv, idTran, mcb->Control[0]));

    my_Request_Return(ReplyOK, 1, 0);
}

/* 148 */
void IO_DdePostAdvise(Graphnode *app)
{
    DWORD idInst   = mcb->Control[0];
    HSZ   hszTopic = mcb->Control[1];
    HSZ   hszItem  = mcb->Control[2];
    DdeInstancenode *dde;
    push_mcb();

    dde = find_instance(app, idInst);
    if (dde != (DdeInstancenode *)NULL)
    {
        dde->nAdviseTotal = 0;
        dde->nAdviseCount = 0;
        dde->hszTopic = hszTopic;
        dde->hszItem = hszItem;
    }
    
    mcb->Control[0] = DdePostAdvise(idInst, hszTopic, hszItem);
    if ((dde != (DdeInstancenode *)NULL) && (dde->nAdviseTotal))
        dde->bAdviseFlag = TRUE;
    DdeDebug(("DdePostAdvise(0x%lx,0x%lx,0x%lx) = 0x%lx", idInst, hszTopic, hszItem, mcb->Control[0]));

    my_Request_Return(ReplyOK, 1, 0);
}

/* 149 */
void IO_DdeEnableCallback(Graphnode *app)
{
    DWORD idInst = mcb->Control[0];
    HCONV hConv  = mcb->Control[1];
    UINT  wCmd   = mcb->Control[2];
    push_mcb();

    mcb->Control[0] = DdeEnableCallback(idInst, hConv, wCmd);
    DdeDebug(("DdeEnableCallback(0x%lx,0x%lx,0x%x) = 0x%lx", idInst, hConv, wCmd, mcb->Control[0]));

    my_Request_Return(ReplyOK, 1, 0);
}

/* 150 */
void IO_DdeNameService(Graphnode *app)
{
    DWORD idInst = mcb->Control[0];
    HSZ   hsz1   = mcb->Control[1];
    HSZ   hsz2   = mcb->Control[2];
    UINT  afCmd  = mcb->Control[3];
    DdeInstancenode *dde;
    push_mcb();

    mcb->Control[0] = DdeNameService(idInst, hsz1, hsz2, afCmd);
    DdeDebug(("DdeNameService(0x%lx,0x%lx,0x%lx,0x%x) = 0x%lx", idInst, hsz1, 
                hsz2, afCmd, mcb->Control[0]));
    if ((afCmd & DNS_REGISTER) && (mcb->Control[0]))
    {
        dde = find_instance(app, idInst);
        if (dde == (DdeInstancenode *)NULL)
        {
            DdeNameService(idInst, hsz1, hsz2, DNS_UNREGISTER);
            mcb->Control[0] = 0;
        }
        else
            add_server(hsz1, dde);
    }

    if ((afCmd & DNS_UNREGISTER) && (mcb->Control[0]))
    {
        if (hsz1 != NULL)
            delete_server(hsz1, idInst);
        else
            remove_servers(idInst);
    }

    my_Request_Return(ReplyOK, 1, 0);
}

/* 151 */
void IO_DdeClientTransaction(Graphnode *app)
{
    void FAR *pData;
    HGLOBAL  hMem      = mcb->Control[0];
    DWORD    cbData    = mcb->Control[1];
    HCONV    hConv     = mcb->Control[2];
    HSZ      hszItem   = mcb->Control[3];
    UINT     wFmt      = mcb->Control[4];
    UINT     wType     = mcb->Control[5];
    DWORD    dwTimeout = mcb->Control[6];
    DWORD    dwResult;

    push_mcb();

    if (cbData == -1)
        pData = (void FAR *)mcb->Control[0];
    else if (cbData > 0)
    {
        pData = (void FAR *)GlobalLock(hMem);
        if (pData == (void FAR *)NULL)
        {
            mcb->Control[0] = 0;
            mcb->Control[1] = 0;
            my_Request_Return(ReplyOK, 2, 0);
            return;
        }
    }
    else
        pData = NULL;

    DdeDebug(("ClientTransaction(0x%lx,0x%lx,0x%lx,0x%lx,0x%x,0x%x,%ld)", pData, cbData, 
                hConv, hszItem, wFmt, wType, dwTimeout));

    mcb->Control[0] = DdeClientTransaction(pData, cbData, hConv, hszItem,
                        wFmt, wType, dwTimeout, &dwResult);

    DdeDebug(("fnrc = 0x%lx, dwResult = 0x%lx", mcb->Control[0], dwResult));
    mcb->Control[1] = dwResult;

    if (cbData > 0)
        GlobalUnlock(hMem);

    my_Request_Return(ReplyOK, 2, 0);
}

/* 152 */
void IO_DdeCreateDataHandle(Graphnode *app)
{
    DWORD   idInst  = mcb->Control[0];
    HGLOBAL hMem    = mcb->Control[1];
    DWORD   cb      = mcb->Control[2];
    DWORD   cbOff   = mcb->Control[3];
    HSZ     hszItem = mcb->Control[4];
    UINT    wFmt    = mcb->Control[5];
    UINT    afCmd   = mcb->Control[6];
    void FAR *pSrc;
    push_mcb();

    if (hMem)
    {
        pSrc = (void FAR *)GlobalLock(hMem);
        if (pSrc == (void FAR *)NULL)
        {
            mcb->Control[0] = NULL;
            my_Request_Return(ReplyOK, 1, 0);
            return;
        }
    }
    else
        pSrc = NULL;

    mcb->Control[0] = DdeCreateDataHandle(idInst, pSrc, cb, cbOff, hszItem,
                        wFmt, afCmd);

    DdeDebug(("CreateDataHandle(0x%lx,0x%ld,0x%ld,0x%lx,0x%x,0x%x) = 0x%lx", 
                idInst, cb, cbOff, hszItem, wFmt, afCmd));

    if (((afCmd & HDATA_APPOWNED) == HDATA_APPOWNED) && (mcb->Control[0]))
        AddHandle(&hashWindows, mcb->Control[0], HT_HDATA, (void FAR *)app);

    if (hMem)
        GlobalUnlock(hMem);
    my_Request_Return(ReplyOK, 1, 0);
}

/* 153 */
void IO_DdeAddData(Graphnode *app)
{
    HDDEDATA hData = mcb->Control[0];
    HGLOBAL  hMem  = mcb->Control[1];
    DWORD    cb    = mcb->Control[2];
    DWORD    cbOff = mcb->Control[3];
    void FAR *pSrc;
    push_mcb();

    pSrc = (void FAR *)GlobalLock(hMem);
    if (pSrc == (void FAR *)NULL)
    {
        mcb->Control[0] = NULL;
        my_Request_Return(ReplyOK, 1, 0);
        return;
    }

    mcb->Control[0] = DdeAddData(hData, pSrc, cb, cbOff);
    DdeDebug(("DdeAddData(0x%lx,,%ld,%ld) = 0x%lx", hData, cb, cbOff, mcb->Control[0]));

    GlobalUnlock(hMem);
    my_Request_Return(ReplyOK, 1, 0);
}

/* 154 */
void IO_DdeGetData(Graphnode *app)
{
    HDDEDATA hData = mcb->Control[0];
    DWORD    cbMax = mcb->Control[1];
    DWORD    cbOff = mcb->Control[2];
    DWORD    dwRet;
    HGLOBAL  hMem;
    void FAR *pDst;
    push_mcb();

    if (cbMax == 0)
    {
        DdeDebug(("DdeGetData called with no buffer as parameter"));
        pDst = NULL;
        hMem = NULL;
    }
    else
    {
        DdeDebug(("DdeGetData allocating buffer of %ld", cbMax));
        hMem = GlobalAlloc(GHND, cbMax);
        DdeDebug(("Handle = 0x%x", hMem));
        pDst = (void FAR *)GlobalLock(hMem);
        DdeDebug(("memory pointer = %x:%x", FP_SEG(pDst), FP_OFF(pDst)));
        if ((hMem == NULL) || (pDst == NULL))
        {
            if (hMem != NULL)
                GlobalFree(hMem);
            ServerDebug("ERROR: DdeGetData - insufficient memory on I/O server");
            mcb->Control[0] = 0;
            mcb->Control[1] = 0;
            my_Request_Return(ReplyOK, 2, 0);
            return;
        }
        else
            DdeDebug(("DdeGetData buffer successfully allocated"));

        AddHandle(&hashWindows, hMem, HT_HGLOBAL, (void FAR *)app);
    }
    
    DdeDebug(("Calling DdeGetData(0x%lx,0x%lx,%ld,%ld)", hData, pDst, cbMax, cbOff));

    dwRet = DdeGetData(hData, pDst, cbMax, cbOff);

    /* we now have the data - return the handle and size */
    DdeDebug(("DdeGetData(0x%lx,%ld,%ld) = 0x%lx, handle = 0x%x", 
                hData, cbMax, cbOff, dwRet, hMem));

    if (cbMax != 0)
        GlobalUnlock(hMem);
    
    mcb->Control[0] = dwRet;
    mcb->Control[1] = hMem;
    
    my_Request_Return(ReplyOK, 2, 0);
}

/* 155 */
void IO_DdeAccessData(Graphnode *app)
{
    /* this function is implemented using DdeGetData */
    ServerDebug("DdeAccessData is defunct - this should not appear");
}

/* 156 */
void IO_DdeUnaccessData(Graphnode *app)
{
    /* this function has a pure Helios implementation - no stub reqd */
    ServerDebug("DdeUnaccessData is defunct - this should not appear");
}

/* 157 */
void IO_DdeFreeDataHandle(Graphnode *app)
{
    HDDEDATA hData = mcb->Control[0];
    push_mcb();

    mcb->Control[0] = DdeFreeDataHandle(hData);
    DdeDebug(("DdeFreeDataHandle(0x%lx) = 0x%lx", hData, mcb->Control[0]));

    RemoveHandle(&hashWindows, hData, HT_HDATA);

    my_Request_Return(ReplyOK, 1, 0);
}

/* 158 */
void IO_DdeGetLastError(Graphnode *app)
{
    DWORD idInst = mcb->Control[0];

    push_mcb();

    mcb->Control[0] = DdeGetLastError(idInst);
    DdeDebug(("DdeGetLastError(0x%lx) = 0x%lx", idInst, mcb->Control[0]));

    my_Request_Return(ReplyOK, 1, 0);
}

/* 159 */
void IO_DdeCreateStringHandle(Graphnode *app)
{
    DWORD  idInst    = mcb->Control[0];
    LPCSTR psz       = (LPCSTR)&mcb->Data[0];
    int    iCodePage = mcb->Control[1];
    
    push_mcb();

    if (mcb->MsgHdr.DataSize == 0)
        psz = NULL;

    mcb->Control[0] = DdeCreateStringHandle(idInst, psz, iCodePage);
    DdeDebug(("DdeCreateStringHandle(0x%lx,,0x%x) = 0x%lx", idInst, iCodePage, mcb->Control[0]));

    my_Request_Return(ReplyOK, 1, 0);
}

/* 160 */
void IO_DdeQueryString(Graphnode *app)
{
    DWORD idInst    = mcb->Control[0];
    HSZ   hsz       = mcb->Control[1];
    LPSTR psz       = (LPSTR)&mcb->Data[0];
    DWORD cchMax    = mcb->Control[2];
    int   iCodePage = mcb->Control[3];
    push_mcb();

    if (cchMax == 0)
        psz = NULL;

    mcb->Control[0] = DdeQueryString(idInst, hsz, psz, cchMax, iCodePage);
    DdeDebug(("DdeQueryStringHandle(0x%lx,0x%lx,,%d,0x%lx) = 0x%lx", idInst, hsz, cchMax, iCodePage, mcb->Control[0]));

    if (psz != NULL)
        my_Request_Return(ReplyOK, 1, mcb->Control[0]+1);
    else
        my_Request_Return(ReplyOK, 1, 0);
}

/* 161 */
void IO_DdeFreeStringHandle(Graphnode *app)
{
    DWORD idInst = mcb->Control[0];
    HSZ   hsz    = mcb->Control[1];
    push_mcb();

    mcb->Control[0] = DdeFreeStringHandle(idInst, hsz);
    DdeDebug(("DdeFreeStringHandle(0x%lx,0x%lx) = 0x%lx", idInst, hsz, mcb->Control[0]));

    my_Request_Return(ReplyOK, 1, 0);
}

/* 162 */
void IO_DdeKeepStringHandle(Graphnode *app)
{
    DWORD idInst = mcb->Control[0];
    HSZ   hsz    = mcb->Control[1];
    push_mcb();

    mcb->Control[0] = DdeKeepStringHandle(idInst, hsz);
    DdeDebug(("DdeKeepStringHandle(0x%lx,0x%lx) = 0x%lx", idInst, hsz, mcb->Control[0]));

    my_Request_Return(ReplyOK, 1, 0);
}

/* 163 */
void IO_DdeCmpStringHandles(Graphnode *app)
{
    HSZ hsz1 = mcb->Control[0];
    HSZ hsz2 = mcb->Control[1];
    push_mcb();

    mcb->Control[0] = DdeCmpStringHandles(hsz1, hsz2);
    DdeDebug(("DdeCmpStringHandles(0x%lx,0x%lx) = 0x%lx", hsz1, hsz2, mcb->Control[0]));

    my_Request_Return(ReplyOK, 1, 0);
}

/* 164 */
void IO_DdeReturnResult(Graphnode *app)
{
    HCONV    hConv = mcb->Control[0];
    HDDEDATA hRet = mcb->Control[1];
    DdeConvnode *conv;
    push_mcb();
    my_Request_Return(ReplyOK, 0, 0);

    conv = find_conv(hConv);
    if (conv != (DdeConvnode *)NULL)
    {
        conv->hData = hRet;
        DdeDebug(("DdeReturnResult(internal) for conv 0x%lx is 0x%lx", hConv, hRet));
        DdeEnableCallback(conv->dde->idInst, hConv, EC_ENABLEALL);
    }
}


/* 165 */
void IO_DdeReturnAdvise(Graphnode *app)
{
    HCONV    hConv = mcb->Control[0];
    HDDEDATA hRet = mcb->Control[1];
    DdeConvnode *conv;
    DdeInstancenode *dde;
    push_mcb();
    my_Request_Return(ReplyOK, 0, 0);

    conv = find_conv(hConv);
    if (conv == (DdeConvnode *)NULL)
        return;
    
    conv->hAdvData = hRet;
    DdeDebug(("DdeReturnAdvise(internal) for conv 0x%lx is 0x%lx", hConv, hRet));
    
    dde = conv->dde;
    dde->nAdviseCount++;

    DdeDebug(("nAdviseCount = %d, nAdviseTotal = %d, bAdviseFlag = 0x%x", dde->nAdviseCount, dde->nAdviseTotal, dde->bAdviseFlag));

    if (dde->nAdviseCount == dde->nAdviseTotal)
    {
        DdePostAdvise(dde->idInst, dde->hszTopic, dde->hszItem);
        dde->bAdviseFlag = FALSE;
    }
}



