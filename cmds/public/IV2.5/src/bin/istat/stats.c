/*
 * IStat and Stats classes
 */

#include "stats.h"
#include "log.h"
#include "indicator.h"

#include <InterViews/box.h>
#include <InterViews/glue.h>
#include <InterViews/border.h>
#include <InterViews/shape.h>
#include <InterViews/sensor.h>
#include <InterViews/paint.h>
#include <InterViews/painter.h>
#include <stdio.h>

#if ultrix
#   include "rstat.h"
#else
#   include <rpcsvc/rstat.h>
#endif

Stats::Stats (const char* host) {
    hostname = (char*)host;
    load1Av = load5Av = 0.0;
    userCPUFract = niceCPUFract = systemCPUFract = 0.0;
    diskIORate = netIORate = 0.0;
    timestamp = 0;
}

void Stats::NewStats () {
    struct statstime st;
    rstat(hostname, &st);

    int diskXfer = 0;
    for (int i = 0; i<DK_NDRIVE; ++i) {
	diskXfer += st.dk_xfer[i];
    }
    if (timestamp != 0 && timestamp != st.curtime.tv_sec) {
	load1Av = float(st.avenrun[0]) / float(1<<8);
	load5Av = float(st.avenrun[1]) / float(1<<8);
	int totalCPU =
	    (st.cp_time[0] - userCPU)	    // want the *difference*
	    +(st.cp_time[1] - niceCPU)
	    +(st.cp_time[2] - systemCPU)
	    +(st.cp_time[3] - idleCPU);
	userCPUFract = float(st.cp_time[0] - userCPU) / float(totalCPU);
	niceCPUFract =  float(st.cp_time[1] - niceCPU) / float(totalCPU);
	systemCPUFract = float(st.cp_time[2] - systemCPU) / float(totalCPU);
	diskIORate =
	    float(diskXfer - diskTransfers)
	    / float(st.curtime.tv_sec - timestamp);
	netIORate =
	    float(st.if_ipackets-netInPackets + st.if_opackets-netOutPackets)
	    / float(st.curtime.tv_sec - timestamp);
    }
    oneMinLoad = st.avenrun[0];		    // save for next call
    fiveMinLoad = st.avenrun[1];
    userCPU = st.cp_time[0];
    niceCPU = st.cp_time[1];
    systemCPU = st.cp_time[2];
    idleCPU = st.cp_time[3];
    diskTransfers = diskXfer;
    netInPackets = st.if_ipackets;
    netOutPackets = st.if_opackets;
    timestamp = st.curtime.tv_sec;
}

boolean IOScaleDown (Indicator* i) {
    Trace * t1 = i->GetLog()->GetTrace(1);
    Trace * t2 = i->GetLog()->GetTrace(2);
    if (t2 != nil && t2->Last() > i->GetScale()) {
	return true;
    } else if (t1 != nil && t1->Last() > i->GetScale()) {
	return true;
    } else {
	return false;
    }
}

boolean IOScaleUp (Indicator* i) {
    Trace* t1 = i->GetLog()->GetTrace(1);
    Trace* t2 = i->GetLog()->GetTrace(2);
    if (t2 != nil && t2->Last() < i->GetScale()/10.0) {
	return true;
    } else if (t1 != nil && t1->Last() < i->GetScale()/10.0) {
	return true;
    } else {
	return false;
    }
}

float LoadScales[] = {
    0.1, 0.2, 0.5, 1.0, 2.0, 5.0, 10.0, 20.0, 50.0, 100.0
};
int LoadScaleCount = sizeof(LoadScales)/sizeof(float);

float IOScales[] = {
    20, 40, 60, 80, 100
};
int IOScaleCount = sizeof(IOScales)/sizeof(float);

IStat::IStat (const char* host, int delay, int w, int h) : () {
    input = new Sensor();
    input->Catch(KeyEvent);
    input->CatchTimer(delay, 0);

    sprintf(bannersample, "%s 00.0", host);
    sprintf(bannerformat, "%s %%4.1f", host);
    st = new Stats(host);

    Loads = new Log(new Trace);
    CPU = new Log(new Trace, new Trace, new Trace);
    IO = new Log(new Trace, new Trace);

    width = w;
    height = h;

    Readout* CurrentLoad = new Readout(Loads, bannersample, bannerformat);
    Graph* LoadHistory = new Graph(Loads);
    LoadHistory->Scaling(LoadScaleCount, 0, LoadScales);
    Bar* CurrentCPU = new Bar(CPU, 8);
    Pointer* CurrentIO = new Pointer(IO);
    CurrentIO->Scaling(IOScaleCount, 0, IOScales);
    CurrentIO->Scalers(&IOScaleUp, &IOScaleDown);

    Insert(
	new VBox(
	    new HBox(
		new HGlue(1, 0, 0),
		CurrentLoad,
		new HGlue(1, 0, hfil)
	    ),
	    new VGlue(1, 0, 0),
	    new HBox(
		new HGlue(1, 0, 0),
		CurrentIO,
		new HGlue(1, 0, 0),
		LoadHistory,
		new HGlue(1, 0, 0),
		CurrentCPU,
		new HGlue(1, 0, 0)
	    ),
	    new VGlue(1, 0, 0)
	)
    );
}

void IStat::Reconfig () {
    MonoScene::Reconfig();
    if (width != 0 && height != 0) {
	shape->Rect(width, height);
    }
}

IStat::~IStat () {
    delete Loads;
    delete CPU;
    delete IO;
}

void IStat::Run () {
    Event e;

    do {
	Read(e);
	e.target->Handle(e);
    } while (e.target != nil);
}

void IStat::Handle (Event& e) {
    switch (e.eventType) {
	case TimerEvent:
	    Tick();
	    break;
	case KeyEvent:
	    if (e.len > 0 && e.keystring[0] == 'q') {
		e.target = nil;
	    }
	    break;
	default:
	    break;
    }
}

void IStat::Tick () {
    st->NewStats();
    Loads->GetTrace(1)->Data(st->load1Av);
    Loads->Tick();
    CPU->GetTrace(1)->Data(st->userCPUFract);
    CPU->GetTrace(2)->Data(st->niceCPUFract);
    CPU->GetTrace(3)->Data(st->systemCPUFract);
    CPU->Tick();
    IO->GetTrace(1)->Data(st->diskIORate);
    IO->GetTrace(2)->Data(st->netIORate);
    IO->Tick();
}
