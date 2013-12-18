/*
 * Object space for managing object spaces.
 */

#include "client.h"
#include "client_stub.h"
#include "manager.h"
#include <InterViews/spaceman.h>
#include <InterViews/connection.h>
#include <InterViews/catalog.h>
#include <InterViews/tagtable.h>
#include <os/fs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>

/*
 * The main program just creates the manager and runs the dispatch loop.
 */

int main (int argc, char* argv[]) {
    Manager* m = new Manager(argc, argv);
    boolean done = false;
    while (!done) {
	m->Dispatch();
    }
    return 0;
}

/*
 * Constructing the manager processes the argument list and
 * creates the server.
 */

Manager::Manager (int argc, char* argv[]) {
    register int i;
    Connection* internal;

    name = "spaceman";
    dir = spaceman_dir;
    for (i = 1; i < argc; i++) {
	if (strcmp(argv[i], "-s") == 0) {
	    ++i;
	    if (i == argc) {
		Usage();
	    }
	    dir = argv[i];
	} else {
	    Usage();
	}
    }
    if (access(dir, R_OK+W_OK+X_OK) != 0 && mkdir(dir, 0777) != 0) {
	fprintf(stderr, "can't make directory %s\n", dir);
	exit(1);
    }
    internal = new Connection;
    strcpy(local, spaceman_mgr);
    internal->CreateLocalService(local);
    StartServer(internal, nil);
}

void Manager::Usage () {
    fprintf(stderr, "usage: spaceman [-d] [-s spacedir]\n");
    exit(1);
}

/*
 * A message to the space means to create a new client.
 * Subsequent messages are handled on a per-client basis.
 */

void Manager::Message (Connection* c, ObjectTag, int, void* msg, int) {
    register int* p = (int*)msg;
    Client* client = new Client(this, c, dictionary, table);
    table->Add(c, p[0], new ClientStub(client, c));
}
