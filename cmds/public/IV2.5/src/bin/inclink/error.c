/*
 * Error handling.
 */

#include "error.h"
#include <stdio.h>
#include <stdlib.h>

const char* errprefix;

ErrorHandler Error(0, nil);
ErrorHandler Warning(0, "warning: ");

ErrorHandler::ErrorHandler (int i, const char* s) {
    startmsg = s;
    count = i;
}

boolean ErrorHandler::StartMessage () {
    fflush(stdout);
    if (errprefix != nil) {
	fprintf(stderr, "%s: ", errprefix);
    }
    if (startmsg != nil) {
	fputs(startmsg, stderr);
    }
    return true;
}

void ErrorHandler::Add (const char* s) {
    fputs(s, stderr);
}

void ErrorHandler::Add (int n) {
    fprintf(stderr, "%d", n);
}

void ErrorHandler::EndMessage () {
    putc('\n', stderr);
    fflush(stderr);
    ++count;
}

void ErrorHandler::operator() (const char* msg) {
    if (StartMessage()) {
	fputs(msg, stderr);
	EndMessage();
    }
}

void ErrorHandler::operator() (const char* fmt, const char* msg) {
    if (StartMessage()) {
	fprintf(stderr, fmt, msg);
	EndMessage();
    }
}

void ErrorHandler::operator() (const char* fmt, int n) {
    if (StartMessage()) {
	fprintf(stderr, fmt, n);
	EndMessage();
    }
}

void ErrorHandler::operator() (const char* fmt, const char* msg, int n) {
    if (StartMessage()) {
	fprintf(stderr, fmt, msg, n);
	EndMessage();
    }
}

void ErrorHandler::operator() (const char* fmt, int n1, int n2) {
    if (StartMessage()) {
	fprintf(stderr, fmt, n1, n2);
	EndMessage();
    }
}

FatalErrorHandler Fatal(0, "fatal error: ");
FatalErrorHandler Panic(0, "internal error: ");

FatalErrorHandler::FatalErrorHandler (int i, const char* s) : (i, s) {
    /* nothing else to do */
}

void FatalErrorHandler::EndMessage () {
    putc('\n', stderr);
    exit(1);
}

DebugMessage Debug;

DebugMessage::DebugMessage () : (0, nil) {
    messages = false;
}

boolean DebugMessage::StartMessage () {
    return messages ? ErrorHandler::StartMessage() : false;
}
