/*
 * Stub for per-client objects.
 */

#include "client.h"
#include "client_stub.h"
#include <InterViews/connection.h>
#include <InterViews/spaceman.h>
#include <stdio.h>
#include <string.h>

ClientStub::ClientStub (Client* cl, Connection* c) {
    client = cl;
    connection = c;
}

ClientStub::~ClientStub () {
    delete client;
}

void ClientStub::Message (Connection*, ObjectTag, int op, void* msg, int) {
    switch (op) {
	case spaceman_UsePath:
	    client->UsePath((char*)msg);
	    break;
	case spaceman_Register:
	    int* i = (int*)msg;
	    client->Register((char*)&i[1], i[0]);
	    break;
	case spaceman_UnRegister:
	    client->UnRegister((char*)msg);
	    break;
	case spaceman_Find:
	    Reply(client->Find((char*)msg));
	    break;
	case spaceman_WaitFor:
	    Port* p = client->Wait(this, (char*)msg);
	    if (p != nil) {
		Reply(p);
	    }
	    break;
	default:
	    /* ignore op */;
    }
}

void ClientStub::Reply (Port* p) {
    int pid = (p == nil) ? 0 : p->pid;
    connection->Write(&pid, sizeof(pid));
}
