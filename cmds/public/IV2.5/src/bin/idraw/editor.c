// $Header: editor.c,v 1.15 89/04/11 19:01:49 vlis Exp $
// implements class Editor.

#include "dialogbox.h"
#include "drawing.h"
#include "drawingview.h"
#include "editor.h"
#include "grid.h"
#include "history.h"
#include "idraw.h"
#include "listchange.h"
#include "listselectn.h"
#include "liststring.h"
#include "rubbands.h"
#include "slellipses.h"
#include "sllines.h"
#include "slpolygons.h"
#include "slsplines.h"
#include "sltext.h"
#include "state.h"
#include "textedit.h"
#include "version.h"
#include <InterViews/event.h>
#include <InterViews/Graphic/util.h>
#include <bstring.h>
#include <stdio.h>

// Editor creates its history and dialog boxes.

Editor::Editor (Interactor* i) {
    history = new History(i);
    dirdialog = new Namer(i, "Change current directory to:");
    numberofdialog = new Messager(i);
    opendialog = new Namer(i, "Enter name of drawing to open:");
    overwritedialog= new Confirmer(i, "already exists; overwrite?");
    precmovedialog =new Namer(i,"Enter X and Y movement in printer's points:");
    precrotdialog = new Namer(i, "Enter rotation in degrees:");
    precscaledialog = new Namer(i, "Enter X and Y scaling:");
    printdialog = new Namer(i, "Enter print command:");
    readonlydialog = new Messager(i, "Drawing is readonly.");
    revertdialog = new Confirmer(i, "Really revert to original?");
    saveasdialog = new Namer(i, "Enter a name for this drawing:");
    savecurdialog = new Confirmer(i, "Save current drawing?");
    spacingdialog = new Namer(i, "Enter grid spacing in printer's points:");
    versiondialog = new Messager(i, version);

    drawing = nil;
    drawingview = nil;
    grid = nil;
    state = nil;
}

// ~Editor frees storage allocated for its history and dialog boxes.

Editor::~Editor () {
    delete history;
    delete dirdialog;
    delete numberofdialog;
    delete opendialog;
    delete overwritedialog;
    delete precmovedialog;
    delete precrotdialog;
    delete precscaledialog;
    delete printdialog;
    delete readonlydialog;
    delete revertdialog;
    delete saveasdialog;
    delete savecurdialog;
    delete spacingdialog;
    delete versiondialog;
}

// Define access functions to set members' values.  Only Idraw sets
// their values.

void Editor::SetDrawing (Drawing* d) {
    drawing = d;
}

void Editor::SetDrawingView (DrawingView* dv) {
    drawingview = dv;
}

void Editor::SetGrid (Grid* g) {
    grid = g;
}

void Editor::SetState (State* s) {
    state = s;
}

// HandleSelect lets the user pick a Selection if one's under the
// mouse, otherwise it lets the user manipulate a rubber rectangle to
// enclose the Selection he wants to pick.  HandleSelect clears all
// previous Selections unless the user holds down the shift key to
// extend the Selections being made.

void Editor::HandleSelect (Event& e) {
    Selection* pick = drawing->PickSelectionIntersecting(e.x, e.y);
    if (!e.shift) {		// replacing previous Selections
	drawingview->EraseHandles();
	if (pick != nil) {
	    drawing->Select(pick);
	} else {
	    RubberRect* rubberrect =
		new RubberRect(nil, nil, e.x, e.y, e.x, e.y);
	    drawingview->Manipulate(e, rubberrect, UpEvent, false);
	    Coord l, b, r, t;
	    rubberrect->GetCurrent(l, b, r, t);
	    delete rubberrect;

	    SelectionList* picklist= drawing->PickSelectionsWithin(l, b, r, t);
	    drawing->Select(picklist);
	    delete picklist;
	}
    } else {			// extending Selections
	if (pick != nil) {
	    drawingview->ErasePickedHandles(pick);
	    drawing->Extend(pick);
	} else {
	    RubberRect* rubberrect =
		new RubberRect(nil, nil, e.x, e.y, e.x, e.y);
	    drawingview->Manipulate(e, rubberrect, UpEvent, false);
	    Coord l, b, r, t;
	    rubberrect->GetCurrent(l, b, r, t);
	    delete rubberrect;

	    SelectionList* picklist= drawing->PickSelectionsWithin(l, b, r, t);
	    drawingview->ErasePickedHandles(picklist);
	    drawing->Extend(picklist);
	    delete picklist;
	}
    }
    drawingview->DrawHandles();
}

// HandleMove lets the user manipulate a sliding rectangle enclosing
// the Selections and moves them the same way when the user releases
// the button.

