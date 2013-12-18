// $Header: editor.h,v 1.9 89/04/03 19:50:37 interran Exp $
// declares class Editor.

#ifndef editor_h
#define editor_h

#include <InterViews/defs.h>

// Declare imported types.

class ChangeNode;
class Confirmer;
class Drawing;
class DrawingView;
class Event;
class Grid;
class History;
class IBrush;
class IColor;
class IFont;
class IPattern;
class Interactor;
class Messager;
class Namer;
class Painter;
class RubberEllipse;
class RubberLine;
class RubberRect;
class State;

// An Editor lets the user perform a drawing or editing operation on
// a Drawing.

class Editor {
public:

    Editor(Interactor*);
    ~Editor();

    void SetDrawing(Drawing*);
    void SetDrawingView(DrawingView*);
    void SetGrid(Grid*);
    void SetState(State*);

    void HandleSelect(Event&);
    void HandleMove(Event&);
    void HandleScale(Event&);
    void HandleStretch(Event&);
    void HandleRotate(Event&);
    void HandleReshape(Event&);
    void HandleMagnify(Event&);
    void HandleText(Event&);
    void HandleLine(Event&);
    void HandleMultiLine(Event&);
    void HandleBSpline(Event&);
    void HandleEllipse(Event&);
    void HandleRect(Event&);
    void HandlePolygon(Event&);
    void HandleClosedBSpline(Event&);

    void New();
    void Revert();
    void Open(const char*);
    void Open();
    void Save();
    void SaveAs();
    void Print();
    void Directory();
    void Quit(Event&);
    void Checkpoint();

    void Undo();
    void Redo();
    void Cut();
    void Copy();
    void Paste();
    void Duplicate();
    void Delete();
    void SelectAll();
    void FlipHorizontal();
    void FlipVertical();
    void _90Clockwise();
    void _90CounterCW();
    void PreciseMove();
    void PreciseScale();
    void PreciseRotate();

    void Group();
    void Ungroup();
    void BringToFront();
    void SendToBack();
    void NumberOfGraphics();

    void SetBrush(IBrush*);
    void SetFgColor(IColor*);
    void SetBgColor(IColor*);
    void SetFont(IFont*);
    void SetPattern(IPattern*);

    void AlignLeftSides();
    void AlignRightSides();
    void AlignBottoms();
    void AlignTops();
    void AlignVertCenters();
    void AlignHorizCenters();
    void AlignCenters();
    void AlignLeftToRight();
    void AlignRightToLeft();
    void AlignBottomToTop();
    void AlignTopToBottom();
    void AlignToGrid();

    void Reduce();
    void Enlarge();
    void ReduceToFit();
    void NormalSize();
    void CenterPage();
    void GriddingOnOff();
    void GridVisibleInvisible();
    void GridSpacing();
    void Orientation();
    void ShowVersion();

protected:

    void Do(ChangeNode*);
    void InputVertices(Event&, Coord*&, Coord*&, int&);
    RubberLine* NewRubberLineOrAxis(Event&);
    RubberEllipse* NewRubberEllipseOrCircle(Event&);
    RubberRect* NewRubberRectOrSquare(Event&);
    boolean OfferToSave();
    void Reset(const char*);

    History* history;		// carries out and logs changes made to drawing
    Namer* dirdialog;		// prompts for a new working directory
    Messager* numberofdialog;	// displays how many graphics the drawing has
    Namer* opendialog;		// prompts for name of a drawing to open
    Confirmer* overwritedialog;	// confirms whether to overwrite a file
    Namer* precmovedialog;	// prompts for X and Y movement in points
    Namer* precrotdialog;	// prompts for rotation in degrees
    Namer* precscaledialog;	// prompts for X and Y scaling
    Namer* printdialog;		// prompts for print command
    Messager* readonlydialog;	// tells user drawing is readonly
    Confirmer* revertdialog;	// confirms whether to revert from a file
    Namer* saveasdialog;	// prompts for name to save drawing as
    Confirmer* savecurdialog;	// confirms whether to save current drawing
    Namer* spacingdialog;	// prompts for grid spacing in points
    Messager* versiondialog;	// displays idraw's version level and author

    Drawing* drawing;		// performs operations on drawing
    DrawingView* drawingview;	// displays drawing
    Grid* grid;			// constrains points to lie on a grid
    State* state;		// stores Graphic and nonGraphic attributes

};

#endif
