/*
 *  IStat
 */

#ifndef stat_h
#define stat_h

#include <InterViews/scene.h>

class Stats {
public:
    Stats( const char * host );

    float load1Av;
    float load5Av;
    float userCPUFract;
    float niceCPUFract;
    float systemCPUFract;
    float diskIORate;
    float netIORate;

    void NewStats();		    
private:
    char * hostname;

    long oneMinLoad;
    long fiveMinLoad;
    int userCPU;
    int niceCPU;
    int systemCPU;
    int idleCPU;
    int diskTransfers;
    int netInPackets;
    int netOutPackets;
    long timestamp;
};

class Log;
class Pattern;

class IStat : public MonoScene {
public:
    IStat( const char * host, int delay, int width=0, int height=0 );
    ~IStat();

    void Run();
    void Tick();
protected:
    virtual void Handle(Event&);
    virtual void Reconfig();
private:
    int width;
    int height;
    char bannersample[256];
    char bannerformat[256];
    Stats* st;
    Log* Loads;
    Log* CPU;
    Log* IO;
};

#endif
