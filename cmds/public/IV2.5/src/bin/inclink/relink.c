/*
 * relink - relink all files in inclink
 *
 * usage:  relink [-qSsXxv] [-o outfile] obj [obj ..] [-llib ...]
 */

#include "error.h"
#include "protocol.h"
#include <InterViews/chief.h>
#include <InterViews/connection.h>
#include <InterViews/space.h>
#include <InterViews/spaceman.h>
#include <bstring.h>
#include <errno.h>
#include <os/proc.h>
#include <os/timing.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

class Main {
public:
    Main();
    ~Main();

    void ScanArgs(int, char**);
    void ChildDied();
    void Run();

private:
    void SendServer (ChiefDeputy*, RequestType, int);
    void StartServerClock(ChiefDeputy*);
    void AddLibrary (char*);
    void AddFile (char*);
    ChiefDeputy* FindServer();
    void ForkServer(char*);
    void Exec(char*);
    void AddCommand(char*);
    void PrepareRelink(ChiefDeputy*);
    void SendFiles(ChiefDeputy*);
    void DoLink(ChiefDeputy*);
    void GetReply(ChiefDeputy*);
    void GetError(int, ChiefDeputy*);
    void EndServerClock(ChiefDeputy*);
    const char* SearchPath(const char*, const char*);

    char* myname;         /* most likely "relink" */
    char* aout;           /* executable output */
    char* inclink;        /* server name */
    char** objFile;       /* array of object files */
    int fileArraySize;    /* size of array */
    int nfiles;           /* number of object files */
    char* command[10];    /* command to exec */
    int cindex;           /* command index */

    boolean dflag;
    boolean fflag;        /* force full link flag */
    boolean kflag;        /* save ctdt flag */
    boolean lflag;        /* imitate ld flag */
    boolean qflag;        /* quiet flag */
    boolean Sflag;        /* strip syms except locals & globals */
    boolean sflag;        /* strip all symbols */
    boolean tinclinkflag; /* use tinclink if true */
    boolean Xflag;        /* save locals except labels */
    boolean xflag;        /* discard local symbols */
    boolean vflag;        /* discard local symbols */
};

Main* program;

/* signal handler */
static const unsigned short ExecFailed = 255;   // max for wait.w_retcode
static void ChildDied () {
    program->ChildDied();
}

Main::Main () {
    myname = "relink";
    aout = "a.out";
    inclink = "inclink";
    objFile = nil;
    fileArraySize = 0;
    nfiles = 0;
    cindex = 0;

    dflag = false;
    fflag = false;
    kflag = false;
    lflag = false;
    qflag = false;
    Sflag = false;
    sflag = false;
    tinclinkflag = false;
    Xflag = false;
    xflag = false;
    vflag = false;
}

Main::~Main () {
}

inline void Main::SendServer (
    ChiefDeputy* deputy, RequestType request, int value = 0
) {
    Debug("sending request '%s', value = %d", requestName[request], value);
    char msg[2];
    sprintf(msg, "%d", value);
    deputy->StringMsg(deputy->Tag(), request, msg);
}

void Main::Run () {
    ChiefDeputy* deputy = FindServer();
    if (deputy != nil) {
        StartServerClock(deputy);
        PrepareRelink(deputy);
        SendFiles(deputy);
        DoLink(deputy);
        EndServerClock(deputy);
    }
}

void Main::ScanArgs (int argc, char* argv[]) {
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
                case '0':
                    lflag = true;
                    break;
                case 'd':
                    dflag = true;
                    Debug.messages = true;
                    break;
                case 'f':
                    fflag = true;
                    break;
                case 'i':
                    if (curarg[++j] != nil) {
                        Warning("space expected between '-i' and linker");
                        j = arglen;
                    }
                    if (++i == argc) {
                        Warning("linker name expected after option '-i'");
                    }
                    inclink = argv[i];
                    break;
                case 'k':
                    kflag = true;
                    break;
                case 'l':
                    if (curarg[++j] == nil) {
                        Warning("library name expected after option '-f'");
                    } else {
                        AddLibrary(&curarg[j]);
                    }
                    j = arglen;
                    break;
                case 'o':
                    j++;
                    if (curarg[j] != 's' && curarg[j] != nil) {
                        Warning(
                            "space expected between '-%s' and outfile",
                             curarg[j-1]
                        );
                        j = arglen;
                    }
                    if (++i == argc) {
                        Warning("outfile expected after option '-o'");
                    }
                    aout = argv[i];
                    break;
                case 'q':
                    qflag = true;
                    break;
                case 'S':
                    Sflag = true;
                    break;
                case 's':
                    sflag = true;
                    break;
                case 't':
                    if (strcmp(&curarg[j], "tinclink") == 0) {
                        tinclinkflag = true;
                        j += strlen("tinclink");
                    } else {
                        Warning("unrecognized option '-t'");
                    }
                    break;
                case 'X':
                    Xflag = true;
                    break;
                case 'x':
                    xflag = true;
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
            "usage: %s [-qSsXxv] [-o outfile] obj [obj ..] [-llib ...]", myname
        );
    }
}

void Main::StartServerClock (ChiefDeputy* deputy) {
    SendServer(deputy, inclink_timer, 1);
    GetReply(deputy);
}

