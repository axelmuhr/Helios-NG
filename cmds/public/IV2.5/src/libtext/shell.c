/*
 * Implementation of shell windows.
 */

#include <InterViews/Text/emulator.h>
#include <InterViews/Text/shell.h>
#include <InterViews/Text/oldtextbuffer.h>
#include <InterViews/banner.h>
#include <InterViews/border.h>
#include <InterViews/box.h>
#include <InterViews/cursor.h>
#include <InterViews/frame.h>
#include <InterViews/sensor.h>
#include <osfcn.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>

Shell::Shell () {
    banner = new Banner("shell view", nil, nil);
    text = new OldTextBuffer;
    Init();
}

Shell::Shell (int rows, int cols) {
    banner = new Banner("shell view", nil, nil);
    text = new OldTextBuffer(rows, cols);
    Init();
}

Shell::Shell (char* name) {
    banner = new Banner("shell view", name, nil);
    text = new OldTextBuffer;
    Init();
}

Shell::Shell (char* name, int rows, int cols) {
    banner = new Banner("shell view", name, nil);
    text = new OldTextBuffer(rows, cols);
    Init();
}

void Shell::Init () {
    input = new Sensor(onoffEvents);
    input->Catch(KeyEvent);
    term = new Emulator(text);
    text->SetCursor(ltextCursor);
    Insert(new TitleFrame(banner, text));
    pty = -1;
}

void Shell::Resize () {
    MonoScene::Resize();
#   ifdef TIOCSWINSZ
	struct winsize w;
	int rows, cols;

	text->GetSize(rows, cols);
	w.ws_row = rows;
	w.ws_col = cols;
	ioctl(pty, TIOCSWINSZ, &w);
#   endif
}

void Shell::Run () {
    Event e;
    int r;
    char buf[4096];

    term->SetDevice(pty);
    input->CatchChannel(pty);
    for (;;) {
	Read(e);
	if (e.target == this) {
	    switch (e.eventType) {
		case OnEvent:
		    banner->highlight = true;
		    banner->Draw();
		    Sync();
		    break;
		case OffEvent:
		    banner->highlight = false;
		    banner->Draw();
		    Sync();
		    break;
		case KeyEvent:
		    if (e.keystring != nil) {
			write(pty, e.keystring, e.len);
		    }
		    break;
		case ChannelEvent:
		    r = read(pty, buf, sizeof(buf));
		    if (r > 0) {
			term->Write(buf, r);
		    } else {
			return;
		    }
		    break;
		default:
		    /* ignore */
		    break;
	    }
	} else {
	    e.target->Handle(e);
	}
    }
}

void Shell::Write (const char* buf, int len) {
    write(pty, buf, len);
}
