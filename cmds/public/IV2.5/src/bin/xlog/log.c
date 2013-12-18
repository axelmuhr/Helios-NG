/*
 * Create a X log header.
 */

#include "log.h"
#include <stdlib.h>
#include <sys/time.h>

LogFile::LogFile () {
    data = nil;
}

LogFile::~LogFile () {
    if (data != nil) {
	fclose(data);
    }
}

void LogFile::Check () {
    if (data != nil) {
	fprintf(stderr, "cannot have multiple log files\n");
	exit(1);
    }
}

void LogFile::Create (const char* name) {
    int version;

    Check();
    data = fopen(name, "w");
    if (data == nil) {
	fprintf(stderr, "can't create %s\n", name);
	exit(1);
    }
    version = VersionNumber;
    fwrite((char*)&version, sizeof(version), 1, data);
}

void LogFile::Open (const char* name) {
    int version;

    Check();
    data = fopen(name, "r");
    if (data == nil) {
	fprintf(stderr, "can't read %s\n", name);
	exit(1);
    }
    fread((char*)&version, sizeof(version), 1, data);
    if (version != VersionNumber) {
	fprintf(
	    stderr, "version mismatch on %s (%d versus expected %d)\n",
	    name, version, VersionNumber
	);
    }
}

int LogFile::Read (int& delay, int& dir, char* buf) {
    LogHeader h;

    int n = fread((char*)&h, 1, sizeof(h), data);
    if (n <= 0) {
	return n;
    }
    if (n != sizeof(h)) {
	fprintf(stderr, "incomplete log header\n");
	exit(1);
    }
    if (h.direction != to_client && h.direction != from_client) {
	fprintf(stderr, "corrupt log header (direction %d)\n", h.direction);
	exit(1);
    }
    if (fread(buf, 1, h.nbytes, data) != h.nbytes) {
	fprintf(stderr, "incomplete log message\n");
	exit(1);
    }
    delay = h.delay;
    dir = h.direction;
    return h.nbytes;
}

void LogFile::Write (int dir, const char* buf, int n) {
    static struct timeval prevtime = { 0, 0 };
    struct timeval curtime;
    struct timezone tz;
    LogHeader h;

    gettimeofday(&curtime, &tz);
    if (prevtime.tv_sec == 0) {
	h.delay = 0;
    } else {
	h.delay = 1000*(curtime.tv_sec - prevtime.tv_sec) +
	    (curtime.tv_usec - prevtime.tv_usec)/1000;
    }
    prevtime = curtime;
    h.direction = dir;
    h.nbytes = n;
    if (fwrite((char*)&h, 1, sizeof(h), data) != sizeof(h)) {
	fprintf(stderr, "error writing log header\n");
	exit(1);
    }
    if (fwrite(buf, 1, n, data) != n) {
	fprintf(stderr, "error writing log message\n");
	exit(1);
    }
}

void LogFile::Close () {
    fclose(data);
}
