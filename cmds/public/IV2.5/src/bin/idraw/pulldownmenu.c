/*
 * Implements pulldown menu classes.
 */

#include "istring.h"
#include "pulldownmenu.h"
#include <InterViews/event.h>
#include <InterViews/font.h>
#include <InterViews/frame.h>
#include <InterViews/painter.h>
#include <InterViews/sensor.h>
#include <InterViews/shape.h>
#include <InterViews/world.h>
#include <bstring.h>

/* Define the initial number of elements to allocate space for. */

static const int INITIALSIZE = 15;

/*
 * PullDownMenuBar starts with no currently active or stored
 * activators although it allocates some initial space to store them.
 */

PullDownMenuBar::PullDownMenuBar () {
    cur = nil;
    sizeactivators = INITIALSIZE;
    numactivators = 0;
    activators = new PullDownMenuActivator*[sizeactivators];
}

/*
 * Free storage allocated for the dynamic array.
 */

PullDownMenuBar::~PullDownMenuBar () {
    delete activators;
}

/*
 * Enter stores an interior activator and tells it it can get its
 * highlight painter from us.
 */

void PullDownMenuBar::Enter (PullDownMenuActivator* act) {
    if (numactivators == sizeactivators) {
	GrowActivators();
    }
    activators[numactivators++] = act;
    act->SetHighlighterParent(this);
}

/*
 * Contains returns true if the child is one of the bar's activators
 * or one of the activators' commands.
 */

boolean PullDownMenuBar::Contains (Interactor* child) {
    for (int i = 0; i < numactivators; i++) {
	if (activators[i] == child) {
	    return true;
	}
    }

    for (i = 0; i < numactivators; i++) {
	if (activators[i]->Contains(child)) {
	    return true;
	}
    }

    return false;
}

/*
 * MenuActive returns true if any of the bar's activators have opened
 * a menu.
 */

boolean PullDownMenuBar::MenuActive () {
    return cur != nil;
}

/*
 * MenuShouldActivate returns true if any of the bar's activators
 * EXCEPT the given one have opened a menu.
 */

boolean PullDownMenuBar::MenuShouldActivate (PullDownMenuActivator* act) {
    return cur != nil && cur != act;
}

/*
 * MenuActivate highlights the given activator, opens the activator's
 * menu, and stores it as the currently active activator.
 */

void PullDownMenuBar::MenuActivate (PullDownMenuActivator* act) {
    act->Highlight();
    act->Open();
    cur = act;
}

/*
 * MenuDeactivate closes the currently active activator's menu,
 * unhighlights the activator, and stores no more active activators.
 */

void PullDownMenuBar::MenuDeactivate () {
    cur->Close();
    cur->Unhighlight();
    cur = nil;
}

/*
 * GrowActivators increases the dynamic array's size to make room for
 * more activators to be stored.
 */

void PullDownMenuBar::GrowActivators () {
    PullDownMenuActivator** oldacts = activators;
    sizeactivators += INITIALSIZE/2;
    activators = new PullDownMenuActivator*[sizeactivators];
    bcopy(oldacts, activators, numactivators * sizeof(PullDownMenuActivator*));
    delete oldacts;
}

/*
 * PullDownMenuActivator stores the bar it belongs to and its text
 * label.  It starts off with an empty menu and no stored commands
 * although it allocates some initial space to store the commands.  It
 * must catch the same mouse button PullDownMenuCommand catches.
 */

PullDownMenuActivator::PullDownMenuActivator (PullDownMenuBar* b,
const char* n) {
    b->Enter(this);
    bar = b;
    name = strdup(n ? n : "");
    menu = new ShadowFrame;
    menu->SetCanvasType(CanvasSaveUnder);
    sizecommands = INITIALSIZE;
    numcommands = 0;
    commands = new PullDownMenuCommand*[sizecommands];
    input = new Sensor(onoffEvents);
    input->CatchButton(DownEvent, LEFTMOUSE);
    input->CatchButton(UpEvent, LEFTMOUSE);
}

/*
 * Free storage allocated for members.
 */

PullDownMenuActivator::~PullDownMenuActivator () {
    delete name;
    delete menu;
    delete commands;
}

/*
 * SetMenu sets the menu's actual contents.
 */

void PullDownMenuActivator::SetMenu (Scene* box) {
    menu->Insert(box);
}

/*
 * Handle works together with the bar to determine whether any
 * activator has opened or should open a menu and accordingly tells
 * the bar to deactivate or activate an activator.
 */

void PullDownMenuActivator::Handle (Event& e) {
    switch (e.eventType) {
    case DownEvent:
	if (!bar->MenuActive()) {
	    bar->MenuActivate(this);
	    while (e.eventType != UpEvent) {
		Read(e);
		if (e.eventType == UpEvent) {
		    bar->MenuDeactivate();
		}
		if (bar->Contains(e.target)) {
		    e.target->Handle(e);
		}
	    }
	}
	break;
    case UpEvent:
	break;
    case OnEvent:
	if (bar->MenuShouldActivate(this)) {
	    bar->MenuDeactivate();
	    bar->MenuActivate(this);
	}
	break;
    case OffEvent:
	break;
    }
}

/*
 * Enter stores an interior command and tells it it can get its
 * highlight painter from our bar like us too.
 */

void PullDownMenuActivator::Enter (PullDownMenuCommand* cmd) {
    if (numcommands == sizecommands) {
	GrowCommands();
    }
    commands[numcommands++] = cmd;
    cmd->SetHighlighterParent(bar);
}

/*
 * Contains returns true if the command is one of the activator's
 * commands.
 */

