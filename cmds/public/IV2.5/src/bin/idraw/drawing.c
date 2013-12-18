// $Header: drawing.c,v 1.14 89/05/29 00:13:16 interran Exp $
// implements class Drawing.

#include "drawing.h"
#include "grid.h"
#include "ipaint.h"
#include "istring.h"
#include "listboolean.h"
#include "listcenter.h"
#include "listgroup.h"
#include "listibrush.h"
#include "listicolor.h"
#include "listifont.h"
#include "listipattern.h"
#include "listselectn.h"
#include "page.h"
#include "pageboundary.h"
#include "slpict.h"
#include <InterViews/Graphic/polygons.h>
#include <InterViews/transformer.h>
#include <os/fs.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>			/* define constants for access call */

// Drawing creates the page, selection list, and clipboard filename.
// It should have gotten w < h, i.e., portrait dimensions.

Drawing::Drawing (double w, double h) {
    page = new Page(w, h);
    grid = new Grid(w, h);
    grid->SetBrush(psingle);
    grid->SetColors(pblack, pwhite);
    grid->FillBg(true);
    userpicture = new PictSelection;
    pageboundary = new PageBoundary(w, h);
    pageboundary->SetColors(pblack, pwhite);
    pageboundary->FillBg(true);
    pageboundary->SetPattern(psolid);
    page->Append(grid, userpicture, pageboundary);
    sl = new SelectionList;
    pagewidth = w;
    pageheight = h;
    orientation = Portrait;

    const char* home = (home = getenv("HOME")) ? home : ".";
    const char* name = ".clipboard";
    clipfilename = new char[strlen(home) + 1 + strlen(name) + 1];
    strcpy(clipfilename, home);
    strcat(clipfilename, "/");
    strcat(clipfilename, name);
}

// ~Drawing frees storage allocated for the page, selection list, and
// clipboard filename.

Drawing::~Drawing () {
    delete page;
    delete sl;
    delete clipfilename;
}

// Define access functions to return members' values.  DrawingView
// uses grid, page, pageboundary, and sl.  Editor uses grid.

Grid* Drawing::GetGrid () {
    return grid;
}

Graphic* Drawing::GetPage () {
    return page;
}

Graphic* Drawing::GetPageBoundary () {
    return pageboundary;
}

SelectionList* Drawing::GetSelectionList () {
    return sl;
}

// Writable returns true only if the given drawing is writable.

boolean Drawing::Writable (const char* path) {
    return (access(path, W_OK) >= 0);
}

// Exists returns true only if the drawing already exists.

boolean Drawing::Exists (const char* path) {
    return (access(path, F_OK) >= 0);
}

// GetCurrentWorkingDirectory returns the current working directory or
// nil if the getwd call fails.

const char* Drawing::GetCurrentWorkingDirectory () {
    const int MAXPATHLEN = 1024;
    static char cwd[MAXPATHLEN];
    return getwd(cwd);
}

// ChangeCurrentWorkingDirectory returns true if it changes the
// current working directory or false if the chdir call fails.

boolean Drawing::ChangeCurrentWorkingDirectory (const char* path) {
    return (chdir(path) == 0);
}

// ClearUserPicture deletes the old picture and creates a new empty
// picture.

void Drawing::ClearUserPicture () {
    SetUserPicture(new PictSelection);
    grid->SetSpacing(GRID_DEFAULTSPACING);
}

// ReadUserPicture reads a new picture and replaces the old picture
// with the new picture if the read succeeds.  It also reads and sets
// the new picture's orientation.

boolean Drawing::ReadUserPicture (const char* path, State* state) {
    boolean successful = false;
    if (path != nil) {
	FILE* stream = fopen(path, "r");
	if (stream != nil) {
	    PictSelection* newpic = new PictSelection(stream, state);
	    fclose(stream);
	    if (newpic->Valid()) {
		SetUserPicture(newpic);
		Orientation ot = UprightUserPicture();
		SetOrientation(ot);
		grid->SetSpacing(newpic->GetGridSpacing());
		successful = true;
	    } else {
		delete newpic;
		fprintf(stderr, "Drawing: input error in reading %s\n", path);
	    }
	}
    }
    return successful;
}

// PrintUserPicture prints the current picture by writing it through a
// pipe to a print command.

