/* time measurement routines
 */

#include "errhandler.h"
#include "timer.h"
#include <sys/types.h>
#include <sys/times.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

Timer::Timer (const char *caller, boolean isOn) {
    started = 0;
    strcpy(this->caller, caller);
    this->isOn = isOn;
}

Timer::~Timer () {}

void Timer::BeginTiming () {
    struct tms tbuf;

    if (isOn) {
	if (started == 0) {
	    times(&tbuf);
	    beginRealTime = time(0);
	    beginUserTime = tbuf.tms_utime;
	    beginSystemTime = tbuf.tms_stime;
	    started = 1;
	}
    }
}

void Timer::EndTiming () {
    struct tms tbuf;
    float realTime;
    float userTime;
    float systemTime;

    if (isOn) {
	if (started) {
	    times(&tbuf);
	    realTime = time(0) - beginRealTime;
	    userTime = (tbuf.tms_utime - beginUserTime) / 60.0;
	    systemTime = (tbuf.tms_stime - beginSystemTime) / 60.0;
            char msg[100];
            sprintf(msg,
		"%s: utime = %.3f, stime = %.3f, total = %.3f, real = %.1f",
		caller, userTime, systemTime, userTime + systemTime, realTime
	    );
	    printf("%s\n", msg);
            Debug(msg);
	}
	started = 0;
    }
}
