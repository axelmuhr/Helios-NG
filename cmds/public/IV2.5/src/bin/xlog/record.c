/*
 * Record between X and a client program.
 */

#include "log.h"
#include "logger.h"
#include <InterViews/connection.h>
#include <os/fs.h>
#include <os/proc.h>

/*
 * Handler for messages from X.
 */

class XRecorder : public ObjectStub {
public:
    XRecorder(Connection*);
private:
    Connection* client;		/* connection to client */

    void ChannelReady(int);
};

XRecorder::XRecorder (Connection* c) {
    client = c;
}

void XRecorder::ChannelReady (int f) {
    char buf[logbufsize];

    int n = read(f, buf, sizeof(buf));
    if (n <= 0) {
	client->Close();
	client = nil;
    } else {
	log->Write(to_client, buf, n);
	client->Write(buf, n);
    }
}

/*
 * Handler for messages from the client.
 */

class ClientRecorder : public ObjectStub {
    int id;			/* client number */
    Connection* server;		/* connection to X */

    void ChannelReady(int);
public:
    ClientRecorder(int, Connection*);
};

ClientRecorder::ClientRecorder (int n, Connection* c) {
    id = n;
    server = c;
}

void ClientRecorder::ChannelReady (int f) {
    char buf[logbufsize];

    int n = read(f, buf, sizeof(buf));
    if (n <= 0) {
	Quit(0);
    } else {
	log->Write(from_client, buf, n);
	server->Write(buf, n);
    }
}

Recorder::Recorder (const char* s) {
    log->Create(s);
}

void Recorder::Run (char** argv) {
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
	for (;;) {
	    Dispatch();
	}
    }
}

void Recorder::AddClient (Connection* c) {
    int x = XConnection()->Descriptor();
    AddChannel(x, new XRecorder(c));
    Attach(x);
    ++nclients;
    AddChannel(
	c->Descriptor(), new ClientRecorder(nclients, new Connection(x))
    );
}
