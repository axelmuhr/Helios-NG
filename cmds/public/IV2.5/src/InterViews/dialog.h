/*
 * Dialog - a simple dialog box input handler.
 */

#ifndef dialog_h
#define dialog_h

#include <InterViews/scene.h>

class ButtonState;

class Dialog : public MonoScene {
public:
    Dialog(ButtonState*, Interactor*, Alignment = Center);
    Dialog(const char*, ButtonState*, Interactor*, Alignment = Center);

    virtual boolean Accept();
    virtual boolean Popup(Event&, boolean placed = true);
    int Status();
protected:
    ButtonState* state;
    Alignment align;
private:
    void Init(ButtonState*, Interactor*, Alignment);
};

#endif
