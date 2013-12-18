/*
 * A frame surrounds the squares view with banner and scroll bars.
 */

#ifndef sframe_h
#define sframe_h

#include <InterViews/scene.h>

class ButtonState;
class Dialog;
class Menu;
class Viewport;
class SquaresFrameStyle;
class SquaresMetaView;
class SquaresView;

class SquaresFrame : public MonoScene {
public:
    SquaresFrame(SquaresView*);
    ~SquaresFrame();

    virtual void Handle(Event&);
private:
    SquaresView* view;
    Viewport* viewport;
    Menu* menu;
    Menu* adjust;
    Menu* quit;
    SquaresMetaView* style;

    SquaresFrame(SquaresFrame*);
    void Init();
    void MakeFrame();
    Interactor* ScrollerFrameInterior();
    Interactor* PannerFrameInterior();
};

#endif
