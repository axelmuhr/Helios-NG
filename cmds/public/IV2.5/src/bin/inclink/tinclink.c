/*
 * tinclink - Modify files in (tinker with) inclink
 *
 * usage:  tinclink [-v] [-o outfile] obj [obj ..]
 */

#include "defs.h"
#include "error.h"
#include "protocol.h"
#include <InterViews/chief.h>
#include <InterViews/connection.h>
#include <InterViews/space.h>
#include <InterViews/spaceman.h>
#include <bstring.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

class Tinclink {
public:
    Tinclink();
    ~Tinclink();

    void ScanArgs (int, char**);
    void Run();

private:
    void CheckArg(char*, boolean);
    void AddFile(char*);
    ChiefDeputy* FindServer();
    void DoOptions(ChiefDeputy*);
    void ModifyFiles(ChiefDeputy*);
    void GetError(int, ChiefDeputy*);
    void StartClock(ChiefDeputy*);
    void EndClock(ChiefDeputy*);

    char* myname;         // argv[0]
    char* aout;           // target name
    char* inclink;        // name of inclink server
    boolean dflag;        // debug flag
    boolean vflag;        // verbose flag
    char** objFile;       // array of object file names
    int fileArraySize;    // size of array
    int nfiles;           // number of object files to modify
};

Tinclink::Tinclink () {
    myname = "tinclink";
    aout = "a.out";
    inclink = "inclink";
    dflag = false;
    vflag = false;
    objFile = nil;
    fileArraySize = 0;
    nfiles = 0;
}

Tinclink::~Tinclink () {
    delete objFile;
}

void Tinclink::ScanArgs (int argc, char* argv[]) {
    myname = argv[0];
    errprefix = myname;
    for (int i = 1; i < argc; ++i) {
        char* curarg = argv[i];
        if (curarg[0] == '-') {
            int arglen= strlen(curarg);
            if (arglen == 1) {
                Warning("unrecognized option '-'");
            }
            for (int j = 1; j < arglen; ++j) {
                switch (curarg[j]) {
                case 'd':
                    dflag = true;
                    break;
                case 'o':
                    j++;
                    i++;
                    CheckArg(&curarg[j], i < argc);
                    j = arglen;
                    aout = argv[i];
                    break;
                case 'v':
                    vflag = true;
                    break;
                default:
                    Warning("unrecognized option '-%c'", curarg[j]);
                }
            }
        } else {
            AddFile(argv[i]);
        }
    }
    if (nfiles == 0) {
        Warning("no object files found");
    }
    if (Warning.NumErrors() > 0) {
        Fatal(
            "usage: %s [-v] [-o outfile] [-i inclinkdir]* obj [obj ...]",
            myname
        );
    }
}

void Tinclink::Run () {
    ChiefDeputy* chief = FindServer();
    if (chief != nil) {
        StartClock(chief);
        DoOptions(chief);
        ModifyFiles(chief);
	EndClock(chief);
    }
    exit(Error.NumErrors());
}

void Tinclink::CheckArg (char* endChar, boolean argExists) {
    if (*endChar != nil) {
        Warning(
            "space expected between '-%s' and next argument", --endChar
        );
    }
    if (argExists == false) {
        Warning("argument expected after '-%c'", *endChar);
    }
}

void Tinclink::AddFile (char* filename) {
    if (nfiles == fileArraySize) {
        char** oldArray;
        if (fileArraySize == 0) {
            fileArraySize = 31;
            oldArray = 0;
        } else {
            fileArraySize += fileArraySize;
            oldArray = objFile;
        }
        objFile = new char*[fileArraySize];
        bcopy(oldArray, objFile, nfiles * sizeof(char*));
        delete oldArray;
    }
    objFile[nfiles++] = filename;
}

ChiefDeputy* Tinclink::FindServer () {
    char* spacename = new char[strlen(aout) + sizeof("/") + strlen(inclink)];
    sprintf(spacename, "%s/%s", aout, inclink);
    SpaceManager* m = new SpaceManager;
    Connection* server = m->Find(spacename);
    delete m;
    delete spacename;
    ChiefDeputy* chief;
    if (server == nil) {
        if (vflag) {
            Warning("can't find inclink server for outfile '%s'", aout);
        }
        chief = nil;
    } else {
        chief = new ChiefDeputy(server);
    }
    return chief;
}

void Tinclink::DoOptions (ChiefDeputy* deputy) {
    char msg[2];
    sprintf(msg, "%d", dflag);
    deputy->StringMsg(deputy->Tag(), inclink_dflag, msg);
}

void Tinclink::ModifyFiles (ChiefDeputy* deputy) {
    for (int i = 0; i < nfiles; ++i) {
        deputy->StringMsg(deputy->Tag(), inclink_modfile, objFile[i]);
        int reply;
        deputy->GetReply(&reply, sizeof(reply));
        while (reply != inclink_ack) {
            GetError(reply, deputy);
            deputy->GetReply(&reply, sizeof(reply));
        }
    }
}

void Tinclink::GetError (int errcode, ChiefDeputy* deputy) {
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


void Tinclink::StartClock (ChiefDeputy* server) {
    char msg[2];
    sprintf(msg, "%d", 1);
    server->StringMsg(server->Tag(), inclink_timer, msg);
    int reply;
    server->GetReply(&reply, sizeof(reply));
}

void Tinclink::EndClock (ChiefDeputy* server) {
    char msg[2];
    sprintf(msg, "%d", 0);
    server->StringMsg(server->Tag(), inclink_timer, msg);
    int reply;
    server->GetReply(&reply, sizeof(reply));
}

/*
 *  Program entry point
 */

main (int argc, char* argv[]) {
    Tinclink* tinclink = new Tinclink();
    tinclink->ScanArgs(argc, argv);
    tinclink->Run();
    delete tinclink;
}