boolean Drawing::PrintUserPicture (const char* cmd) {
    boolean successful = false;
    if (cmd != nil) {
	FILE* stream = popen(cmd, "w");
	if (stream != nil) {
	    userpicture->SetGridSpacing(grid->GetSpacing());
	    OrientUserPicture();
	    successful = userpicture->WritePicture(stream, true);
	    UprightUserPicture();
	    pclose(stream);
	}
    }
    return successful;
}

// WriteUserPicture writes the current picture to a file.

boolean Drawing::WriteUserPicture (const char* path) {
    boolean successful = false;
    if (path != nil) {
	FILE* stream = fopen(path, "w");
	if (stream != nil) {
	    userpicture->SetGridSpacing(grid->GetSpacing());
	    OrientUserPicture();
	    successful = userpicture->WritePicture(stream, true);
	    UprightUserPicture();
	    fclose(stream);
	}
    }
    return successful;
}

// ReadClipboard returns copies of the Selections within the clipboard
// file in a newly allocated list.

SelectionList* Drawing::ReadClipboard (State* state) {
    SelectionList* sl = new SelectionList;
    FILE* stream = fopen(clipfilename, "r");
    if (stream != nil) {
	PictSelection* newpic = new PictSelection(stream, state);
	fclose(stream);
	if (newpic->Valid()) {
	    newpic->Propagate();
	    for (newpic->First(); !newpic->AtEnd(); newpic->RemoveCur()) {
		Selection* child = (Selection*) newpic->GetCurrent();
		sl->Append(new SelectionNode(child));
	    }
	}
	delete newpic;
    } else {
	fprintf(stderr, "Drawing: can't open %s\n", clipfilename);
    }
    return sl;
}

// WriteClipboard writes the picked Selections to the clipboard file,
// overwriting its previous contents.

void Drawing::WriteClipboard () {
    FILE* stream = fopen(clipfilename, "w");
    if (stream != nil) {
	PictSelection* newpic = new PictSelection;
	newpic->SetGridSpacing(grid->GetSpacing());
	for (sl->First(); !sl->AtEnd(); sl->Next()) {
	    Graphic* copy = sl->GetCur()->GetSelection()->Copy();
	    newpic->Append(copy);
	}
	newpic->WritePicture(stream, false);
	fclose(stream);
	delete newpic;
    } else {
	fprintf(stderr, "Drawing: can't open %s\n", clipfilename);
    }
}

// ToggleOrientation toggles the page boundary between portrait and
// landscape orientations without modifying the picture.

void Drawing::ToggleOrientation () {
    Orientation ot = (orientation == Landscape) ? Portrait : Landscape;
    SetOrientation(ot);
}

// GetBox gets the smallest box bounding all the Selections.

void Drawing::GetBox (Coord& l, Coord& b, Coord& r, Coord& t) {
    BoxObj btotal;
    BoxObj bselection;

    if (sl->Size() >= 1) {
	sl->First()->GetSelection()->GetBox(btotal);
	for (sl->Next(); !sl->AtEnd(); sl->Next()) {
	    sl->GetCur()->GetSelection()->GetBox(bselection);
	    btotal = btotal + bselection;
	}
	l = btotal.left;
	b = btotal.bottom;
	r = btotal.right;
	t = btotal.top;
    }
}

// GetBrush returns the Selections' brush attributes in a newly
// allocated list.

IBrushList* Drawing::GetBrush () {
    IBrushList* brushlist = new IBrushList;
    for (sl->First(); !sl->AtEnd(); sl->Next()) {
	IBrush* brush = (IBrush*) sl->GetCur()->GetSelection()->GetBrush();
	brushlist->Append(new IBrushNode(brush));
    }
    return brushlist;
}

// GetCenter returns the Selections' centers in a newly allocated
// list.  It converts the centers from absolute (screen) coordinates
// to page coordinates so zooming the view won't change the centers.

CenterList* Drawing::GetCenter () {
    CenterList* centerlist = new CenterList;
    for (sl->First(); !sl->AtEnd(); sl->Next()) {
	float abscx, abscy;
	sl->GetCur()->GetSelection()->GetCenter(abscx, abscy);

	Transformer screen;
	userpicture->TotalTransformation(screen);
	float cx, cy;
	screen.InvTransform(abscx, abscy, cx, cy);
	centerlist->Append(new CenterNode(cx, cy));
    }
    return centerlist;
}

// GetChildren returns the Selections and their children, if any, in a
// newly allocated list.

