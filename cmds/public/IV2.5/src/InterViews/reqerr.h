/*
 * Handling errors from window server.
 */

#ifndef reqerr_h
#define reqerr_h

class ReqErr {
public:
    int msgid;
    int code;
    int request;
    int detail;
    void* id;
    char message[256];

    ReqErr();
    ~ReqErr();

    ReqErr* Install();
    virtual void Error();
};

#endif
