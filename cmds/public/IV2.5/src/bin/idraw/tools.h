// $Header: tools.h,v 1.6 88/09/24 15:09:21 interran Exp $
// declares class Tools.

#ifndef tools_h
#define tools_h

#include "panel.h"

// Declare imported types.

class Editor;
class MapKey;

// A Tools displays several drawing tools to choose from.

class Tools : public Panel {
public:

    Tools(Editor*, MapKey*);

    void Handle(Event&);

protected:

    void Init(Editor*, MapKey*);
    void Reconfig();

};

#endif
