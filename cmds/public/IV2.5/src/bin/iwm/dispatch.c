#include "dispatch.h"
#include "desktop.h"
#include <InterViews/cursor.h>
#include <InterViews/paint.h>
#include <InterViews/world.h>
#include <osfcn.h>
#include <stdlib.h>
#include <string.h>

#ifdef X10
#include <InterViews/X10/Xinput.h>
#endif

#ifdef X11
#include <InterViews/X11/Xlib.h>
#endif

extern char* DesktopFunctionNames[Operations];		   /* desktop.c */
extern DesktopFunction DesktopFunctions(OperationCode);	   /* desktop.c */

static CursorPattern subcurbits = {
   0x0000, 0x0000, 0x0000, 0x0000,
   0x0010, 0x0018, 0x001c, 0x7ffe,
   0x7ffe, 0x001c, 0x0018, 0x0010,
   0x0000, 0x0000, 0x0000, 0x0000
};

static CursorPattern subcurmask = {
   0x0000, 0x0000, 0x0000, 0x0038,
   0x003c, 0x003e, 0xffff, 0xffff,
   0xffff, 0xffff, 0x003e, 0x003c,
   0x0038, 0x0000, 0x0000, 0x0000
};

static Cursor* subCursor;

inline char tolower (char c) {
    if (c >= 'A' && c <= 'Z') {
	return c + 'a' - 'A';
    } else {
	return c;
    }
}

boolean Equal (const char* s1, const char* s2) {
    if (s1 != nil && s2 != nil) {
	if (tolower(*s1) == tolower(*s2)) {
	    if (*s1 == '\0') {
		return true;
	    } else {
		return Equal(++s1, ++s2);
	    }
	} else {
	    return false;
	}
    } else {
	return s1 == s2;
    }
}

class DesktopMenu : public Menu {
    char* name;
    DesktopMenu* next;
    boolean persistent;
public:
    DesktopMenu(
	const char* name, DesktopMenu* next, boolean persistent
    ) : (persistent) {
	this->name = new char[strlen(name)+1];
	strcpy(this->name, name);
	this->next = next;
	this->persistent = persistent;
    }
    ~DesktopMenu() { delete name; }

    DesktopMenu* Find(const char* name) {
	if (Equal(name, this->name)) {
	    return this;
	} else if (next != nil) {
	    return next->Find(name);
	} else return nil;
    }

    DesktopMenu* Next() { return next; }
    boolean Persistent() { return persistent; }
};

class DesktopSelection : public TextItem {
protected:
    Operation* op;
public:
    DesktopSelection(const char* s) : (s) {
	op = new Operation;
	op->f = nil; op->uop = nil; op->menu = nil;
    }
    virtual Operation* Op();
};

Operation* DesktopSelection::Op () { return op; }

class FunctionSelection : public DesktopSelection {
public:
    FunctionSelection(const char* s, DesktopFunction f) : (s) {
	op->f = f;
    }
};

class UserSelection : public DesktopSelection {
public:
    UserSelection(const char* s, const char* uop) : (s) {
	op->uop = (char*)uop;
    }
    ~UserSelection() { delete op->uop; }
};

class SubMenuSelection : public DesktopSelection {
    DesktopMenu* menu;
    DesktopSelection* ms;
    Sensor* sensor;
    int threshold;
    boolean ready, needs_highlighting;
public:
    SubMenuSelection (const char* s, DesktopMenu* menu) : (s) {
	op->menu = menu;
	this->menu = menu;
	ms = nil;
	if (!menu->Persistent()) {
	    sensor = new Sensor(onoffEvents);
	    sensor->Catch(MotionEvent);
	    Listen(sensor);
	    ready = false;
	    needs_highlighting = false;
	    threshold = 0;
	}
    }

    Operation* Op () {
	if (menu->Persistent()) {
	    return op;
	} else if (ms != nil) {
	    return ms->Op();
	} else {
	    return nil;
	}
    }

    void Handle(Event&);
};

void SubMenuSelection::Handle (Event& e) {
    switch (e.eventType) {
	case MotionEvent:
	    if (!threshold) threshold = xmax - (xmax / 8) + 1;
	    if (ready) {
		if (e.x >= threshold) {
                    MenuItem* selection;
		    menu->Popup(e, selection);
                    ms = (DesktopSelection*)selection;
		    ready = false;
		    UnHighlight();
		    needs_highlighting = true;
		}
	    } else {
		if (needs_highlighting) {
		    Highlight();
		    needs_highlighting = false;
		}
		if (e.x < threshold) ready = true;
	    }
	    break;
	case OnEvent:
	    if (!menu->Persistent()) {
		SetCursor(subCursor);
	    }
	    /* fall through */
	default:
	    DesktopSelection::Handle(e);
	    ready = false;
    }
}