GroupList* Drawing::GetChildren () {
    GroupList* grouplist = new GroupList;
    for (sl->First(); !sl->AtEnd(); sl->Next()) {
	PictSelection* parent = (PictSelection*) sl->GetCur()->GetSelection();
	boolean haschildren = parent->HasChildren();
	SelectionList* children = new SelectionList;
	if (haschildren) {
	    for (parent->First(); !parent->AtEnd(); parent->Next()) {
		Selection* child = parent->GetCurrent();
		children->Append(new SelectionNode(child));
	    }
	}
	grouplist->Append(new GroupNode(parent, haschildren, children));
	delete children;
    }
    return grouplist;
}

// GetFgColor returns the Selections' FgColor attributes in a newly
// allocated list.

IColorList* Drawing::GetFgColor () {
    IColorList* fgcolorlist = new IColorList;
    for (sl->First(); !sl->AtEnd(); sl->Next()) {
	IColor* fgcolor = (IColor*) sl->GetCur()->GetSelection()->GetFgColor();
	fgcolorlist->Append(new IColorNode(fgcolor));
    }
    return fgcolorlist;
}

// GetBgColor returns the Selections' BgColor attributes in a newly
// allocated list.

IColorList* Drawing::GetBgColor () {
    IColorList* bgcolorlist = new IColorList;
    for (sl->First(); !sl->AtEnd(); sl->Next()) {
	IColor* bgcolor = (IColor*) sl->GetCur()->GetSelection()->GetBgColor();
	bgcolorlist->Append(new IColorNode(bgcolor));
    }
    return bgcolorlist;
}

// GetDuplicates duplicates the Selections, offsets them by one grid
// spacing, and returns them in a newly allocated list.

SelectionList* Drawing::GetDuplicates () {
    SelectionList* duplicates = new SelectionList;
    for (sl->First(); !sl->AtEnd(); sl->Next()) {
	Selection* dup = (Selection*) sl->GetCur()->GetSelection()->Copy();

	Coord dx = round(grid->GetSpacing()*points);
	Coord dy = dx;
	dup->Translate(dx, dy);

	duplicates->Append(new SelectionNode(dup));
    }
    return duplicates;
}

// GetFillBg returns the Selections' fillbg attributes in a newly
// allocated list.

booleanList* Drawing::GetFillBg () {
    booleanList* fillbglist = new booleanList;
    for (sl->First(); !sl->AtEnd(); sl->Next()) {
	boolean fillbg = sl->GetCur()->GetSelection()->BgFilled();
	fillbglist->Append(new booleanNode(fillbg)); 
    }
    return fillbglist;
}

// GetFont returns the Selections' Font attributes in a newly
// allocated list.

IFontList* Drawing::GetFont () {
    IFontList* fontlist = new IFontList;
    for (sl->First(); !sl->AtEnd(); sl->Next()) {
	IFont* font = (IFont*) sl->GetCur()->GetSelection()->GetFont();
	fontlist->Append(new IFontNode(font));
    }
    return fontlist;
}

// GetNumberOfGraphics returns the number of graphics in the
// Selections.

int Drawing::GetNumberOfGraphics () {
    int num = 0;
    for (sl->First(); !sl->AtEnd(); sl->Next()) {
	Selection* s = sl->GetCur()->GetSelection();
	if (s->HasChildren()) {
	    num += NumberOfGraphics((PictSelection*) s);
	} else {
	    ++num;
	}
    }
    return num;
}

// GetParent returns the Selections and their new parent in a newly
// allocated list if there are enough Selections to form a Group.

GroupList* Drawing::GetParent () {
    GroupList* grouplist = new GroupList;
    if (sl->Size() >= 2) {
	PictSelection* parent = new PictSelection;
	boolean haschildren = true;
	grouplist->Append(new GroupNode(parent, haschildren, sl));
    }
    return grouplist;
}

// GetPattern returns the Selections' Pattern attributes in a newly
// allocated list.

IPatternList* Drawing::GetPattern () {
    IPatternList* patternlist = new IPatternList;
    for (sl->First(); !sl->AtEnd(); sl->Next()) {
	IPattern* pattern =
	    (IPattern*) sl->GetCur()->GetSelection()->GetPattern();
	patternlist->Append(new IPatternNode(pattern));
    }
    return patternlist;
}

// GetPrevs returns the Selections' predecessors within the picture in
// a newly allocated list.

