// $Header: dialogbox.c,v 1.9 89/05/12 14:59:26 calder Exp $
// implements class DialogBox and DialogBox subclasses.

#include "dialogbox.h"
#include "istring.h"
#include <InterViews/Text/stringedit.h>
#include <InterViews/box.h>
#include <InterViews/button.h>
#include <InterViews/canvas.h>
#include <InterViews/event.h>
#include <InterViews/font.h>
#include <InterViews/frame.h>
#include <InterViews/glue.h>
#include <InterViews/message.h>
#include <InterViews/painter.h>
#include <InterViews/sensor.h>
#include <InterViews/shape.h>
#include <InterViews/world.h>

// An IMessage displays its own text, not somebody else's.

class IMessage : public Message {
public:
    IMessage(const char* = nil, Alignment a = Center);
    ~IMessage();

    void SetText(const char* = nil, const char* = nil);
protected:
    char* buffer;		// stores own copy of text
};

// IMessage creates a buffer to store its own copy of the text.

IMessage::IMessage (const char* msg, Alignment a) : (nil, a) {
    buffer = strdup(msg ? msg : "");
    text = buffer;
}

// Free storage allocated for the text buffer.

IMessage::~IMessage () {
    delete buffer;
}

// SetText stores the new text and changes the IMessage's shape to fit
// the new text's width.

void IMessage::SetText (const char* beg, const char* end) {
    beg = beg ? beg : "";
    end = end ? end : "";
    delete buffer;
    buffer = new char[strlen(beg) + strlen(end) + 1];
    strcpy(buffer, beg);
    strcat(buffer, end);
    text = buffer;
    if (canvas != nil && canvas->Status() == CanvasMapped) {
	Reconfig();
	Parent()->Change(this);
    }
}

// DialogBox creates two IMessages to display a message and a warning
// and stores its underlying Interactor.  DialogBox won't delete the
// IMessages so its derived classes can put them in boxes which will
// delete them when the boxes are deleted.

DialogBox::DialogBox (Interactor* u, const char* msg) {
    SetCanvasType(CanvasSaveUnder); // speed up expose redrawing if possible
    input = allEvents;
    input->Reference();
    message = new IMessage(msg);
    warning = new IMessage;
    underlying = u;
}

// SetMessage sets the message's text.

void DialogBox::SetMessage (const char* beg, const char* end) {
    message->SetText(beg, end);
}

// SetWarning sets the warning's text.

void DialogBox::SetWarning (const char* beg, const char* end) {
    warning->SetText(beg, end);
}

// SetUnderlying sets the underlying Interactor over which the
// DialogBox will pop up itself.

void DialogBox::SetUnderlying (Interactor* u) {
    underlying = u;
}

// PopUp pops up the DialogBox centered over the underlying
// Interactor's canvas.

void DialogBox::PopUp () {
    World* world = underlying->GetWorld();
    Coord x, y;
    underlying->Align(Center, 0, 0, x, y);
    underlying->GetRelative(x, y, world);
    world->InsertTransient(this, underlying, x, y, Center);
}

// Disappear removes the DialogBox.  Since the user should see
// warnings only once, Disappear clears the warning's text so the next
// PopUp won't display it.

void DialogBox::Disappear () {
    Parent()->Remove(this);
    SetWarning();
    Sync();
}

// Messager creates its button state and initializes its view.

Messager::Messager (Interactor* u, const char* msg) : (u, msg) {
    ok = new ButtonState(false);
    okbutton = new PushButton("  OK  ", ok, true);
    Init();
}

// Free storage allocated for the message's button state.
// Okbutton detach itself from the button state when deleting itself
// so we have to delete the state last of all.

Messager::~Messager () {
    delete ok;
}

// Display pops up the Messager and removes it when the user
// acknowledges the message.

void Messager::Display () {
    ok->SetValue(false);

    PopUp();

    int okay = false;
    while (!okay) {
	Event e;
	Read(e);
	if (e.eventType == KeyEvent && e.len > 0) {
	    switch (e.keystring[0]) {
	    case '\r':		// CR
	    case '\007':	// ^G
		ok->SetValue(true);
		break;
	    default:
		break;
	    }
	} else if (e.target == okbutton) {
	    e.target->Handle(e);
	}
	ok->GetValue(okay);
    }

    Disappear();
}

// Init composes Messager's view with boxes, glue, and frames.

void Messager::Init () {
    SetClassName("Messager");

    VBox* vbox = new VBox;
    vbox->Align(Center);
    vbox->Insert(new VGlue);
    vbox->Insert(warning);
    vbox->Insert(new VGlue);
    vbox->Insert(message);
    vbox->Insert(new VGlue);
    vbox->Insert(okbutton);
    vbox->Insert(new VGlue);

    Insert(new Frame(vbox, 2));
}

// Skew comments/code ratio to work around cpp bug
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

// Reconfig pads Messager's shape to make the view look less crowded.

void Messager::Reconfig () {
    DialogBox::Reconfig();
    Font* font = output->GetFont();
    shape->width += 2 * font->Width("mmmm");
    shape->height += 4 * font->Height();
}

// Confirmer creates its button states and initializes its view.

Confirmer::Confirmer (Interactor* u, const char* prompt) : (u, prompt) {
    yes		 = new ButtonState(false);
    no		 = new ButtonState(false);
    cancel	 = new ButtonState(false);
    yesbutton    = new PushButton(" Yes  ", yes, true);
    nobutton     = new PushButton("  No  ", no, true);
    cancelbutton = new PushButton("Cancel", cancel, true);
    Init();
}

