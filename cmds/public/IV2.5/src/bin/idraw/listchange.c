// $Header: listchange.c,v 1.10 89/04/17 00:31:05 linton Exp $
// implements subclasses of ChangeNode and class ChangeList.

#include "drawing.h"
#include "drawingview.h"
#include "listcenter.h"
#include "listchange.h"
#include "listgroup.h"
#include "listibrush.h"
#include "listicolor.h"
#include "listifont.h"
#include "listipattern.h"
#include "listselectn.h"
#include "selection.h"
#include "slpict.h"

// ChangeNode creates a dummy ChangeNode object for the header of the
// ChangeList.

ChangeNode::ChangeNode () {
    drawing = nil;
    drawingview = nil;
    oldsl = nil;
}

// ChangeNode stores the Drawing and DrawingView.  It copies the
// SelectionList, optionally sorting the SelectionList first.

ChangeNode::ChangeNode (Drawing* d, DrawingView* dv, boolean sort) {
    drawing = d;
    drawingview = dv;
    if (sort) {
	drawing->Sort();
    }
    oldsl = drawing->GetSelections();
}

// Free storage allocated for the list of old Selections.

ChangeNode::~ChangeNode () {
    delete oldsl;
}

// Do carries out the change to the Drawing.

void ChangeNode::Do () {
    // nop
}

// Undo removes the change to the Drawing.

void ChangeNode::Undo () {
    // nop
}

// MoveChange stores the Selections' translation in reversible form.

MoveChange::MoveChange (Drawing* d, DrawingView* dv, float xdisp, float ydisp)
: (d, dv) {
    dx = xdisp;
    dy = ydisp;
    undodx = -xdisp;
    undody = -ydisp;
}

// Do moves the Selections.

void MoveChange::Do () {
    drawingview->EraseExcessHandles(oldsl);
    drawing->Select(oldsl);
    drawingview->Damaged();
    drawing->Move(dx, dy);
    drawingview->Damaged();
    drawingview->Repair();
}

// Undo moves the Selections back to their original places.

void MoveChange::Undo () {
    drawingview->EraseExcessHandles(oldsl);
    drawing->Select(oldsl);
    drawingview->Damaged();
    drawing->Move(undodx, undody);
    drawingview->Damaged();
    drawingview->Repair();
}

// ScaleChange stores the Selections' scaling in reversible form.

ScaleChange::ScaleChange (Drawing* d, DrawingView* dv, float xsc, float ysc)
: (d, dv) {
    sx = xsc;
    sy = ysc;
    undosx = 1.0 / xsc;
    undosy = 1.0 / ysc;
}

// Do scales the Selections.

void ScaleChange::Do () {
    drawingview->EraseExcessHandles(oldsl);
    drawing->Select(oldsl);
    drawingview->Damaged();
    drawing->Scale(sx, sy);
    drawingview->Damaged();
    drawingview->Repair();
}

// Undo scales the Selections back to their former sizes.

void ScaleChange::Undo () {
    drawingview->EraseExcessHandles(oldsl);
    drawing->Select(oldsl);
    drawingview->Damaged();
    drawing->Scale(undosx, undosy);
    drawingview->Damaged();
    drawingview->Repair();
}

// StretchChange stores the Selections' stretching in reversible form.

StretchChange::StretchChange (Drawing* d, DrawingView* dv, float str,
Alignment sd) : (d, dv) {
    stretch = str;
    undostretch = 1/str;
    side = sd;
    undoside = (str < 0) ? OppositeSide(sd) : sd;
}

// Do stretches the Selections.

void StretchChange::Do () {
    drawingview->EraseExcessHandles(oldsl);
    drawing->Select(oldsl);
    drawingview->Damaged();
    drawing->Stretch(stretch, side);
    drawingview->Damaged();
    drawingview->Repair();
}

// Undo stretches the Selections back to their former sizes.

void StretchChange::Undo () {
    drawingview->EraseExcessHandles(oldsl);
    drawing->Select(oldsl);
    drawingview->Damaged();
    drawing->Stretch(undostretch, undoside);
    drawingview->Damaged();
    drawingview->Repair();
}

// OppositeSide returns the side opposite the given side.

