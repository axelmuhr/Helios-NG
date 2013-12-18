/*
 * istat : a workstation statistics display for InterViews
 */

#include "stats.h"
#include <InterViews/frame.h>
#include <InterViews/sensor.h>
#include <InterViews/world.h>
#include <os/host.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static PropertyData props[] = {
    { "*host", "" },
    { "*delay", "5" },
    { "*font", "6x10" },
    { "*geometry", "100x60" },
    { nil }
};

static OptionDesc options[] = {
    { "-delay", "*delay", OptionValueNext },
    { "-host", "*host", OptionValueNext },
    { nil }
};

static char hostname[64];

int main (int argc, char* argv[]) {
    World* world = new World("IStat", props, options, argc, argv);
    if (argc > 1) {
        fprintf(stderr, "usage: %s [-delay sec] [-host name]\n", argv[0]);
        exit(1);
    }

    int delay = atoi(world->GetAttribute("delay"));
    strcpy(hostname, world->GetAttribute("host"));
    if (hostname[0] == '\0') {
	gethostname(hostname, sizeof(hostname));
    }

    IStat* istat = new IStat(hostname, delay);
    world->InsertApplication(istat);
    istat->Run();
    return 0;
}
