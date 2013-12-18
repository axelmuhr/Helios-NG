/*
 * Testing mode.
 */

#include "log.h"
#include "logger.h"
#include <InterViews/connection.h>
#include <bstring.h>
#include <os/proc.h>
#include <fcntl.h>
#include <memory.h>

Tester::Tester (const char* s) {
    log->Open(s);
}

void Tester::Run (char** argv) {
    service = new Connection;
    service->CreateLocalService(name);
    StartServer(service, nil);
    int pid = fork();
    if (pid == -1) {
	fprintf(stderr, "can't fork\n");
	exit(1);
    } else if (pid == 0) {
	service->Close();
	Child(argv);
    } else {
	/* just wait for child to connect, which will call AddClient */
	Dispatch();
    }
}

void Tester::AddClient (Connection* c) {
    int delay, dir, n, count;
    char logbuf[logbufsize];
    char clientbuf[logbufsize];

    fcntl(c->Descriptor(), F_SETFL, /* allowing blocking reads */ 0);
    count = 0;
    for (;;) {
	n = log->Read(delay, dir, logbuf);
	if (n <= 0) {
	    break;
	}
	if (dir == from_client) {
	    int nread = 0;
	    int nleft = n;
	    while (nread < n) {
		int r = c->Read(&clientbuf[nread], nleft);
		if (r == -1) {
		    fprintf(stderr, "error reading from client\n");
		    exit(2);
		}
		nread += r;
		nleft -= r;
	    }
	    if (bcmp(clientbuf, logbuf, n) != 0) {
		/*
		 * Find out exactly which byte differs.
		 */
		register char* p = clientbuf;
		register char* q = logbuf;
		/*
		 * If the "memcmp" library routine correctly detects
		 * a difference, this loop will terminate before
		 * the pointers reach the end of the buffer.
		 * It will leave "p" pointing after the difference.
		 */
		while (*p++ == *q++);
		count += (p - clientbuf - 1);
		fprintf(stderr, "client output differs at %d\n", count);
		exit(1);
	    }
	    count += n;
	} else {
	    c->Write(logbuf, n);
	}
    }
}
