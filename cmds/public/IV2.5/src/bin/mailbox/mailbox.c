/*
 * Watch a mailbox and display its contents.
 */

#include <InterViews/border.h>
#include <InterViews/box.h>
#include <InterViews/frame.h>
#include <InterViews/glue.h>
#include <InterViews/interactor.h>
#include <InterViews/paint.h>
#include <InterViews/painter.h>
#include <InterViews/sensor.h>
#include <InterViews/shape.h>
#include <InterViews/world.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>

World* world;
char mailpath[128];

const char* MAILPATH = "/usr/spool/mail/";
const char* DOMAIN = ".stanford";

class Viewer {
public:
    Viewer (Interactor *i) { view = i; next = nil; }
    Interactor* view;
    Viewer* next;
};

class MailBox;

class MailView : public Interactor {
public:
    MailView(MailBox*);
    ~MailView();

    void Update () { Draw(); }
protected:
    MailBox* mailbox;
};

class MailBeep : public MailView {
public:
    MailBeep(MailBox* m) : (m) { lastcount = 0; }
    void Update();
private:
    int lastcount;
};

class MailText : public MailView {
public:
    MailText(MailBox* m, int r, int c);
    void Reconfig();
    void Redraw(Coord, Coord, Coord, Coord);
private:
    int rows, cols;
};

class MailFlag : public MailView {
public:
    MailFlag(MailBox*, boolean showcount=false);
    ~MailFlag();

    virtual void Update();
    virtual void Redraw(Coord, Coord, Coord, Coord);
protected:
    void Reconfig();
private:
    boolean showcount;
    int lastcount;
    Painter* highlight;
};

const int MaxItemCount = 100;
const int MaxItemSize = 100;

enum Status { All, New, Unread };

class MailBox : public MonoScene {
public:
    MailBox(
	char* path, int delay, Status stat,
	boolean noflag=false, boolean notext=false, boolean silent=false,
	boolean showcount=false,
	int rows=0, int columns=0,
	int width=0, int height=0
    );
    ~MailBox();

    void Attach(Interactor*);
    void Detach(Interactor*);
    char* GetItem(int);
    int GetCount();
    void Handle(Event&);
    void Notify();
    void Scan();
    void Tick();
private:
    int width;
    int height;
    Viewer* views;
    char* mailboxpath;
    int lastsize;
    Status status;
    int count;
    char* items[MaxItemCount];

    virtual void Reconfig();
};

inline char* MailBox::GetItem (int i) { return items[i]; }
inline int MailBox::GetCount () { return count; }

MailView::MailView (MailBox* m) {
    mailbox = m;
    m->Attach(this);
}

MailView::~MailView () {
     mailbox->Remove(this);
}

MailText::MailText (MailBox* m, int r, int c) : (m) {
    rows = r;
    cols = c;
}

void MailText::Reconfig () {
    if (rows != 0 && cols != 0) {
	Font* f = output->GetFont();
	shape->width = cols * f->Width("m");
	shape->height = rows * f->Height();
    }
}

void MailText::Redraw (Coord x1, Coord y1, Coord x2, Coord y2) {
    output->ClearRect(canvas, x1, y1, x2, y2);
    output->Clip(canvas, x1, y1, x2, y2);
    int height = output->GetFont()->Height();
    Coord h;
    int i;
    for (
	i = 0, h = ((ymax+1) % height) / 2;
	i < mailbox->GetCount() && h <= (ymax+1)-height;
	++i, h += height
    ) {
	output->Text(canvas, mailbox->GetItem(i), 0, h);
    }
    output->NoClip();
}

void MailBeep::Update() {
    int count = mailbox->GetCount();
    if (count > lastcount) {
	world->RingBell(0);
	world->RingBell(0);
    }
    lastcount = count;
}

MailFlag::MailFlag (MailBox* m, boolean count) : (m) {
    showcount = count;
    lastcount = 0;
    highlight = nil;
}

MailFlag::~MailFlag () {
    delete highlight;
}

void MailFlag::Reconfig () {
    delete highlight;
    highlight = new Painter(output);
    highlight->SetColors(output->GetBgColor(), output->GetFgColor());
    Font* f = output->GetFont();
    shape->width = f->Width("M") + 6;
    shape->height = f->Height();
    shape->Rigid(0, vfil/1000, 0, vfil);
}

void MailFlag::Update () {
    int count = mailbox->GetCount();
    if (count > lastcount) {
	int i;
	const int bignum = 10000;
	lastcount = 0;
	Draw();
	Sync();
	for (i = 0; i < bignum; ++i);
	lastcount = count;
	Draw();
	Sync();
	for (i = 0; i < bignum; ++i);
	lastcount = 0;
	Draw();
	Sync();
	for (i = 0; i < bignum; ++i);
	lastcount = count;
	Draw();
	Sync();
    } else {
	lastcount = count;
	Draw();
    }
}

