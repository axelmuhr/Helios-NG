/*
 * Log : a data logging package
 * $Header: log.c,v 1.2 89/05/29 09:25:32 linton Exp $
 */

#include "log.h"
#include <InterViews/interactor.h>
#include <bstring.h>

Trace::Trace (int s, float w) {
    size = s;
    weight = w;
    history = new float[ size ];
    for ( int i=0; i<size; ++i ) {
	history[i] = 0.0;
    }
    smoothed = 0.0;
    last = 0;
}

Trace::~Trace () {
    delete history;
}

void Trace::Data (float data) {
    last = (last<size-1) ? last+1 : 0;
    history[ last ] = data;
    smoothed = double(smoothed*weight) + data;
}

void Trace::Stats( int period, float &max, float &min, float &average ) {
    double total = 0;
    max = min = history[last];
    float * h = History(period);
    for ( int i = period; i>0; --i ) {
    	float data = *h;
	max = (data>max) ? data : max;
	min = (data<min) ? data : min;
	total += data;
	++h;
    }
    average = total/period;
}

float Trace::Max (int period) {
    float max = history[last];
    float * h = History(period);
    for ( int i = period; i>0; --i ) {
    	float data = *h;
	max = (data>max) ? data : max;
	++h;
    }
    return max;
}

float Trace::Min (int period) {
    float min = history[last];
    float * h = History(period);
    for ( int i = period; i>0; --i ) {
    	float data = *h;
	min = (data<min) ? data : min;
	++h;
    }
    return min;
}

float Trace::Average (int period) {
    double total = 0;
    float * h = History(period);
    for ( int i = period; i>0; --i ) {
    	float data = *h;
	total += data;
	++h;
    }
    return total/period;
}

float * Trace::History (int period) {
    if ( last >= period-1 ) {
	return history + last - period + 1;
    } else if (period > size) {
	float * newhistory = new float[period];
	for ( int i=0; i<period; ++i ) {
	    newhistory[i] = 0.0;
	}
	bcopy(
	    (char*)(history+last+1),
	    (char*)(newhistory+period-size),
	    (size-last-1) * sizeof(float)
	);
	bcopy(
	    (char*)history,
	    (char*)(newhistory+period-last-1),
	    (last+1) * sizeof(float)
	);
	last = period - 1;
	size = period;
	delete history;
	history = newhistory;
	return history;
    } else {
	int shift = period - last - 1;
	float * temp = new float[ shift ];
	bcopy(
	    (char*)(history+size-shift),
	    (char*)temp,
	    shift * sizeof(float)
	);
	bcopy(
	    (char*)history,
	    (char*)(history+shift),
	    (size-shift) * sizeof(float)
	);
	bcopy(
	    (char*)temp,
	    (char*)history,
	    shift * sizeof(float)
	);
	last += shift;
	delete temp;
	return history;
    }
}

Log::Log() {
    firstViewer = nil;
    traces[0] = nil;
    traces[1] = nil;
    traces[2] = nil;
}

Log::Log( Trace * t1 ) {
    firstViewer = nil;
    traces[0] = t1;
    traces[1] = nil;
    traces[2] = nil;
}

Log::Log( Trace * t1, Trace * t2 ) {
    firstViewer = nil;
    traces[0] = t1;
    traces[1] = t2;
    traces[2] = nil;
}

Log::Log( Trace * t1, Trace * t2, Trace * t3 ) {
    firstViewer = nil;
    traces[0] = t1;
    traces[1] = t2;
    traces[2] = t3;
}

Log::~Log() {
    delete traces[0];
    delete traces[1];
    delete traces[2];
    while ( firstViewer != nil ) {
	Viewer * viewer = firstViewer;
	firstViewer = firstViewer->next;
	delete viewer->view;
	delete viewer;
    }
}


void Log::Tick() {
    for (Viewer *v = firstViewer; v != nil; v = v->next ) {
	v->view->Update();
    }
}

void Log::Attach (Interactor * i) {
    Viewer * newViewer = new Viewer;
    newViewer->view = i;
    newViewer->next = firstViewer;
    firstViewer = newViewer;
}

void Log::Detach (Interactor * i) {
    Viewer * prev = nil;
    for ( Viewer * v = firstViewer; v != nil; v = v->next ) {
	if ( v->view == i ) {
	    if ( prev == nil ) {
		firstViewer = v->next;
	    } else {
		prev->next = v->next;
	    }
	    delete v->view;
	    delete v;
	    break;
	} else {
	    prev = v;
	}
    }
}
