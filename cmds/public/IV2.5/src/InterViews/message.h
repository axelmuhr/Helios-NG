/*
 * Message - a simple interactor containing a text string
 */

#ifndef message_h
#define message_h

#include <InterViews/interactor.h>

class Message : public Interactor {
public:
    Message(const char*, Alignment = Center);
    Message(const char*, const char*, Alignment = Center);
    Message(Painter*, const char*, Alignment = Center);

    void Realign(Alignment);
protected:
    const char* text;
    Alignment alignment;

    virtual void Reconfig();
    virtual void Redraw(Coord, Coord, Coord, Coord);
private:
    void Init(const char*, Alignment);
};

#endif
