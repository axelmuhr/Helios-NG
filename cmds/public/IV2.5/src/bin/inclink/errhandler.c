/*
 * Error handling.
 */

#include "errhandler.h"
#include "inclink.h"
#include "replies.h"
#include <stdlib.h>
#include <stdio.h>

const char* errprefix = nil;

ErrorHandler Error(0, inclink_error);
ErrorHandler Warning(0, inclink_warning);

const int buffSize = 252;
char addBuff[buffSize];

void ErrorHandler::Flush () {
    if (server) {
        server->SendMsg(errlevel, msgbuff, msglen);
        msgend = msgbuff;
        msglen = 0;
    } else {
        fflush(stderr);
    }
}

inline void ErrorHandler::Copy (const char* s) {
    register char c;
    while (c = *s++) {
        if (msglen == buffSize) {
            Flush();
        }
        *msgend++ = c;
        msglen++;
    }
}

ErrorHandler::ErrorHandler (int i, int l) {
    errlevel = l;
    count = i;
    msgbuff = new char[buffSize];
    msgend = msgbuff;
    msglen = 0;
}

boolean ErrorHandler::StartMessage () {
    msgend = msgbuff;
    msglen = 0;
    return true;
}

void ErrorHandler::Add (const char* s) {
    Copy(s);
}

void ErrorHandler::Add (int n) {
    if (msglen + 10 > buffSize) {
        Flush();
    }
    sprintf(addBuff, "%d", n);
    Copy(addBuff);
}

void ErrorHandler::EndMessage () {
    Flush();
    ++count;
}

void ErrorHandler::operator() (const char* msg) {
    if (StartMessage()) {
	Add(msg);
	EndMessage();
    }
}

void ErrorHandler::operator() (const char* fmt, const char* msg) {
    if (StartMessage()) {
        sprintf(addBuff, fmt, msg);
        Add(addBuff);
	EndMessage();
    }
}

void ErrorHandler::operator() (const char* fmt, int n) {
    if (StartMessage()) {
        sprintf(addBuff, fmt, n);
        Add(addBuff);
	EndMessage();
    }
}

void ErrorHandler::operator() (const char* fmt, const char* msg, int n) {
    if (StartMessage()) {
        sprintf(addBuff, fmt, msg, n);
        Add(addBuff);
	EndMessage();
    }
}

void ErrorHandler::operator() (const char* fmt, int n1, int n2) {
    if (StartMessage()) {
	sprintf(addBuff, fmt, n1, n2);
        Add(addBuff);
	EndMessage();
    }
}

FatalErrorHandler Fatal(0, inclink_fatal);
FatalErrorHandler Panic(0, inclink_panic);

FatalErrorHandler::FatalErrorHandler (int i, int l) : (i, l) {
    /* nothing else to do */
}

void FatalErrorHandler::EndMessage () {
    Flush();
    exit(1);
}

DebugMessage Debug;

DebugMessage::DebugMessage () : (0, inclink_debug) {
    messages = false;
}

boolean DebugMessage::StartMessage () {
    return messages ? ErrorHandler::StartMessage() : false;
}
