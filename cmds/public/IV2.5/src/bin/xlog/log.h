/*
 * X log file format.
 */

#ifndef log_h

#include <InterViews/defs.h>
#include <stdio.h>

static const int VersionNumber = 1;

static const char* const default_xpath = "/tmp/.X11-unix/X";
static const char* const default_x10path = "/tmp/X";
static const char* const display_format = "DISPLAY=unix:%05d.";

static const int to_client = 0;		/* input data */
static const int from_client = 1;	/* output data */

static const int logbufsize = 4096;	/* maximum size of log entry */

class LogFile {
public:
    LogFile();
    ~LogFile();

    void Create(const char*);
    void Open(const char*);
    int Read(int& delay, int& dir, char*);
    void Write(int dir, const char*, int);
    void Close();
private:
    FILE* data;

    struct LogHeader {
	unsigned int delay : 15;	/* milliseconds since previous entry */
	unsigned int direction : 1;	/* to_client or from_client */
	unsigned int nbytes : 16;	/* size of log entry in bytes */
    };
    void Check();
};

#endif
