#include "desktop.h"
#include "dispatch.h"
#include <InterViews/world.h>
#include <osfcn.h>
#include <stdio.h>
#include <stdlib.h>

extern boolean Equal(const char*, const char*);

char* program_name;
World* world;

int main (int argc, char* argv[]) {
    int pid;
    Desktop* v;
    boolean dofork = true;
    const char* s;

    program_name = "iwm";
    world = new World(program_name, argc, argv);
    s = world->GetAttribute("fork");
    if (s != nil && Equal(s, "false")) {
	dofork = false;
    }
    v = new Desktop(world);
    if (dofork) {
	pid = fork();
	if (pid < 0) {
	    fprintf(stderr, "iwm: cannot fork\n");
	    exit(1);
	} else if (pid == 0) {
	    /* child closes fds to let parent exit */
	    close(0); close(1); close(2);
	} else {
	    /* parent just exits and lets child takeover */
	    exit(0);
	}
    }
    v->Run();
    return 0;
}