void MailFlag::Redraw (Coord x1, Coord y1, Coord x2, Coord y2) {
    Painter* p = (lastcount > 0) ? highlight : output;
    Font* f = p->GetFont();
    int height = f->Height();
    p->ClearRect(canvas, x1, y1, x2, y2);
    if (ymax+1 < height) {
	;
    } else if (showcount) {
	char c;
	if (mailbox->GetCount() <= 0) {
	    c = '-';
	} else if (mailbox->GetCount() > 9) {
	    c = '*';
	} else {
	    c = '0' + mailbox->GetCount();
	}
	p->Text(canvas, &c, 1, xmax/2 - f->Width("M")/2, ymax/2 - height/2);
    } else if (ymax+1 < 4*height) {
	p->Text(canvas, "M", xmax/2 - f->Width("M")/2, ymax/2 - height/2);
    } else {
	p->Text(canvas, "M", xmax/2 - f->Width("M")/2, ymax/2 + height);
	p->Text(canvas, "a", xmax/2 - f->Width("a")/2, ymax/2);
	p->Text(canvas, "i", xmax/2 - f->Width("i")/2, ymax/2 - height);
	p->Text(canvas, "l", xmax/2 - f->Width("l")/2, ymax/2 - 2*height);
    }
}

MailBox::MailBox(
    char* path, int delay, Status stat,
    boolean noflag, boolean notext, boolean silent, boolean showcount,
    int rows, int cols, int w, int h
) {
    mailboxpath = path;
    status = stat;
    lastsize = 0;
    for (int i = 0; i < MaxItemCount; i++) {
	items[i] = nil;
    }
    count = 0;
    input = new Sensor;
    input->CatchTimer(delay, 0);
    input->Catch(KeyEvent);
    views = nil;
    if (!silent) {
	new MailBeep(this);
    }

    HBox* contents = new HBox;
    if (!noflag) {
	contents->Insert(new MailFlag(this, showcount));
    }
    if (!noflag && !notext) {
	contents->Insert(new VBorder);
    }
    if (!notext) {
	contents->Insert(new HGlue(2, 0, 0));
	contents->Insert(
	    new VBox(
		new VGlue(2, 0, 0),
		new MailText(this, rows, cols),
		new VGlue(2, 0, 0)
	    )
	);
	contents->Insert(new HGlue(2, 0, 0));
    }
    Insert(contents);
    width = w;
    height = h;
}

void MailBox::Reconfig () {
    MonoScene::Reconfig();
    if (width != 0 && height != 0) {
	shape->width = width;
	shape->height = height;
    }
}

void MailBox::Handle (Event &e) {
    switch (e.eventType) {
	case TimerEvent:
	    Tick();
	    break;
	case KeyEvent:
	    if (e.keystring[0] == 'q') {
		e.target = nil;
	    }
	    break;
    }
}

MailBox::~MailBox () {
    while (views != nil) {
	Viewer* doomed = views;
	delete views->view;
	views = views->next;
	delete doomed;
    }
}

void MailBox::Attach (Interactor* i) {
    Viewer* newViewer = new Viewer(i);
    newViewer->next = views;
    views = newViewer;
}

void MailBox::Detach (Interactor* i) {
    register Viewer* v, * prev;

    prev = nil;
    for (v = views; v != nil; v = v->next) {
	if (v->view == i) {
	    if (prev == nil) {
		views = v->next;
	    } else {
		prev->next = v->next;
	    }
	    delete v;
	    break;
	} else {
	    prev = v;
	}
    }
}

void MailBox::Notify() {
    register Viewer* v;

    for (v = views; v != nil; v = v->next) {
	v->view->Update();
    }
}

void MailBox::Tick () {
    struct stat statBuffer;

    if (lstat(mailboxpath,&statBuffer) >= 0) {
	if (statBuffer.st_size != lastsize) {
	    count = 0;
	    Scan();
	    lastsize = statBuffer.st_size;
	    Notify();
	}
    } else if (lastsize > 0) {
	count = 0;
	lastsize = 0;
	Notify();
    }
}

