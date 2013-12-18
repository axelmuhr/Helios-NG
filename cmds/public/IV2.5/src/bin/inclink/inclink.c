// implemenatation of class Inclink

#include "base.h"
#include "errhandler.h"
#include "inclink.h"
#include "program.h"
#include "protocol.h"
#include "timer.h"
#include <InterViews/connection.h>
#include <InterViews/space.h>
#include <stdlib.h>

Inclink::Inclink (char* name) : (name) { }

void Inclink::Message (Connection* c, ObjectTag, int op, void* msg, int l) {
    client = c;
    if (op >= 0 && op < inclink_maxreq) {
        Debug("received request '%s'", requestName[op]);
        if (l > 0) {
            Debug("  message string '%s'", (char*) msg);
        }
    }
    char* s = (char*) msg;
    boolean flag = atoi(s);

    switch (op) {
    case inclink_dflag:
        Debug.messages = flag;
        break;
    case inclink_dolink:
        currProg->DoLink();
        SendAck();
        break;
    case inclink_fulllink:
        if (flag) {
            currProg->PrepareLink(true);
        } else {
            currProg->PrepareLink(currProg->fullLink);
        }
        break;
    case inclink_kflag:
        currProg->k_flag = flag;
        break;
    case inclink_ldflag:
        currProg->addSlop = flag;
        break;
    case inclink_linkfile:
        currProg->LinkFile(s);
        SendAck();
        break;
    case inclink_modfile:
        currProg->Reread(s);
        SendAck();
        break;
    case inclink_quit:
        SendAck();
        exit(0);                  // program exits here
        break;
    case inclink_Sflag:
        /* not implemented */
        break;
    case inclink_sflag:
        currProg->SetsFlag(flag);
        break;
    case inclink_timer:
        if (flag) {
            timer->BeginTiming();
        } else {
            timer->EndTiming();
        }
        SendAck();
        break;
    case inclink_tinclink:
        currProg->tinclink = flag;
        break;
    case inclink_Xflag:
        currProg->SetXFlag(flag);
        break;
    case inclink_xflag:
        currProg->SetxFlag(flag);
        break;
    default:
        Panic("received unrecognized operation '%d'", op);
    }
    client = nil;
}

void Inclink::SendMsg (int code, char* msg, int msglen) {
    if (client) {
        client->Write(&code, sizeof(code));
        client->Write(&msglen, sizeof(msglen));
        client->Write(msg, msglen);
    } else {
        fprintf(stderr, "(%d) %s\n", code, msg);
    }
}

void Inclink::SendAck () {
    int code = inclink_ack;
    client->Write(&code, sizeof(code));
}