DesktopDispatcher::DesktopDispatcher(Desktop* d) {
    register DesktopMenu* m;
    char* home;
    char* iwmpath;

    forked = true;
    const char* s = world->GetAttribute("fork");
    if (s != nil && Equal(s, "false")) {
	forked = false;
    }
    subCursor = new Cursor(15, 8, subcurbits, subcurmask, black, white);
    desk = d;
    home = getenv("HOME");
    iwmpath = new char[strlen(home)+strlen(".iwmrc")+1];
    strcpy(iwmpath, home);
    strcat(iwmpath, "/.iwmrc");
    configfile = fopen(iwmpath, "r");
    if (configfile == nil) {
	configfile = fopen("/usr/local/lib/iwmrc.default", "r");
	if (configfile == nil) {
	    fprintf(stderr, "iwm: no configuration file found\n");
	    exit(1);
	}
    }
    for (int i = 0; i < Activators; i++) {
	ops[i] = nil;
    }
    menus = nil;
    buttons = nil;
    Config();
    m = menus;
    while (m != nil) {
	m->Compose();
	m = m->Next();
    }
}

DesktopDispatcher::~DesktopDispatcher () {
    register DesktopMenu* m;
    register ButtonList* b;
    register i;

    for (i = 0; i < Activators; i++) {
	delete ops[i];
    }
    while (menus != nil) {
	m = menus->Next();
	delete(menus);
	menus = m;
    }
    while (buttons != nil) {
	b = buttons->next;
	delete(buttons);
	buttons = b;
    }
}

void DesktopDispatcher::Perform (Operation* op, Event& e) {
    DesktopSelection* s;

    if (op != nil) {
	if (op->f != nil) {
	    (*op->f)(desk, e);
	} else if (op->uop != nil) {
	    int pid = vfork();
	    if (pid < 0) {
		fprintf(stderr, "can't fork to execute '%s'\n", op->uop);
	    } else if (pid == 0) {
		/* child */
		int i;

		for (i = 3; close(i) != -1; i++);
		/* fork again so that sh process belongs to init */
		pid = vfork();
		if (pid < 0) {
		    fprintf(stderr, "can't fork to execute '%s'\n", op->uop);
		} else if (pid == 0) {
		    execl("/bin/sh", "sh", "-c", op->uop, 0);
		    _exit(127);
		}
		exit(0);
	    } else {
		/* parent */
		int status;

		wait(&status);
	    }
	} else if (op->menu != nil) {
            MenuItem* selection;
	    op->menu->Popup(e, selection);
            s = (DesktopSelection*)selection;
	    if (s != nil) {
		Perform(s->Op(), e);
	    }
	}
    }
}

void DesktopDispatcher::Perform (Event& e) {
    Perform(GetOperation(e), e);
}

Operation* DesktopDispatcher::GetOperation (Event& e) {
    register Activator a;
    Logo* logo = desk->GetLogo();

    switch (e.button) {
	case LEFTMOUSE:
	    a = LEFT;
	    break;
	case MIDDLEMOUSE:
	    a = MIDDLE;
	    break;
	case RIGHTMOUSE:
	    a = RIGHT;
	    break;
    }
    if (e.control) {
	a |= CTRL_MASK;
    }
    if (e.meta) {
	a |= META_MASK;
    }
    if (e.shift && !e.shiftlock) {
	a |= SHIFT_MASK;
    }
    if (logo->Hit(e)) {
	a |= LOGO_MASK;
    }
    if (a < Activators) {
	if (ops[a] != nil) {
	    return ops[a];
	}
	a &= ~LOGO_MASK;
	if (ops[a] != nil) {
	    return ops[a];
	}
	if (e.shiftlock) {
	    a |= SHIFT_LOCK_MASK;
	    if (ops[a] != nil) {
		return ops[a];
	    } else {
		a |= SHIFT_MASK;
		if (ops[a] != nil) {
		    return ops[a];
		} else {
		    a &= ~SHIFT_LOCK_MASK;
		    if (ops[a] != nil) {
			return ops[a];
		    } else {
			a &= (LEFT | MIDDLE | RIGHT);
			return ops[a];
		    }
		}
	    }
	} else {
	    a &= (LEFT | MIDDLE | RIGHT);
	    return ops[a];
	}
    }
}

