// $Header: commands.h,v 1.7 89/03/19 12:17:11 interran Exp $ 
// declares class Commands.

#ifndef commands_h
#define commands_h

#include "pulldownmenu.h"

// Declare imported types.

class Editor;
class MapKey;
class State;

// A Commands displays a PullDownMenuBar containing several
// PullDownMenuActivators each of which contains a PullDownMenu.

class Commands : public PullDownMenuBar {
public:

    Commands(Editor*, MapKey*, State*);

protected:

    void Init(Editor*, MapKey*, State*);
    void Reconfig();

};

#endif
