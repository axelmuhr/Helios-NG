/*
 * klink - Kill inclink
 *
 * usage:  klink [outfile]
 */

#include "error.h"
#include "protocol.h"
#include <InterViews/chief.h>
#include <InterViews/connection.h>
#include <InterViews/space.h>
#include <InterViews/spaceman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* myname;
static char* aout = "a.out";
static char* inclink = "inclink";

static void ScanArgs (int, char**);
static Connection* FindServer();
static void KillServer(Connection*);
static void GetError(int, ChiefDeputy*);

main (int argc, char* argv[]) {
    ScanArgs(argc, argv);
    Connection* inclink = FindServer();
    if (inclink != nil) {
        KillServer(inclink);
    }
    exit(Error.NumErrors());
}

static void ScanArgs (int argc, char* argv[]) {
    myname = argv[0];
    errprefix = myname;
    boolean found = false;
    for (int i = 1; i < argc; ++i) {
        char* curarg = argv[i];
        if (curarg[0] == '-') {
            int arglen= strlen(curarg);
            if (arglen == 1) {
                Warning("unrecognized option '-'");
            }
            for (int j = 1; j < arglen; ++j) {
                switch (curarg[j]) {
                default:
                    Warning("unrecognized option '-%c'", curarg[j]);
                }
            }
        } else if (!found) {
            aout = argv[i];
            found = true;
        } else {
            Warning("extra argument '%s'", curarg);
        }
    }
    if (Warning.NumErrors() > 0) {
        Fatal("usage: %s [outfile]", myname);
    }
}

static Connection* FindServer () {
    char* spacename = new char[strlen(aout) + sizeof("/") + strlen(inclink)];
    sprintf(spacename, "%s/%s", aout, inclink);
    SpaceManager* m = new SpaceManager;
    Connection* server = m->Find(spacename);
    delete m;
    delete spacename;
    if (server == nil) {
        Warning("can't find inclink server for outfile '%s'", aout);
    }
    return server;
}

static void KillServer (Connection* server) {
    ChiefDeputy* deputy = new ChiefDeputy(server);
    deputy->Msg(deputy->Tag(), inclink_quit);
    int reply;
    deputy->GetReply(&reply, sizeof(reply));
    while (reply != inclink_ack) {
        GetError(reply, deputy);
        deputy->GetReply(&reply, sizeof(reply));
    }
}
       
static void GetError (int errcode, ChiefDeputy* deputy) {
    const char* savedPrefix = errprefix;
    errprefix = inclink;
    int errlen;
    deputy->GetReply(&errlen, sizeof(errlen));
    char* errmsg = new char[errlen + 1];
    deputy->GetReply(errmsg, errlen);
    errmsg[errlen] = '\0';
    switch(errcode) {
    case inclink_debug:
        Debug(errmsg);
        break;
    case inclink_warning:
        Warning(errmsg);
        break;
    case inclink_error:
        Error(errmsg);
        break;
    case inclink_fatal:
        Warning(errmsg);
        errprefix = savedPrefix;
        Fatal.StartMessage();
            Fatal.Add("both ");
            Fatal.Add(inclink);
            Fatal.Add(" and ");
            Fatal.Add(myname);
            Fatal.Add(" exiting");
        Fatal.EndMessage();
        break;
    case inclink_panic:
        Warning(errmsg);
        errprefix = savedPrefix;
        Panic.StartMessage();
            Panic.Add("both ");
            Panic.Add(inclink);
            Panic.Add(" and ");
            Panic.Add(myname);
            Panic.Add(" exiting");
        Panic.EndMessage();
        break;
    default:
        Panic("unrecognized reply '%d'", errcode);
    }
    delete errmsg;
    errprefix = savedPrefix;
}
