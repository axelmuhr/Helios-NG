// $Header: main.c,v 1.10 89/06/05 14:25:49 interran Exp $
// runs idraw.

#include "idraw.h"
#include <InterViews/world.h>

// Predefine default properties for the window size, paint menus, and
// history.

static PropertyData properties[] = {
    { "*font1",		"*-courier-medium-r-*-80-*    Courier 8" },
    { "*font2",		"*-courier-medium-r-*-100-*   Courier 10" },
    { "*font3",		"*-courier-bold-r-*-120-*     Courier-Bold 12" },
    { "*font4",		"*-helvetica-medium-r-*-120-* Helvetica 12" },
    { "*font5",		"*-helvetica-medium-r-*-140-* Helvetica 14" },
    { "*font6",		"*-helvetica-bold-r-*-140-*   Helvetica-Bold 14" },
    { "*font7",		"*-helvetica-medium-o-*-140-* Helvetica-Oblique 14" },
    { "*font8",		"*-times-medium-r-*-120-*     Times-Roman 12" },
    { "*font9",		"*-times-medium-r-*-140-*     Times-Roman 14" },
    { "*font10",	"*-times-bold-r-*-140-*       Times-Bold 14" },
    { "*font11",	"*-times-medium-i-*-140-*     Times-Italic 14" },
    { "*brush1",	"none" },
    { "*brush2",	"ffff 1 0 0" },
    { "*brush3",	"ffff 1 1 0" },
    { "*brush4",	"ffff 1 0 1" },
    { "*brush5",	"ffff 1 1 1" },
    { "*brush6",	"3333 1 0 0" },
    { "*brush7",	"3333 2 0 0" },
    { "*brush8",	"ffff 2 0 0" },
    { "*pattern1",	"none" },
    { "*pattern2",	"0.0" },
    { "*pattern3",	"1.0" },
    { "*pattern4",	"0.75" },
    { "*pattern5",	"0.5" },
    { "*pattern6",	"0.25" },
    { "*pattern7",	"1248" },
    { "*pattern8",	"8421" },
    { "*pattern9",	"f000" },
    { "*pattern10",	"8888" },
    { "*pattern11",	"f888" },
    { "*pattern12",	"8525" },
    { "*pattern13",	"cc33" },
    { "*pattern14",	"7bed" },
    { "*fgcolor1",	"Black" },
    { "*fgcolor2",	"Brown 42240 10752 10752" },
    { "*fgcolor3",	"Red" },
    { "*fgcolor4",	"Orange" },
    { "*fgcolor5",	"Yellow" },
    { "*fgcolor6",	"Green" },
    { "*fgcolor7",	"Blue" },
    { "*fgcolor8",	"Indigo 48896 0 65280" },
    { "*fgcolor9",	"Violet 20224 12032 20224" },
    { "*fgcolor10",	"White" },
    { "*fgcolor11",	"LtGray 50000 50000 50000" },
    { "*fgcolor12",	"DkGray 33000 33000 33000" },
    { "*bgcolor1",	"Black" },
    { "*bgcolor2",	"Brown 42240 10752 10752" },
    { "*bgcolor3",	"Red" },
    { "*bgcolor4",	"Orange" },
    { "*bgcolor5",	"Yellow" },
    { "*bgcolor6",	"Green" },
    { "*bgcolor7",	"Blue" },
    { "*bgcolor8",	"Indigo 48896 0 65280" },
    { "*bgcolor9",	"Violet 20224 12032 20224" },
    { "*bgcolor10",	"White" },
    { "*bgcolor11",	"LtGray 50000 50000 50000" },
    { "*bgcolor12",	"DkGray 33000 33000 33000" },
    { "*initialfont",	"2" },
    { "*initialbrush",	"2" },
    { "*initialpattern","2" },
    { "*initialfgcolor","1" },
    { "*initialbgcolor","10" },
    { "*history",	"20" },
    { "*reverseVideo",	"off" },
    { "*small",		"true" },
    { nil }
};

// Define window size options.

static OptionDesc options[] = {
    { "-l", "*small", OptionValueImplicit, "false" },
    { "-s", "*small", OptionValueImplicit, "true" },
    { nil }
};

// main creates a connection to the display server, creates idraw, and
// opens idraw's window.  After idraw stops running, main closes
// idraw's window, deletes everything it created, and returns success.

int main (int argc, char** argv) {
    World* world = new World("Idraw", properties, options, argc, argv);
    Idraw* idraw = new Idraw(argc, argv);

    world->InsertApplication(idraw);
    idraw->Run();
    world->Remove(idraw);

    delete idraw;
    delete world;

    const int SUCCESS = 0;
    return SUCCESS;
}
