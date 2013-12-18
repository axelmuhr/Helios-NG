#include <InterViews/box.h>
#include <InterViews/cursor.h>
#include <InterViews/glue.h>
#include <InterViews/paint.h>
#include <InterViews/painter.h>
#include <InterViews/sensor.h>
#include <os/auth.h>
#include <string.h>
#include "lock.h"

static CursorPattern PadlockBits = {
   0x0000, 0x03c0, 0x0420, 0x0810,
   0x0810, 0x0810, 0x0810, 0x1ff8,
   0x1ff8, 0x1ff8, 0x1ff8, 0x1ff8,
   0x1ff8, 0x1ff8, 0x0000, 0x0000
};

static CursorPattern PadlockMask = {
   0x03c0, 0x07e0, 0x0ff0, 0x1e78,
   0x1c38, 0x1c38, 0x3ffc, 0x3ffc,
   0x3ffc, 0x3ffc, 0x3ffc, 0x3ffc,
   0x3ffc, 0x3ffc, 0x3ffc, 0x0000
};
 
static Cursor* padlock;

class Message : public Interactor {
    const char* message;
public:
    Message (Painter* p, const char *s);
    void Draw();
    void Redraw(Coord, Coord, Coord, Coord);
};

Message::Message(Painter *p, const char *s) : (nil, p) {
    message = s;
    shape->width = output->GetFont()->Width(s);
    shape->height = output->GetFont()->Height();
    shape->Rigid(0, hfil, 0, 0);
}

void Message::Draw() {
    output->ClearRect(canvas, 0, 0, xmax, ymax);
    output->MoveTo((xmax-shape->width)/2, (ymax-shape->height)/2);
    output->Text(canvas, message);
}

void Message::Redraw(Coord x1, Coord y1, Coord x2, Coord y2) {
    output->ClearRect(canvas, x1, y1, x2, y2);
    output->MoveTo((xmax-shape->width)/2, (ymax-shape->height)/2);
    output->Text(canvas, message);
}

ScreenLock::ScreenLock(Painter* p) : (nil, p)
{
    padlock = new Cursor(7, 9, PadlockBits, PadlockMask, black, white);

    passwd* pwentry = getpwuid(getuid());
    strcpy(username, pwentry->pw_gecos);
    strcpy(userpasswd, pwentry->pw_passwd);
    pwentry = getpwuid(0);
    strcpy(rootpasswd, pwentry->pw_passwd);

    VBox* contents = new VBox;
    contents->Insert(new VGlue(output, 20, 0, 0));
    contents->Insert(new Message(output,"This workstation is locked by"));
    contents->Insert(new Message(output, username));
    contents->Insert(new VGlue(output, 5, 0, 0));
    contents->Insert(new Message(output,"Enter login password ..."));
    contents->Insert(new VGlue(output, 20, 0, 0));

    lockdialog = new Frame(
	output,
	new HBox(
	    new HGlue(output, 30, 0, 0),
	    contents,
	    new HGlue(output, 30, 0, 0)
	),
	3
    );

    Sensor* keys = new Sensor;
    keys->Catch(KeyEvent);
    lockdialog->Listen(keys);
}

Cursor* ScreenLock::LockCursor()
{
    return(padlock);
}

void ScreenLock::Activate()
{
    const int PWSIZE = 10;
    char buffer[PWSIZE];		/* holds entered password */
    boolean broken = false;

    while (!broken) {
	Event e;
	int k = 0;
	char c;
	for (;;) {
	    lockdialog->Read(e);
	    if (e.eventType == KeyEvent) {
		if (e.keystring != nil) {
		    c = e.keystring[0];
		    if ( c == '\n' || c == '\r' || k == PWSIZE-1 ) {
			buffer[k] = '\0';
			break;
		    } else {
			buffer[k] = c;
			++k;
		    }
		}
	    }
	}
	if (strcmp(crypt(buffer,userpasswd), userpasswd)==0 ||
	    strcmp(crypt(buffer,rootpasswd), rootpasswd)==0) {
	    broken = true;
	} else {
	    world->RingBell(0);
	    world->RingBell(0);
	}
    }
}