Alignment StretchChange::OppositeSide (Alignment original) {
    Alignment opposite;
    switch (original) {
    case Left:
	opposite = Right;
	break;
    case Right:
	opposite = Left;
	break;
    case Bottom:
	opposite = Top;
	break;
    case Top:
	opposite = Bottom;
	break;
    }
    return opposite;
}

// RotateChange stores the Selections' rotation in reversible form.

RotateChange::RotateChange (Drawing* d, DrawingView* dv, float a) : (d, dv) {
    angle = a;
    undoangle = -a;
}

// Do rotates the Selections.

void RotateChange::Do () {
    drawingview->EraseExcessHandles(oldsl);
    drawing->Select(oldsl);
    drawingview->Damaged();
    drawing->Rotate(angle);
    drawingview->Damaged();
    drawingview->Repair();
}

// Undo rotates the Selections back to their original places.

void RotateChange::Undo () {
    drawingview->EraseExcessHandles(oldsl);
    drawing->Select(oldsl);
    drawingview->Damaged();
    drawing->Rotate(undoangle);
    drawingview->Damaged();
    drawingview->Repair();
}

// Skew comments/code ratio to work around cpp bug
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

// ReplaceChange stores the replaced and replacing Selections.

ReplaceChange::ReplaceChange (Drawing* d, DrawingView* dv, Selection* ee,
Selection* er) : (d, dv) {
    replacee = ee;
    replacer = er;
}

// Free storage allocated for the Selection not in the
// Drawing, which always resides in replacer.

ReplaceChange::~ReplaceChange () {
    delete replacer;
}

// Do swaps the replaced and replacing Selections.

void ReplaceChange::Do () {
    drawingview->EraseHandles();
    drawing->Select(replacee);
    drawingview->Damaged();
    drawing->Replace(replacee, replacer);
    drawing->Select(replacer);
    drawingview->Damaged();
    drawingview->Repair();
    Selection* temp = replacer;
    replacer = replacee;
    replacee = temp;
}

// Undo unswaps the replaced and replacing Selections.

void ReplaceChange::Undo () {
    Do();
}

// SetBrushChange stores the Selections' original brushes and the new
// brush to set.

SetBrushChange::SetBrushChange (Drawing* d, DrawingView* dv, IBrush* br)
: (d, dv) {
    brush = br;
    undobrushlist = drawing->GetBrush();
}

// Free storage allocated for the list of original brushes.

SetBrushChange::~SetBrushChange () {
    delete undobrushlist;
}

// Do sets the Selections' new brush.

void SetBrushChange::Do () {
    drawingview->EraseExcessHandles(oldsl);
    drawing->Select(oldsl);
    drawingview->Damaged();
    drawing->SetBrush(brush);
    drawingview->Damaged();
    drawingview->Repair();
}

// Undo restores the Selections' original brushes.

void SetBrushChange::Undo () {
    drawingview->EraseExcessHandles(oldsl);
    drawing->Select(oldsl);
    drawingview->Damaged();
    drawing->SetBrush(undobrushlist);
    drawingview->Damaged();
    drawingview->Repair();
}

// SetFgColorChange stores the Selections' original foreground colors
// and the new foreground color to set.

SetFgColorChange::SetFgColorChange (Drawing* d, DrawingView* dv, IColor* fg)
: (d, dv) {
    fgcolor = fg;
    undofglist = drawing->GetFgColor();
}

// Free storage allocated for the list of original colors.

SetFgColorChange::~SetFgColorChange () {
    delete undofglist;
}

// Do sets the Selections' new foreground color.

void SetFgColorChange::Do () {
    drawingview->EraseExcessHandles(oldsl);
    drawing->Select(oldsl);
    drawingview->Damaged();
    drawing->SetFgColor(fgcolor);
    drawingview->Damaged();
    drawingview->Repair();
}

// Undo restores the Selections' original foreground colors.

void SetFgColorChange::Undo () {
    drawingview->EraseExcessHandles(oldsl);
    drawing->Select(oldsl);
    drawingview->Damaged();
    drawing->SetFgColor(undofglist);
    drawingview->Damaged();
    drawingview->Repair();
}

// Skew comments/code ratio to work around cpp bug
;;;;;;;;;;;;;;;;;;;;;;;;;;;

// SetBgColorChange stores the Selections' original background colors
// and the new background color to set.

