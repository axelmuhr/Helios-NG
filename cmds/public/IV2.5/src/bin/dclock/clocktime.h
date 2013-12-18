/*
 * dclock timing information
 */

#ifndef clocktime_h
#define clocktime_h

#include <sys/time.h>

class Clock {
public:
    Clock();

    int NextTick();	    /* seconds until next minute */
    void GetTime(char *date, int& hours, int &mins, int &secs);
private:
    struct timeval gmt;
    long nextMinute;	    /* next minute in gmt */
};

#endif