void Editor::HandleMove (Event& e) {
    Selection* pick = drawing->PickSelectionIntersecting(e.x, e.y);
    if (pick != nil) {
	drawingview->EraseUngraspedHandles(pick);
	drawing->Grasp(pick);
	drawingview->DrawHandles();

	Coord l, b, r, t;
	drawing->GetBox(l, b, r, t);
	grid->Constrain(e.x, e.y);
	SlidingRect* slidingrect =
	    new SlidingRect(nil, nil, l, b, r, t, e.x, e.y);
	drawingview->Manipulate(e, slidingrect, UpEvent);
	Coord nl, nb, nr, nt;
	slidingrect->GetCurrent(nl, nb, nr, nt);
	drawing->GetPageCoords(l, b, nl, nb);
	delete slidingrect;

	if (nl != l || nb != b) {
	    Do(new MoveChange(drawing, drawingview, nl - l, nb - b));
	}
    }
}

// HandleScale lets the user manipulate a scaling rectangle enclosing
// the picked Selection and scales the Selections to the new scale
// when the user releases the button.

void Editor::HandleScale (Event& e) {
    Selection* pick = drawing->PickSelectionIntersecting(e.x, e.y);
    if (pick != nil) {
	drawingview->EraseUngraspedHandles(pick);
	drawing->Grasp(pick);
	drawingview->DrawHandles();

	float l, b, r, t;
	pick->GetBounds(l, b, r, t);
	float cx, cy;
	pick->GetCenter(cx, cy);
	ScalingRect* scalingrect =
	    new ScalingRect(nil, nil, round(l), round(b), round(r), round(t),
			    round(cx), round(cy));
	drawingview->Manipulate(e, scalingrect, UpEvent);
	float scale = scalingrect->CurrentScaling();
	delete scalingrect;

	if (scale != 0) {
	    Do(new ScaleChange(drawing, drawingview, scale, scale));
	}
    }
}

// HandleStretch lets the user manipulate a stretching rectangle
// enclosing the picked Selection and stretches the Selections the
// same way when the user releases the button.

void Editor::HandleStretch (Event& e) {
    Selection* pick = drawing->PickSelectionIntersecting(e.x, e.y);
    if (pick != nil) {
	drawingview->EraseUngraspedHandles(pick);
	drawing->Grasp(pick);
	drawingview->DrawHandles();

	float l, b, r, t;
	pick->GetBounds(l, b, r, t);
	IStretchingRect* istretchingrect = new
	    IStretchingRect(nil, nil, round(l), round(b), round(r), round(t));
	drawingview->Manipulate(e, istretchingrect, UpEvent);
	float stretch = istretchingrect->CurrentStretching();
	Side side = istretchingrect->CurrentSide();
	delete istretchingrect;

	if (stretch != 0) {
	    Do(new StretchChange(drawing, drawingview, stretch, side));
	}
    }
}

// Skew comments/code ratio to work around cpp bug
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

// HandleRotate lets the user manipulate a rotating rectangle
// enclosing the picked Selection and rotates the Selections the same
// way when the user releases the button.

void Editor::HandleRotate (Event& e) {
    Selection* pick = drawing->PickSelectionIntersecting(e.x, e.y);
    if (pick != nil) {
	drawingview->EraseUngraspedHandles(pick);
	drawing->Grasp(pick);
	drawingview->DrawHandles();

	Coord l, b, r, t;
	pick->GetBox(l, b, r, t);
	float cx, cy;
	pick->GetCenter(cx, cy);
	grid->Constrain(e.x, e.y);
	RotatingRect* rotatingrect =
	    new RotatingRect(nil, nil, l, b, r, t, round(cx), round(cy),
			     e.x, e.y);
	drawingview->Manipulate(e, rotatingrect, UpEvent);
	float angle = rotatingrect->CurrentAngle();
	delete rotatingrect;

	if (angle != 0) {
	    Do(new RotateChange(drawing, drawingview, angle));
	}
    }
}

// HandleReshape lets the user reshape an already existing Selection
// and replaces the Selection with the reshaped Selection.  Text
// Selections "reshape" themselves using different code below.

