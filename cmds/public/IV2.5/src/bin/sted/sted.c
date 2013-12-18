/*
 * sted - a simple text editor
 */

#include <InterViews/border.h>
#include <InterViews/box.h>
#include <InterViews/button.h>
#include <InterViews/glue.h>
#include <InterViews/regexp.h>
#include <InterViews/scene.h>
#include <InterViews/scroller.h>
#include <InterViews/sensor.h>
#include <InterViews/stringeditor.h>
#include <InterViews/textbuffer.h>
#include <InterViews/texteditor.h>
#include <InterViews/world.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

static const int MINTEXTSIZE = 10000;
static const int BUFFERSIZE = 1000;

int alive = 0;
World* world;

class Sted : public MonoScene {
public:
    Sted(const char* filename);
    virtual ~Sted();
protected:
    virtual void Handle(Event&);
    virtual void Update();
protected:
    void Edit(const char* filename);
    void Do(const char*);
    void InsertCharacter(char);
    void DeleteCharacter(int count);

    boolean modified;
    boolean closed;
    boolean prefix1;
    boolean prefix2;
    char* buffer;
    int size;
    TextBuffer* text;
    TextEditor* editor;
    StringEditor* command;
    ButtonState* state;
};

Sted::Sted (const char* name) {
    input = new Sensor();
    input->Catch(KeyEvent);
    input->Catch(DownEvent);
    state = new ButtonState(false);
    state->Attach(this);
    command = new StringEditor(state, true, false, "                    ");
    buffer = nil;
    size = 0;
    text = nil;
    editor = new TextEditor(24, 80, 8, Reversed);
    Insert(
        new VBox(
            new HBox(
                new HGlue(5, 0, 0),
                new VBox(
                    new VGlue(3, 0, 0),
                    editor,
                    new VGlue(3, 0, 0)
                ),
                new HGlue(5, 0, 0),
                new VBorder,
                new VScroller(editor)
            ),
            new HBorder,
            new VGlue(2, 0, 0),
            new HBox(
                new HGlue(5, 0, 0),
                command,
                new HGlue(5, 0, 0)
            ),
            new VGlue(2, 0, 0)
        )
    );
    ++alive;
    prefix1 = false;
    prefix2 = false;
    closed = false;
    Edit(name);
}

Sted::~Sted () {
    delete text;
    delete buffer;
    state->Detach(this);
    delete state;
    --alive;
}

void Sted::Edit (const char* name) {
    delete buffer;
    delete text;
    FILE* f = fopen(name, "r");
    if (f != nil) {
        struct stat filestats;
        stat(name, &filestats);
        size = max(round(filestats.st_size * 1.2), MINTEXTSIZE);
        buffer = new char[size];
        char* b = buffer;
        int remaining = size;
        while (remaining > 1 && fgets(b, remaining, f) != nil) {
            int l = strlen(b);
            remaining -= l;
            b += l;
        }
        fclose(f);
        text = new TextBuffer(buffer, b-buffer, size);
        command->Message("");
    } else {
        size = MINTEXTSIZE;
        buffer = new char[size];
        text = new TextBuffer(buffer, 0, size);
        command->Message("new file");
    }
    editor->Edit(text);
    modified = false;
    SetName(name);
}

void Sted::Update () {
    int value;
    state->GetValue(value);
    if (value == true) {
        Do(command->Text());
    }
    state->SetValue(false);
}

void Sted::Do (const char* s) {
    static char operation[32];
    static char parameter[32];
    sscanf((char*)s, "%s %s", operation, parameter);
    if (strcmp(operation, "quit") == 0) {
        alive = 0;
    } else if (strcmp(operation, "close") == 0) {
        closed = true;
    } else if (strcmp(operation, "visit") == 0) {
        int x0 = 0;
        int y0 = 0;
        GetRelative(x0, y0);
        world->InsertApplication(new Sted(parameter), x0 - 15, y0 - 20);
    } else if (strcmp(operation, "file") == 0) {
        Edit(parameter);
    } else if (strcmp(operation, "search") == 0) {
        Regexp re(parameter);
        if (
            text->ForwardSearch(&re, editor->Dot()) >= 0
            || text->ForwardSearch(&re, text->BeginningOfText()) >= 0
        ) {
            editor->Select(re.EndOfMatch(), re.BeginningOfMatch());
        } else {
            world->RingBell(1);
        }
    } else if (strcmp(operation, "goto") == 0) {
        editor->Select(text->LineIndex(atoi(parameter)));
    } else {
        world->RingBell(1);
    }
}