SelectionList* Drawing::GetPrevs () {
    SelectionList* prevlist = new SelectionList;
    for (sl->First(); !sl->AtEnd(); sl->Next()) {
	userpicture->SetCurrent(sl->GetCur()->GetSelection());
	Selection* prev = userpicture->Prev();
	prevlist->Append(new SelectionNode(prev));
    }
    return prevlist;
}

// GetSelections returns the Selections in a newly allocated list.

SelectionList* Drawing::GetSelections () {
    SelectionList* newsl = new SelectionList;
    for (sl->First(); !sl->AtEnd(); sl->Next()) {
	Selection* s = sl->GetCur()->GetSelection();
	newsl->Append(new SelectionNode(s));
    }
    return newsl;
}

// PickSelectionIntersecting returns the last Selection intersecting a
// box around the given point.

Selection* Drawing::PickSelectionIntersecting (Coord x, Coord y) {
    const int SLOP = 2;
    BoxObj pickpoint(x - SLOP, y - SLOP, x + SLOP, y + SLOP);
    return userpicture->LastSelectionIntersecting(pickpoint);
}

// PickSelectionShapedBy returns the last Selection shaped by a point
// close to the given point.

Selection* Drawing::PickSelectionShapedBy (Coord x, Coord y) {
    const float SLOP = 6.;
    for (userpicture->Last(); !userpicture->AtEnd(); userpicture->Prev()) {
	Selection* pick = userpicture->GetCurrent();
	if (pick->ShapedBy(x, y, SLOP)) {
	    return pick;
	}
    }
    return nil;
}

// PickSelectionsWithin returns all the Selections within the given
// box.

SelectionList* Drawing::PickSelectionsWithin (Coord l, Coord b, Coord r,
Coord t) {
    Selection** picks = nil;
    int numpicks = userpicture->SelectionsWithin(BoxObj(l, b, r, t), picks);
    SelectionList* picklist = new SelectionList;
    for (int i = 0; i < numpicks; i++) {
	picklist->Append(new SelectionNode(picks[i]));
    }
    delete picks;
    return picklist;
}

// GetPageCoords transforms the given absolute (screen) coordinates
// to be relative to the page's coordinate system.

void Drawing::GetPageCoords (Coord& x0, Coord& y0) {
    Transformer screen;
    userpicture->TotalTransformation(screen);
    screen.InvTransform(x0, y0);
}

void Drawing::GetPageCoords (Coord& x0, Coord& y0, Coord& x1, Coord& y1) {
    Transformer screen;
    userpicture->TotalTransformation(screen);
    screen.InvTransform(x0, y0);
    screen.InvTransform(x1, y1);
}

// Clear empties the SelectionList.

void Drawing::Clear () {
    sl->DeleteAll();
}

// Extend extends the SelectionList to include the picked Selection
// unless it's already there, in which case it removes the Selection.

void Drawing::Extend (Selection* pick) {
    if (!sl->Find(pick)) {
	sl->Append(new SelectionNode(pick));
    } else {
	sl->DeleteCur();
    }
}

// Extend extends the SelectionList to include the picked Selections
// unless they're already there, in which case it removes them.

void Drawing::Extend (SelectionList* picklist) {
    for (picklist->First(); !picklist->AtEnd(); picklist->Next()) {
	Selection* pick = picklist->GetCur()->GetSelection();
	Extend(pick);
    }
}

// Grasp selects the picked Selection only if the SelectionList does
// not already include it.

void Drawing::Grasp (Selection* pick) {
    if (!sl->Find(pick)) {
	Select(pick);
    }
}

// Select selects the picked Selection.

void Drawing::Select (Selection* pick) {
    sl->DeleteAll();
    sl->Append(new SelectionNode(pick));
}

// Select selects the picked Selections.

void Drawing::Select (SelectionList* picklist) {
    sl->DeleteAll();
    for (picklist->First(); !picklist->AtEnd(); picklist->Next()) {
	Selection* pick = picklist->GetCur()->GetSelection();
	sl->Append(new SelectionNode(pick));
    }
}

// SelectAll selects all of the Selections in the picture.

void Drawing::SelectAll () {
    sl->DeleteAll();
    for (userpicture->First(); !userpicture->AtEnd(); userpicture->Next()) {
	Selection* pick = userpicture->GetCurrent();
	sl->Append(new SelectionNode(pick));
    }
}

