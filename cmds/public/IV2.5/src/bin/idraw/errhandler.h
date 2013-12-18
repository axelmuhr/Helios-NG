// $Header: errhandler.h,v 1.7 89/03/27 19:05:36 interran Exp $
// declares class ErrHandler.

#ifndef errhandler_h
#define errhandler_h

#include <InterViews/reqerr.h>

// Declare imported types.

typedef void (NewHandler)();
class Editor;

// ErrHandler calls upon the Editor to save the current drawing if an
// X request error occurs.

class ErrHandler : public ReqErr {
public:

    ErrHandler();
    ~ErrHandler();

    void SetEditor(Editor*);
    ReqErr* Install();
    void Error();

protected:

    ErrHandler* SetErrHandler(ErrHandler*);
    void OutOfMemory();
    friend void out_of_memory();

    static ErrHandler* _err_handler; // used by out_of_memory

    Editor* editor;		     // handles drawing and editing operations
    ErrHandler* olderr;		     // used to restore previous handler
    NewHandler* oldnew;		     // used to restore previous handler

};

#endif