void Editor::HandleReshape (Event& e) {
    Selection* pick = drawing->PickSelectionShapedBy(e.x, e.y);
    if (pick != nil) {
	drawingview->EraseHandles();
	drawing->Select(pick);
	drawingview->DrawHandles();
	Selection* reshapedpick = nil;
	if (pick->IsA(TEXTSELECTION)) {
	    StringList* stringlist = ((TextSelection*) pick)->GetOriginal();
	    TextEdit* textedit = new TextEdit(stringlist, pick);
	    delete stringlist;
	    float l, b, r, t;
	    pick->GetBounds(l, b, r, t);
	    e.x = round(l);
	    e.y = round(t);
	    textedit->Handle(e);
	    stringlist = textedit->GetText();
	    delete textedit;
	    reshapedpick = new TextSelection(stringlist, pick);
	    delete stringlist;
	} else {
	    Rubberband* shape = pick->CreateShape(e.x, e.y);
	    drawingview->Manipulate(e, shape, UpEvent);
	    reshapedpick = pick->GetReshapedCopy();
	}
	if (reshapedpick != nil) {
	    Do(new ReplaceChange(drawing, drawingview, pick, reshapedpick));
	}
    }
}

// HandleMagnify lets the user manipulate a rubber rectangle and
// expands the given area to fill the view.

void Editor::HandleMagnify (Event& e) {
    RubberRect* rubberrect = NewRubberRectOrSquare(e);
    drawingview->Manipulate(e, rubberrect, UpEvent, false);
    Coord fx, fy;
    rubberrect->GetCurrent(fx, fy, e.x, e.y);
    delete rubberrect;

    drawingview->Magnify(fx, fy, e.x, e.y);
}

// HandleText lets the user type some text and creates a new
// TextSelection when the user finishes typing the text.  It must
// clear the selection list because DrawingView will redraw the
// handles obscured by the TextEdit if the list's not empty.

void Editor::HandleText (Event& e) {
    grid->Constrain(e.x, e.y);
    Coord l = e.x;
    Coord t = e.y;
    drawingview->EraseHandles();
    drawing->Clear();
    TextEdit* textedit = new TextEdit(nil, *state);
    textedit->Handle(e);
    StringList* stringlist = textedit->GetText();
    delete textedit;
    TextSelection* textselection = new TextSelection(stringlist, *state);
    delete stringlist;
    drawing->GetPageCoords(l, t);
    textselection->Translate(l, t);

    drawing->Select(textselection);
    Do(new AddChange(drawing, drawingview));
}

// HandleLine lets the user manipulate a rubber line and creates
// a LineSelection when the user releases the button.

void Editor::HandleLine (Event& e) {
    drawingview->EraseHandles();
    grid->Constrain(e.x, e.y);
    RubberLine* rubberline = NewRubberLineOrAxis(e);
    drawingview->Manipulate(e, rubberline, UpEvent);
    Coord x0, y0, x1, y1;
    rubberline->GetCurrent(x0, y0, x1, y1);
    drawing->GetPageCoords(x0, y0, x1, y1);
    delete rubberline;

    drawing->Select(new LineSelection(x0, y0, x1, y1, *state));
    Do(new AddChange(drawing, drawingview));
}

// HandleMultiLine lets the user draw a series of connected lines and
// creates a MultiLineSelection when the user presses the middle
// button.

void Editor::HandleMultiLine (Event& e) {
    Coord* x;
    Coord* y;
    int n;

    drawingview->EraseHandles();
    InputVertices(e, x, y, n);

    drawing->Select(new MultiLineSelection(x, y, n, *state));
    Do(new AddChange(drawing, drawingview));
}

// HandleBSpline lets the user draw a series of connected lines and
// creates a BSplineSelection when the user presses the middle button.

void Editor::HandleBSpline (Event& e) {
    Coord* x;
    Coord* y;
    int n;

    drawingview->EraseHandles();
    InputVertices(e, x, y, n);

    drawing->Select(new BSplineSelection(x, y, n, *state));
    Do(new AddChange(drawing, drawingview));
}

// HandleEllipse lets the user manipulate a rubber ellipse and creates
// an EllipseSelection when the user releases the button.

void Editor::HandleEllipse (Event& e) {
    drawingview->EraseHandles();
    grid->Constrain(e.x, e.y);
    RubberEllipse* rubberellipse = NewRubberEllipseOrCircle(e);
    drawingview->Manipulate(e, rubberellipse, UpEvent);
    Coord cx, cy, rx, ry;
    rubberellipse->GetCurrent(cx, cy, rx, ry);
    int xr, yr;
    rubberellipse->CurrentRadii(xr, yr);
    rx = cx + xr;
    ry = cy + yr;
    drawing->GetPageCoords(cx, cy, rx, ry);
    xr = abs(rx - cx);
    yr = abs(ry - cy);
    delete rubberellipse;

    drawing->Select(new EllipseSelection(cx, cy, xr, yr, *state));
    Do(new AddChange(drawing, drawingview));
}

// HandleRect lets the user manipulate a rubber rectangle and creates
// a RectSelection when the user releases the button.

