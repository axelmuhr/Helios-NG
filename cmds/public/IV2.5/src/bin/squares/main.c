/*
 * InterViews squares demo program.
 */

#include "sframe.h"
#include "squares.h"
#include "view.h"
#include <InterViews/sensor.h>
#include <InterViews/world.h>

static Squares* MakeInitialSquares();
static SquaresView* MakeInitialView();

static PropertyData props[] = {
    { "*adjustersize", "s" },
    { nil }
};

static OptionDesc options[] = {
    { "-panner", "*panner", OptionValueImplicit, "0" },
    { "-above", "*above", OptionValueImplicit, "true" },
    { "-left", "*left", OptionValueImplicit, "true" },
    { nil }
};

int main (int argc, char* argv[]) {
    World* world = new World("Squares", props, options, argc, argv);
    world->InsertApplication(new SquaresFrame(MakeInitialView()));
    world->Run();
    return 0;
}

static Squares* MakeInitialSquares () {
    const initnumsquares = 3;
    Squares* s;
    int i;

    s = new Squares;
    for (i = 0; i < initnumsquares; i++) {
	s->Add();
    }
    return s;
}

static SquaresView* MakeInitialView () {
    return new SquaresView(MakeInitialSquares());
}