SetBgColorChange::SetBgColorChange (Drawing* d, DrawingView* dv, IColor* bg)
: (d, dv) {
    bgcolor = bg;
    undobglist = drawing->GetBgColor();
}

// Free storage allocated for the list of original colors.

SetBgColorChange::~SetBgColorChange () {
    delete undobglist;
}

// Do sets the Selections' new background color.

void SetBgColorChange::Do () {
    drawingview->EraseExcessHandles(oldsl);
    drawing->Select(oldsl);
    drawingview->Damaged();
    drawing->SetBgColor(bgcolor);
    drawingview->Damaged();
    drawingview->Repair();
}

// Undo restores the Selections' original background colors.

void SetBgColorChange::Undo () {
    drawingview->EraseExcessHandles(oldsl);
    drawing->Select(oldsl);
    drawingview->Damaged();
    drawing->SetBgColor(undobglist);
    drawingview->Damaged();
    drawingview->Repair();
}

// SetFontChange stores the Selections' original fonts and
// the new font to set.

SetFontChange::SetFontChange (Drawing* d, DrawingView* dv, IFont* f)
: (d, dv) {
    font = f;
    undofontlist = drawing->GetFont();
}

// Free storage allocated for the list of original fonts.

SetFontChange::~SetFontChange () {
    delete undofontlist;
}

// Do sets the Selections' new font.

void SetFontChange::Do () {
    drawingview->EraseExcessHandles(oldsl);
    drawing->Select(oldsl);
    drawingview->Damaged();
    drawing->SetFont(font);
    drawingview->Damaged();
    drawingview->Repair();
}

// Undo restores the Selections' original fonts.

void SetFontChange::Undo () {
    drawingview->EraseExcessHandles(oldsl);
    drawing->Select(oldsl);
    drawingview->Damaged();
    drawing->SetFont(undofontlist);
    drawingview->Damaged();
    drawingview->Repair();
}

// SetPatternChange stores the Selections' original patterns and the
// new pattern to set.

SetPatternChange::SetPatternChange (Drawing* d, DrawingView* dv, IPattern* pat)
: (d, dv) {
    pattern = pat;
    undopatternlist = drawing->GetPattern();
}

// Free storage allocated for the list of original patterns.

SetPatternChange::~SetPatternChange () {
    delete undopatternlist;
}

// Do sets the Selections' new pattern.

void SetPatternChange::Do () {
    drawingview->EraseExcessHandles(oldsl);
    drawing->Select(oldsl);
    drawing->SetPattern(pattern);
    drawingview->Damaged();
    drawingview->Repair();
}

// Undo restores the Selections' original patterns.

void SetPatternChange::Undo () {
    drawingview->EraseExcessHandles(oldsl);
    drawing->Select(oldsl);
    drawing->SetPattern(undopatternlist);
    drawingview->Damaged();
    drawingview->Repair();
}

// AddChange knows it hasn't done its change yet.

AddChange::AddChange (Drawing* d, DrawingView* dv) : (d, dv) {
    done = false;
}

// Free storage allocated for the Selections if AddChange
// never added them to the Drawing.

AddChange::~AddChange () {
    if (!done) {
	for (oldsl->First(); !oldsl->AtEnd(); oldsl->Next()) {
	    delete oldsl->GetCur()->GetSelection();
	}
    }
}

// Do appends the Selections to the Drawing.

void AddChange::Do () {
    drawingview->EraseHandles();
    drawing->Select(oldsl);
    drawing->Append();
    drawingview->Added();
    drawingview->Repair();
    done = true;
}

// Undo removes the Selections from the Drawing.

void AddChange::Undo () {
    drawingview->EraseExcessHandles(oldsl);
    drawing->Select(oldsl);
    drawingview->Damaged();
    drawing->Remove();
    drawing->Clear();
    drawingview->Repair();
    done = false;
}

// DeleteChange stores the Selections' predecessors.

DeleteChange::DeleteChange (Drawing* d, DrawingView* dv) : (d, dv, true) {
    prevlist = drawing->GetPrevs();
    done = false;
}

// Free storage allocated for the Selections if DeleteChange
// removed them from the Drawing.

