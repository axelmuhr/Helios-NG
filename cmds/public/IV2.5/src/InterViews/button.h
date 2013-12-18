/*
 * A button is a view of some value that is normally set when
 * the button is pressed.
 */

#ifndef button_h
#define button_h

#include <InterViews/interactor.h>
#include <InterViews/subject.h>

class Button;
class ButtonList;
class Bitmap;

class ButtonState : public Subject {
public:
    ButtonState();
    ButtonState(int);
    ButtonState(void*);

    void operator=(ButtonState&);
    void SetValue(int);
    void SetValue(void*);
    void GetValue (int& v) { v = (int)value; }
    void GetValue (boolean& v) { v = (boolean)value; }
    void GetValue (void*& v) { v = value; }
protected:
    void* value;

    void Modify(void*);
};

class Button : public Interactor {
public:
    void Attach(Button*);
    void Detach(Button*);

    void Enable();
    void Disable();

    void Choose();
    void UnChoose();

    virtual void Press();
    virtual void Refresh();

    void SetDimensions(int width, int height);

    virtual void Handle(Event&);
    virtual void Update();
protected:
    Button(ButtonState*, void*);
    Button(const char*, ButtonState*, void*);
    Button(Painter*, ButtonState*, void*);
    ~Button();

    void* value;		/* value associated with this button */
    ButtonState* subject;	/* set to this->value when pressed */
    ButtonList* associates;	/* enable/disable when chosen/unchosen */
    boolean enabled;		/* can be pressed */
    boolean chosen;		/* currently toggled on */
    boolean hit;		/* currently being pushed */
    static Painter* grayout;	/* for disabled buttons */
private:
    void Init(ButtonState*, void*);
};

class TextButton : public Button {
protected:
    const char* text;
    Painter* background;

    TextButton(const char*, const char*, ButtonState*, void*);
    TextButton(const char*, ButtonState*, void*);
    TextButton(const char*, ButtonState*, void*, Painter*);
    ~TextButton();

    void MakeBackground();
    void MakeShape();
private:
    void Init(const char*);
};

class PushButton : public TextButton {
public:
    PushButton(const char*, ButtonState*, int);
    PushButton(const char*, ButtonState*, void*);
    PushButton(const char*, const char*, ButtonState*, int);
    PushButton(const char*, const char*, ButtonState*, void*);
    PushButton(const char*, ButtonState*, int, Painter*);
    PushButton(const char*, ButtonState*, void*, Painter*);

    virtual void Refresh();
protected:
    virtual void Reconfig();
    virtual void Redraw(Coord, Coord, Coord, Coord);
private:
    void Init();
};

class RadioButton : public TextButton {
public:
    RadioButton(const char*, ButtonState*, int);
    RadioButton(const char*, ButtonState*, void*);
    RadioButton(const char*, const char*, ButtonState*, int);
    RadioButton(const char*, const char*, ButtonState*, void*);
    RadioButton(const char*, ButtonState*, int, Painter*);
    RadioButton(const char*, ButtonState*, void*, Painter*);

    virtual void Refresh();
protected:
    virtual void Reconfig();
    virtual void Redraw(Coord, Coord, Coord, Coord);
private:
    void Init();
};

class CheckBox : public TextButton {
public:
    CheckBox(const char*, ButtonState*, int, int);
    CheckBox(const char*, ButtonState*, void*, void*);
    CheckBox(const char*, const char*, ButtonState*, int, int);
    CheckBox(const char*, const char*, ButtonState*, void*, void*);
    CheckBox(const char*, ButtonState*, int, int, Painter*);
    CheckBox(const char*, ButtonState*, void*, void*, Painter*);

    virtual void Press();
    virtual void Refresh();
    virtual void Update();
protected:
    virtual void Reconfig();
    virtual void Redraw(Coord, Coord, Coord, Coord);
private:
    void* offvalue;

    void Init(void*);
};

#endif
