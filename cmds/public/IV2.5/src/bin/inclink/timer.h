/* Logs and prints times */

#ifndef _timer_
#define _timer_

#include "defs.h"

class Timer {
    long beginRealTime;	    /* beginning times */
    long beginUserTime;
    long beginSystemTime;
    boolean isOn;
    boolean started;	    /* is the timing started? */
    char  caller[32];	    /* name of the caller */

public:
    Timer(const char* caller, boolean isOn);
    ~Timer();

    void BeginTiming();
    void EndTiming();
};

extern Timer* timer;

#endif