DeleteChange::~DeleteChange () {
    delete prevlist;
    if (done) {
	for (oldsl->First(); !oldsl->AtEnd(); oldsl->Next()) {
	    delete oldsl->GetCur()->GetSelection();
	}
    }
}

// Do removes the Selections from the Drawing.

void DeleteChange::Do () {
    drawingview->EraseExcessHandles(oldsl);
    drawing->Select(oldsl);
    drawingview->Damaged();
    drawing->Remove();
    drawing->Clear();
    drawingview->Repair();
    done = true;
}

// Undo puts the Selections back where they came from in the Drawing.

void DeleteChange::Undo () {
    drawingview->EraseExcessHandles(oldsl);
    drawing->Select(oldsl);
    drawing->InsertAfterPrev(prevlist);
    drawingview->Damaged();
    drawingview->Repair();
    done = false;
}

// CutChange passes its arguments to its DeleteChange constructor.

CutChange::CutChange (Drawing* d, DrawingView* dv) : (d, dv) {
}

// Do removes the Selections from the Drawing and writes them to the
// clipboard file, overwriting the clipboard file's previous contents.

void CutChange::Do () {
    drawingview->EraseExcessHandles(oldsl);
    drawing->Select(oldsl);
    drawing->WriteClipboard();
    DeleteChange::Do();
}

// CopyChange must sort the Selections.

CopyChange::CopyChange (Drawing* d, DrawingView* dv) : (d, dv, true) {
}

// Do writes the Selections to the clipboard file, overwriting
// whatever was there previously.

void CopyChange::Do () {
    drawingview->EraseExcessHandles(oldsl);
    drawing->Select(oldsl);
    drawing->WriteClipboard();
}

// PasteChange reads the clipboard file and stores the clippings for
// pasting into the Drawing later.

PasteChange::PasteChange (Drawing* d, DrawingView* dv, State* state)
: (d, dv) {
    delete oldsl;
    oldsl = drawing->ReadClipboard(state);
}

// DuplicateChange stores duplicates of the picked Selections for
// pasting into the Drawing later.

DuplicateChange::DuplicateChange (Drawing* d, DrawingView* dv) : (d, dv) {
    drawing->Sort();
    delete oldsl;
    oldsl = drawing->GetDuplicates();
}

// GroupChange stores the Selections' new parent and their
// predecessors.

GroupChange::GroupChange (Drawing* d, DrawingView* dv) : (d, dv, true) {
    grouplist = drawing->GetParent();
    prevlist = drawing->GetPrevs();
    done = false;
}

// Delete frees storage allocated for the Selections' new parent if
// they never end up grouped under it and the lists themselves.

GroupChange::~GroupChange () {
    if (!done) {
	for (grouplist->First(); !grouplist->AtEnd(); grouplist->Next()) {
	    delete grouplist->GetCur()->GetParent();
	}
    }
    delete grouplist;
    delete prevlist;
}

// Do groups the Selections under their parent.

void GroupChange::Do () {
    drawingview->EraseHandles();
    drawing->Group(grouplist);
    drawingview->Damaged();
    drawingview->Repair();
    done = true;
}

// Undo ungroups the Selections and puts them back where they came
// from in the Drawing.

void GroupChange::Undo () {
    drawingview->EraseHandles();
    drawing->Ungroup(grouplist);
    drawing->Remove();
    drawing->InsertAfterPrev(prevlist);
    drawingview->Damaged();
    drawingview->Repair();
    done = false;
}

// UngroupChange stores the Selections' children.

UngroupChange::UngroupChange (Drawing* d, DrawingView* dv) : (d, dv, true) {
    undogrouplist = drawing->GetChildren();
    done = false;
}

// Delete frees storage allocated for the Selections if they were
// ungrouped and for the list itself.

UngroupChange::~UngroupChange () {
    if (done) {
	for (undogrouplist->First(); !undogrouplist->AtEnd();
	     undogrouplist->Next())
	{
	    boolean haschildren = undogrouplist->GetCur()->GetHasChildren();
	    if (haschildren) {
		delete undogrouplist->GetCur()->GetParent();
	    }
	}
    }
    delete undogrouplist;
}

// Do replaces all Selections which contain children with their
// children.

void UngroupChange::Do () {
    drawingview->EraseHandles();
    drawing->Ungroup(undogrouplist);
    drawingview->RedrawHandles();
    done = true;
}