void Editor::HandleRect (Event& e) {
    drawingview->EraseHandles();
    grid->Constrain(e.x, e.y);
    RubberRect* rubberrect = NewRubberRectOrSquare(e);
    drawingview->Manipulate(e, rubberrect, UpEvent);
    Coord l, b, r, t;
    rubberrect->GetCurrent(l, b, r, t);
    drawing->GetPageCoords(l, b, r, t);
    delete rubberrect;

    drawing->Select(new RectSelection(l, b, r, t, *state));
    Do(new AddChange(drawing, drawingview));
}

// HandlePolygon lets the user draw a series of connected lines and
// creates a PolygonSelection when the user presses the middle button.

void Editor::HandlePolygon (Event& e) {
    Coord* x;
    Coord* y;
    int n;

    drawingview->EraseHandles();
    InputVertices(e, x, y, n);

    drawing->Select(new PolygonSelection(x, y, n, *state));
    Do(new AddChange(drawing, drawingview));
}

// HandleClosedBSpline lets the user draw a series of connected lines
// and creates a ClosedBSplineSelection when the user presses the
// middle button.

void Editor::HandleClosedBSpline (Event& e) {
    Coord* x;
    Coord* y;
    int n;

    drawingview->EraseHandles();
    InputVertices(e, x, y, n);

    drawing->Select(new ClosedBSplineSelection(x, y, n, *state));
    Do(new AddChange(drawing, drawingview));
}

// New offers to write an unsaved drawing and creates a new empty
// drawing if the save succeeds or the user refuses the offer.

void Editor::New () {
    boolean successful = OfferToSave();
    if (successful) {
	drawing->ClearUserPicture();
	Reset(nil);
    }
}

// Revert rereads the drawing from its file.  It asks for confirmation
// before reverting an unsaved drawing.

void Editor::Revert () {
    const char* name = state->GetDrawingName();
    if (name != nil) {
	char response = revertdialog->Confirm();
	if (response == 'y') {
	    boolean successful = drawing->ReadUserPicture(name, state);
	    if (successful) {
		Reset(name);
	    } else {
		savecurdialog->
		    SetWarning("couldn't revert! (file nonexistent?)");
		Open();
	    }
	}
    }
}

// Open reads a drawing from the given file.  If it fails, it calls
// the interactive Open to ask the user to type another name.

void Editor::Open (const char* name) {
    boolean successful = drawing->ReadUserPicture(name, state);
    if (successful) {
	Reset(name);
    } else {
	opendialog->SetWarning("couldn't open ", name);
	Open();
    }
}

// Open prompts for a file name and reads a drawing from that file.
// It offers to save an unsaved drawing and it keeps trying to read a
// drawing until it succeeds or the user cancels the command.

void Editor::Open () {
    boolean successful = OfferToSave();
    if (successful) {
	char* name = nil;
	for (;;) {
	    delete name;
	    name = opendialog->Edit(nil);
	    if (name == nil) {
		break;
	    }
	    boolean successful = drawing->ReadUserPicture(name, state);
	    if (successful) {
		Reset(name);
		break;
	    } else {
		opendialog->SetWarning("couldn't open ", name);
	    }
	}
	delete name;
    }
}

// Save writes the drawing to the file it was read from unless there's
// no file or it can't write the drawing to that file, in which case
// it hands the job off to SaveAs.

void Editor::Save () {
    const char* name = state->GetDrawingName();
    if (name == nil) {
	SaveAs();
    } else if (state->GetModifStatus() == ReadOnly) {
	saveasdialog->SetWarning("can't save in readonly file ", name);
	SaveAs();
    } else {
	boolean successful = drawing->WriteUserPicture(name);
	if (successful) {
	    state->SetModifStatus(Unmodified);
	    state->UpdateViews();
	} else {
	    saveasdialog->SetWarning("couldn't save as ", name);
	    SaveAs();
	}
    }
}

// SaveAs prompts for a file name and writes the drawing to that file.
// It asks for confirmation before overwriting an already existing
// file and it keeps trying to write the drawing until it succeeds or
// the user cancels the command.

void Editor::SaveAs () {
    char* name = nil;
    for (;;) {
	delete name;
	name = saveasdialog->Edit(nil);
	if (name == nil) {
	    break;
	}
	if (drawing->Exists(name)) {
	    overwritedialog->SetWarning("a drawing named ", name);
	    char response = overwritedialog->Confirm();
	    if (response != 'y') {
		break;
	    }
	}
	boolean successful = drawing->WriteUserPicture(name);
	if (successful) {
	    state->SetDrawingName(name);
	    state->SetModifStatus(Unmodified);
	    state->UpdateViews();
	    break;
	} else {
	    saveasdialog->SetWarning("couldn't save as ", name);
	}
    }
    delete name;
}