void Main::AddLibrary (char* tail) {
    static const int BuffSize = 256;
    static const char* LibPath = "/lib:/usr/local/lib:/usr/lib";
    static char libName[BuffSize];
    sprintf(libName, "lib%s.a", tail);
    const char* fullpath = SearchPath(libName, LibPath);
    if (fullpath == nil) {
        Warning("library '%s' not found", libName);
    } else {
        char* filename = new char[strlen((char*)fullpath) + 1];

        strcpy(filename, fullpath);
        AddFile(filename);
    }
}

void Main::AddFile (char* filename) {
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

ChiefDeputy* Main::FindServer () {
    char* spacename = new char[strlen(aout) + sizeof("/") + strlen(inclink)];
    sprintf(spacename, "%s/%s", aout, inclink);
    SpaceManager* m = new SpaceManager;
    Connection* inclink = m->Find(spacename);
    if (inclink == nil) {
        if (!qflag) {
            printf("%s: starting inclink server\n", myname);
        }
        ForkServer(spacename);
        inclink = m->Find(spacename, true);
        fflag = true;
    }
    ChiefDeputy* deputy = new ChiefDeputy(inclink);
    return deputy;
}

void Main::ForkServer (char* spacename) {
    signal(SIGCHLD, ::ChildDied);
    signal(SIGHUP, SignalIgnore);
    int pid = fork();
    switch (pid) {
    case -1:
        perror(myname);
        Panic("can't fork");
    case 0:
        // in child
        Exec(spacename);
    }
    // in parent, returns after execv();
}

void Main::AddCommand (char* cmd) {
    Debug("AddCommand(\"%s\")", cmd);
    command[cindex] = cmd;
    cindex++;
}

void Main::Exec (char* spacename) {
    const char* fullpath = SearchPath(inclink, getenv("PATH"));
    if (fullpath == nil) {
        Fatal("cannot find '%s'", inclink);
    }

    AddCommand(inclink);
    if (dflag) {
        AddCommand("-d");
    }
    AddCommand("-o");
    AddCommand(aout);
    AddCommand(spacename);
    AddCommand(nil);

    execv(fullpath, command);

    // only gets here if execv() fails
    exit(ExecFailed);
}

void Main::ChildDied () {
    union WaitStatus status;
    int pid = wait(&status.w_status);
    if (status.w_T.w_Retcode == ExecFailed) {
        Panic("can't exec '%s'", inclink);
    } else {
        Panic("unexpected termination of '%s'", inclink);
    }
}

void Main::PrepareRelink (ChiefDeputy* deputy) {
    SendServer(deputy, inclink_dflag, dflag);
    SendServer(deputy, inclink_kflag, kflag);
    SendServer(deputy, inclink_ldflag, lflag);
    SendServer(deputy, inclink_xflag, xflag); 
    SendServer(deputy, inclink_Xflag, Xflag);
    SendServer(deputy, inclink_sflag, sflag);
    SendServer(deputy, inclink_Sflag, Sflag);
    SendServer(deputy, inclink_tinclink, tinclinkflag);
    SendServer(deputy, inclink_fulllink, fflag);
}

void Main::SendFiles (ChiefDeputy* deputy) {
    for (int i = 0; i < nfiles; ++i) {
        char* file = objFile[i];
        if (vflag) {
            printf("%s: sending file '%s'\n", file);
        }
        deputy->StringMsg(deputy->Tag(), inclink_linkfile, file);
        GetReply(deputy);
    }
}

void Main::DoLink (ChiefDeputy* deputy) {
    SendServer(deputy, inclink_dolink);
    GetReply(deputy);
    if (vflag) {
        fprintf(stdout, "relink completed\n");
    }
}

void Main::GetReply (ChiefDeputy* deputy) {
    int reply;
    deputy->GetReply(&reply, sizeof(reply));
    while (reply != inclink_ack) {
        GetError(reply, deputy);
        deputy->GetReply(&reply, sizeof(reply));
    }
}

void Main::GetError (int errcode, ChiefDeputy* deputy) {
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

void Main::EndServerClock (ChiefDeputy* deputy) {
    SendServer(deputy, inclink_timer, 0);
    GetReply(deputy);
}

// search colon separated directory path for name, return nil or path/name.
const char* Main::SearchPath (const char* name, const char* path) {
    static const int BuffSize = 1024;
    static char pathBuff[1024];
    const char *start = path;
    const char *p = start;
    boolean found = false;
    boolean done = false;
    while (!done) {
        done = (*p == '\0' || *p == '\n' || p-&path[0] >= BuffSize);
        if (*p == ':' || *p == '\n' || *p == nil) {
            if (start != p) {
                int dirlen = p-start;
                strncpy(pathBuff, start, dirlen);
                if (*p != '/') {
                    pathBuff[dirlen] = '/';
                    dirlen++;
                }
                strcpy(&pathBuff[dirlen], name);
                FILE* f = fopen(pathBuff, "r");
                if (f != nil) {
                    found = true;
                    fclose(f);
                    break;
                } else {
                    start = p + 1;
                }
            }
        }
        p++;
    }
    if (!found) {
        return nil;
    } else {
        return &pathBuff[0];
    }
}

main (int argc, char* argv[]) {
    program = new Main();
    program->ScanArgs(argc, argv);
    program->Run();
    exit(Error.NumErrors());
}
