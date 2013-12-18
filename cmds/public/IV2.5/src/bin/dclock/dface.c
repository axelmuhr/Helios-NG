/*
 * digital clockface class
 */

#include "dclock.h"
#include "dface.h"
#include "digit.h"
#include "clocktime.h"
#include <string.h>

static const int FadeDelay = 10000;

void DFace::DrawFace () {
    output->ClearRect(canvas, 0, 0, xmax, ymax);
}

void DFace::DrawColon () {
    output->FillPolygon(canvas, colon[0].x, colon[0].y, colon[0].count);
    output->FillPolygon(canvas, colon[1].x, colon[1].y, colon[1].count);
}

void DFace::DrawAMPM (Painter *painter) {
    if (AMPMmode != BLANK) {
	if (AMPMmode == AM) {
	    painter->FillPolygon(canvas, A.x, A.y, A.count);
	} else {
	    painter->FillPolygon(canvas, P.x, P.y, P.count);
	}
	painter->FillPolygon(canvas, M.x, M.y, M.count);
    }
}

void DFace::DrawDate () {
    if (showDate && date.len != 0) {
	Font* f = output->GetFont();
	Coord dateYPos = ymax - f->Height();
	Coord dateXPos = 2;

	int availWidth = xmax - dateXPos - 2;
	int dayWidth = f->Width( date.text, 3 );
	int dayDateWidth = f->Width( date.text, 10 );
	int wholeWidth = f->Width( date.text, date.len );

	output->ClearRect(canvas, 0, ymax - f->Height(), xmax, ymax);
	if (wholeWidth < availWidth) {
	    output->Text(canvas, date.text, date.len, dateXPos, dateYPos);
	} else if (dayDateWidth < availWidth) {
	    output->Text(canvas, date.text, 10, dateXPos, dateYPos);
	} else if (dayWidth < availWidth) {
	    output->Text(canvas, date.text, 3, dateXPos, dateYPos);
	}
    }
}

void DFace::DrawBorder () {
    if (showDate && showTime) {
	int ypos = ymax - output->GetFont()->Height() - 2;
	output->Line(canvas, 0, ypos, xmax, ypos);
    }
}

void DFace::Tick () {
    int nextTick = clock->NextTick();
    if (nextTick > 0) {
	input->CatchTimer(nextTick + 1, 0); // this extra second makes sure the
    } else {				    // timeout is after the next minute
	int h, m, s;
	char date[50];
	clock->GetTime(date, h, m, s);
	Set(date, h, m);
    }
}

void DFace::Set (char *today, int hours, int minutes) {
    int h = hours;
    if (mode==CIVIL) {
	if (hours > 12) {
	    h -= 12;
	} else if ( hours == 0 ) {
	    h = 12;				// midnight is 12:xx
	}
    }

    Event e;
    Sensor* wait = new Sensor;
    wait->CatchTimer(0,0);
    Listen(wait);
    int lasttime;
    int fade = FadeDelay * (1 << (min(4,max(0,FadeRate))) );

    boolean done = false;
    while (showTime && !done) {
	Read(e);
	lasttime = e.timestamp;
	done = true;
	if (mode == CIVIL && h < 10) {
	    done &= ht->Set(-1);		// blank digit
	} else {
	    done &= ht->Set(h/10);
	}
	done &= hu->Set(h%10);
	done &= mt->Set(minutes/10);
	done &= mu->Set(minutes%10);

	wait->CatchTimer(0, 0);
	Read(e);
	wait->CatchTimer(0, lasttime+fade - e.timestamp);
    }

    Listen(input);
    delete wait;

    if (showTime && mode==CIVIL) {
	AMPMMODE newAMPM = (hours >= 12) ? PM : AM;

	if (AMPMmode == BLANK) {
	    AMPMmode = newAMPM;
	    DrawAMPM(output);
	} else if (AMPMmode != newAMPM) {
	    DrawAMPM(invertor);				// erase old
	    AMPMmode = newAMPM;
	    DrawAMPM(output);				// draw new
	}
    }
    if (showDate && strcmp(date.text, today) != 0) {
	strcpy(date.text, today);
	date.len = strlen(today);
	DrawDate();
	if (showBorder) {
	    DrawBorder();
	}
    }
}

