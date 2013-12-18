// $Header: dialogbox.h,v 1.7 89/04/17 00:30:15 linton Exp $
// declares class DialogBox and DialogBox subclasses.

#ifndef dialogbox_h
#define dialogbox_h

#include <InterViews/scene.h>

// Declare imported types.

class ButtonState;
class IMessage;
class StringEdit;

// A DialogBox knows how to set its message and warning text and how
// to pop up itself over the underlying Interactor.

class DialogBox : public MonoScene {
public:

    void SetMessage(const char* = nil, const char* = nil);
    void SetWarning(const char* = nil, const char* = nil);
    void SetUnderlying(Interactor*);

protected:

    DialogBox(Interactor*, const char* = nil);

    void PopUp();
    void Disappear();

    IMessage* message;		// displays message text
    IMessage* warning;		// displays warning text
    Interactor* underlying;	// we'll insert ourselves into its parent

};

// A Messager displays a message until it's acknowledged.

class Messager : public DialogBox {
public:

    Messager(Interactor*, const char* = nil);
    ~Messager();

    void Display();

protected:

    void Init();
    void Reconfig();

    ButtonState* ok;		// stores status of "ok" button
    Interactor* okbutton;	// displays "ok" button

};

// A Confirmer displays a message until it's confirmed or cancelled.

class Confirmer : public DialogBox {
public:

    Confirmer(Interactor*, const char* = nil);
    ~Confirmer();

    char Confirm();

protected:

    void Init();
    void Reconfig();

    ButtonState* yes;		// stores status of "yes" button
    ButtonState* no;		// stores status of "no" button
    ButtonState* cancel;	// stores status of "cancel" button
    Interactor* yesbutton;	// displays "yes" button
    Interactor* nobutton;	// displays "no" button
    Interactor* cancelbutton;	// displays "cancel" button

};

// A Namer displays a string until it's edited or cancelled.

class Namer : public DialogBox {
public:

    Namer(Interactor*, const char* = nil);
    ~Namer();

    char* Edit(const char*);

protected:

    void Init();
    void Reconfig();

    ButtonState* accept;	// stores status of "accept" button
    ButtonState* cancel;	// stores status of "cancel" button
    Interactor* acceptbutton;	// displays "accept" button
    Interactor* cancelbutton;	// displays "cancel" button
    StringEdit* stringeditor;	// displays and edits a string

};

#endif
