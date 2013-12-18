// $Header: errhandler.c,v 1.8 89/03/27 19:05:33 interran Exp $
// implements class ErrHandler.

#include "editor.h"
#include "errhandler.h"
#include <stdio.h>
#include <stdlib.h>

// Declare imported functions.

extern NewHandler* set_new_handler(NewHandler*);

// out_of_memory relays the out of memory exception to the current
// instance of ErrHandler.

static void out_of_memory () {
    ErrHandler::_err_handler->OutOfMemory();
}

// ErrHandler doesn't have an Editor yet.

ErrHandler::ErrHandler () {
    editor = nil;
    olderr = nil;
    oldnew = nil;
}

// ~ErrHandler restores the previous handlers for out of memory exceptions.

ErrHandler::~ErrHandler () {
    SetErrHandler(olderr);
    set_new_handler(oldnew);
}

// SetEditor sets the Editor which ErrHandler calls upon.

void ErrHandler::SetEditor (Editor* e) {
    editor = e;
}

// Install installs this instance of ErrHandler as the handler for
// both X protocol request errors and out of memory exceptions.

ReqErr* ErrHandler::Install () {
    olderr = SetErrHandler(this);
    oldnew = set_new_handler(&out_of_memory);

    return ReqErr::Install();
}

// Error prints the X error, checkpoints the current drawing, and
// terminates the program's execution.

void ErrHandler::Error () {
    fprintf(stderr, "X Error: %s\n", message);
    fprintf(stderr, "         Request code: %d\n", request);
    fprintf(stderr, "         Request function: %d\n", detail);
    fprintf(stderr, "         Request window 0x%x\n", id);
    fprintf(stderr, "         Error Serial #%d\n", msgid);
    if (editor != nil) {
	editor->Checkpoint();
    }
    const int ERROR = 1;
    exit(ERROR);
}

// SetErrHandler updates the static variable read by out_of_memory

ErrHandler* ErrHandler::SetErrHandler (ErrHandler* n) {
    ErrHandler* o = _err_handler;
    _err_handler = n;
    return o;
}

// OutOfMemory aborts the program.  I wish it could checkpoint the
// drawing, but writing the drawing consumes a significant amount of
// memory.

void ErrHandler::OutOfMemory () {
    fprintf(stderr, "operator new failed: out of memory\n");
    abort();
}
