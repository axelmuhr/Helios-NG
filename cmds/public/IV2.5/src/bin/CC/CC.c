/* 
 * C++ compiler driver
 */
				 
static char rcsid[] = "$Header: CC.c,v 1.10 89/05/29 01:52:09 interran Exp $";

/* exit status codes
 * 2	too many errors
 * 1	internal error
 * 0	all went well
 * -1	nothing done
 */

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/file.h>

#include "stdconfig.h"

#define nil 0
#define EOL '\0'

typedef char *String;
typedef char **StringVec;
typedef int FileDesc;
typedef enum {false, true} boolean;

#define BADEXEC -1

#define IsChild(pid) ((pid)==0)

#define stdinDesc	0
#define stdoutDesc	1

static String PREPROCESSOR;
static String CCfrontEnd;
static String Ccompiler;
static String ASSEMBLER;
static String LINKER;
static String MUNCHER;
static String FILTER;
static String SEDFILTER;
static String MV;
static String COPY;
static String StdCClib;
static String frontEndSuffix;
static String runtimeLib;
static String cclib;

static String DEFAULTPATHFILE = ".CCpath";
static String ENVPATH = "CCPATH=";
static String DEFAULTLINKER = CC_ld;
static String INCLINK = "inclink";
static String RELINK = "relink";

#define ARGSIZE		2048
#define FILENAMESIZE	256
#define MAXARGS		1024
#define MAXTMPFILES	64
#define PATHLEN		1024
#define SET		true

boolean Cflag = false;
boolean Eflag = false;
boolean Fflag = false;
boolean FcFlag = false;
boolean plusIflag = false;
boolean suffixFlag = false;
boolean verboseFlag = false;
boolean fakeFlag = false;
boolean objFlag = false;
boolean ignErrFlag = false;
boolean skipAsmFlag = false;
boolean cFlag = false;
boolean noMunchFlag = false;
boolean gFlag = false;
boolean gprofFlag = false;
boolean profFlag = false;
boolean genRelocInfoFlag = false;
boolean keepctdtFlag = false;

boolean translator;	/* true if compiler generates C code */
boolean ccIsLinker;	/* true if using cc for link phase */

String ccArgs[MAXARGS];
StringVec ccArgv = &ccArgs[0];

String cppArgs[MAXARGS];
StringVec cppArgv = cppArgs;

String frontEndArgs[MAXARGS];
StringVec frontEndArgv = frontEndArgs;

String linkArgs[MAXARGS];
StringVec linkArgv = linkArgs;

String linkFlagList[MAXARGS];
StringVec linkFlags = linkFlagList;

String cppPathArgs[MAXARGS];
StringVec cppPathv = cppPathArgs;

String commandv[MAXARGS];
StringVec command;

String objSpaceArgs[MAXARGS];
StringVec objSpaceArgv = &objSpaceArgs[0];

char tmpFilesList[MAXTMPFILES][FILENAMESIZE];
char xBuff[FILENAMESIZE];		/* for +x file flag */
int tmpFileCount = 0;

extern StringVec environ;

char libDirBuff[128] = "-I/usr/include/CC";
static String libDir = libDirBuff;

char fileName[FILENAMESIZE], filterName[FILENAMESIZE];
char cppName[FILENAMESIZE], frontEndName[FILENAMESIZE];
char destname[FILENAMESIZE];
char objName[FILENAMESIZE];
char ccPath[PATHLEN];
String myname, linkFile;
String suffix;

int cppDesc, dest;
int devnull;			/* file desc for /dev/null */
int numErrors = 0;

void Error (msg1, msg2)
String msg1, msg2;
{
    if (msg2 == nil || *msg2 == EOL) {
	fprintf(stderr, "%s : %s\n", myname, msg1);
    } else {
	fprintf(stderr, "%s : %s %s\n", myname, msg1, msg2);
    }
    exit(1);
}

void WarningMsg (msg1, msg2)
String msg1, msg2;
{
    if (msg2 == nil || *msg2 == EOL) {
	fprintf(stderr, "%s : %s\n", myname, msg1);
    } else {
	fprintf(stderr, "%s : %s %s\n", myname, msg1, msg2);
    }
}

void Remove (file)
String file; 
{
    if (verboseFlag) {
	fprintf(stderr, "rm %s\n", file);
    }
    if (!fakeFlag && file != nil && *file != EOL) {
	unlink(file);
    }
}

