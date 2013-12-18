/*
 * Base class operations.
 */

#include "log.h"
#include "logger.h"
#include <InterViews/connection.h>
#include <bstring.h>
#include <os/fs.h>
#include <os/proc.h>
#include <stdlib.h>
#include <string.h>

Logger::Logger () {
    nclients = 0;
}

Logger::~Logger () {
    if (service != nil) {
	service->Close();
	unlink(name);
    }
}

/*
 * Create the client connection, X connection, and display names.
 */

void Logger::MakeNames (
    const char* path, int pid, const char* display, const char* screen
) {
    name = new char[strlen(path) + 6];
    sprintf(name, "%s%05d", path, pid);

    xname = new char[strlen(path) + strlen(display) + 1];
    strcpy(xname, path);
    strcat(xname, display);

    xdisplay = new char[strlen(display_format) + 1 + strlen(screen) + 1];
    sprintf(xdisplay, display_format, pid);
    strcat(xdisplay, screen);
}

/*
 * No default Run operation -- should always use subclass.
 */

void Logger::Run (char**) {
    fprintf(stderr, "Logger::Run called!\n");
    exit(1);
}

/*
 * Execute the program with the special display environment variable
 * to connect to us instead of X.
 */

void Logger::Child (char** argv) {
    register int i;
    char* s;

    for (i = 0; environ[i] != nil; i++) {
	s = strchr(environ[i], '=');
	if (s != nil) {
	    *s = '\0';
	    if (strcmp(environ[i], "DISPLAY") == 0) {
		environ[i] = xdisplay;
		break;
	    } else {
		*s = '=';
	    }
	}
    }
    if (environ[i] == nil) {
	char** newenv = new char*[i+2];
	bcopy(environ, newenv, i*sizeof(char*));
	environ = newenv;
	environ[i] = xdisplay;
	environ[i+1] = nil;
    }
    execvp(*argv, argv);
    fprintf(stderr, "can't exec %s\n", *argv);
    exit(1);
}

/*
 * Create a connection to X.
 */

Connection* Logger::XConnection () {
    Connection* c = new Connection;
    if (!c->OpenLocalService(xname)) {
	fprintf(stderr, "can't connect to X\n");
	exit(1);
    }
    return c;
}
