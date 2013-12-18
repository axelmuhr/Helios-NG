// $Header: commands.c,v 1.11 89/04/17 00:30:02 linton Exp $
// implements class Commands.

#include "commands.h"
#include "editor.h"
#include "ipaint.h"
#include "istring.h"
#include "keystrokes.h"
#include "mapipaint.h"
#include "mapkey.h"
#include "sllines.h"
#include "state.h"
#include <InterViews/box.h>
#include <InterViews/painter.h>
#include <InterViews/sensor.h>
#include <InterViews/shape.h>

// An IdrawCommand enters itself into the MapKey so KeyEvents may be
// mapped to IdrawCommands.

class IdrawCommand : public PullDownMenuCommand {
public:
    IdrawCommand(PullDownMenuActivator*, const char*, char, Editor*,
		 MapKey* = nil);
protected:
    Editor* editor;		// handles drawing and editing operations
};

// IdrawCommand passes a printable string representing the given
// character for its key string and enters itself into the character's
// slot in the MapKey.

IdrawCommand::IdrawCommand (PullDownMenuActivator* a, const char* n, char c,
Editor* e, MapKey* mk) : (a, n, mk ? mk->ToStr(c) : "") {
    editor = e;
    if (mk != nil) {
	mk->Enter(this, c);
    }
}

// Each class below encapsulates a label, character, and command.

class NewCommand : public IdrawCommand {
public:
    NewCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "New", NEWCHAR, e, mk) {}
    void Execute (Event&) {
	editor->New();
    }
};

class RevertCommand : public IdrawCommand {
public:
    RevertCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Revert", REVERTCHAR, e, mk) {}
    void Execute (Event&) {
	editor->Revert();
    }
};

class OpenCommand : public IdrawCommand {
public:
    OpenCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Open...", OPENCHAR, e, mk) {}
    void Execute (Event&) {
	editor->Open();
    }
};

class SaveCommand : public IdrawCommand {
public:
    SaveCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Save", SAVECHAR, e, mk) {}
    void Execute (Event&) {
	editor->Save();
    }
};

class SaveAsCommand : public IdrawCommand {
public:
    SaveAsCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Save As...", SAVEASCHAR, e, mk) {}
    void Execute (Event&) {
	editor->SaveAs();
    }
};

class PrintCommand : public IdrawCommand {
public:
    PrintCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Print...", PRINTCHAR, e, mk) {}
    void Execute (Event&) {
	editor->Print();
    }
};

class DirectoryCommand : public IdrawCommand {
public:
    DirectoryCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Directory...", DIRECTORYCHAR, e, mk) {}
    void Execute (Event&) {
	editor->Directory();
    }
};

class QuitCommand : public IdrawCommand {
public:
    QuitCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Quit", QUITCHAR, e, mk) {}
    void Execute (Event& e) {
	editor->Quit(e);
    }
};

class UndoCommand : public IdrawCommand {
public:
    UndoCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Undo", UNDOCHAR, e, mk) {}
    void Execute (Event&) {
	editor->Undo();
    }
};

class RedoCommand : public IdrawCommand {
public:
    RedoCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Redo", REDOCHAR, e, mk) {}
    void Execute (Event&) {
	editor->Redo();
    }
};

class CutCommand : public IdrawCommand {
public:
    CutCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Cut", CUTCHAR, e, mk) {}
    void Execute (Event&) {
	editor->Cut();
    }
};

class CopyCommand : public IdrawCommand {
public:
    CopyCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Copy", COPYCHAR, e, mk) {}
    void Execute (Event&) {
	editor->Copy();
    }
};

class PasteCommand : public IdrawCommand {
public:
    PasteCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Paste", PASTECHAR, e, mk) {}
    void Execute (Event&) {
	editor->Paste();
    }
};

class DuplicateCommand : public IdrawCommand {
public:
    DuplicateCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Duplicate", DUPLICATECHAR, e, mk) {}
    void Execute (Event&) {
	editor->Duplicate();
    }
};

class DeleteCommand : public IdrawCommand {
public:
    DeleteCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Delete", DELETECHAR, e, mk) {}
    void Execute (Event&) {
	editor->Delete();
    }
};

class SelectAllCommand : public IdrawCommand {
public:
    SelectAllCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Select All", SELECTALLCHAR, e, mk) {}
    void Execute (Event&) {
	editor->SelectAll();
    }
};