void RemoveTmpFiles ()
{
    int i;
    for (i=0; i<tmpFileCount; i++) {
	Remove(tmpFilesList[i]);
	tmpFilesList[i][0] = EOL;
    }
}

void Quit (exitStatus)
int exitStatus;
{
    RemoveTmpFiles();
    exit(exitStatus);
}

void myclose (desc)
FileDesc desc;
{
    if (!fakeFlag && close(desc) != 0) {
	perror("oh darnit.");
	fprintf(stderr, "Close failed: %d\n", desc);
    }
}

FileDesc myopen (filename, flags, mode)
String filename;
int flags, mode;
{
    int f;
    if (!fakeFlag) {
	f = open(filename, flags, mode);
	if (f < 0) {
	    fprintf(stderr, "(%s)  ", filename);
	    perror("CC : Open failed - internal error");
	    Quit(1);
	}
    } else
	f = -1;
    return f;
}

StringVec AddArg (l, s)
StringVec l;
String s;
{
    if (s != 0 && *s != EOL) {
	*l++ = s;
    }
    return l;
}

StringVec AddList (v, list)
StringVec v;
StringVec list;
{
    if (list == 0 || *list == EOL) {
/* 	fprintf(stderr, "%s: AddList: null string\n", myname); */
    } else {
	while (*list != 0) {
	    *v++ = *list++;
	}
    }
    return v;
}

String newString (s)
String s;
{
     String p;
     p = (char*) malloc(FILENAMESIZE);
     strcpy(p, s);
     return p;
}

void RegisterTmpFile (filename)
String filename;
{
    int i;

    if (tmpFileCount >= MAXTMPFILES) {
	for (i = 0; i<MAXTMPFILES ; i++) {
	    unlink(tmpFilesList[i]);
	    tmpFilesList[i][0] = EOL;
	}
	tmpFileCount = 0;
    }
    strcpy(tmpFilesList[tmpFileCount], filename);
    tmpFileCount++;
}