boolean DesktopDispatcher::NoFunction (Event& e) {
    Operation* op = GetOperation(e);
    if (op == nil || (op->f == nil && op->uop == nil)) {
	return true;
    }
    return false;
}

static const int argblocks = 6;
static const int maxlinesize = 1024;
static char argblock[argblocks][maxlinesize];

static char x[10];
static char s[maxlinesize];

boolean GetLine(FILE* CF, int& user_arg) {
    char* p;
    int matches;

    do {
	p = fgets(s, maxlinesize, CF);
    } while (p != nil && (s[0] == '#' || s[0] == '\n'));

    if (p == nil) {
	return false;
    }

    *argblock[0] = '\0'; *argblock[1] = '\0'; *argblock[2] = '\0';
    *argblock[3] = '\0'; *argblock[4] = '\0'; *argblock[5] = '\0';

    if (strchr(s, '"') != nil) {
	matches = sscanf(
	    s, "%s \"%[^\"]%[\"] %[^\n]\n",
	    argblock[0], argblock[1], x, argblock[2]
	);
	if (matches < 3) {
	    matches = sscanf(
		s, "%s %s \"%[^\"]%[\"] %[^\n]\n",
		argblock[0], argblock[1], argblock[2], x, argblock[3]
	    );
	    if (matches < 4) {
		matches = sscanf(
		    s, "%s %s %s \"%[^\"]%[\"] %[^\n]\n", argblock[0],
		    argblock[1], argblock[2], argblock[3], x, argblock[4]
		);
		if (matches < 5) {
		    matches = sscanf(
			s, "%s %s %s %s \"%[^\"]%[\"] %[^\n]\n", argblock[0],
			argblock[1], argblock[2], argblock[3], 
			argblock[4], x, argblock[5]
		    );
		    if (matches < 6) {
			fprintf(stderr, "iwm: config: invalid entry `%s'\n",s);
			return GetLine(CF, user_arg);
		    } else {
			user_arg = 4;
		    }
		} else {
		    user_arg = 3;
		}
	    } else {
		user_arg = 2;
	    }
	} else {
	    user_arg = 1;
	}
    } else {
	matches = sscanf(
	    s, "%s %s %s %s %s %[^\n]\n", argblock[0], argblock[1], 
	    argblock[2], argblock[3], argblock[4], argblock[5]
	);
	if (matches < 5) {
	    matches = sscanf(
		s, "%s %s %s %s %[^\n]\n", argblock[0],
		argblock[1], argblock[2], argblock[3], argblock[4]
	    );
	    if (matches < 4) {
		matches = sscanf(
		    s, "%s %s %s %[^\n]\n",
		    argblock[0], argblock[1], argblock[2], argblock[3]
		);
		if (matches < 3) {
		    matches = sscanf(
			s, "%s %s %[^\n]\n",
			argblock[0], argblock[1], argblock[2]
		    );
		    if (matches < 2) {
			fprintf(stderr, "iwm: config: invalid entry `%s'\n",s);
			return GetLine(CF, user_arg);
		    }
		}
	    }
	}
	user_arg = 0;
    }
    return true;
}

void DesktopDispatcher::InitMasks (
    int& arg, unsigned& keymask, Activator& a, boolean& logomask
) {
    if (Equal(argblock[arg], "shift")) {
	keymask |= ShiftMask; a |= SHIFT_MASK; arg++;
    } else if (
	Equal(argblock[arg], "ctrl") || Equal(argblock[arg], "control")
    ) {
	keymask |= ControlMask; a |= CTRL_MASK; arg++;
    } else if (Equal(argblock[arg], "meta")) {
	keymask |= Mod1Mask; a |= META_MASK; arg++;
    } else if (Equal(argblock[arg], "lock")) {
	keymask |= LockMask; a |= SHIFT_LOCK_MASK; arg++;
    } else if (Equal(argblock[arg], "logo")) {
	logomask = true; a |= LOGO_MASK; arg++;
    }
}