class FlipHorizontalCommand : public IdrawCommand {
public:
    FlipHorizontalCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Flip Horizontal", FLIPHORIZONTALCHAR, e, mk) {}
    void Execute (Event&) {
	editor->FlipHorizontal();
    }
};

class FlipVerticalCommand : public IdrawCommand {
public:
    FlipVerticalCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Flip Vertical", FLIPVERTICALCHAR, e, mk) {}
    void Execute (Event&) {
	editor->FlipVertical();
    }
};

class _90ClockwiseCommand : public IdrawCommand {
public:
    _90ClockwiseCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "90 Clockwise", _90CLOCKWISECHAR, e, mk) {}
    void Execute (Event&) {
	editor->_90Clockwise();
    }
};

class _90CounterCWCommand : public IdrawCommand {
public:
    _90CounterCWCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "90 CounterCW", _90COUNTERCWCHAR, e, mk) {}
    void Execute (Event&) {
	editor->_90CounterCW();
    }
};

class PreciseMoveCommand : public IdrawCommand {
public:
    PreciseMoveCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Precise Move...", PRECISEMOVECHAR, e, mk) {}
    void Execute (Event&) {
	editor->PreciseMove();
    }
};

class PreciseScaleCommand : public IdrawCommand {
public:
    PreciseScaleCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Precise Scale...", PRECISESCALECHAR, e, mk) {}
    void Execute (Event&) {
	editor->PreciseScale();
    }
};

class PreciseRotateCommand : public IdrawCommand {
public:
    PreciseRotateCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Precise Rotate...", PRECISEROTATECHAR, e, mk) {}
    void Execute (Event&) {
	editor->PreciseRotate();
    }
};

class GroupCommand : public IdrawCommand {
public:
    GroupCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Group", GROUPCHAR, e, mk) {}
    void Execute (Event&) {
	editor->Group();
    }
};

class UngroupCommand : public IdrawCommand {
public:
    UngroupCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Ungroup", UNGROUPCHAR, e, mk) {}
    void Execute (Event&) {
	editor->Ungroup();
    }
};

class BringToFrontCommand : public IdrawCommand {
public:
    BringToFrontCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Bring To Front", BRINGTOFRONTCHAR, e, mk) {}
    void Execute (Event&) {
	editor->BringToFront();
    }
};

class SendToBackCommand : public IdrawCommand {
public:
    SendToBackCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Send To Back", SENDTOBACKCHAR, e, mk) {}
    void Execute (Event&) {
	editor->SendToBack();
    }
};

class NumberOfGraphicsCommand : public IdrawCommand {
public:
    NumberOfGraphicsCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Number of Graphics", NUMBEROFGRAPHICSCHAR, e, mk) {}
    void Execute (Event&) {
	editor->NumberOfGraphics();
    }
};

class FontCommand : public IdrawCommand {
public:
    FontCommand (PullDownMenuActivator* a, Editor* e, IFont* f)
    : (a, f->GetPrintFontAndSize(), '\0', e) {
	font = f;
    }
    void Execute (Event&) {
	editor->SetFont(font);
    }
protected:
    void Reconfig () {
	if (output->GetFont() != *font) {
	    Painter* copy = new Painter(output);
	    delete output;
	    output = copy;
	    output->SetFont(*font);
	}
	IdrawCommand::Reconfig();
    }
    void Resize () {		// need constant left pad to line up entries
	const int xpad = 6;
	name_x = xpad;
	name_y = (ymax - output->GetFont()->Height() + 1) / 2;
	key_x = key_y = 0;
    }
    IFont* font;		// stores font to give Editor
};

static const int PICXMAX = 47;	// chosen to minimize scaling for canvas
static const int PICYMAX = 14;