FileDesc MakeTmpFile (tmpname, characteristic)
String tmpname;
String characteristic;
{
    int t;
    sprintf(tmpname, "/tmp/C++%d%s", getpid(), characteristic);
    if (!fakeFlag) {
	t = open(tmpname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (t < 1) {
	    perror("Couldn't open /tmp file.  Shucks.");
	} else {
	    RegisterTmpFile(tmpname);
	}
    } else {
	t = stdoutDesc;
    }
    return t;
}

void PrintArgList (prog, argv)
String prog;
StringVec argv;
{
    int i;
    
    fprintf(stderr, "%s: ", prog);
    for (i=0; argv[i] != 0; i++) {
	fprintf(stderr, "%s ", argv[i]);
    }
    fprintf(stderr,"\n");
}

void initexec (progname)
String progname;
{
    command = commandv;
    *command++ = progname;
}

void RedirectIn (in)
FileDesc in;
{
    if (in != stdinDesc) {
	dup2(in, stdinDesc);
	close(in);
    }
}

void RedirectOut (out)
FileDesc out;
{
    if (out != stdoutDesc) {
	dup2(out, stdoutDesc);
	close(out);
    }
}

/* return pid of child */
int NoWaitExec (name, argv, in, out)
String name;
StringVec argv;
FileDesc in;
FileDesc out;
{
    int pid;
    
    pid=fork();
    if IsChild(pid) {
	RedirectIn(in);
	RedirectOut(out);
	execvp(name, argv);
	fprintf(stderr, "%s: can't execute %s\n", myname, name);
	_exit(BADEXEC);
    }
    return pid;
}

int mywait (pid)
int pid;
{
    int status;
    
    if (pid != -1) {
	while ( wait(&status) != pid ) {
	    ;
	}
	if (status != 0) {
	    if ((status & 0xff)== 0) {
		status = status >> 8;
	    }
	}
    } else {
	status = -1;
    }
    return status;
}

int myexec(name, argv, in, out)
String name;
String argv[];
FileDesc in;
FileDesc out;
{
    int pid, status;

    pid = NoWaitExec(name, argv, in, out);
    status = mywait(pid);
    return status;
}

/* 
 * if 0 is passed as in or out, use the defaults, stdin and stdout. 
 */
int execute (progfile, argv, in, out) 
String progfile;
StringVec argv;
FileDesc in, out;
{
    int status;

    *command++ = nil;		/* terminate arg list HACK !! */
    if (verboseFlag) {
	PrintArgList(progfile, commandv);
    }
    status = 0;
    if (!fakeFlag) {
	status = myexec(progfile, commandv, in, out);
	if (status != 0) {
	    numErrors++;
	}
	if (verboseFlag) {
	    fprintf(stderr,"** $status=%d **	%s exited, (%s)\n",
		status, progfile, myname);
	}
    }

    return status;
}

void NoMoreOpt (opt, arg)
char opt;
String arg;
{
    if (opt != EOL) {
	fprintf(stderr,
	    "%s: %c not understood in option %s\n", myname, opt, arg
	);
	Quit(1);
    }
}

typedef enum 
    {INVALID, CFILE, CPLUSPLUSFILE, OBJECTFILE, ASSEMBLERFILE, LIBRARYFILE}
    FileType;

struct FileInfo {
    char suffix;
    FileType ft;
};

struct FileInfo fileTypeInfo[] = {
    {'c', CPLUSPLUSFILE},
    {'C', CPLUSPLUSFILE},
    {'i', CFILE},
    {'o', OBJECTFILE},
    {'s', ASSEMBLERFILE},
    {'a', LIBRARYFILE},
    {EOL, INVALID}
};

/* 
 * dir, tail, ext are VAR parameters
 */
char DetermineFileType (filename, dir, tail, ext)
String filename;
String *dir;
String *tail;
String *ext;		/* var parameters */
{
    String p;
    char kind;
    struct FileInfo *fi;

    *dir = nil;				/* defaults */
    *ext = nil;
    *tail = fileName;
    
    p = strlen(filename) + filename;
    while (p != fileName && *tail == fileName) {
	if (*p == '.' && *ext == nil) {
	    *ext = p;
	} else if (*p == '/' && *tail == fileName) {
	    *tail = p + 1;
	    *p = EOL;
	    *dir = fileName;
	}
	p--;
    }

    				/* determine kind of file it is */
    if ( *ext == nil ) {
	kind = 'a'; 		/* assume lib file */
    } else {
	kind = *(*ext + 1);
	if ( kind == 'c' || kind == 'a' || kind == 'o' || kind == 's' 
	  || kind == 'C') {
	    if (*(*ext+2) != EOL) {
		kind = '?';
	    }
	} else {
	    kind = '?';
	}
    }

    return kind;
}

void SetUpHandler (sig, handler)
int sig;
void (*handler)();
{
    void (*signalHandler)();		/* pointer to integer function */

    signalHandler = signal(sig, handler);
    if (signalHandler == SIG_IGN) {
	(void) signal(sig, SIG_IGN);
    } else if ((int)signalHandler == -1) {
	perror("Couldn't set up signal handler");
    }
}

void interrupt ()
{
    RemoveTmpFiles();
    exit(3);
}

void PrintVersionAndPath ()
{
    static boolean printed = false;
    if (!printed) {
	printf("CCPATH = %s\n", ccPath);
	printed = true;			/* only print this once */
    }
}

void HandleArgs (argc, argv)
int argc;
String *argv;
{
    register int i;
    for (i = 1; i < argc; i++) {
				/* handle options of the form '-XXX' */
	if (argv[i][0]==EOL) {
	    			/* skip null argument */
	} else if (argv[i][0]=='-') {
	    switch (argv[i][1]) {
		case 'c':
		    cFlag = SET;
		    NoMoreOpt(argv[i][2], argv[i]);
		    break;
		case 'C':
		    NoMoreOpt(argv[i][2], argv[i]);
		    frontEndArgv = AddArg(frontEndArgv, argv[i]);
		    break;
		case 'd':
		    NoMoreOpt(argv[i][2], argv[i]);
		    ccArgv = AddArg(ccArgv, argv[i]);
		    break;
		case 'E':
		    Eflag = SET;
		    NoMoreOpt(argv[i][2], argv[i]);
		    break;
		case 'F':
		    Fflag = SET;
		    if (argv[i][2] == 'c') {
			NoMoreOpt(argv[i][3], argv[i]);
                        FcFlag = SET;
		    } else {
			NoMoreOpt(argv[i][2], argv[i]);
		    } /* if */
		    break;
		case 'i':		/* added this */
		    ignErrFlag = SET;
		    break;
		case 'k':
		    keepctdtFlag = SET;
		    break;
		case 'l':
		    linkArgv = AddArg(linkArgv, argv[i]);
		    break;
		case 'N':		/* added this */
		    if (strcmp(argv[i],"-NOMUNCH")==0) {
			noMunchFlag = SET;
		    } else {
			ccArgv = AddArg(ccArgv, argv[i]);
		    }
		    break;
		case 'n':		/* added this */
		    NoMoreOpt(argv[i][2], argv[i]);
		    verboseFlag = SET;
		    fakeFlag = SET;
		    PrintVersionAndPath();
		    break;
                case 'o':
                    NoMoreOpt(argv[i][2], argv[i]);
                    i++;
                    if (argv[i] == EOL) {
                        WarningMsg(
                            "No object specified after -o, using a.out", nil);
                    } else {
                        objFlag = SET;
                        linkArgv = AddArg(linkArgv, argv[i-1]);
                        linkArgv = AddArg(linkArgv, argv[i]);
                        strcpy(objName, argv[i]);
                    }
                    break;
		case 'p':
                    if (argv[i][2] == 'g') {            /* -pg */
                        NoMoreOpt(argv[i][3], argv[i]);
                        gprofFlag = SET;
                        profFlag = false;
                    } else {                            /* -p */
                        NoMoreOpt(argv[i][2], argv[i]);
                        profFlag = SET;
                        gprofFlag = false;
                    }
		    ccArgv = AddArg(ccArgv, argv[i]);
		    break;
		case 'r':
		    genRelocInfoFlag = SET;
		    NoMoreOpt(argv[i][2], argv[i]);
		    ccArgv = AddArg(ccArgv, argv[i]);
		    break;
		case 'S':
		    skipAsmFlag = SET;
		    NoMoreOpt(argv[i][2], argv[i]);
		    ccArgv = AddArg(ccArgv, argv[i]);
		    break;
		case 'v':		/* added this */
		    verboseFlag = SET;
		    NoMoreOpt(argv[i][2], argv[i]);
		    PrintVersionAndPath();
		    break;
		case 'D':
		case 'I':
		case 'U':
		    cppArgv = AddArg(cppArgv, argv[i]);
		    break;
		case '.':
		    if (suffixFlag) {
			Error("not allowed to specify more than suffix", nil);
		    } /* if */
		    suffixFlag = SET;
		    suffix = &argv[i][1];
		    break;
		default:
		    ccArgv = AddArg(ccArgv, argv[i]);
		    break;
	    }	/* switch (argv[i][1]) */

	} else if (argv[i][0] == '+') {
				/* handle options of the form '+XXX' */
	    switch (argv[i][1]) {
		case 'd':
		    frontEndArgv = AddArg(frontEndArgv, argv[i]);
		    break;
		case 'e':
		    frontEndArgv = AddArg(frontEndArgv, argv[i]);
		    break;
		case 'V':
		    NoMoreOpt(argv[i][2], argv[i]);
		    strcpy(libDir, " -I/usr/include");
		    break;
		case 'L':
		    NoMoreOpt(argv[i][2], argv[i]);
		    break;
		case 'x':
		    if (argv[i][2] != '\0') {
			frontEndArgv = AddArg(frontEndArgv, argv[i]);
		    } else {
			i++;
			if (argv[i] == EOL) {
			    Error("No arg specified after +x", nil);
			} else {
			    strcpy(xBuff, "+x");
			    strcat(xBuff, argv[i]);
			    frontEndArgv = AddArg(frontEndArgv, xBuff);
			}
		    }
		    break;
		case 'S':
		    NoMoreOpt(argv[i][2], argv[i]);
		    frontEndArgv = AddArg(frontEndArgv, argv[i]);
		    break;
		case 'I':
		    plusIflag = SET;
		    break;
		default:
		    WarningMsg("Unknown flag", argv[i]);
		    break;
	    }
	} else {
	    if (processFile(argv[i]) != 0) {
		Quit(1);
	    }
	}
    }
}

String SetDefaultFromEnv();
void GetCCPath();

void Init (envp)
StringVec envp;
{
    PREPROCESSOR = SetDefaultFromEnv(envp, "ccpC=", CC_cpp);
    CCfrontEnd = SetDefaultFromEnv(envp, "cfrontC=", CC_compiler);
    frontEndSuffix = SetDefaultFromEnv(envp, "suffixC=", CC_suffix);
    StdCClib = SetDefaultFromEnv(envp, "LIBRARY=", CC_library);
    runtimeLib = SetDefaultFromEnv(envp, "runTimeLibC", CC_rt);
    translator = (boolean)(strcmp(frontEndSuffix, "..c") == 0);
    Ccompiler = SetDefaultFromEnv(envp, "ccC=", CC_cc);
    ASSEMBLER = SetDefaultFromEnv(envp, "assemblerC=", CC_as);
    LINKER = SetDefaultFromEnv(envp, "linkerC=", DEFAULTLINKER);
    MUNCHER = SetDefaultFromEnv(envp, "munchC=", CC_munch);
    FILTER = "/bin/grep";
    SEDFILTER	= "/bin/sed";
    MV = "/bin/mv";
    COPY = "/bin/cp";
    cclib = "-lc";
    GetCCPath(environ);
}

int main (argc, argv, envp)
int argc;
StringVec argv, envp;
{
    int i;
	
    myname = argv[0];
    if (argc <= 1) {
	fprintf(stderr,"usage: %s [options] [ file.{c,o,s} ] \n", myname);
	exit(-1);
    }
    SetUpHandler(SIGINT, interrupt);
    SetUpHandler(SIGQUIT, interrupt);
    SetUpHandler(SIGHUP, interrupt);
    devnull = open("/dev/null", O_WRONLY, 666);
    if (devnull == -1) {
	devnull = stdoutDesc;
    }
    Init(envp);
    HandleArgs(argc, argv);
    if (!objFlag) {
	strcpy(objName, "a.out");
    }
    if (numErrors != 0) {
	Quit(2);
    }
    if (Eflag || Fflag || FcFlag || skipAsmFlag) {
        Quit(0);
    }
    if (cFlag) {
        Quit(0);
    }
    Quit(LinkPhase());
}

#define INCLUDEFLAG "-I"
#define INCLUDEFLAGLEN 2

void AddIncludes (line)
String line;
{
    String a, b, t;
    boolean done;
    
    if (line == nil) {
	return;
    }
    a = &line[0];
    b = a;
    done = false;
    while (!done) {
	done = (boolean) (*b == EOL || *b == '\n' || (b-&line[0]) >= ARGSIZE);
	if (*b == ':' || *b == '\n' || *b == EOL) {
	    *b = EOL;
	    t = (char *) malloc(b-a + 1 + INCLUDEFLAGLEN);  /* +1=EOL +2=-I */
	    strcpy(t, INCLUDEFLAG);
	    strcat(t, a);
	    cppPathv = AddArg(cppPathv, t);
	    *b = ':';			/* restore so can print ccPath */
	    a = b+1;
	}
	b++;
    }
}

String ReadEnv (env, pathVar)
StringVec env;
String pathVar;
{
    int i, len;
    
    len = strlen(pathVar);
    for (i = 0; env[i] != nil; i++) {
	if (strncmp(env[i], pathVar, len) == 0) {
	    return &env[i][len];
	}
    }
    return nil;
}

String SetDefaultFromEnv (envp, envName, defaultVal)
StringVec envp;
String envName;
String defaultVal;
{
    String tmp;

    tmp = ReadEnv(envp, envName);
    if (tmp == nil) {
	tmp = defaultVal;
    }
    return tmp;
}

void GetCCPath (env)
StringVec env;
{
    int i, l;
    FILE *pathFile;
    String tmp;
    
    l = strlen(ENVPATH);
    pathFile = fopen(DEFAULTPATHFILE, "r");
    if (pathFile != nil) {
	if (ccPath == fgets(ccPath, PATHLEN, pathFile)) {
	    AddIncludes(ccPath);
	}
    } else {
	tmp = ReadEnv(env, ENVPATH);
	if (tmp != nil) {
	    strcpy(ccPath, tmp);
	    AddIncludes(ccPath);
	}
    }
}

int Move (filename, oldsuffix, newsuffix)
String filename, oldsuffix, newsuffix;
{
    char aBuffer[FILENAMESIZE], aBuffer2[FILENAMESIZE];

    initexec("mv");
    strcpy(aBuffer, filename);
    strcat(aBuffer, oldsuffix);
    command = AddArg(command, aBuffer);
    strcpy(aBuffer2, filename);
    strcat(aBuffer2, newsuffix);
    command = AddArg(command, aBuffer2);
    return execute("/bin/mv", commandv, stdinDesc, stdoutDesc);
}

/* 
 * process a given file.
 * either cc, as or just link it. 
 */
int processFile (fullPathName)
String fullPathName;
{
    String ext, dir, fileTail;
    char fileType;
    int status;

    linkFile = nil;
    strcpy(fileName, fullPathName);
    fileType = DetermineFileType(fileName, &dir, &fileTail, &ext);
    if (fileType == 'c' || fileType == 's' || 
      fileType == 'C' || fileType == 'i') {
	*(ext + 1) = 'o';		/* change xxx.c to xxx.o */
	linkFile = newString(fileTail);
	if (fileType == 'c' && !cFlag) {
	    RegisterTmpFile(linkFile);
	}
    } else if (fileType == 'o') {
	linkFile = newString(fullPathName);
    } else if (fileType == 'a' || fileType == 'i' || fileType == '?') {
	linkFile = newString(fullPathName);
    }
    linkArgv = AddArg(linkArgv, linkFile);
    if (ext != nil) {
	*ext = EOL;			/* truncate so fileTail has no ext */
    }
    if (fileType == 'o' || fileType == 'a' || fileType == '?') {
	return 0;
    }
    if (fileType == 'c' || fileType == 'C' || fileType == 'i') {
	status = CPPphase(fullPathName, fileTail);
	if (Eflag || status != 0) {
	    return status;
	}
	cppDesc = myopen(cppName, O_RDONLY, 0644);
	status = FrontEndPhase(fullPathName, fileTail, cppDesc);
	if (status != 0)
	    return status;
	if (plusIflag && (!Fflag || suffixFlag)) {
	    status = FilterPhase(frontEndName, fileTail);
	    if (status != 0) {
		return status;
	    }
	}
	if (Fflag) {
	    return status;
	}
	status = ccPhase();
	if (status != 0) {
	    return status;
	}
	if (!plusIflag) {
	    Remove(frontEndName);
	}
	if (translator) {
	    if (skipAsmFlag) {
		return Move(fileTail, "..s", ".s");
	    } else {
		return Move(fileTail, "..o", ".o");
	    }
	}
    }

    if (fileType == 's' && !skipAsmFlag) {
	return asmPhase(fullPathName, fileTail);
    }
    return 0;
}

int LinkPhase () {
    int status;

    if (strcmp(LINKER, INCLINK) == 0) {
	ccIsLinker = false;
        status = Inclink();
    } else {
	ccIsLinker = true;
        status = DefaultLink();
    }
    return status;
}

int Inclink () {
    TransformCCArgs();
    return PassOne(RELINK);
}

/*
 *  The following tranformations are done by '/bin/cc'.
 *  We have to do them ourselves.
 *
 *      '-g'  ==> '-lg' at end of args
 *      '-pg' ==> '/lib/gcrt0.o'
 *      '-p'  ==> '/lib/mcrt0.o'
 */
int TransformCCArgs () {
    register int i, j = 0;
    boolean copy;

    for (i = 0; i < MAXARGS; i++) {
        if (ccArgs[i] == 0) {
            break;
        }
        copy = true;
        if (ccArgs[i][0] == '-') {
            switch (ccArgs[i][1]) {
            case 'g':
                gFlag = true;
                copy = false;
                break;
            case 'p':
                if (ccArgs[i][2] == 'g') {
                    runtimeLib = "/lib/gcrt0.o";
                } else {
                    runtimeLib = "/lib/mcrt0.o";
                }
                copy = false;
                break;
            }
        }
        if (copy) {
            ccArgs[j++] = ccArgs[i];
        }
    }
    ccArgs[j++] = runtimeLib;
    while (j < MAXARGS && ccArgs[j] != 0) {
        ccArgs[j++] = 0;
    }
}

int PassOne (linker)
char* linker;
{
    int status;

    initexec(linker);
    command = AddList(command, ccArgs);
    if (keepctdtFlag) {
	command = AddArg(command, "-k");
    }
    command = AddList(command, linkArgs);
    command = AddArg(command, StdCClib);
    if (!ccIsLinker) {
        if (gFlag) {
            command = AddArg(command, "-lg");
        }
        command = AddArg(command, cclib);
	if (linker == RELINK) {
	    command = AddList(command, objSpaceArgs);
	}
    }
    status = execute(linker, commandv, stdinDesc, stdoutDesc);
    return status;
}

int DefaultLink () {
    int status;

    if (runtimeLib[0] != '\0') {
	ccIsLinker = false;
	TransformCCArgs();
	linkArgv = AddArg(linkArgv, "-C");
    }
    status = PassOne(DEFAULTLINKER);
#if defined(CC_domunch)
    if (status != 0) {
        return status;
    }
    status = PassTwo();
#endif
    return status;
}

int PassTwo () {
    int status;
    int nmDesc, munchDesc;
    char nmName[FILENAMESIZE];

    if (!genRelocInfoFlag && !noMunchFlag) {
	nmDesc = MakeTmpFile(nmName, "nm");
	initexec("nm");
	command = AddArg(command, objName);
	status = execute("/bin/nm", commandv, stdinDesc, nmDesc);
	if (status != 0) {
	    return status;
	}
	close(nmDesc);	
	close(stdoutDesc);
	nmDesc = myopen(nmName, O_RDONLY, 0644); /* input */
					/* output */
	munchDesc = myopen("__ctdt.c", O_WRONLY | O_CREAT | O_TRUNC, 0644); 
	if (!keepctdtFlag) {
	    RegisterTmpFile("__ctdt.c");    
	}

	initexec(MUNCHER);
	if (profFlag) {
	    command = AddArg(command, "-p");
	} else if (gprofFlag) {
            command = AddArg(command, "-pg");
        }
	status = execute(MUNCHER, commandv, nmDesc, munchDesc);

	if (status != 0) {
	    status = RecompileWithMunchPhase();
	    if (status != 0) {
		return status;
	    }
	}
    }
    return 0;
}

int CPPphase (fullPathName, tail)
String fullPathName;
String tail;
{
    int status;

    initexec(PREPROCESSOR);
#if !defined(hpux)
    command = AddArg(command, "-B");
#endif
    command = AddList(command, cppArgs);
    command = AddArg(command, "-Dc_plusplus=1");
#if defined(mips)
    command = AddArg(command, "-DLANGUAGE_C");
#endif
    if (translator) {
	command = AddArg(command, libDir);
    }
    command = AddList(command, cppPathArgs);
    command = AddArg(command, fullPathName);
    if (Eflag) {
	if (suffixFlag) {
	    strcpy(destname, tail);
	    strcat(destname, suffix);
	    dest = myopen(
		destname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	    status = execute(PREPROCESSOR, commandv, stdinDesc, dest);
	    myclose(dest);
	} else {
	    dest = stdoutDesc;		/* else put on std output */
	    status = execute(PREPROCESSOR, commandv, stdinDesc, dest);
	}
    } else {
	cppDesc = MakeTmpFile(cppName, "cpp");
	status = execute(PREPROCESSOR, commandv, stdinDesc, cppDesc);
	myclose(cppDesc);
    }
    return status;
}

int FrontEndPhase (fullPathName, tail, in)
String fullPathName;
String tail;
FileDesc in;
{
    int status;
    int frontEndDesc;
    char aBuffer[FILENAMESIZE];
    
    initexec(CCfrontEnd);
    command = AddList(command, frontEndArgs);
    if (translator) {
	/* cfront */
	command = AddArg(command, "+L");
	strcpy(aBuffer, "+f");
	strcat(aBuffer, fullPathName);
	command = AddArg(command, aBuffer);
    } else {
	/* g++ */
	command = AddArg(command, "-quiet");
    }
    if (Fflag) {
	if (suffixFlag) {
	    strcpy(frontEndName, tail);
	    strcat(frontEndName, suffix);
	    dest = myopen(
		frontEndName, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	    status = execute(CCfrontEnd, commandv, in, dest);
	    myclose(dest);
	} else {
	    dest = stdoutDesc;
	    status = execute(CCfrontEnd, commandv, in, dest);
	}
    } else {
	strcpy(frontEndName, tail);	/* where to put front end out */
	strcat(frontEndName, frontEndSuffix);
	frontEndDesc = myopen(
	    frontEndName, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	status = execute(CCfrontEnd, commandv, in, frontEndDesc);
	myclose(frontEndDesc);
    }
    return status;
} /* FrontEndPhase */

int FilterPhase (fullPathName, tail)
String fullPathName;
String tail;
{
    int status;
    int filterDesc;

    if (plusIflag) {
	filterDesc = MakeTmpFile(filterName, "grep");
	initexec(FILTER);		/* default is /bin/grep */
	command = AddArg(command, "-v");
	command = AddArg(command, "^#");
	command = AddArg(command, fullPathName);
	status = execute(FILTER, commandv, stdinDesc, filterDesc);
	if (status != 0) {
	    WarningMsg("+I option error {grep}", nil);
	} else {
	    myclose(filterDesc);
	    initexec("mv");
	    command = AddArg(command, filterName);
	    command = AddArg(command, frontEndName);
	    status = execute(MV, commandv, stdinDesc, stdoutDesc);
	    if (status != 0) {
		perror(status);
	    }
	}
    }
    return status;
} /* FilterPhase */

int ccPhase ()
{
    int status;

    initexec(Ccompiler);
    command = AddList(command, ccArgs);
    command = AddArg(command, "-c");
    command = AddArg(command, frontEndName);
    status = execute(Ccompiler, commandv, stdinDesc, stdoutDesc);
    if (status != 0) {
	Remove(linkFile);
    }
    return status;
} /* ccPhase */

int asmPhase (fullPathName, tail)
String fullPathName, *tail;
{
    char aBuffer[FILENAMESIZE];

    initexec(ASSEMBLER);
    command = AddArg(command, "-o");
    strcpy(aBuffer, tail);
    strcat(aBuffer, ".o");	/* out from cc -S */
    command = AddArg(command, aBuffer);
    command = AddArg(command, fullPathName);
    return execute(ASSEMBLER, commandv, stdinDesc, stdoutDesc);
} /* asmPhase */

int RecompileWithMunchPhase ()
{
    int status;

    initexec(Ccompiler);
    command = AddArg(command, "-c");
    command = AddArg(command, "__ctdt.c");
    if (!keepctdtFlag) {
	RegisterTmpFile("__ctdt.o");
    }
    (void) execute(Ccompiler, commandv, stdinDesc, devnull);
    
    initexec(LINKER);
    command = AddList(command, ccArgs);
    command = AddArg(command, "__ctdt.o");
    command = AddList(command, linkArgs);
    command = AddArg(command, StdCClib);
    
    status = execute(LINKER, commandv, stdinDesc, stdoutDesc);
    return status;
} /* RecompileWithMunchPhase */

/* 
 * this phase isn't used any more.  I just left it here for reference
 * It used to run the output of cfront through sed(1) to strip the 
 * '_auto_' prefix off of locals and parameters, as this made
 * debugging CC programs with dbx(1) frustrating. 
 */
int dbxPhase (fullPath)
String fullPath;
{
    int status;
    FileDesc dbxDesc;
    char dbxName[FILENAMESIZE];
    
    sprintf(dbxName, ".c++.%s", fullPath);
    RegisterTmpFile(dbxName);
    dbxDesc = myopen(
	dbxName, O_WRONLY | O_CREAT | O_TRUNC,  0644
    );
    if (dbxDesc < 0 && !fakeFlag) {
	perror("CC : Couldn't create tmp for -dbx flag.  Darn.");
	return dbxDesc;
    }
    initexec("sed");
/*     command = AddArg(command, "-e");
 *     command = AddArg(command, "s/ _new/ __new/g");
 *     command = AddArg(command, "-e");
 *     command = AddArg(command, "s/ _delete/ __delete/g");
 *     command = AddArg(command, "-e");
 *     command = AddArg(command, "s/ _/ /g");
 *     command = AddArg(command, "-e");
 *     command = AddArg(command, "s/^_//g"); 
 */
    command = AddArg(command, "-e");
    command = AddArg(command, "s/_auto_//g");
    command = AddArg(command, fullPath);
    status = execute(SEDFILTER, commandv, stdinDesc, dbxDesc);
    myclose(dbxDesc);
    if (status == 0 || fakeFlag) {
	initexec("mv");
	command = AddArg(command, dbxName);
	command = AddArg(command, fullPath);
	status = execute(MV, commandv, stdinDesc, stdoutDesc);
    }
    return status;
} /* dbxPhase */