DFace::DFace (
    boolean showDate, boolean showBorder, boolean showTime,
    TMode timeMode, int width, int height
) {
    clock = new Clock();
    mode = timeMode;
    AMPMmode = BLANK;
    A.count = AData.count;
    P.count = PData.count;
    M.count = MData.count;
    colon[0].count = ColonData[0].count;
    colon[1].count = ColonData[1].count;
    this->showDate = showDate;
    this->showBorder = showBorder;
    this->showTime = showTime;
    ht = new Digit(HTx, ALLy);
    hu = new Digit(HUx, ALLy);
    mt = new Digit(MTx, ALLy);
    mu = new Digit(MUx, ALLy);
    selected = false;
    done = false;
    date.len = 0;
    shape->Rect(width, height);
    shape->Rigid(hfil, hfil, vfil, vfil);
    invertor = nil;
    input = new Sensor();
    input->CatchTimer(0, 0);
    input->Catch(KeyEvent);
}

void DFace::Reconfig () {
    delete invertor;
    invertor = new Painter(output);
    invertor->SetColors(invertor->GetBgColor(), invertor->GetFgColor());
    ht->Reconfig(output);
    hu->Reconfig(output);
    mt->Reconfig(output);
    mu->Reconfig(output);
}

DFace::~DFace () {
    delete clock;
    delete ht;
    delete hu;
    delete mt;
    delete mu;
    delete invertor;
}

void DFace::Resize () {
    int i;

    int w = xmax;
    int h = ymax;
    if (showDate) {
	// adjust vertical size for date
	h -= output->GetFont()->Height();
    }
    // resize colon
    for (i = 0; i < colon[0].count; i++) {
	colon[0].x[i] = Coord(ColonData[0].x[i] * w );
	colon[0].y[i] = Coord(ColonData[0].y[i] * h );
	colon[1].x[i] = Coord(ColonData[1].x[i] * w );
	colon[1].y[i] = Coord(ColonData[1].y[i] * h );
    }
    // resize AM/PM
    for (i = 0; i < 12; i++) {
	A.x[i] = Coord(AData.x[i] * w);
	A.y[i] = Coord(AData.y[i] * h);
	P.x[i] = Coord(PData.x[i] * w);
	P.y[i] = Coord(PData.y[i] * h);
	M.x[i] = Coord(MData.x[i] * w);
	M.y[i] = Coord(MData.y[i] * h);
    }
    if (showTime) {
	ht->Resize(canvas, h);
	hu->Resize(canvas, h);
	mt->Resize(canvas, h);
	mu->Resize(canvas, h);
    }
}

void DFace::Redraw (Coord left, Coord bottom, Coord right, Coord top) {
    output->Clip(canvas, left, bottom, right, top);
    Draw();
    output->NoClip();
}

void DFace::RedrawList (int, Coord[], Coord[], Coord[], Coord[]) {
    Redraw(0, 0, xmax, ymax);
}

void DFace::Draw () {
    DrawFace();
    if (showDate) {
	DrawDate();
    }
    if (showTime) {
	DrawColon();
	DrawAMPM(output);
	ht->Redraw();
	hu->Redraw();
	mt->Redraw();
	mu->Redraw();
    }
    if (showBorder) {
	DrawBorder();
    }
}

void DFace::Handle (Event &event) {
    switch (event.eventType) {
	case KeyEvent:
	    if (event.len > 0 && event.keystring[0] == 'q') {
		done = true;
	    }
	    break;
	case TimerEvent:
	    Tick();
	    break;
    }
}

void DFace::Run () {
    Event event;

    while (!done) {
	Read(event);
	Handle(event);
	Tick();
    }
}
