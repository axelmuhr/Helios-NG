/*
 *  alert - displays a message in a dialog box at centre screen
 */

#include <InterViews/button.h>
#include <InterViews/box.h>
#include <InterViews/dialog.h>
#include <InterViews/font.h>
#include <InterViews/frame.h>
#include <InterViews/glue.h>
#include <InterViews/message.h>
#include <InterViews/painter.h>
#include <InterViews/world.h>
#include <stdio.h>
#include <string.h>

static PropertyData props[] = {
    { "*quitbutton", "OK, OK ..." },
    { "*font", "9x15" },
    { "*transient", "on" },
    { nil }
};

static OptionDesc options[] = {
    { "font=", "*font", OptionValueAfter },
    { "button=", "*quitbutton", OptionValueAfter },
    { "-top", "*transient", OptionValueImplicit, "off" },
    { nil }
};

static Interactor* MakeDialog(ButtonState*, const char*);

int main (int argc, char* argv[]) {
    World* world = new World("alert", props, options, argc, argv);

    ButtonState* quit = new ButtonState;
    Interactor* body = MakeDialog(quit, world->GetAttribute("quitbutton"));

    Dialog* dialog;
    Coord x = world->Width() / 2;
    Coord y = world->Height() / 2;
    if (strcmp(world->GetAttribute("transient"), "on") == 0) {
	dialog = new Dialog(quit, new ShadowFrame(body));
	world->InsertTransient(dialog, dialog, x, y, Center);
    } else {
	dialog = new Dialog(quit, body);
	world->Insert(dialog, x, y, Center);
    }
    world->RingBell(0);
    return dialog->Accept() ? 0 : 1;
}

static Interactor* MakeMessage(Button*);

static Interactor* MakeDialog (ButtonState* quit, const char* label) {
    return new VBox(
	new VGlue(round(.25*inch)),
	new HBox(
	    new HGlue(round(.5*inch)),
	    MakeMessage(new PushButton(label, quit, true)),
	    new HGlue(round(.5*inch))
	),
	new VGlue(round(.25*inch))
    );
}

static Interactor* MakeMessage (Button* quit) {
    VBox* b;
    char buffer[1024];
    char* s;
    int n;

    b = new VBox;
    while (fgets(buffer, sizeof(buffer), stdin) != nil) {
	n = strlen(buffer) - 1;
	if (buffer[n] == '\n') {
	    buffer[n] = '\0';
	} else {
	    ++n;
	}
	s = new char[n + 1];
	strcpy(s, buffer);
	b->Insert(new HBox(new Message(s), new HGlue));
    }
    b->Insert(new VGlue(round(.25*inch)));
    b->Insert(new HBox(new HGlue, quit));
    return b;
}