class BrushCommand : public IdrawCommand {
public:
    BrushCommand (PullDownMenuActivator* a, Editor* e, IBrush* b)
    : (a, "None", '\0', e) {
	brush = b;
	brindic = new LineSelection(0, 0, PICXMAX, 0);
	brindic->SetBrush(brush);
	brindic->SetColors(pblack, pwhite);
	brindic->FillBg(true);
	brindic->SetPattern(psolid);
    }
    ~BrushCommand () {
	delete brindic;
    }
    void Execute (Event&) {
	editor->SetBrush(brush);
    }
    void Highlight () {
	if (!highlighted) {
	    brindic->SetColors(brindic->GetBgColor(), brindic->GetFgColor());
	}
	IdrawCommand::Highlight();
    }
    void Unhighlight () {
	if (highlighted) {
	    brindic->SetColors(brindic->GetBgColor(), brindic->GetFgColor());
	}
	IdrawCommand::Unhighlight();
    }
protected:
    void Reconfig () {
	IdrawCommand::Reconfig();
	PColor* fg = brindic->GetFgColor();
	PColor* bg = brindic->GetBgColor();
	if (*fg != output->GetFgColor() || *bg != output->GetBgColor()) {
	    fg = new IColor(output->GetFgColor(), "");
	    bg = new IColor(output->GetBgColor(), "");
	    brindic->SetColors(fg, bg);
	}
    }
    void Redraw (Coord l, Coord b, Coord r, Coord t) {
	if (brush->None()) {
	    IdrawCommand::Redraw(l, b, r, t);
	} else {
	    output->ClearRect(canvas, l, b, r, t);
	    brindic->Draw(canvas);
	}
    }
    void Resize () {
	IdrawCommand::Resize();
	float xmag = float(xmax - 2*name_x) / PICXMAX;
	float hy = float(ymax) / 2;
	brindic->SetTransformer(nil);
	brindic->Scale(xmag, 1.);
	brindic->Translate(float(name_x), hy);
    }
    IBrush* brush;		// stores brush to give Editor
    Graphic* brindic;		// displays line to demonstrate brush's effect
};

class PatternCommand : public IdrawCommand {
public:
    PatternCommand (PullDownMenuActivator* a, Editor* e, IPattern* p, State* s)
    : (a, "None", '\0', e) {
	fgcolor = s->GetFgColor();
	bgcolor = s->GetBgColor();
	pattern = p;
	patindic = nil;
    }
    ~PatternCommand () {
	delete patindic;
    }
    void Execute (Event&) {
	editor->SetPattern(pattern);
    }
protected:
    void Reconfig () {
	IdrawCommand::Reconfig();
	if (patindic == nil) {
	    patindic = new Painter(output);
	    patindic->SetColors(*fgcolor, *bgcolor);
	    patindic->SetPattern(*pattern);
	}
    }
    void Redraw (Coord l, Coord b, Coord r, Coord t) {
	if (pattern->None()) {
	    IdrawCommand::Redraw(l, b, r, t);
	} else {
	    output->ClearRect(canvas, l, b, r, t);
	    patindic->FillRect(canvas, name_x,name_y,xmax-name_x,ymax-name_y);
	    output->Rect(canvas, name_x, name_y, xmax-name_x, ymax-name_y);
	}
    }
    IColor* fgcolor;		// stores initial foreground color
    IColor* bgcolor;		// stores initial background color
    IPattern* pattern;		// stores pattern to give Editor
    Painter* patindic;		// fills rect to demonstrate pat's effect
};

class ColorCommand : public IdrawCommand {
public:
    ColorCommand (PullDownMenuActivator* a, Editor* e, IColor* c)
    : (a, c->GetName(), '\0', e) {
	key = "   ";
	color = c;
	colorindic = nil;
    }
    ~ColorCommand () {
	key = nil;
	delete colorindic;
    }
protected:
    void Reconfig () {
	IdrawCommand::Reconfig();
	if (colorindic == nil) {
	    colorindic = new Painter(output);
	    colorindic->SetColors(*color, colorindic->GetBgColor());
	}
    }
    void Redraw (Coord l, Coord b, Coord r, Coord t) {
	IdrawCommand::Redraw(l, b, r, t);
	colorindic->FillRect(canvas, key_x, key_y, xmax-name_x, ymax-name_y);
	output->Rect(canvas, key_x, key_y, xmax-name_x, ymax-name_y);
    }
    IColor* color;		// stores color to give Editor
    Painter* colorindic;	// fills rect to demonstrate color's effect
};

class FgColorCommand : public ColorCommand {
public:
    FgColorCommand (PullDownMenuActivator* a, Editor* e, IColor* c)
    : (a, e, c) {}
    void Execute (Event&) {
	editor->SetFgColor(color);
    }
};

class BgColorCommand : public ColorCommand {
public:
    BgColorCommand (PullDownMenuActivator* a, Editor* e, IColor* c)
    : (a, e, c) {}
    void Execute (Event&) {
	editor->SetBgColor(color);
    }
};

class AlignLeftSidesCommand : public IdrawCommand {
public:
    AlignLeftSidesCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Left Sides", ALIGNLEFTSIDESCHAR, e, mk) {}
    void Execute (Event&) {
	editor->AlignLeftSides();
    }
};

