/*
 * Record interaction between X and a client.
 * We fork a process that executes the client
 * with a special DISPLAY environment variable set to an IPC name
 * to which the parent (logger) listens.  This logger also connects
 * to X and writes message traffic into a file.
 */

#include "log.h"
#include "logger.h"
#include <InterViews/connection.h>
#include <os/proc.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

LogFile* log;			/* input or output log */

static class Logger* logger;

static void GetDisplay(char*, char*&, char*&);
static void NextArg(int&, int, const char*);

int main (int argc, char* argv[]) {
    int i;
    const char* xpath;
    char* display;
    char* screen;

    GetDisplay(getenv("DISPLAY"), display, screen);
    logger = nil;
    log = new LogFile;
    xpath = default_xpath;
    for (i = 1; i < argc; i++) {
	if (strcmp(argv[i], "-xp") == 0) {
	    NextArg(i, argc, argv[i]);
	    xpath = argv[i];
	} else if (strcmp(argv[i], "-x10") == 0) {
	    xpath = default_x10path;
	} else if (strcmp(argv[i], "-o") == 0) {
	    NextArg(i, argc, argv[i]);
	    logger = new Recorder(argv[i]);
	} else if (strcmp(argv[i], "-i") == 0) {
	    NextArg(i, argc, argv[i]);
	    logger = new Replayer(argv[i]);
	} else if (strcmp(argv[i], "-t") == 0) {
	    NextArg(i, argc, argv[i]);
	    logger = new Tester(argv[i]);
	} else {
	    break;
	}
    }
    if (logger == nil) {
	fprintf(stderr, "must specify one of -o, -i, or -t\n");
	exit(1);
    }

    signal(SIGHUP, Quit);
    signal(SIGINT, Quit);
    signal(SIGQUIT, Quit);

    logger->MakeNames(xpath, getpid(), display, screen);
    logger->Run(&argv[i]);
    return 0;
}

/*
 * Extract the display and screen numbers as strings from the
 * given DISPLAY string.  The assumption is that "str" is of the form
 * <host>:<display number>.<screen number> where the screen number
 * is optional.
 *
 * It would be nice if we could use a standard X routine here
 * so that we didn't have to replicate knowledge about this format.
 */

static void GetDisplay (char* str, char*& display, char*& screen) {
    if (str == nil) {
	fprintf(stderr, "DISPLAY undefined\n");
	exit(1);
    }
    display = strchr(str, ':');
    if (display == nil) {
	fprintf(stderr, "missing ':' in DISPLAY\n");
	exit(1);
    }
    display += 1;
    screen = strchr(display, '.');
    if (screen == nil) {
	screen = "";
    } else {
	*screen = '\0';
	screen += 1;
    }
}

static void NextArg (int& i, int upper, const char* s) {
    ++i;
    if (i >= upper) {
	fprintf(stderr, "missing argument after %s\n", s);
	exit(1);
    }
}

void Quit (int n) {
    delete log;
    delete logger;
    exit(n);
}
