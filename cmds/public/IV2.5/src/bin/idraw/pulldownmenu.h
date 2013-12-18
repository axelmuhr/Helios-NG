// $Header: pulldownmenu.h,v 1.8 89/04/17 00:31:32 linton Exp $
// declares pulldown menu classes.

#ifndef pulldownmenu_h
#define pulldownmenu_h

#include "highlighter.h"

// Declare imported and used-before-defined types.

class PullDownMenuActivator;
class PullDownMenuCommand;

// A PullDownMenuBar displays several activators and coordinates which
// activator will open its menu.

class PullDownMenuBar : public HighlighterParent {
public:

    PullDownMenuBar();
    ~PullDownMenuBar();

    void Enter(PullDownMenuActivator*);
    boolean Contains(Interactor*);

    boolean MenuActive();
    boolean MenuShouldActivate(PullDownMenuActivator*);

    void MenuActivate(PullDownMenuActivator*);
    void MenuDeactivate();

protected:

    void GrowActivators();

    PullDownMenuActivator* cur;	// stores currently active activator
    int sizeactivators;		// stores current size of dynamic array
    int numactivators;		// stores number of activators in array
    PullDownMenuActivator**
	activators;		// stores bar's interior activators

};

// A PullDownMenuActivator displays a text label and opens a menu when
// you activate it.

class PullDownMenuActivator : public Highlighter {
public:

    PullDownMenuActivator(PullDownMenuBar*, const char*);
    ~PullDownMenuActivator();

    void SetMenu(Scene*);
    void Handle(Event&);

    void Enter(PullDownMenuCommand*);
    boolean Contains(Interactor*);

    void Open();
    void Close();

protected:

    void Reconfig();
    void Redraw(Coord, Coord, Coord, Coord);
    void Resize();
    void GrowCommands();

    PullDownMenuBar* bar;	// stores bar containing this activator
    char* name;			// stores activator's text label
    Scene* menu;		// stores menu to be opened when activated
    int sizecommands;		// stores current size of dynamic array
    int numcommands;		// stores number of commands in array
    PullDownMenuCommand**
	commands;		// stores activator's interior commands

    Coord name_x, name_y;	// stores position at which to display name

};

// A PullDownMenuCommand displays a text label and executes a command.

class PullDownMenuCommand : public Highlighter {
public:

    PullDownMenuCommand(PullDownMenuActivator*, const char*, const char*);
    ~PullDownMenuCommand();

    void Handle(Event&);

    virtual void Execute(Event&);

protected:

    void Reconfig();
    void Redraw(Coord, Coord, Coord, Coord);
    void Resize();

    PullDownMenuActivator*
	activator;		// stores activator which this cmd belongs to
    char* name;			// stores command's text label
    char* key;			// stores label of key which selects this cmd

    Coord name_x, name_y;	// stores position at which to display name
    Coord key_x, key_y;		// stores position at which to display key

};

// A PullDownMenuDivider displays a horizontal line extending the full
// width of the menu, dividing it into two submenus.

class PullDownMenuDivider : public PullDownMenuCommand {
public:

    PullDownMenuDivider();

protected:

    void Redraw(Coord, Coord, Coord, Coord);

};

#endif
