/*
 * inclink - incremental linking server
 *
 * usage:  inclink [-d] [-o outfile]
 */

#include "errhandler.h"
#include "hash.h"
#include "inclink.h"
#include "program.h"
#include "strtable.h"
#include "timer.h"
#include <InterViews/spaceman.h>
#include <osfcn.h>
#include <stdio.h>
#include <string.h>

Program*   currProg;
FILE*      dumpFile;
boolean    k_flag = false;
Inclink*   server = nil;
StrTable*  strTab;
HashTable* symTab;
Timer*     timer;
HashTable* units;

static char* myname;
static char* aout = "a.out";
static char* inclink = "inclink";
static char* spacename = nil;
static int nerrors = 0;

static void ScanArgs (int, char**);
static void StartServer();
static void Init(char*);

main (int argc, char* argv[]) {
    ScanArgs(argc, argv);
    StartServer();
    for (;;) {
        server->Dispatch();
    }
}

static void ScanArgs (int argc, char* argv[]) {
    myname = argv[0];
    errprefix = myname;
    for (int i = 1; i < argc; ++i) {
        char* curarg = argv[i];
        if (curarg[0] == '-') {
            int arglen= strlen(curarg);
            if (arglen == 1) {
                fprintf(stderr, "unrecognized option '-'");
                nerrors++;
            }
            for (int j = 1; j < arglen; ++j) {
                switch (curarg[j]) {
                case 'd':
                    Debug.messages = true;
                    break;
                case 'i':
                    if (curarg[++j] != nil) {
                        fprintf(
                            stderr, "space expected between '-i' and linker"
                        );
                        nerrors++;
                        j = arglen;
                    }
                    if (++i == argc) {
                        fprintf(
                            stderr, "linker name expected after option '-i'"
                        );
                        nerrors++;
                    }
                    inclink = argv[i];
                    break;
                case 'o':
                    if (curarg[++j] != nil) {
                        fprintf(
                            stderr, "space expected between '-o' and outfile"
                        );
                        nerrors++;
                        j = arglen;
                    }
                    if (++i == argc) {
                        fprintf(stderr, "outfile expected after option '-o'");
                        nerrors++;
                    }
                    aout = argv[i];
                    break;
                default:
                    fprintf(stderr, "unrecognized option '-%c'", curarg[j]);
                    nerrors++;
                }
            }
        } else {
            if (spacename) {
                fprintf(stderr, "ignoring extra argument '%s'", argv[i]);
                nerrors++;
            } else {
                spacename = argv[i];
            }
        }
    }
    if (nerrors) {
        fprintf(stderr, "usage: %s [-v] [-o outfile]", myname);
        exit(nerrors);
    }
}

#include <sys/time.h>
#include <sys/resource.h>

static void AllowCoreDump () {
    const int NewLimit = 10000000;
    struct rlimit rlp;
    getrlimit(RLIMIT_CORE, &rlp);
    if (NewLimit > rlp.rlim_cur) {
        rlp.rlim_cur = NewLimit;
        setrlimit(RLIMIT_CORE, &rlp);
    }
}

static void StartServer () {
    if (spacename == nil) {
        char* spacename =
            new char[strlen(aout) + sizeof("/") + strlen(inclink)];
        sprintf(spacename, "%s/%s", aout, inclink);
    }
    AllowCoreDump();
    Init(spacename);
}

static void Init (char* spacename) {
    dumpFile = stdout;
    server = new Inclink(spacename);
    strTab = new StrTable();
    symTab = new HashTable(DEFAULTTABLESIZE, strTab);
    units = new HashTable(128, new StrTable());
    currProg = new Program(aout);
    timer = new Timer("inclink", true);
}
