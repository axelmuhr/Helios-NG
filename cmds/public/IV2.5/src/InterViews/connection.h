/*
 * Connections provide an interface to
 * the OS IPC mechanism.
 */

#ifndef connection_h
#define connection_h

#include <InterViews/defs.h>

class Connection {
public:
    Connection();
    Connection(int);
    ~Connection();

    void CreateService(const char* host, int port);
    void CreateLocalService(const char*);
    boolean OpenService(const char* host, int port);
    boolean OpenLocalService(const char*);
    Connection* AcceptClient();

    int Pending();
    int Read(void*, int);
    int Write(const void*, int);
    int WritePad(const void*, int, int);
    int Descriptor () { return fd; }
    void Close();
protected:
    int fd;
    int domain;
    const char* name;
    int port;

    boolean MakeSocket (struct sockaddr_in&);
    int MakeLocalSocket (struct sockaddr_un&);
};

inline int WordAlign (int n) {
    return (n + sizeof(int) - 1) & ~(sizeof(int) - 1);
}

#endif
