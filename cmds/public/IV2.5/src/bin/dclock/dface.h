/* 
 * digital clockface class
 */

#ifndef dface_h
#define dface_h

struct CharPoints {
    int count;
    Coord x[12];
    Coord y[12];
};

enum AMPMMODE { BLANK, AM, PM };

class DFace : public Interactor {
public:
    DFace(
	boolean showDate, boolean showBorder, boolean showTime,
	TMode timeMode, int width = 0, int height = 0
    );
    ~DFace();

    virtual void Reconfig();
    virtual void Resize();
    virtual void Draw();
    virtual void Handle(Event&);
    void Run();
protected:
    void DrawFace();
    void DrawColon();
    void DrawAMPM(Painter*);
    void DrawDate();
    void DrawBorder();
    void Redraw(Coord left, Coord bottom, Coord right, Coord top);
    void RedrawList(int, Coord[], Coord[], Coord[], Coord[]);
    void Set(char* today, int hours, int minutes);
    void Tick();
private:
    Clock* clock;
    TMode mode;				/* civil or military */
    AMPMMODE AMPMmode;
    SegPoints colon[2];			/* colon shape data */
    CharPoints A, P, M;			/* character shape data */
    Painter* invertor;			/* for highlights and erasing */
    boolean showDate;			/* visibility of date */
    boolean showBorder;			/* visibility of date/time line */
    boolean showTime;			/* visibility of time */
    boolean selected;			/* highlight date if true */
    boolean done;			/* program terminator */

    Digit* ht, * hu, * mt, * mu;

    struct {				/* date string */
	int len;
	char text[50];
    } date;
};

#endif
