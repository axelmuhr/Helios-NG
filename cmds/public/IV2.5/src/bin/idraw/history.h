// $Header: history.h,v 1.6 88/09/24 15:05:25 interran Exp $
// declares class History.

#ifndef history_h
#define history_h

#include <InterViews/defs.h>

// Declare imported types.

class ChangeNode;
class ChangeList;
class Interactor;

// A History maintains a log of changes made to the drawing to permit
// changes to be undone.

class History {
public:

    History(Interactor*);
    ~History();

    boolean IsEmpty();
    void Clear();
    void Do(ChangeNode*);
    void Redo();
    void Undo();

protected:

    int maxhistory;		// maximum number of changes to store
    ChangeList* changelist;	// stores changes to drawing

};

#endif