// Print prompts for a print command and writes the drawing through a
// pipe to that command's standard input.  It keeps trying to print
// the drawing until it succeeds or the user cancels the command.

void Editor::Print () {
    if (state->GetModifStatus() == Modified) {
	savecurdialog->SetWarning("a broken pipe signal won't be caught");
    }
    boolean successful = OfferToSave();
    if (successful) {
	char* cmd = nil;
	for (;;) {
	    delete cmd;
	    cmd = printdialog->Edit(nil);
	    if (cmd == nil) {
		break;
	    }
	    boolean successful = drawing->PrintUserPicture(cmd);
	    if (successful) {
		break;
	    } else {
		printdialog->SetWarning("couldn't execute ", cmd);
	    }
	}
	delete cmd;
    }
}

// Directory prompts for a path and changes the working directory to
// that path.

void Editor::Directory () {
    const char* cwd;
    char* path = nil;
    for (;;) {
	cwd = drawing->GetCurrentWorkingDirectory();
	delete path;
	path = dirdialog->Edit(cwd);
	if (path == nil) {
	    break;
	}
	if (drawing->ChangeCurrentWorkingDirectory(path)) {
	    break;
	} else {
	    dirdialog->SetWarning("couldn't chdir to ", path);
	}
    }
    delete path;
}

// Skew comments/code ratio to work around cpp bug
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

// Quit offers to save an unsaved drawing and tells Idraw to quit
// running if the save succeeds or the user refuses the offer.

void Editor::Quit (Event& e) {
    boolean successful = OfferToSave();
    if (successful) {
	e.target = nil;
    }
}

// Checkpoint writes an unsaved drawing to a temporary filename.  The
// program currently calls Checkpoint only when an X error occurs.

void Editor::Checkpoint () {
    if (state->GetModifStatus() == Modified) {
	char* path = tempnam("./", "idraw");
	boolean successful = drawing->WriteUserPicture(path);
	if (successful) {
	    fprintf(stderr, "saved drawing as \"%s\"\n", path);
	} else {
	    fprintf(stderr, "sorry, couldn't save drawing as \"%s\"\n", path);
	}
	delete path;
    } else {
	fprintf(stderr, "drawing was unmodified, didn't save it\n");
    }
}

// Undo undoes the last change made to the drawing.  Undo does nothing
// if all stored changes have been undone.

void Editor::Undo () {
    history->Undo();
}

// Redo redoes the last undone change made to the drawing, i.e., it
// undoes an Undo.  Redo does nothing if it follows a Do.

void Editor::Redo () {
    history->Redo();
}

// Cut removes the Selections and writes them to the clipboard file,
// overwriting whatever was there previously.

void Editor::Cut () {
    Do(new CutChange(drawing, drawingview));
}

// Copy copies the Selections and writes them to the clipboard file,
// overwriting whatever was there previously.

void Editor::Copy () {
    Do(new CopyChange(drawing, drawingview));
}

// Paste reads new Selections from the clipboard file and appends them
// to the drawing.

void Editor::Paste () {
    Do(new PasteChange(drawing, drawingview, state));
}

// Duplicate duplicates the Selections and appends the new Selections
// to the drawing.

void Editor::Duplicate () {
    Do(new DuplicateChange(drawing, drawingview));
}

// Delete deletes all of the Selections.

void Editor::Delete () {
    Do(new DeleteChange(drawing, drawingview));
}

// SelectAll selects all of the Selections in the drawing.

void Editor::SelectAll () {
    drawing->SelectAll();
    drawingview->DrawHandles();
}

// FlipHorizontal flips the Selections horizontally by scaling them by
// -1 along the x axis about their centers.

void Editor::FlipHorizontal () {
    Do(new ScaleChange(drawing, drawingview, -1, 1));
}

// FlipVertical flips the Selections vertically by scaling them by -1
// along the y axis about their centers.

void Editor::FlipVertical () {
    Do(new ScaleChange(drawing, drawingview, 1, -1));
}

// _90Clockwise rotates the Selections 90 degrees clockwise about
// their centers.

void Editor::_90Clockwise () {
    Do(new RotateChange(drawing, drawingview, -90.));
}

// _90CounterCW rotates the Selections 90 degrees counter-clockwise
// about their centers.

void Editor::_90CounterCW () {
    Do(new RotateChange(drawing, drawingview, 90.));
}

// PreciseMove prompts the user for the x and y movement and moves the
// Selections that much from their original places in units of points.
// If we didn't use points as units, the user wouldn't be able to move
// a Selection one grid spacing by typing "8 0".