// ResetAllHandles resets all of the Selections' handles because the
// Selections may have moved out from under their handles.

void Drawing::ResetAllHandles () {
    for (userpicture->First(); !userpicture->AtEnd(); userpicture->Next()) {
	Selection* s = userpicture->GetCurrent();
	s->ResetHandles();
    }
}

// Move translates the Selections.

void Drawing::Move (float xdisp, float ydisp) {
    for (sl->First(); !sl->AtEnd(); sl->Next()) {
	Selection* s = sl->GetCur()->GetSelection();
	s->Translate(xdisp, ydisp);
    }
}

// Scale scales the Selections about their centers.

void Drawing::Scale (float xscale, float yscale) {
    for (sl->First(); !sl->AtEnd(); sl->Next()) {
	Selection* s = sl->GetCur()->GetSelection();
	float cx, cy;
	s->GetCenter(cx, cy);
	s->Scale(xscale, yscale, cx, cy);
    }
}

// Stretch stretches the Selections while keeping the given side
// fixed.

void Drawing::Stretch (float stretch, Alignment side) {
    for (sl->First(); !sl->AtEnd(); sl->Next()) {
	Selection* s = sl->GetCur()->GetSelection();
	float l, b, r, t;
	s->GetBounds(l, b, r, t);
	switch (side) {
	case Left:
	    s->Scale(stretch, 1, r, t);
	    break;
	case Bottom:
	    s->Scale(1, stretch, r, t);
	    break;
	case Right:
	    s->Scale(stretch, 1, l, b);
	    break;
	case Top:
	    s->Scale(1, stretch, l, b);
	    break;
	default:
	    fprintf(stderr, "inappropriate enum passed to Drawing::Stretch\n");
	    break;
	}
    }
}

// Rotate rotates the Selections about their centers.

void Drawing::Rotate (float angle) {
    for (sl->First(); !sl->AtEnd(); sl->Next()) {
	Selection* s = sl->GetCur()->GetSelection();
	float cx, cy;
	s->GetCenter(cx, cy);
	s->Rotate(angle, cx, cy);
    }
}

// Align either aligns up all of the Selections or abuts all of them
// side to side, depending on whether the moving Selection's side or
// center aligns with the fixed Selection's same side or center.

void Drawing::Align (Alignment falign, Alignment malign) {
    if (falign == malign) {
	Selection* stays = sl->First()->GetSelection();
	for (sl->Next(); !sl->AtEnd(); sl->Next()) {
	    Selection* moves = sl->GetCur()->GetSelection();
	    stays->Align(falign, moves, malign);
	}
    } else {
	Selection* stays = sl->First()->GetSelection();
	for (sl->Next(); !sl->AtEnd(); sl->Next()) {
	    Selection* moves = sl->GetCur()->GetSelection();
	    stays->Align(falign, moves, malign);
	    stays = moves;
	}
    }
}

// AlignToGrid aligns the Selections' lower left corners to the
// nearest grid point.

void Drawing::AlignToGrid () {
    boolean gridding = grid->GetGridding();
    grid->SetGridding(true);

    for (sl->First(); !sl->AtEnd(); sl->Next()) {
	Selection* s = sl->GetCur()->GetSelection();
	float fl, fb, fr, ft;
	s->GetBounds(fl, fb, fr, ft);
	Coord l = round(fl);
	Coord b = round(fb);
	Coord nl = l;
	Coord nb = b;
	grid->Constrain(nl, nb);
	GetPageCoords(l, b, nl, nb);
	s->Translate(nl - l, nb - b);
    }

    grid->SetGridding(gridding);
}

// SetBrush sets the Selections' brush attributes with the given brush
// attribute.

void Drawing::SetBrush (IBrush* brush) {
    for (sl->First(); !sl->AtEnd(); sl->Next()) {
	sl->GetCur()->GetSelection()->SetBrush(brush);
    }
}

// SetBrush sets each Selection's brush attribute with the
// corresponding brush attribute in the provided list.

void Drawing::SetBrush (IBrushList* brushlist) {
    for (sl->First(), brushlist->First(); !sl->AtEnd() && !brushlist->AtEnd();
	 sl->Next(), brushlist->Next())
    {
	IBrush* brush = brushlist->GetCur()->GetBrush();
	sl->GetCur()->GetSelection()->SetBrush(brush);
    }
}

// SetCenter centers each of the Selections over the corresponding
// position in the provided list.  It expects the passed postions to
// be already in page coordinates.

