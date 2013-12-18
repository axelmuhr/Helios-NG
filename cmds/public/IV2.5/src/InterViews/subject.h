/*
 * A subject is an object that has one or more views
 * that it wishes to notify when it changes.
 */

#ifndef subject_h
#define subject_h

#include <InterViews/defs.h>

class Interactor;

class Subject {
public:
    Subject();
    ~Subject();

    virtual void Attach(Interactor*);
    virtual void Detach(Interactor*);
    virtual void Notify();
    void DeleteViews();
private:
    class ViewList* views;
};

#endif
