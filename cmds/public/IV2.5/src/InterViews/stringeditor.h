/*
 * StringEditor - interactive editor for character strings
 */

#ifndef stringeditor_h
#define stringeditor_h

#include <InterViews/interactor.h>

class ButtonState;
class TextDisplay;
class TextBuffer;

class StringEditor : public Interactor {
public:
    StringEditor(ButtonState*, int accept, int cancel, const char* sample);
    StringEditor(const char* name, ButtonState*, int, int, const char*);
    virtual ~StringEditor();

    void Message(const char* text);
    void Select(int);
    void Select(int left, int right);
    void Edit();
    void Edit(const char* text, int left, int right);

    const char* Text();
protected:
    virtual boolean HandleChar(char);
    void InsertText(const char*, int);

    virtual void Handle(Event&);
    virtual void Reconfig();
    virtual void Redraw(Coord, Coord, Coord, Coord);
    virtual void Resize();

    TextBuffer* text;
    int left, right;

    ButtonState* subject;
    int accept, cancel;
private:
    void Init(ButtonState*, int, int, const char*);

    const char* sample;
    char* buffer;
    int size;
    TextDisplay* display;
};

static const int SEBeginningOfLine = '\001';
static const int SEEndOfLine = '\005';
static const int SESelectAll = '\025';
static const int SESelectWord = '\027';
static const int SEPreviousCharacter = '\002';
static const int SENectCharacter = '\006';
static const int SEDeleteNextCharacter = '\004';
static const int SEDeletePreviousCharacter = '\177';
static const int SEDeletePreviousCharacterAlt = '\010';
static const int SECancelEdit = '\007';
static const int SEAcceptEdit = '\015';

#endif