void Editor::PreciseMove () {
    char* movement = nil;
    for (;;) {
	delete movement;
	movement = precmovedialog->Edit(nil);
	if (movement == nil) {
	    break;
	}
	float xdisp, ydisp;
	if (sscanf(movement, "%f %f", &xdisp, &ydisp) == 2) {
	    if (xdisp != 0 || ydisp != 0) {
		xdisp *= points;
		ydisp *= points;
		Do(new MoveChange(drawing, drawingview, xdisp, ydisp));
	    }
	    break;
	} else {
	    precmovedialog->SetWarning("couldn't parse ", movement);
	}
    }
    delete movement;
}

// PreciseScale prompts the user for the x and y scales and scales the
// Selections that much about their centers.

void Editor::PreciseScale () {
    char* scaling = nil;
    for (;;) {
	delete scaling;
	scaling = precscaledialog->Edit(nil);
	if (scaling == nil) {
	    break;
	}
	float xscale, yscale;
	if (sscanf(scaling, "%f %f", &xscale, &yscale) == 2) {
	    if (xscale !=0 && yscale != 0) {
		Do(new ScaleChange(drawing, drawingview, xscale, yscale));
	    }
	    break;
	} else {
	    precscaledialog->SetWarning("couldn't parse ", scaling);
	}
    }
    delete scaling;
}

// PreciseRotate prompts the user for the angle and rotates the
// Selections that many degrees about their centers.

void Editor::PreciseRotate () {
    char* rotation = nil;
    for (;;) {
	delete rotation;
	rotation = precrotdialog->Edit(nil);
	if (rotation == nil) {
	    break;
	}
	float angle;
	if (sscanf(rotation, "%f", &angle) == 1) {
	    if (angle != 0) {
		Do(new RotateChange(drawing, drawingview, angle));
	    }
	    break;
	} else {
	    precrotdialog->SetWarning("couldn't parse ", rotation);
	}
    }
    delete rotation;
}

// Group groups the Selections together.

void Editor::Group () {
    Do(new GroupChange(drawing, drawingview));
}

// Ungroup ungroups each PictSelection into its component Selections.

void Editor::Ungroup () {
    Do(new UngroupChange(drawing, drawingview));
}

// BringToFront brings the Selections to the front of the drawing.

void Editor::BringToFront () {
    Do(new BringToFrontChange(drawing, drawingview));
}

// SendToBack sends the Selections to the back of the drawing.

void Editor::SendToBack () {
    Do(new SendToBackChange(drawing, drawingview));
}

// NumberOfGraphics counts the number of graphics in the drawing and
// displays the count.

void Editor::NumberOfGraphics () {
    int num = drawing->GetNumberOfGraphics();
    if (num == 1) {
	numberofdialog->SetMessage("The selections contain 1 graphic.");
    } else {
	char buf[50];
	sprintf(buf, "The selections contain %d graphics.", num);
	numberofdialog->SetMessage(buf);
    }
    numberofdialog->Display();
}

// SetBrush sets the Selections' brush and updates the views to
// display the new current brush.

void Editor::SetBrush (IBrush* brush) {
    state->SetBrush(brush);
    state->UpdateViews();
    Do(new SetBrushChange(drawing, drawingview, brush));
}

// Skew comments/code ratio to work around cpp bug
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

// SetFgColor sets the Selections' foreground color and updates the
// views to display the new current foreground color.

void Editor::SetFgColor (IColor* fg) {
    state->SetFgColor(fg);
    state->UpdateViews();
    Do(new SetFgColorChange(drawing, drawingview, fg));
}

// SetBgColor sets the Selections' background color and updates the
// views to display the new current background color.

void Editor::SetBgColor (IColor* bg) {
    state->SetBgColor(bg);
    state->UpdateViews();
    Do(new SetBgColorChange(drawing, drawingview, bg));
}

// SetFont sets the Selections' font and updates the views to display
// the new current font.

void Editor::SetFont (IFont* font) {
    state->SetFont(font);
    state->UpdateViews();
    Do(new SetFontChange(drawing, drawingview, font));
}

// SetPattern sets the Selections' pattern and updates the views to
// display the new current pattern.

void Editor::SetPattern (IPattern* pattern) {
    state->SetPattern(pattern);
    state->UpdateViews();
    Do(new SetPatternChange(drawing, drawingview, pattern));
}

// AlignLeftSides aligns the rest of the Selections's left sides with
// the first Selection's left side.

void Editor::AlignLeftSides () {
    Do(new AlignChange(drawing, drawingview, Left, Left));
}

