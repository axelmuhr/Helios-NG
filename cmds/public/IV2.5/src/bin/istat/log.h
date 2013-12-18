/*
 * Log : a data logging package
 * $Header: log.h,v 1.1 87/06/17 11:27:52 calder Exp $
 */

#ifndef log_h
#define log_h

static const int TraceDefaultSize = 256;
static const float TraceDefaultWeight = 0.99;

class Interactor;
struct Viewer {
    Interactor * view;
    Viewer * next;
};

class Trace {
    int size;			    // how many data points to remember
    int last;			    // index of last data point
    float * history;		    // array of data points
    float weight;		    // smoothing weighting factor
    double smoothed;		    // weighted average of all data
public:
    Trace(
	int size =TraceDefaultSize,
	float weighting =TraceDefaultWeight
    );
    ~Trace();

    void Data( float );		    // new data point

    float Smoothed() { return smoothed*(1-weight); }
    float Last() { return history[last]; }
    float * History( int period );  // array of last period data points

    float Max( int period );	    // maximum over last period data points
    float Min( int period );	    // minimum ...
    float Average( int period );    // average ...
    void Stats( int period, float &max, float &min, float &average );
};

class Log {
    Trace * traces[3];
    Viewer * firstViewer;
public:
    Log();
    Log( Trace* );
    Log( Trace*, Trace* );
    Log( Trace*, Trace*, Trace* );
    ~Log();			    // and delete viewers

    void Tick();		    // update viewers
    Trace * GetTrace(int i) { return traces[i-1]; }

    void Attach( Interactor * );
    void Detach( Interactor * );
};

#endif
