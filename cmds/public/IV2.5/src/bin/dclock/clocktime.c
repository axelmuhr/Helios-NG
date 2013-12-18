/*
 * time related functions for digital clock
 */

#include "clocktime.h"
#include <string.h>
#include <time.h>

Clock::Clock () {
    gettimeofday(&gmt, 0);
    nextMinute = gmt.tv_sec;
}

int Clock::NextTick () {
    gettimeofday(&gmt, 0);
    return nextMinute - gmt.tv_sec;
}

void Clock::GetTime (char* date, int& h, int& m, int& s) {
    struct tm local;

    local = * localtime(&gmt.tv_sec);
    h = local.tm_hour;
    m = local.tm_min;
    s = local.tm_sec;
    char ds[26];
    strcpy(ds, asctime(&local));
    strncpy(date, ds, 10);		/* day, month, day of month */
    date[10] = '\0';
    strncat(date, ds+19, 5);		/* year */
    date[15] = '\0';
    nextMinute = gmt.tv_sec + (60 - s);
}