// Free storage allocated for the button states.  Yesbutton,
// nobutton, and cancelbutton detach themselves from the button states
// when deleting themselves so we have to delete the states last.

Confirmer::~Confirmer () {
    delete yes;
    delete no;
    delete cancel;
}

// Confirm pops up the Confirmer, lets the user confirm the message or
// not, removes the Confirmer, and returns the confirmation.

char Confirmer::Confirm () {
    yes->SetValue(false);
    no->SetValue(false);
    cancel->SetValue(false);

    PopUp();

    int confirmed = false;
    int denied = false;
    int cancelled = false;
    while (!confirmed && !denied && !cancelled) {
	Event e;
	Read(e);
	if (e.eventType == KeyEvent && e.len > 0) {
	    switch (e.keystring[0]) {
	    case 'y':
	    case 'Y':
		yes->SetValue(true);
		break;
	    case 'n':
	    case 'N':
		no->SetValue(true);
		break;
	    case '\r':		// CR
	    case '\007':	// ^G
		cancel->SetValue(true);
		break;
	    default:
		break;
	    }
	} else if (e.target == yesbutton || e.target == nobutton ||
		   e.target == cancelbutton)
	{
	    e.target->Handle(e);
	}
	yes->GetValue(confirmed);
	no->GetValue(denied);
	cancel->GetValue(cancelled);
    }

    Disappear();

    char answer = 'n';
    answer = confirmed ? 'y' : answer;
    answer = cancelled ? 'c' : answer;
    return answer;
}

// Init composes Confirmer's view with boxes, glue, and frames.

void Confirmer::Init () {
    SetClassName("Confirmer");

    HBox* buttons = new HBox;
    buttons->Insert(new HGlue);
    buttons->Insert(yesbutton);
    buttons->Insert(new HGlue);
    buttons->Insert(nobutton);
    buttons->Insert(new HGlue);
    buttons->Insert(cancelbutton);
    buttons->Insert(new HGlue);

    VBox* vbox = new VBox;
    vbox->Align(Center);
    vbox->Insert(new VGlue);
    vbox->Insert(warning);
    vbox->Insert(new VGlue);
    vbox->Insert(message);
    vbox->Insert(new VGlue);
    vbox->Insert(buttons);
    vbox->Insert(new VGlue);

    Insert(new Frame(vbox, 2));
}

// Reconfig pads Confirmer's shape to make the view look less crowded.

void Confirmer::Reconfig () {
    DialogBox::Reconfig();
    Font* font = output->GetFont();
    shape->width += 4 * font->Width("mmmm");
    shape->height += 4 * font->Height();
}

// Namer creates its button states and initializes its view.

Namer::Namer (Interactor* u, const char* prompt) : (u, prompt) {
    accept = new ButtonState(false);
    cancel = new ButtonState(false);
    acceptbutton = new PushButton("  OK  ", accept, true);
    cancelbutton = new PushButton("Cancel", cancel, true);
    const char* sample = "                                                 ";
    stringeditor = new StringEdit(sample, 3);
    stringeditor->SetString("");
    stringeditor->Extra(15);
    Init();
}

// Free storage allocated for the button states.  Acceptbutton
// and cancelbutton detach themselves from the button states when
// deleting themselves so we have to delete the states last of all.

Namer::~Namer () {
    delete accept;
    delete cancel;
}

// Edit pops up the Namer, lets the user edit the given string,
// removes the Namer, and returns the edited string unless the user
// cancelled it.

char* Namer::Edit (const char* string) {
    accept->SetValue(false);
    cancel->SetValue(false);
    stringeditor->SetString(string);

    PopUp();

    int accepted = false;
    int cancelled = false;
    while (!accepted && !cancelled) {
	Event e;
	e.eventType = OnEvent;
	stringeditor->Handle(e);
	Read(e);
	if (e.eventType == KeyEvent && e.len > 0) {
	    if (e.keystring[0] == '\r') {
		accept->SetValue(true);
	    } else {
		cancel->SetValue(true);
	    }
	} else if (e.target == acceptbutton || e.target == cancelbutton) {
	    e.target->Handle(e);
	}
	accept->GetValue(accepted);
	cancel->GetValue(cancelled);
    }

    Disappear();

    char* result = nil;
    if (accepted) {
	result = stringeditor->GetString();
	if (result[0] == '\0') {
	    delete result;
	    result = nil;
	}
    }
    return result;
}

// Init composes Namer's view with boxes, glue, and frames.

void Namer::Init () {
    SetClassName("Namer");

    HBox* buttons = new HBox;
    buttons->Insert(new HGlue);
    buttons->Insert(acceptbutton);
    buttons->Insert(new HGlue);
    buttons->Insert(cancelbutton);
    buttons->Insert(new HGlue);

    VBox* vbox = new VBox;
    vbox->Align(Center);
    vbox->Insert(new VGlue);
    vbox->Insert(warning);
    vbox->Insert(new VGlue);
    vbox->Insert(message);
    vbox->Insert(new VGlue);
    vbox->Insert(new Frame(stringeditor, 1));
    vbox->Insert(new VGlue);
    vbox->Insert(buttons);
    vbox->Insert(new VGlue);
    vbox->Propagate(false);	// so we can reshape stringeditor w/o looping

    Insert(new Frame(vbox, 2));
}

// Reconfig pads Namer's shape to make the view look less crowded.

void Namer::Reconfig () {
    DialogBox::Reconfig();
    Shape s = *stringeditor->GetShape();
    s.Rigid();
    stringeditor->Reshape(s);
    Font* font = output->GetFont();
    shape->width += 2 * font->Width("mmmm");
    shape->height += 4 * font->Height();
}