void MailBox::Scan () {
    FILE* f = fopen(mailboxpath, "r");
    char line[256];
    char info[256];
    char mail[256];
    while (fgets(line, 255, f) != 0) {
        if (sscanf(line, "From %s", info) > 0) {
	    if (info[0] == ':') {
		continue;
	    }
	    char * ip = info;
	    while (ip[0] == '<' || ip[0] == '(') {
		ip ++;
	    }
	    if (ip[0] == '@') {
		ip ++;
		char * colonp = strchr(ip, ':');
		if (colonp != nil) {
		    ip = colonp + 1;
		}
	    }
	    char * atp = strchr(ip, '@');
	    if (atp != nil) {
		char * dotp = strchr(atp, '.');
		if (dotp != nil) {
		    for (int i = 0; i < strlen(DOMAIN); ++i) {
			char c = dotp[i];
			c = isupper(c) ? c - 'A' + 'a' : c;
			if (c != DOMAIN[i]) {
			    break;
			}
		    }
		    if (i == strlen(DOMAIN)) {
			*dotp = '\0';
		    }
		}
	    }
	    for (char* end = & ip[strlen(ip) - 1];
		    *end == '>' || *end == ')';
		    end --
	    ) {
		*end = '\0';
	    }
	    strcpy(mail, ip);
	} else if (*mail != '\0' &&
		    sscanf(line, "Subject: %s", info) > 0) {
	    char * p = strchr(line, '\n');
	    if (p != 0) {
		*p = '\0';
	    }
	    strcat(mail, " << ");
	    strcat(mail, line+9);
	} else if (*mail != '\0' &&
		    sscanf(line, "Status: %s", info) > 0) {
	    switch (status) {
	    case New:
		if (strchr(info, 'O') != nil) {
		    *mail = '\0';
		}
		break;
	    case Unread:
		if (strchr(info, 'R') != nil) {
		    *mail = '\0';
		}
		break;
	    default:
		break;
	    }
        } else if (*line == '\n' && *mail != '\0') {
	    delete items[MaxItemCount-1];
	    for (int i=MaxItemCount-2; i>=0; --i) {
		items[i+1] = items[i];
	    }
	    items[0] = new char[MaxItemSize];
	    strcpy(items[0], mail);
	    *mail = '\0';
	    ++count;
        }
    }
    fclose(f);
}

MailBox* mailbox;

int cols = 45;		    /* size of the text part - cols */
int rows = 4;		    /* .. and lines */
int width = 0;		    /* initial width of window */
int height = 0;		    /* ... height */
int xpos = 0;		    /* initial position of lower left corner - x */
int ypos = 0;		    /* ... y */
int delay = 60;		    /* seconds between checks of mailbox */
Status status = All;	    /* which messages to display */
boolean count = false;	    /* display a count of the mail items in flag */
boolean noflag = false;	    /* show flag by default */
boolean notext = false;	    /* show text by default */
boolean silent = false;	    /* ring bell when mail arrives by default */

static OptionDesc options[] = {
    { "font=", "*font", OptionValueAfter },
    { nil }
};

int main (int argc, char *argv[]) {
    int i, p1, p2;
    char* curarg;
    char buffer[128];

    world = new World("mailbox", options, argc, argv);
    for (i = 1; i < argc; i++) {
	curarg = argv[i];
	if (sscanf(curarg, "delay=%d", &p1) == 1) {
	    delay = p1;
	} else if (sscanf(curarg, "pos=%d,%d", &p1, &p2) == 2) {
	    xpos = p1; ypos = p2;
	} else if (sscanf(curarg, "size=%d,%d", &p1, &p2) == 2) {
	    width = p1; height = p2; rows=0; cols=0;
	} else if (sscanf(curarg, "rows=%d", &p1) == 1) {
	    rows = p1; height = 0; width = 0;
	} else if (sscanf(curarg, "cols=%d", &p1) == 1) {
	    cols = p1; height = 0; width = 0;
	} else if (sscanf(curarg, "mailbox=%s", mailpath) == 1) {
	    /* nothing else to do */
	} else if (strcmp(curarg, "count") == 0) {
	    count = true;
	} else if (strcmp(curarg, "new") == 0) {
	    status = New;
	} else if (strcmp(curarg, "unread") == 0) {
	    status = Unread;
	} else if (strcmp(curarg, "all") == 0) {
	    status = All;
	} else if (strcmp(curarg, "noflag") == 0) {
	    noflag = true; notext = false;
	} else if (strcmp(curarg, "notext") == 0) {
	    notext = true; noflag = false;
	} else if (strcmp(curarg, "silent") == 0) {
	    silent = true;
	} else {
	    fprintf(stderr, "%s: unexpected argument '%s'\n", argv[0], curarg);
	    fprintf(stderr, "usage: %s %s %s %s %s\n",
		argv[0], "[pos=#,#] [size=#,#] [rows=#] [cols=#] [delay=#]",
		"[new] [unread] [all]",
		"[count] [silent] [noflag] [notext]",
		"[font=name] [mailbox=path]"
	    );
	    exit(1);
	}
    }

    if (strlen(mailpath) == 0) {
	strcpy(mailpath, MAILPATH);
	strcat(mailpath, getenv("USER"));
    }

    mailbox = new MailBox(
	mailpath, delay, status,
	noflag, notext, silent, count,
	rows, cols, width, height
    );
    if (xpos == 0 && ypos == 0) {
	world->InsertApplication(mailbox);
    } else {
	world->InsertApplication(mailbox, xpos, ypos);
    }

    mailbox->Tick();
    mailbox->Run();
    return 0;
}