boolean PullDownMenuActivator::Contains (Interactor* cmd) {
    for (int i = 0; i < numcommands; i++) {
	if (commands[i] == cmd) {
	    return true;
	}
    }
    return false;
}

/*
 * Open inserts the menu into the scene with the menu's top left
 * corner aligned with the activator's bottom left corner.
 */

void PullDownMenuActivator::Open () {
    World* world = GetWorld();
    Coord l = 0;
    Coord b = 0;
    GetRelative(l, b, world);
    world->InsertPopup(menu, l, b, TopLeft);
}

/*
 * Close makes the activator's menu disappear.
 */

void PullDownMenuActivator::Close () {
    menu->Parent()->Remove(menu);
}

/*
 * Reconfig pads the activator's shape to accomodate its text label.
 * Basing padding on the font in use ensures the padding will change
 * proportionally with changes in the font's size.
 */

static const float WIDTHPAD = 1.0;/* fraction of font->Width(EM) */
static const float CMDHTPAD = 0.1;/* fraction of font->Height() */
static const float ACTHTPAD = 0.2;/* fraction of font->Height() */
static const char* EM = "m";	  /* widest alphabetic character in any font */

void PullDownMenuActivator::Reconfig () {
    Highlighter::Reconfig();
    Font* font = output->GetFont();
    int xpad = round(WIDTHPAD * font->Width(EM));
    int ypad = round(ACTHTPAD * font->Height());
    shape->width = font->Width(name) + (2 * xpad);
    shape->height = font->Height() + (2 * ypad);
    shape->Rigid(shape->width - xpad, 0, 2 * ypad, 0);
}

/*
 * Redraw displays the text label.
 */

void PullDownMenuActivator::Redraw (Coord l, Coord b, Coord r, Coord t) {
    output->ClearRect(canvas, l, b, r, t);
    output->Text(canvas, name, name_x, name_y);
}

/*
 * Resize calculates the text label's position.
 */

void PullDownMenuActivator::Resize () {
    Font* font = output->GetFont();
    name_x = max(0, (xmax - font->Width(name) + 1) / 2);
    name_y = (ymax - font->Height() + 1) / 2;
}

/*
 * GrowCommands increases the dynamic array's size to make room for
 * more commands to be stored.
 */

void PullDownMenuActivator::GrowCommands () {
    PullDownMenuCommand** oldcmds = commands;
    sizecommands += INITIALSIZE/2;
    commands = new PullDownMenuCommand*[sizecommands];
    bcopy(oldcmds, commands, numcommands * sizeof(PullDownMenuCommand*));
    delete oldcmds;
}

/*
 * PullDownMenuCommand stores the activator it belongs to and its text
 * labels.  It catches only one mouse button to prevent the user from
 * accidentally executing a command upon another button's release.
 */

PullDownMenuCommand::PullDownMenuCommand (PullDownMenuActivator* a,
const char* n, const char* k) {
    if (a != nil) {
	a->Enter(this);
    }
    activator = a;
    name = strdup(n ? n : "");
    key = strdup(k ? k : "");
    input = new Sensor(onoffEvents);
    input->CatchButton(UpEvent, LEFTMOUSE);
}

PullDownMenuCommand::~PullDownMenuCommand () {
    delete name;
    delete key;
}

/*
 * Highlight or unhighlight the command when the mouse passes
 * over and execute the command when the user releases the button
 * types its associated character (mapped by the program).
 */

void PullDownMenuCommand::Handle (Event& e) {
    switch (e.eventType) {
    case OnEvent:
	Highlight();
	break;
    case OffEvent:
	Unhighlight();
	break;
    case UpEvent:
	Execute(e);
	Unhighlight();
	break;
    case KeyEvent:
	activator->Highlight();
	activator->Flush();
	Execute(e);
	activator->Unhighlight();
	break;
    }
}

/*
 * Execute carries out the command's purpose.
 */

void PullDownMenuCommand::Execute (Event&) {
    /* define it in your subclass */
}

/*
 * Reconfig pads the command's shape to accomodate its text labels.
 */

void PullDownMenuCommand::Reconfig () {
    Highlighter::Reconfig();
    Font* font = output->GetFont();
    int xpad = round(WIDTHPAD * font->Width(EM));
    int ypad = round(CMDHTPAD * font->Height());
    shape->width = font->Width(name) + (2 * xpad);
    shape->width += font->Width(key) + (2 * xpad);
    shape->height = font->Height() + (2 * ypad);
    shape->Rigid(0, hfil, 0, 0);
}

/*
 * Redraw displays the text labels.
 */

void PullDownMenuCommand::Redraw (Coord l, Coord b, Coord r, Coord t) {
    output->ClearRect(canvas, l, b, r, t);
    output->Text(canvas, name, name_x, name_y);
    output->Text(canvas, key, key_x, key_y);
}

/*
 * Resize calculates the text labels' positions.
 */

void PullDownMenuCommand::Resize () {
    Font* font = output->GetFont();
    int xpad = round(WIDTHPAD * font->Width(EM));
    name_x = xpad;
    name_y = (ymax - font->Height() + 1) / 2;
    key_x = xmax - font->Width(key) - xpad;
    key_y = name_y;
}

/*
 * PullDownMenuDivider listens to no events so it will neither
 * highlight itself nor execute any command.
 */

PullDownMenuDivider::PullDownMenuDivider () : (nil, nil, nil) {
    Listen(noEvents);
}

/*
 * Redraw displays a horizontal line spanning the canvas.
 */

void PullDownMenuDivider::Redraw (Coord l, Coord b, Coord r, Coord t) {
    output->ClearRect(canvas, l, b, r, t);
    Coord hy = ymax / 2;
    output->Line(canvas, l, hy, r, hy);
}