class AlignRightSidesCommand : public IdrawCommand {
public:
    AlignRightSidesCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Right Sides", ALIGNRIGHTSIDESCHAR, e, mk) {}
    void Execute (Event&) {
	editor->AlignRightSides();
    }
};

class AlignBottomsCommand : public IdrawCommand {
public:
    AlignBottomsCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Bottoms", ALIGNBOTTOMSCHAR, e, mk) {}
    void Execute (Event&) {
	editor->AlignBottoms();
    }
};

class AlignTopsCommand : public IdrawCommand {
public:
    AlignTopsCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Tops", ALIGNTOPSCHAR, e, mk) {}
    void Execute (Event&) {
	editor->AlignTops();
    }
};

class AlignVertCentersCommand : public IdrawCommand {
public:
    AlignVertCentersCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Vert Centers", ALIGNVERTCENTERSCHAR, e, mk) {}
    void Execute (Event&) {
	editor->AlignVertCenters();
    }
};

class AlignHorizCentersCommand : public IdrawCommand {
public:
    AlignHorizCentersCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Horiz Centers", ALIGNHORIZCENTERSCHAR, e, mk) {}
    void Execute (Event&) {
	editor->AlignHorizCenters();
    }
};

class AlignCentersCommand : public IdrawCommand {
public:
    AlignCentersCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Centers", ALIGNCENTERSCHAR, e, mk) {}
    void Execute (Event&) {
	editor->AlignCenters();
    }
};

class AlignLeftToRightCommand : public IdrawCommand {
public:
    AlignLeftToRightCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Left To Right", ALIGNLEFTTORIGHTCHAR, e, mk) {}
    void Execute (Event&) {
	editor->AlignLeftToRight();
    }
};

class AlignRightToLeftCommand : public IdrawCommand {
public:
    AlignRightToLeftCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Right To Left", ALIGNRIGHTTOLEFTCHAR, e, mk) {}
    void Execute (Event&) {
	editor->AlignRightToLeft();
    }
};

class AlignBottomToTopCommand : public IdrawCommand {
public:
    AlignBottomToTopCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Bottom To Top", ALIGNBOTTOMTOTOPCHAR, e, mk) {}
    void Execute (Event&) {
	editor->AlignBottomToTop();
    }
};

class AlignTopToBottomCommand : public IdrawCommand {
public:
    AlignTopToBottomCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Top To Bottom", ALIGNTOPTOBOTTOMCHAR, e, mk) {}
    void Execute (Event&) {
	editor->AlignTopToBottom();
    }
};

class AlignToGridCommand : public IdrawCommand {
public:
    AlignToGridCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Align To Grid", ALIGNTOGRIDCHAR, e, mk) {}
    void Execute (Event&) {
	editor->AlignToGrid();
    }
};

class ReduceCommand : public IdrawCommand {
public:
    ReduceCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Reduce", REDUCECHAR, e, mk) {}
    void Execute (Event&) {
	editor->Reduce();
    }
};

class EnlargeCommand : public IdrawCommand {
public:
    EnlargeCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Enlarge", ENLARGECHAR, e, mk) {}
    void Execute (Event&) {
	editor->Enlarge();
    }
};

class NormalSizeCommand : public IdrawCommand {
public:
    NormalSizeCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Normal Size", NORMALSIZECHAR, e, mk) {}
    void Execute (Event&) {
	editor->NormalSize();
    }
};

class ReduceToFitCommand : public IdrawCommand {
public:
    ReduceToFitCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Reduce To Fit", REDUCETOFITCHAR, e, mk) {}
    void Execute (Event&) {
	editor->ReduceToFit();
    }
};

class CenterPageCommand : public IdrawCommand {
public:
    CenterPageCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Center Page", CENTERPAGECHAR, e, mk) {}
    void Execute (Event&) {
	editor->CenterPage();
    }
};

class GriddingOnOffCommand : public IdrawCommand {
public:
    GriddingOnOffCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Gridding on/off", GRIDDINGONOFFCHAR, e, mk) {}
    void Execute (Event&) {
	editor->GriddingOnOff();
    }
};

class GridVisibleInvisibleCommand : public IdrawCommand {
public:
    GridVisibleInvisibleCommand (PullDownMenuActivator* a,Editor* e,MapKey* mk)
    : (a, "Grid visible/invisible", GRIDVISIBLEINVISIBLECHAR, e, mk) {}
    void Execute (Event&) {
	editor->GridVisibleInvisible();
    }
};

