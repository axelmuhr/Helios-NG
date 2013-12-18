// $Header: drawing.h,v 1.9 89/05/12 14:58:39 calder Exp $
// declares class Drawing.

#ifndef drawing_h
#define drawing_h

#include <InterViews/defs.h>

// Declare imported types.

class CenterList;
class Graphic;
class Grid;
class GroupList;
class IBrush;
class IBrushList;
class IColor;
class IColorList;
class IFont;
class IFontList;
class IPattern;
class IPatternList;
class Page;
class PageBoundary;
class PictSelection;
class Picture;
class Selection;
class SelectionList;
class State;
class Transformer;
class booleanList;

// An Orientation specifies a page's orientation (upright or turned).

enum Orientation {
    Portrait,
    Landscape
};

// A Drawing contains the user's picture and provides the interface
// through which editing operations modify it.

class Drawing {
public:

    Drawing(double, double);
    ~Drawing();

    Grid* GetGrid();
    Graphic* GetPage();
    Graphic* GetPageBoundary();
    SelectionList* GetSelectionList();

    boolean Writable(const char*);
    boolean Exists(const char*);
    const char* GetCurrentWorkingDirectory();
    boolean ChangeCurrentWorkingDirectory(const char*);
    void ClearUserPicture();
    boolean ReadUserPicture(const char*, State*);
    boolean PrintUserPicture(const char*);
    boolean WriteUserPicture(const char*);
    SelectionList* ReadClipboard(State*);
    void WriteClipboard();
    void ToggleOrientation();

    void GetBox(Coord&, Coord&, Coord&, Coord&);
    IBrushList* GetBrush();
    CenterList* GetCenter();
    GroupList* GetChildren();
    IColorList* GetFgColor();
    IColorList* GetBgColor();
    SelectionList* GetDuplicates();
    booleanList* GetFillBg();
    IFontList* GetFont();
    int GetNumberOfGraphics();
    GroupList* GetParent();
    IPatternList* GetPattern();
    SelectionList* GetPrevs();
    SelectionList* GetSelections();

    Selection* PickSelectionIntersecting(Coord, Coord);
    Selection* PickSelectionShapedBy(Coord, Coord);
    SelectionList* PickSelectionsWithin(Coord, Coord, Coord, Coord);
    void GetPageCoords(Coord&, Coord&);
    void GetPageCoords(Coord&, Coord&, Coord&, Coord&);

    void Clear();
    void Extend(Selection*);
    void Extend(SelectionList*);
    void Grasp(Selection*);
    void Select(Selection*);
    void Select(SelectionList*);
    void SelectAll();
    void ResetAllHandles();

    void Move(float, float);
    void Scale(float, float);
    void Stretch(float, Alignment);
    void Rotate(float);
    void Align(Alignment, Alignment);
    void AlignToGrid();

    void SetBrush(IBrush*);
    void SetBrush(IBrushList*);
    void SetCenter(CenterList*);
    void SetFgColor(IColor*);
    void SetFgColor(IColorList*);
    void SetBgColor(IColor*);
    void SetBgColor(IColorList*);
    void SetFillBg(boolean);
    void SetFillBg(booleanList*);
    void SetFont(IFont*);
    void SetFont(IFontList*);
    void SetPattern(IPattern*);
    void SetPattern(IPatternList*);

    void Append();
    void Group(GroupList*);
    void InsertAfterPrev(SelectionList*);
    void Prepend();
    void Remove();
    void Replace(Selection*, Selection*);
    void Sort();
    void Ungroup(GroupList*);

protected:

    int NumberOfGraphics(PictSelection*);
    void SetOrientation(Orientation);
    void SetUserPicture(PictSelection*);
    void OrientUserPicture();
    Orientation UprightUserPicture();
    boolean Rotated90(Transformer*);
    boolean Identity(Transformer*);

    Page* page;			// draws entire page
    Grid* grid;			// draws grid
    PictSelection* userpicture;	// draws user's picture
    PageBoundary* pageboundary;	// draws boundary around page
    SelectionList* sl;		// lists picked Selections
    double pagewidth;		// stores width of page
    double pageheight;		// stores height of page
    Orientation orientation;	// stores current orientation of page
    char* clipfilename;		// filename under which to store clippings

};

#endif
