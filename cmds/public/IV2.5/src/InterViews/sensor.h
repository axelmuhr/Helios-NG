/*
 * Sensors describe input events of interest.
 */

#ifndef sensor_h
#define sensor_h

#include <InterViews/event.h>
#include <InterViews/resource.h>

class Sensor : public Resource {
public:
    Sensor();
    Sensor(Sensor*);
    ~Sensor();

    void Catch(EventType);
    void CatchButton(EventType, int);
    void CatchChannel(int);
    void CatchTimer(int, int);
    void Ignore(EventType);
    void IgnoreButton(EventType, int);
    void IgnoreChannel(int);
    void CatchRemote();
    void IgnoreRemote();
    boolean Interesting(void*, Event&);
private:
    friend class Interactor;
    friend class Scene;

    unsigned mask;
    unsigned down[8];
    unsigned up[8];
    unsigned channels;
    int maxchannel;
    boolean timer;
    int sec, usec;

    int ButtonIndex (unsigned b) { return (b >> 5) & 07; }
    int ButtonFlag (unsigned b) { return 1 << (b & 0x1f); }
    void SetButton (unsigned a[], unsigned b) {
	a[ButtonIndex(b)] |= ButtonFlag(b);
    }
    void ClearButton (unsigned a[], unsigned b) {
	a[ButtonIndex(b)] &= ~ButtonFlag(b);
    }
    boolean ButtonIsSet (unsigned a[], unsigned b) {
	return (a[ButtonIndex(b)] & ButtonFlag(b)) != 0;
    }
    void SetMouseButtons (unsigned a[]) { a[0] |= 0x7; }
    void ClearMouseButtons (unsigned a[]) { a[0] &= ~0x7; }
    boolean MouseButtons (unsigned a[]) { return (a[0] & 0x7) != 0; }
};

extern Sensor* allEvents;
extern Sensor* onoffEvents;
extern Sensor* updownEvents;
extern Sensor* noEvents;

#endif