// AlignRightSides aligns the rest of the Selections' right sides with
// the first Selection's right side.

void Editor::AlignRightSides () {
    Do(new AlignChange(drawing, drawingview, Right, Right));
}

// AlignBottomSides aligns the rest of the Selections' bottom sides
// with the first Selection's bottom side.

void Editor::AlignBottoms () {
    Do(new AlignChange(drawing, drawingview, Bottom, Bottom));
}

// AlignTopSides aligns the rest of the Selections' top sides with the
// first Selection's top side.

void Editor::AlignTops () {
    Do(new AlignChange(drawing, drawingview, Top, Top));
}

// AlignVertCenters aligns the rest of the Selections' vertical
// centers with the first Selection's vertical center.

void Editor::AlignVertCenters () {
    Do(new AlignChange(drawing, drawingview, VertCenter, VertCenter));
}

// AlignHorizCenters aligns the rest of the Selections' horizontal
// centers with the first Selection's horizontal center.

void Editor::AlignHorizCenters () {
    Do(new AlignChange(drawing, drawingview, HorizCenter, HorizCenter));
}

// AlignCenters aligns the rest of the Selections' centers with the
// first Selection's center.

void Editor::AlignCenters () {
    Do(new AlignChange(drawing, drawingview, Center, Center));
}

// AlignLeftToRight aligns each Selection's left side with its
// predecessor's right side.

void Editor::AlignLeftToRight () {
    Do(new AlignChange(drawing, drawingview, Right, Left));
}

// AlignRightToLeft aligns each Selection's right side with its
// predecessor's left side.

void Editor::AlignRightToLeft () {
    Do(new AlignChange(drawing, drawingview, Left, Right));
}

// AlignBottomToTop aligns each Selection's bottom side with its
// predecessor's top side.

void Editor::AlignBottomToTop () {
    Do(new AlignChange(drawing, drawingview, Top, Bottom));
}

// AlignTopToBottom aligns each Selection's top side with its
// predecessor's bottom side.

void Editor::AlignTopToBottom () {
    Do(new AlignChange(drawing, drawingview, Bottom, Top));
}

// AlignToGrid aligns the Selections' lower left corners with the
// closest grid point.

void Editor::AlignToGrid () {
    Do(new AlignToGridChange(drawing, drawingview));
}

// Reduce reduces the drawing's magnification by a factor of two.

void Editor::Reduce () {
    drawingview->Reduce();
}

// Enlarge enlarges the drawing's magnification by a factor of two.

void Editor::Enlarge () {
    drawingview->Enlarge();
}

// NormalSize resets the drawing's magnification.

void Editor::NormalSize () {
    drawingview->NormalSize();
}

// ReduceToFit reduces the drawing's magnification enough to fit all
// of the drawing in the window.

void Editor::ReduceToFit () {
    drawingview->ReduceToFit();
}

// CenterPage scrolls the drawing so its center coincidences with the
// window's center.

void Editor::CenterPage () {
    drawingview->CenterPage();
}

// GriddingOnOff toggles the grid's constraint on or off.

void Editor::GriddingOnOff () {
    grid->SetGridding(!grid->GetGridding());
    state->SetGridding(grid->GetGridding());
    state->UpdateViews();
}

// GridVisibleInvisible toggles the grid's visibility on or off.

void Editor::GridVisibleInvisible () {
    grid->SetVisibility(!grid->GetVisibility());
    drawingview->Update();
}

// GridSpacing prompts the user for the new grid spacing in units of
// points.  If we didn't use points as units, the same grid spacing
// would not be portable to different displays.

void Editor::GridSpacing () {
    static char oldspacing[50];
    sprintf(oldspacing, "%lg", grid->GetSpacing());
    char* spacing = nil;
    for (;;) {
	delete spacing;
	spacing = spacingdialog->Edit(oldspacing);
	if (spacing == nil) {
	    break;
	}
	float s = 0.;
	if (sscanf(spacing, "%f", &s) == 1) {
	    if (s > 0.) {
		grid->SetSpacing(s);
		if (grid->GetVisibility() == true) {
		    drawingview->Update();
		}
	    }
	    break;
	} else {
	    spacingdialog->SetWarning("couldn't parse ", spacing);
	}
    }
    delete spacing;
}

// Orientation toggles the page boundary between portrait and
// landscape orientations.

void Editor::Orientation () {
    drawing->ToggleOrientation();
    drawingview->Update();
}

// ShowVersion displays idraw's version level and author.

void Editor::ShowVersion () {
    versiondialog->Display();
}

// Do performs a change to the drawing and updates the drawing's
// modification status if it was unmodified.