void Drawing::SetCenter (CenterList* centerlist) {
    for (sl->First(), centerlist->First();
	 !sl->AtEnd() && !centerlist->AtEnd();
	 sl->Next(), centerlist->Next())
    {
	float newcx = centerlist->GetCur()->GetCx();
	float newcy = centerlist->GetCur()->GetCy();

	Selection* s = sl->GetCur()->GetSelection();
	float absoldcx, absoldcy;
	s->GetCenter(absoldcx, absoldcy);

	Transformer screen;
	userpicture->TotalTransformation(screen);
	float oldcx, oldcy;
	screen.InvTransform(absoldcx, absoldcy, oldcx, oldcy);
	s->Translate(newcx - oldcx, newcy - oldcy);
    }
}

// SetFgColor sets the Selections' foreground color attributes with
// the given color attribute.

void Drawing::SetFgColor (IColor* fgcolor) {
    for (sl->First(); !sl->AtEnd(); sl->Next()) {
	Selection* s = sl->GetCur()->GetSelection();
	IColor* bgcolor = (IColor*) s->GetBgColor();
	s->SetColors(fgcolor, bgcolor);
    }
}

// SetFgColor sets the Selections' foreground color attributes with
// the corresponding color attributes in the provided list.

void Drawing::SetFgColor (IColorList* fgcolorlist) {
    for (sl->First(), fgcolorlist->First();
	 !sl->AtEnd() && !fgcolorlist->AtEnd();
	 sl->Next(), fgcolorlist->Next())
    {
	Selection* s = sl->GetCur()->GetSelection();
	IColor* fgcolor = fgcolorlist->GetCur()->GetColor();
	IColor* bgcolor = (IColor*) s->GetBgColor();
	s->SetColors(fgcolor, bgcolor);
    }
}

// SetBgColor sets the Selections' background color attributes with
// the given color attribute.

void Drawing::SetBgColor (IColor* bgcolor) {
    for (sl->First(); !sl->AtEnd(); sl->Next()) {
	Selection* s = sl->GetCur()->GetSelection();
	IColor* fgcolor = (IColor*) s->GetFgColor();
	s->SetColors(fgcolor, bgcolor);
    }
}

// SetBgColor sets the Selections' background color attributes with
// the corresponding color attributes in the provided list.

void Drawing::SetBgColor (IColorList* bgcolorlist) {
    for (sl->First(), bgcolorlist->First();
	 !sl->AtEnd() && !bgcolorlist->AtEnd();
	 sl->Next(), bgcolorlist->Next())
    {
	Selection* s = sl->GetCur()->GetSelection();
	IColor* fgcolor = (IColor*) s->GetFgColor();
	IColor* bgcolor = bgcolorlist->GetCur()->GetColor();
	s->SetColors(fgcolor, bgcolor);
    }
}

// SetFillBg sets the Selections' fillbg attributes with the given
// fillbg attribute.

void Drawing::SetFillBg (boolean fillbg) {
    for (sl->First(); !sl->AtEnd(); sl->Next()) {
	sl->GetCur()->GetSelection()->FillBg(fillbg);
    }
}

// SetFillBg sets each Selection's fillbg attribute with the
// corresponding fillbg attribute in the provided list.

void Drawing::SetFillBg (booleanList* fillbglist) {
    for (sl->First(), fillbglist->First();
	 !sl->AtEnd() && !fillbglist->AtEnd();
	 sl->Next(), fillbglist->Next())
    {
	boolean fillbg = fillbglist->GetCur()->GetBoolean();
	sl->GetCur()->GetSelection()->FillBg(fillbg);
    }
}

// SetFont sets the Selections' font attributes with the given font
// attribute.

void Drawing::SetFont (IFont* font) {
    for (sl->First(); !sl->AtEnd(); sl->Next()) {
	Selection* s = sl->GetCur()->GetSelection();
	s->SetFont(font);
    }
}

// SetFont sets each Selection's font attribute with the corresponding
// font attribute in the provided list.

void Drawing::SetFont (IFontList* fontlist) {
    for (sl->First(), fontlist->First(); !sl->AtEnd() && !fontlist->AtEnd();
	 sl->Next(), fontlist->Next())
    {
	IFont* font = fontlist->GetCur()->GetFont();
	Selection* s = sl->GetCur()->GetSelection();
	s->SetFont(font);
    }
}

