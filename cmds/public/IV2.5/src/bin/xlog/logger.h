/*
 * Object spaces that log or replay messages.
 */

#ifndef logger_h

#include <InterViews/space.h>

class Logger : public ObjectSpace {
public:
    ~Logger();

    void MakeNames(const char* path, int pid, const char* d, const char* s);
    virtual void Run(char** argv);
protected:
    char* name;
    class Connection* service;
    int nclients;
    char* xname;
    char* xdisplay;

    Logger();

    void Child(char**);
    Connection* XConnection();
};

class Recorder : public Logger {
public:
    Recorder(const char*);

    virtual void Run(char**);
private:
    void AddClient(Connection*);
};

class Replayer : public Logger {
public:
    Replayer(const char*);

    virtual void Run(char**);
};

class Tester : public Logger {
public:
    Tester(const char*);

    virtual void Run(char**);
private:
    void AddClient(Connection*);
};

extern class LogFile* log;

extern void Quit(int);

#endif