void Editor::Do (ChangeNode* changenode) {
    switch (state->GetModifStatus()) {
    case Unmodified:
	history->Do(changenode);
	state->SetModifStatus(Modified);
	state->UpdateViews();
	break;
    default:
	history->Do(changenode);
	break;
    }
}

// InputVertices lets the user keep drawing a series of connected
// lines until the user presses a button other than the left button.
// It returns the vertices inputted by the user.

void Editor::InputVertices (Event& e, Coord*& xret, Coord*& yret, int& nret) {
    const int INITIALSIZE = 100;
    static int sizebuffers = 0;
    static RubberLine** rubberlines = nil;
    static Coord* x = nil;
    static Coord* y = nil;
    if (INITIALSIZE > sizebuffers) {
	sizebuffers = INITIALSIZE;
	rubberlines = new RubberLine*[sizebuffers];
	x = new Coord[sizebuffers];
	y = new Coord[sizebuffers];
    }

    int n = 0;
    grid->Constrain(e.x, e.y);
    rubberlines[0] = nil;
    x[0] = e.x;
    y[0] = e.y;
    drawing->GetPageCoords(x[0], y[0]);
    ++n;

    while (e.button == LEFTMOUSE) {
	rubberlines[n] = NewRubberLineOrAxis(e);
	e.eventType = UpEvent;
	drawingview->Manipulate(e, rubberlines[n], DownEvent, true, false);
	Coord dummy;
	rubberlines[n]->GetCurrent(dummy, dummy, e.x, e.y);
	x[n] = e.x;
	y[n] = e.y;
	drawing->GetPageCoords(x[n], y[n]);
	++n;

	if (n == sizebuffers) {
	    RubberLine** oldrubberlines = rubberlines;
	    Coord* oldx = x;
	    Coord* oldy = y;
	    sizebuffers += INITIALSIZE/2;
	    rubberlines = new RubberLine*[sizebuffers];
	    x = new Coord[sizebuffers];
	    y = new Coord[sizebuffers];
	    bcopy(oldrubberlines, rubberlines, n * sizeof(RubberLine*));
	    bcopy(oldx, x, n * sizeof(Coord));
	    bcopy(oldy, y, n * sizeof(Coord));
	    delete oldrubberlines;
	    delete oldx;
	    delete oldy;
	}
    }

    xret = x;
    yret = y;
    nret = n;
    for (int i = 1; i < n; i++) {
	rubberlines[i]->Erase();
	delete rubberlines[i];
    }
}

// NewRubberLineOrAxis creates and returns a new RubberLine or a new
// RubberAxis depending on whether the shift key's being depressed.

RubberLine* Editor::NewRubberLineOrAxis (Event& e) {
    return (!e.shift ? 
	    new RubberLine(nil, nil, e.x, e.y, e.x, e.y) :
	    new RubberAxis(nil, nil, e.x, e.y, e.x, e.y));
}

// NewRubberEllipseOrCircle creates and returns a new RubberEllipse or
// a new RubberCircle depending on the shift key's state.

RubberEllipse* Editor::NewRubberEllipseOrCircle (Event& e) {
    return (!e.shift ?
	    new RubberEllipse(nil, nil, e.x, e.y, e.x, e.y) :
	    new RubberCircle(nil, nil, e.x, e.y, e.x, e.y));
}

// Skew comments/code ratio to work around cpp bug
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

// NewRubberRectOrSquare creates and returns a new RubberRect or a new
// RubberSquare depending on whether the shift key's being depressed.

RubberRect* Editor::NewRubberRectOrSquare (Event& e) {
    return (!e.shift ? 
	    new RubberRect(nil, nil, e.x, e.y, e.x, e.y) :
	    new RubberSquare(nil, nil, e.x, e.y, e.x, e.y));
}

// OfferToSave returns true if it saves an unsaved drawing or the user
// refuses the offer or no changes need to be saved.

boolean Editor::OfferToSave () {
    boolean successful = false;
    if (state->GetModifStatus() == Modified) {
	char response = savecurdialog->Confirm();
	if (response == 'y') {
	    Save();
	    if (state->GetModifStatus() == Unmodified) {
		successful = true;
	    }
	} else if (response == 'n') {
	    successful = true;
	}
    } else {
	successful = true;
    }
    return successful;
}

// Reset redraws the view, clears the history, and resets state
// information about the drawing's name and its modification status.

void Editor::Reset (const char* name) {
    history->Clear();
    state->SetDrawingName(name);
    if (name != nil && !drawing->Writable(name)) {
	state->SetModifStatus(ReadOnly);
    } else {
	state->SetModifStatus(Unmodified);
    }
    state->UpdateViews();
    drawingview->Update();
}