void Sted::DeleteCharacter (int count) {
    if (editor->Dot() != editor->Mark()) {
        editor->DeleteSelection();
    } else {
        editor->DeleteText(count);
    }
    modified = true;
}

void Sted::InsertCharacter (char c) {
    editor->DeleteSelection();
    editor->InsertText(&c, 1);
    modified = true;
}

void Sted::Handle (Event& e) {
    switch (e.eventType) {
    case KeyEvent:
        if (e.len > 0) {
            char c = e.keystring[0];
            if (prefix1) {
                prefix1 = false;
                switch(c) {
                  case 'v':     editor->BackwardPage(1); break;
                  case 'x':     command->Edit("", 0, 0); break;
                  case '<':     editor->BeginningOfText(); break;
                  case '>':     editor->EndOfText(); break;
                  case '\r':    Do(""); break;
                  case '=':     command->Edit("goto ", 5, 5); break;
                  default:      world->RingBell(1); break;
                }
            } else if (prefix2) {
                prefix2 = false;
                switch(c) {
                  case '\006':  command->Edit("file ", 5, 5); break;
                  case '\026':  command->Edit("visit ", 6, 6); break;
                  case '\003':  command->Edit("quit", 0, 4); break;
                  case 'k':     command->Edit("close", 0, 5); break;
                  default:      world->RingBell(1); break;
                }
            } else {
                switch (c) {
                  case '\007':  world->RingBell(1); break;
                  case '\033':  prefix1 = true; break;
                  case '\030':  prefix2 = true; break;
                  case '\026':  editor->ForwardPage(1); break;
                  case '\001':  editor->BeginningOfLine(); break;
                  case '\005':  editor->EndOfLine(); break;
                  case '\006':  editor->ForwardCharacter(1); break;
                  case '\002':  editor->BackwardCharacter(1); break;
                  case '\016':  editor->ForwardLine(1); break;
                  case '\020':  editor->BackwardLine(1); break;
                  case '\023':  command->Edit("search ", 7, 7); break;
                  case '\004':  DeleteCharacter(1); break;
                  case '\010':  DeleteCharacter(-1); break;
                  case '\177':  DeleteCharacter(-1); break;
                  case '\011':  InsertCharacter('\t'); break;
                  case '\015':  InsertCharacter('\n'); break;
                  default:
                    if (!iscntrl(c)) {
                        InsertCharacter(c);
                    } else {
                        world->RingBell(1);
                    }
                    break;
                }
            }
            editor->ScrollToSelection();
            command->Message("");
        }
        break;
    case DownEvent:
        if (e.button == LEFTMOUSE) {
            GetRelative(e.x, e.y, editor);
            editor->Select(editor->Locate(e.x, e.y));
            do {
                editor->ScrollToView(e.x, e.y);
                editor->SelectMore(editor->Locate(e.x, e.y));
                Poll(e);
                GetRelative(e.x, e.y, editor);
            } while (e.leftmouse);
        } else if (e.button == MIDDLEMOUSE) {
            GetRelative(e.x, e.y, editor);
            int y = e.y;
            do {
                editor->ScrollBy(0, y - e.y);
                y = e.y;
                Poll(e);
                GetRelative(e.x, e.y, editor);
            } while (e.middlemouse);
        } else if (e.button == RIGHTMOUSE) {
            GetRelative(e.x, e.y, editor);
            int y = e.y;
            do {
                editor->ScrollBy(0, e.y - y);
                Poll(e);
                GetRelative(e.x, e.y, editor);
            } while (e.rightmouse);
        }
        break;
    }
    if (closed) {
        world->Remove(this);
        delete this;
    }
    if (alive == 0) {
        e.target = nil;
    }
}

int main (int argc, char* argv[]) {
    world = new World("sted", argc, argv);
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            world->InsertApplication(new Sted(argv[i]));
        }
    } else {
        world->InsertApplication(new Sted("/interviews/lib/all/sted.help"));
    }
    world->Run();
    return 0;
}