// SetPattern sets the Selections' pattern attributes with the given
// pattern attribute.

void Drawing::SetPattern (IPattern* pattern) {
    for (sl->First(); !sl->AtEnd(); sl->Next()) {
	sl->GetCur()->GetSelection()->SetPattern(pattern);
    }
}

// SetPattern sets each Selection's pattern attribute with the
// corresponding pattern attribute in the provided list.

void Drawing::SetPattern (IPatternList* patternlist) {
    for (sl->First(), patternlist->First();
	 !sl->AtEnd() && !patternlist->AtEnd();
	 sl->Next(), patternlist->Next())
    {
	IPattern* pattern = patternlist->GetCur()->GetPattern();
	sl->GetCur()->GetSelection()->SetPattern(pattern);
    }
}

// Append appends the Selections to the picture.

void Drawing::Append () {
    for (sl->First(); !sl->AtEnd(); sl->Next()) {
	Selection* s = sl->GetCur()->GetSelection();
	userpicture->Append(s);
    }
}

// Group groups each parent's children, if any, under their parent and
// returns the resulting Selections in the SelectionList.

void Drawing::Group (GroupList* grouplist) {
    if (grouplist->Size() >= 1) {
	sl->DeleteAll();
	for (grouplist->First(); !grouplist->AtEnd(); grouplist->Next()) {
	    GroupNode* gn = grouplist->GetCur();
	    PictSelection* parent = gn->GetParent();
	    boolean haschildren = gn->GetHasChildren();
	    SelectionList* children = gn->GetChildren();
	    SelectionList* childrengs = gn->GetChildrenGS();
	    if (haschildren) {
		for (children->First(), childrengs->First();
		     !children->AtEnd() && !childrengs->AtEnd();
		     children->Next(), childrengs->Next())
		{
		    Graphic* child = children->GetCur()->GetSelection();
		    Graphic* childgs = childrengs->GetCur()->GetSelection();
		    *child = *childgs;
		    userpicture->SetCurrent(child);
		    userpicture->Remove(child);
		    parent->Append(child);
		}
		userpicture->InsertBeforeCur(parent);
	    }
	    sl->Append(new SelectionNode(parent));
	}
	Sort();
    }
}

// InsertAfterPrev inserts each Selection after its corresponding
// predecessor in the provided list.

void Drawing::InsertAfterPrev (SelectionList* prevlist) {
    for (sl->First(), prevlist->First(); !sl->AtEnd() && !prevlist->AtEnd();
	 sl->Next(), prevlist->Next())
    {
	Selection* prev = prevlist->GetCur()->GetSelection();
	userpicture->SetCurrent(prev);
	Selection* s = sl->GetCur()->GetSelection();
	userpicture->InsertAfterCur(s);
    }
}

// Prepend prepends the Selections to the picture.

void Drawing::Prepend () {
    for (sl->Last(); !sl->AtEnd(); sl->Prev()) {
	Selection* s = sl->GetCur()->GetSelection();
	userpicture->Prepend(s);
    }
}

// Remove removes the Selections from the picture.

void Drawing::Remove () {
    for (sl->First(); !sl->AtEnd(); sl->Next()) {
	Selection* s = sl->GetCur()->GetSelection();
	userpicture->Remove(s);
    }
}

// Replace replaces a Selection in the picture with a Selection not in it.

void Drawing::Replace (Selection* replacee, Selection* replacer) {
    userpicture->SetCurrent(replacee);
    userpicture->Remove(replacee);
    userpicture->InsertBeforeCur(replacer);
}

// Sort sorts the Selections so they occur in the same order as they
// do in the picture.

void Drawing::Sort () {
    if (sl->Size() >= 2) {
	for (userpicture->First(); !userpicture->AtEnd();userpicture->Next()) {
	    Selection* g = userpicture->GetCurrent();
	    if (sl->Find(g)) {
		SelectionNode* s = sl->GetCur();
		sl->RemoveCur();
		sl->Append(s);
	    }
	}
    }
}

// Ungroup replaces all Selections which contain children with their
// children and returns the resulting Selections in the SelectionList.

