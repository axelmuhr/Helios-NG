/*
 * Per-client information.
 */

#include "client.h"
#include "client_stub.h"
#include "manager.h"
#include <InterViews/catalog.h>
#include <InterViews/tagtable.h>
#include <stdio.h>
#include <string.h>

class WaitingList {
private:
    friend class Port;

    ClientStub* requestor;
    WaitingList* next;
};

Port::Port (int n) {
    pid = n;
    waiting = nil;
}

void Port::Sleep (ClientStub* c) {
    register WaitingList* i = new WaitingList;
    i->requestor = c;
    i->next = waiting;
    waiting = i;
}

void Port::Wakeup () {
    register WaitingList* i, * next;

    for (i = waiting; i != nil; i = next) {
	next = i->next;
	i->requestor->Reply(this);
	delete i;
    }
    waiting = nil;
}

void Port::Delete () {
    owner->Remove(name);
    delete name;
}

Client::Client (Manager* m, Connection* c, Catalog* d, ObjectTable* t) {
    mgr = m;
    owner = c;
    dictionary = d;
    table = t;
    path[0] = '\0';
    pathlen = 0;
}

Client::~Client () {
}

const char* Client::FullName (const char* name) {
    if (name[0] == '/') {
	return name;
    } else {
	strcpy(&path[pathlen], name);
	return path;
    }
}

void Client::UsePath (const char* name) {
    if (name[0] == '/') {
	strcpy(path, name);
	pathlen = strlen(path);
    } else {
	if (pathlen > 0 && path[pathlen-1] != '/') {
	    path[pathlen] = '/';
	    ++pathlen;
	}
	strcpy(&path[pathlen], name);
	pathlen += strlen(name);
    }
    if (pathlen > 0 && path[pathlen-1] != '/') {
	path[pathlen] = '/';
	++pathlen;
    }
}

void Client::Register (const char* name, int pid) {
    Port* p;
    const char* full;
    ObjectStub* stub;

    full = FullName(name);
    if (dictionary->Find(stub, full)) {
        p = (Port*)stub;
	if (p->pid != 0) {
	    return;
	}
	p->pid = pid;
	p->Wakeup();
    } else {
	p = new Port(pid);
	int n = strlen(full);
	p->name = new char[n+1];
	strcpy(p->name, full);
	dictionary->Register(full, p);
    }
    p->owner = this;
    table->Add(owner, pid, p);
}

void Client::UnRegister (const char* name) {
    Port* p;

    p = Find(FullName(name));
    if (p != nil) {
	table->Remove(owner, p->pid);
	delete p;
    }
}

Port* Client::Find (const char* name) {
    ObjectStub* stub;

    if (dictionary->Find(stub, FullName(name))) {
        return (Port*)stub;
    } else {
        return nil;
    }
}

Port* Client::Wait (ClientStub* c, const char* name) {
    Port* p;
    const char* full;
    ObjectStub* stub;

    full = FullName(name);
    if (dictionary->Find(stub, full)) {
        p = (Port*)stub;
	if (p->pid != 0) {
	    return p;
	}
    } else {
	p = new Port(0);
	int n = strlen(full);
	p->name = new char[n+1];
	strcpy(p->name, full);
	dictionary->Register(full, p);
    }
    p->Sleep(c);
    return nil;
}

void Client::Remove (const char* name) {
    dictionary->UnRegister(name);
}
