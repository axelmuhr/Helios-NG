/*
 * Pages - demo for Text and Layout
 */

#ifndef pages_h
#define pages_h

#include <InterViews/scene.h>
#include <stdio.h>

class Deck;
class Layout;
class TextPainter;
class Text;
class Sensor;
class Event;
class Painter;
class EditWord;

class Pages : public MonoScene {
public:
    Pages(FILE*, int pages, int columns);
protected:
    virtual void Delete();
    virtual void Handle(Event&);
protected:
    Deck* deck;
    Layout* layout;
    int columns;
    int pages;

    TextPainter* highlight;
    TextPainter* normal;

    Sensor* sensor;

    Text* hit;
    Text* after;
    EditWord* edit;
    char editbuffer[256];
};

#endif
