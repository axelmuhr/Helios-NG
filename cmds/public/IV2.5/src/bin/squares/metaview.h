/*
 * The metaview customizes the squares frame.
 */

#ifndef metaview_h
#define metaview_h

#include <InterViews/scene.h>

class Button;
class ButtonState;
class Event;
class Interactor;
class Painter;
class Deck;

/*
 * Squares customization information.
 */

enum AdjusterType { AdjustByScrollers, AdjustByPanner };
enum AdjusterSize { Small, Medium, Large };

class SquaresMetaView : public MonoScene {
public:
    AdjusterType type;
    AdjusterSize size;
    Alignment align;
    boolean right;
    boolean below;
    boolean hscroll;
    boolean vscroll;

    SquaresMetaView();
    SquaresMetaView(SquaresMetaView*);
    ~SquaresMetaView();

    boolean Popup(Event&);
private:
    friend class Builder;

    Deck* deck;
    ButtonState* typeButton;
    ButtonState* sizeButton;
    ButtonState* rightButton;
    ButtonState* belowButton;
    ButtonState* alignButton;
    ButtonState* hscrollButton;
    ButtonState* vscrollButton;
    ButtonState* accept;

    void Init();
    void Make();
};

#endif