void DesktopDispatcher::Config () {
    Activator a, b;
    OperationCode op;
    DesktopFunction f;
    char* uop;
    DesktopMenu* a_menu;
    DesktopMenu* op_menu;
    int arg;
    char* t;
    int user_arg;
    unsigned keymask;
    boolean logomask, anyKeyChords;

    while (GetLine(configfile, user_arg)) {
	a = NO_ACTIVATOR; f = nil; uop = nil; keymask = 0; logomask = false;
	anyKeyChords = true; a_menu = nil; op_menu = nil; arg = 0;

	InitMasks(arg, keymask, a, logomask);
	for (int i = 0; arg && i < 4; ++i) {
	    InitMasks(arg, keymask, a, logomask);
	}
	anyKeyChords = keymask&(Mod1Mask|ShiftMask|ControlMask|LockMask);

	if (Equal(argblock[arg], "left")) {
	    if (!logomask || anyKeyChords) keymask |= LeftMask;
	    a |= LEFT; arg++;
	} else if (Equal(argblock[arg], "middle")) {
	    if (!logomask || anyKeyChords) keymask |= MiddleMask;
	    a |= MIDDLE; arg++;
	} else if (Equal(argblock[arg], "right")) {
	    if (!logomask || anyKeyChords) keymask |= RightMask;
	    a |= RIGHT; arg++;
	} else {
	    if (menus == nil ||
		(a_menu = menus->Find(argblock[arg])) == nil) {
		fprintf(stderr, "iwm: config: unknown activator `%s'\n",
			argblock[arg]);
		continue;
	    } else {
		arg++;
	    }
	}

	if (keymask && !logomask && !anyKeyChords) {
	    keymask |= Mod1Mask; a |= META_MASK;
	}

	if (user_arg == arg) {
	    uop = new char[strlen(argblock[arg])+(forked ? 30 : 3)];
	    strcpy(uop, argblock[arg]);
	    if (forked) {
		if (strrchr(uop, '<') == nil) {
		    strcat(uop, " </dev/null");
		}
		if (strrchr(uop, '>') == nil) {
		    strcat(uop, " >/dev/null");
		}
		strcat(uop, " 2>&1");
	    }
	} else {
	    op = 0;
	    while ((op < Operations) &&
		   !Equal(DesktopFunctionNames[op], argblock[arg])) {
		op++;
	    }
	    if (op < Operations) {
		f = DesktopFunctions(op);
	    } else {
		if ((menus == nil) ||
		    (op_menu = menus->Find(argblock[arg])) == nil) {
		    if (!strcmp(argblock[arg+1], "*")) {
			op_menu = new DesktopMenu(
			    argblock[arg], menus, true
			);
			menus = op_menu;
			arg++;
		    } else {
			op_menu = new DesktopMenu(
			    argblock[arg], menus, false
			);
			menus = op_menu;
		    }
		}
	    }
	}

	arg++;

	if (a_menu != nil) {
	    if (strlen(argblock[arg]) > 0) {
		t = new char[81];
		strcpy(t, argblock[arg]); arg++;
		while (arg < argblocks && strlen(argblock[arg]) > 0) {
		    strcat(t, " ");
		    strcat(t, argblock[arg]);
		    arg++;
		}
	    } else {
		if (op_menu == nil) {
		    t = (char*)DesktopFunctionNames[op];
		} else {
		    fprintf(stderr, "iwm: config: no title for menu entry\n");
		    t = "???";
		}
	    }

	    if (f != nil) {
		a_menu->Insert(new FunctionSelection(t, f));
	    } else if (uop != nil) {
		a_menu->Insert(new UserSelection(t, uop));
	    } else if (op_menu != nil) {
		a_menu->Insert(new SubMenuSelection(t, op_menu));
	    } else {
		a_menu->Insert(new FunctionSelection(t, nil));
	    }
	} else {
	    delete ops[a];
	    ops[a] = new Operation;
	    ops[a]->f = f;
	    ops[a]->uop = uop;
	    ops[a]->menu = op_menu;
	    b = a & (LEFT|MIDDLE|RIGHT);
	    if (ops[b] == nil) {
		ops[b] = ops[a];
	    } else {
		ops[b] = nil;
		ops[b] = new Operation;
		ops[b]->f = nil;
		ops[b]->uop = nil;
		ops[b]->menu = nil;
	    }
	    if (ops[a]->f != nil) {
		AddButton(keymask, crosshairs);
	    } else if (ops[a]->uop != nil) {
		AddButton(keymask, noCursor);
	    } else if (ops[a]->menu != nil) {
		AddButton(keymask, defaultCursor);
	    }
	}
    }
}