// Undo regroups each set of children under their former parents.

void UngroupChange::Undo () {
    drawingview->EraseHandles();
    drawing->Group(undogrouplist);
    drawingview->RedrawHandles();
    done = false;
}

// Skew comments/code ratio to work around cpp bug
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

// BringToFrontChange stores the Selections' predecessors.

BringToFrontChange::BringToFrontChange (Drawing* d, DrawingView* dv)
: (d, dv, true) {
    prevlist = drawing->GetPrevs();
}

// Delete frees storage allocated for the Selections' predecessors.

BringToFrontChange::~BringToFrontChange () {
    delete prevlist;
}

// Do brings the Selections to the front by removing them from and
// appending them to the Drawing.

void BringToFrontChange::Do () {
    drawingview->EraseHandles();
    drawing->Select(oldsl);
    drawing->Remove();
    drawing->Append();
    drawingview->Added();
    drawingview->Repair();
}

// Undo puts the Selections back where they came from in the Drawing.

void BringToFrontChange::Undo () {
    drawingview->EraseExcessHandles(oldsl);
    drawing->Select(oldsl);
    drawing->Remove();
    drawing->InsertAfterPrev(prevlist);
    drawingview->Damaged();
    drawingview->Repair();
}

// SendToBackChange stores the Selections' predecessors.

SendToBackChange::SendToBackChange (Drawing* d, DrawingView* dv)
: (d, dv, true) {
    prevlist = drawing->GetPrevs();
}

// Delete frees storage allocated for the Selections' predecessors.

SendToBackChange::~SendToBackChange () {
    delete prevlist;
}

// Do sends the Selections to the back by removing them from and
// prepending them to the Drawing.

void SendToBackChange::Do () {
    drawingview->EraseExcessHandles(oldsl);
    drawing->Select(oldsl);
    drawing->Remove();
    drawing->Prepend();
    drawingview->Damaged();
    drawingview->Repair();
}

// Undo puts the Selections back where they came from in the Drawing.

void SendToBackChange::Undo () {
    drawingview->EraseExcessHandles(oldsl);
    drawing->Select(oldsl);
    drawing->Remove();
    drawing->InsertAfterPrev(prevlist);
    drawingview->Damaged();
    drawingview->Repair();
}

// AlignChange stores the Selections' original positions and their
// desired alignments.

AlignChange::AlignChange (Drawing* d, DrawingView* dv, Alignment fix,
Alignment move) : (d, dv) {
    falign = fix;
    malign = move;
    centerlist = drawing->GetCenter();
}

// Delete frees storage allocated for the list of original positions.

AlignChange::~AlignChange () {
    delete centerlist;
}

// Do aligns the Selections.

void AlignChange::Do () {
    drawingview->EraseExcessHandles(oldsl);
    drawing->Select(oldsl);
    drawingview->Damaged();
    drawing->Align(falign, malign);
    drawingview->Damaged();
    drawingview->Repair();
}

// Undo moves the Selections to their original positions.

void AlignChange::Undo () {
    drawingview->EraseExcessHandles(oldsl);
    drawing->Select(oldsl);
    drawingview->Damaged();
    drawing->SetCenter(centerlist);
    drawingview->Damaged();
    drawingview->Repair();
}

// AlignToGridChange stores the Selections' original positions.

AlignToGridChange::AlignToGridChange (Drawing* d, DrawingView* dv) : (d, dv) {
    centerlist = drawing->GetCenter();
}

// Delete frees storage allocated for the list of original positions.

AlignToGridChange::~AlignToGridChange () {
    delete centerlist;
}

// Do aligns the Selections' lower left corners to the nearest grid
// point.

void AlignToGridChange::Do () {
    drawingview->EraseExcessHandles(oldsl);
    drawing->Select(oldsl);
    drawingview->Damaged();
    drawing->AlignToGrid();
    drawingview->Damaged();
    drawingview->Repair();
}

// Undo moves the Selections to their original positions.

void AlignToGridChange::Undo () {
    drawingview->EraseExcessHandles(oldsl);
    drawing->Select(oldsl);
    drawingview->Damaged();
    drawing->SetCenter(centerlist);
    drawingview->Damaged();
    drawingview->Repair();
}