class GridSpacingCommand : public IdrawCommand {
public:
    GridSpacingCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Grid spacing...", GRIDSPACINGCHAR, e, mk) {}
    void Execute (Event&) {
	editor->GridSpacing();
    }
};

class OrientationCommand : public IdrawCommand {
public:
    OrientationCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "Orientation", ORIENTATIONCHAR, e, mk) {}
    void Execute (Event&) {
	editor->Orientation();
    }
};

class ShowVersionCommand : public IdrawCommand {
public:
    ShowVersionCommand (PullDownMenuActivator* a, Editor* e, MapKey* mk)
    : (a, "", SHOWVERSIONCHAR, e, mk) {
	Listen(noEvents);
    }
    void Execute (Event&) {
	editor->ShowVersion();
    }
protected:
    void Reconfig () {
	shape->width = shape->height = 0;
    }
};

// Commands creates its commands.

Commands::Commands (Editor* e, MapKey* mk, State* s) {
    Init(e, mk, s);
}

// Init creates the activators and commands, inserts the commands into
// menus, gives the menus to the activators, and inserts the activators.

void Commands::Init (Editor* e, MapKey* mk, State* state) {
    PullDownMenuActivator* file = new PullDownMenuActivator(this, "File");
    PullDownMenuActivator* edit = new PullDownMenuActivator(this, "Edit");
    PullDownMenuActivator* strc = new PullDownMenuActivator(this, "Structure");
    PullDownMenuActivator* font = new PullDownMenuActivator(this, "Font");
    PullDownMenuActivator* brush = new PullDownMenuActivator(this, "Brush");
    PullDownMenuActivator* pat = new PullDownMenuActivator(this, "Pattern");
    PullDownMenuActivator* fgcolor = new PullDownMenuActivator(this,"FgColor");
    PullDownMenuActivator* bgcolor = new PullDownMenuActivator(this,"BgColor");
    PullDownMenuActivator* align = new PullDownMenuActivator(this, "Align");
    PullDownMenuActivator* option = new PullDownMenuActivator(this, "Option");

    Scene* filemenu = new VBox;
    filemenu->Insert(new NewCommand(file, e, mk));
    filemenu->Insert(new RevertCommand(file, e, mk));
    filemenu->Insert(new PullDownMenuDivider);
    filemenu->Insert(new OpenCommand(file, e, mk));
    filemenu->Insert(new SaveCommand(file, e, mk));
    filemenu->Insert(new SaveAsCommand(file, e, mk));
    filemenu->Insert(new PrintCommand(file, e, mk));
    filemenu->Insert(new DirectoryCommand(file, e, mk));
    filemenu->Insert(new PullDownMenuDivider);
    filemenu->Insert(new QuitCommand(file, e, mk));

    Scene* editmenu = new VBox;
    editmenu->Insert(new UndoCommand(edit, e, mk));
    editmenu->Insert(new RedoCommand(edit, e, mk));
    editmenu->Insert(new CutCommand(edit, e, mk));
    editmenu->Insert(new CopyCommand(edit, e, mk));
    editmenu->Insert(new PasteCommand(edit, e, mk));
    editmenu->Insert(new DuplicateCommand(edit, e, mk));
    editmenu->Insert(new DeleteCommand(edit, e, mk));
    editmenu->Insert(new SelectAllCommand(edit, e, mk));
    editmenu->Insert(new PullDownMenuDivider);
    editmenu->Insert(new FlipHorizontalCommand(edit, e, mk));
    editmenu->Insert(new FlipVerticalCommand(edit, e, mk));
    editmenu->Insert(new _90ClockwiseCommand(edit, e, mk));
    editmenu->Insert(new _90CounterCWCommand(edit, e, mk));
    editmenu->Insert(new PullDownMenuDivider);
    editmenu->Insert(new PreciseMoveCommand(edit, e, mk));
    editmenu->Insert(new PreciseScaleCommand(edit, e, mk));
    editmenu->Insert(new PreciseRotateCommand(edit, e, mk));

    Scene* structuremenu = new VBox;
    structuremenu->Insert(new GroupCommand(strc, e, mk));
    structuremenu->Insert(new UngroupCommand(strc, e, mk));
    structuremenu->Insert(new BringToFrontCommand(strc, e, mk));
    structuremenu->Insert(new SendToBackCommand(strc, e, mk));
    structuremenu->Insert(new PullDownMenuDivider);
    structuremenu->Insert(new NumberOfGraphicsCommand(strc, e, mk));

    Scene* fontmenu = new VBox;
    MapIFont* mf = state->GetMapIFont();
    for (IFont* f = mf->First(); !mf->AtEnd(); f = mf->Next()) {
	fontmenu->Insert(new FontCommand(font, e, f));
    }

    Scene* brushmenu = new VBox;
    MapIBrush* mb = state->GetMapIBrush();
    for (IBrush* b = mb->First(); !mb->AtEnd(); b = mb->Next()) {
	brushmenu->Insert(new BrushCommand(brush, e, b));
    }

    Scene* patternmenu = new VBox;
    MapIPattern* mp = state->GetMapIPattern();
    for (IPattern* p = mp->First(); !mp->AtEnd(); p = mp->Next()) {
	patternmenu->Insert(new PatternCommand(pat, e, p, state));
    }

    Scene* fgcolormenu = new VBox;
    MapIColor* mfg = state->GetMapIFgColor();
    for (IColor* fg = mfg->First(); !mfg->AtEnd(); fg = mfg->Next()) {
	fgcolormenu->Insert(new FgColorCommand(fgcolor, e, fg));
    }

    Scene* bgcolormenu = new VBox;
    MapIColor* mbg = state->GetMapIBgColor();
    for (IColor* bg = mbg->First(); !mbg->AtEnd(); bg = mbg->Next()) {
	bgcolormenu->Insert(new BgColorCommand(bgcolor, e, bg));
    }

    Scene* alignmenu = new VBox;
    alignmenu->Insert(new AlignLeftSidesCommand(align, e, mk));
    alignmenu->Insert(new AlignRightSidesCommand(align, e, mk));
    alignmenu->Insert(new AlignBottomsCommand(align, e, mk));
    alignmenu->Insert(new AlignTopsCommand(align, e, mk));
    alignmenu->Insert(new AlignVertCentersCommand(align, e, mk));
    alignmenu->Insert(new AlignHorizCentersCommand(align, e, mk));
    alignmenu->Insert(new AlignCentersCommand(align, e, mk));
    alignmenu->Insert(new AlignLeftToRightCommand(align, e, mk));
    alignmenu->Insert(new AlignRightToLeftCommand(align, e, mk));
    alignmenu->Insert(new AlignBottomToTopCommand(align, e, mk));
    alignmenu->Insert(new AlignTopToBottomCommand(align, e, mk));
    alignmenu->Insert(new AlignToGridCommand(align, e, mk));

    Scene* optionmenu = new VBox;
    optionmenu->Insert(new ReduceCommand(option, e, mk));
    optionmenu->Insert(new EnlargeCommand(option, e, mk));
    optionmenu->Insert(new NormalSizeCommand(option, e, mk));
    optionmenu->Insert(new ReduceToFitCommand(option, e, mk));
    optionmenu->Insert(new CenterPageCommand(option, e, mk));
    optionmenu->Insert(new PullDownMenuDivider);
    optionmenu->Insert(new GriddingOnOffCommand(option, e, mk));
    optionmenu->Insert(new GridVisibleInvisibleCommand(option, e, mk));
    optionmenu->Insert(new GridSpacingCommand(option, e, mk));
    optionmenu->Insert(new OrientationCommand(option, e, mk));
    optionmenu->Insert(new ShowVersionCommand(option, e, mk));

    file->SetMenu(filemenu);
    edit->SetMenu(editmenu);
    strc->SetMenu(structuremenu);
    font->SetMenu(fontmenu);
    brush->SetMenu(brushmenu);
    pat->SetMenu(patternmenu);
    fgcolor->SetMenu(fgcolormenu);
    bgcolor->SetMenu(bgcolormenu);
    align->SetMenu(alignmenu);
    option->SetMenu(optionmenu);

    Scene* activators = new HBox;
    activators->Insert(file);
    activators->Insert(edit);
    activators->Insert(strc);
    activators->Insert(font);
    activators->Insert(brush);
    activators->Insert(pat);
    activators->Insert(fgcolor);
    activators->Insert(bgcolor);
    activators->Insert(align);
    activators->Insert(option);

    Insert(activators);
}

// Reconfig makes Commands' shape unstretchable but shrinkable.

void Commands::Reconfig () {
    PullDownMenuBar::Reconfig();
    shape->Rigid(hfil, 0, 0, 0);
}
