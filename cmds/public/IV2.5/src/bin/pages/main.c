/*
 * a demonstration of Text, Layout and TextBlock
 */

const char* PAGESFILE = "PAGESTEXT";
const char* DEBUG = "DEBUG";
const char* DEFAULTFILE = "pages.text";

#include <InterViews/world.h>
#include <InterViews/frame.h>
#include <stdio.h>
#include <stdlib.h>
#include "pages.h"

int main (int argc, char* argv[]) {
    World* world = new World("Pages", argc, argv);
    const char* textfile = getenv(PAGESFILE);
    if (textfile == nil) {
        textfile = DEFAULTFILE;
    }
    FILE* f = fopen(textfile, "r");
    if (f == nil) {
	fprintf(stderr, "%s : can't open file \"%s\"\n", argv[0], textfile);
	exit(1);
    }
    int pages = 4;
    int columns = 2;
    for (int i = 1; i < argc; ++i) {
        sscanf(argv[i], "-c%d", &columns);
        sscanf(argv[i], "-p%d", &pages);
    }

    Frame* frame = new Frame(new Pages(f, pages, columns), 2);
    world->InsertApplication(frame);
    world->Run();
    return 0;
}