void Drawing::Ungroup (GroupList* grouplist) {
    if (grouplist->Size() >= 1) {
	sl->DeleteAll();
	for (grouplist->First(); !grouplist->AtEnd(); grouplist->Next()) {
	    GroupNode* gn = grouplist->GetCur();
	    PictSelection* parent = gn->GetParent();
	    boolean haschildren = gn->GetHasChildren();
	    SelectionList* children = gn->GetChildren();
	    if (haschildren) {
		parent->Propagate();
		userpicture->SetCurrent(parent);
		for (children->First(); !children->AtEnd(); children->Next()) {
		    Selection* child = children->GetCur()->GetSelection();
		    parent->Remove(child);
		    userpicture->InsertBeforeCur(child);
		    sl->Append(new SelectionNode(child));
		}
		userpicture->Remove(parent);
	    } else {
		sl->Append(new SelectionNode(parent));
	    }
	}
	Sort();
    }
}

// NumberOfGraphics returns the number of graphics in the picture,
// calling itself recursively to count the number of graphics in
// subpictures.

int Drawing::NumberOfGraphics (PictSelection* picture) {
    int num = 0;
    for (picture->First(); !picture->AtEnd(); picture->Next()) {
	Selection* s = picture->GetCurrent();
	if (s->HasChildren()) {
	    num += NumberOfGraphics((PictSelection*) s);
	} else {
	    ++num;
	}
    }
    return num;
}

// SetOrientation updates the grid and boundary to reflect the page's
// new orientation if it has changed.

void Drawing::SetOrientation (Orientation ot) {
    if (orientation != ot) {
	if (ot == Landscape) {
	    page->SetLandscape();
	    grid->SetLandscape();
	    pageboundary->SetLandscape();
	    orientation = Landscape;
	} else {
	    page->SetPortrait();
	    grid->SetPortrait();
	    pageboundary->SetPortrait();
	    orientation = Portrait;
	}
    }
}

// SetUserPicture replaces the old picture with a new picture and
// deletes the old picture.  Clearing the SelectionList removes some
// dangling pointers to the old picture.

void Drawing::SetUserPicture (PictSelection* newpic) {
    sl->DeleteAll();
    page->SetCurrent(userpicture);
    page->Remove(userpicture);
    page->InsertBeforeCur(newpic);
    delete userpicture;
    userpicture = newpic;
}

// OrientUserPicture turns the picture on its side if it should be
// printed in landscape orientation.

void Drawing::OrientUserPicture () {
    if (orientation == Landscape) {
	Transformer parentXform;
	userpicture->TotalTransformation(parentXform);
	float l, b;
	parentXform.Transform(0.0, 0.0, l, b);
	userpicture->Rotate(-90., l, b);
	userpicture->Translate(0.0, pageheight);
    }
}

// UprightUserPicture uprights the picture if it was on its side and
// returns which orientation the picture was in.

Orientation Drawing::UprightUserPicture () {
    Transformer* t = userpicture->GetTransformer();
    Orientation ot = Portrait;
    if (t != nil && Rotated90(t)) {
	ot = Landscape;
	userpicture->Translate(0.0, -pageheight);
	Transformer parentXform;
	userpicture->TotalTransformation(parentXform);
	float l, t;
	parentXform.Transform(0.0, 0.0, l, t);
	userpicture->Rotate(90., l, t);
    }
    if (t != nil && Identity(t)) {
	userpicture->SetTransformer(nil);
    }
    return ot;
}

// Rotated90 returns true if the matrix would rotate a point n*90
// degrees (even if some entries are very small numbers, not 0).

boolean Drawing::Rotated90 (Transformer* t) {
    float mat00, mat01, mat10, mat11, mat20, mat21;
    t->GetEntries(mat00, mat01, mat10, mat11, mat20, mat21);
    return
	-1e-5 < mat00 && mat00 < 1e-5 &&
	0 != mat01 &&
	0 != mat10 &&
	-1e-5 < mat11 && mat11 < 1e-5;
}

// Identity returns true if the matrix would not rotate, scale, or
// translate a point (even if some entries are very small numbers).

boolean Drawing::Identity (Transformer* t) {
    float mat00, mat01, mat10, mat11, mat20, mat21;
    t->GetEntries(mat00, mat01, mat10, mat11, mat20, mat21);
    return
	1 == mat00 &&
	-1e-10 < mat01 && mat01 < 1e-10 &&
	-1e-10 < mat10 && mat10 < 1e-10 &&
	1 == mat11 &&
	-1e-10 < mat20 && mat20 < 1e-10 &&
	-1e-10 < mat21 && mat21 < 1e-10;
}
