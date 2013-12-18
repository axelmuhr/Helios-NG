/*
 * Interface to IPC for defining and accessing servers.
 */

#include <InterViews/connection.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <bstring.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <os/fs.h>
#include <os/ipc.h>
#include <os/proc.h>
#include <sys/socket.h>

inline void fatal (const char* s) {
    perror(s);
    exit(1);
}

static struct hostent* GetHostEnt (const char* hostname) {
    typedef struct hostent* Lookup(const char*);
    Lookup* findhost = (Lookup*)&gethostbyname;
    return (*findhost)(hostname);
}

static u_short PortNumber (u_short p) {
    return short_host_to_net(p);
}

static u_long FromAddr () {
    return long_host_to_net(INADDR_ANY);
}

Connection::Connection () {
    fd = -1;
}

/*
 * Sometimes we want to use a file descriptor as a connection.
 */

Connection::Connection (int d) {
    fd = d;
    domain = 0;
    name = nil;
    port = 0;
}

Connection::~Connection () {
    if (fd != -1) {
	close(fd);
    }
}

/*
 * Make an internet socket; return whether or not successful.
 */

boolean Connection::MakeSocket (struct sockaddr_in& s) {
    register struct hostent* h;

    Close();
    fd = socket(domain, SOCK_STREAM, 0);
    if (fd < 0) {
	return false;
    }
    h = GetHostEnt(name);
    if (h == nil || h->h_addrtype != domain) {
	return false;
    }
    bzero(&s, sizeof(s));
    s.sin_family = domain;
    s.sin_port = PortNumber(port);
    s.sin_addr.s_addr = FromAddr();
    return true;
}

void Connection::CreateService (const char* hostname, int n) {
    struct sockaddr_in s;

    domain = AF_INET;
    name = hostname;
    port = n;
    if (!MakeSocket(s)) {
	fatal("can't create socket");
    }
    if (bind(fd, &s, sizeof(s)) < 0) {
	fatal("can't bind socket");
    }
    if (listen(fd, 5) != 0) {
	fatal("can't listen to socket");
    }
}

/*
 * Create a UNIX domain socket; return the length of the socket address.
 */

int Connection::MakeLocalSocket (struct sockaddr_un& s) {
    Close();
    fd = socket(domain, SOCK_STREAM, 0);
    if (fd < 0) {
	return 0;
    }
    bzero(&s, sizeof(s));
    s.sun_family = domain;
    strncpy(s.sun_path, name, sizeof(s.sun_path) - 1);
    return sizeof(s.sun_family) + sizeof(s.sun_path);
}

void Connection::CreateLocalService (const char* servname) {
    struct sockaddr_un s;
    int n;

    domain = AF_UNIX;
    name = servname;
    port = -1;
    n = MakeLocalSocket(s);
    if (n == 0) {
	fatal("can't create socket");
    }
    unlink(s.sun_path);
    if (bind(fd, &s, n) < 0) {
	fatal("can't bind socket");
    }
    if (listen(fd, 5) != 0) {
	fatal("can't listen to socket");
    }
    /* UNIX domain sockets need to be mode 777 on 4.3 */
    chmod(s.sun_path, 0777);
}

boolean Connection::OpenService (const char* host, int n) {
    struct sockaddr_in s;

    domain = AF_INET;
    name = host;
    port = n;
    if (!MakeSocket(s)) {
	return false;
    }
    if (connect(fd, &s, sizeof(s)) < 0) {
	Close();
	return false;
    }
    fcntl(fd, F_SETFL, /* delay */ 0);
    return true;
}

boolean Connection::OpenLocalService (const char* str) {
    struct sockaddr_un s;
    int n;

    domain = AF_UNIX;
    name = str;
    n = MakeLocalSocket(s);
    if (n == 0) {
	return false;
    }
    if (connect(fd, &s, n) < 0) {
	Close();
	return false;
    }
    fcntl(fd, F_SETFL, /* delay */ 0);
    return true;
}

/*
 * Accept a request by a client to connect.  Since this routine
 * is used by servers and servers usually do not want to block
 * reading from a client, the connection is made non-blocking.
 */

Connection* Connection::AcceptClient () {
    int f;
    int len;
    struct sockaddr_in inet;
    struct sockaddr_un local;
    register Connection* c;

    if (domain == AF_INET) {
	len = sizeof(inet);
	f = accept(fd, &inet, &len);
    } else {
	len = sizeof(local);
	f = accept(fd, &local, &len);
    }
    if (f >= 0) {
	fcntl(f, F_SETFL, O_NDELAY);
	c = new Connection;
	c->domain = domain;
	c->name = name;
	c->port = port;
	c->fd = f;
    } else {
	c = nil;
    }
    return c;
}

void Connection::Close () {
    if (fd > 0) {
	close(fd);
    }
    fd = -1;
}

int Connection::Pending () {
    int nbytes;

    if (ioctl(fd, FIONREAD, &nbytes) < 0) {
	nbytes = -1;
    }
    return nbytes;
}

int Connection::Read (void* msg, int n) {
    return read(fd, msg, n);
}

int Connection::Write (const void* msg, int n) {
    return write(fd, msg, n);
}

int Connection::WritePad (const void* msg, int n, int padto) {
    register int r, left;
    static int zero[32];

    r = write(fd, msg, n);
    left = padto - n;
    while (left > sizeof(zero)) {
	r += write(fd, zero, sizeof(zero));
	left -= sizeof(zero);
    }
    return r + write(fd, zero, left);
}
