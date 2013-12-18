/*
 * A chief deputy manages the connection to an object space.
 */

#include <InterViews/chief.h>
#include <InterViews/packet.h>
#include <InterViews/tag.h>
#include <InterViews/connection.h>
#include <bstring.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ChiefDeputy::ChiefDeputy (Connection* c) {
    server = c;
    cur = 0;
}

ChiefDeputy::~ChiefDeputy () {
    Sync();
    delete server;
}

/*
 * Flush the outgoing message buffer.
 */

void ChiefDeputy::Sync () {
    if (cur > 0) {
	server->Write(buffer, cur);
	cur = 0;
    }
}

/*
 * Allocate space in the buffer for a packet.
 */

void ChiefDeputy::Alloc (void*& dst, ObjectTag t, int op, int len) {
    register Packet* p;
    register int n, prev;

    n = WordAlign(len + sizeof(Packet));
    prev = cur;
    cur += n;
    if (cur >= sizeof(buffer)) {
	if (prev == 0) {
	    fprintf(stderr, "object too large to send\n");
	    exit(1);
	}
	server->Write(buffer, prev);
	prev = 0;
	cur = n;
    }
    p = (Packet*) &buffer[prev];
    p->tag = t;
    p->op = op;
    p->count = 1;
    p->extend = 0;
    p->length = len;
    dst = &p[1];
}

/*
 * Put a string into a given place in the buffer.
 * The length of the null-terminated string is rounded up to
 * a word boundary, adding nulls if necessary.
 * The total number of bytes written in the buffer is returned.
 */

int ChiefDeputy::PackString (const char* s, void* dst) {
    return PackString(s, strlen(s), dst);
}

int ChiefDeputy::PackString (const char* s, int len, void* dst) {
    register int n = WordAlign(len + 1);
    bcopy(s, dst, len);
    if (n > len) {
	bzero(((char*)dst) + len, n - len);
    }
    return n;
}

/*
 * Short-hand for simple case when a message has no operands.
 */

void ChiefDeputy::Msg (ObjectTag t, int op) {
    void* msg;

    Alloc(msg, t, op, 0);
}

/*
 * Put a message in the buffer consisting of an op with a single string
 * as operand.  This call is short-hand for a combination of
 * Alloc and PackString.
 */

void ChiefDeputy::StringMsg (ObjectTag t, int op, const char* s) {
    void* msg;
    int n;

    n = strlen(s) + 1;
    Alloc(msg, t, op, n);
    PackString(s, n, msg);
}

/*
 * Put an integer in the buffer consisting of an op with a single integer
 * as operand.
 */

void ChiefDeputy::IntegerMsg (ObjectTag t, int op, int val) {
    int *msg;
    Alloc(msg, t, op, sizeof(int));
    *msg = val;
}

/*
 * Try to read a certain size reply.  Return the actual size read.
 */

int ChiefDeputy::ReadReply (void* reply, int len) {
    register int n, t;
    register char* p;

    Sync();
    p = (char*)reply;
    t = len;
    do {
	n = server->Read(p, t);
	if (n <= 0) {
	    return len - t;
	}
	p += n;
	t -= n;
    } while (t > 0);
    return len;
}

/*
 * Wait for a reply of a certain size from the server.
 */

void ChiefDeputy::GetReply (void* reply, int len) {
    if (ReadReply(reply, len) != len) {
	fprintf(stderr, "premature eof in GetReply\n");
	exit(1);
    }
}

/*
 * Read a string of the given length, including any padding
 * for alignment.  Space for the string is allocated here;
 * it is up to the caller to deallocate it.
 */

void ChiefDeputy::GetString (const char*& str, int len) {
    register int n = WordAlign(len);
    char* s = new char[n+1];
    s[n] = '\0';
    GetReply(s, n);
    str = s;
}
