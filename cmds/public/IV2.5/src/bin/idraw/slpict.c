// $Header: slpict.c,v 1.14 89/06/03 14:52:10 linton Exp $
// implements class PictSelection.

#include "ipaint.h"
#include "istring.h"
#include "listifont.h"
#include "slellipses.h"
#include "sllines.h"
#include "slpict.h"
#include "slpolygons.h"
#include "slsplines.h"
#include "sltext.h"
#include <InterViews/transformer.h>
#include <stream.h>

// PictSelection initializes its graphic state.

PictSelection::PictSelection (Graphic* gs) : (gs) {
    valid = true;
}

// PictSelection knows it's the outermost PictSelection because it was
// called with a FILE* pointer, so it must read a version number, skip
// over its name, read its graphic state and children, and scale
// itself back to screen coordinates when it's finished.

PictSelection::PictSelection (FILE* stream, State* state) : (nil) {
    int fd = fileno(stream);
    istream from(fd);
    ReadVersion(from);
    ReadGridSpacing(from);
    if (versionnumber < 3) {
	Skip(from);
    }
    ReadPictGS(from, state);
    ReadChildren(from, state);
    ScaleToScreenCoords();
    valid = from.good();
}

// Copy returns a copy of the PictSelection.

Graphic* PictSelection::Copy () {
    Selection* copy = new PictSelection(this);
    for (First(); !AtEnd(); Next()) {
	copy->Append(GetCurrent()->Copy());
    }
    return copy;
}

// HasChildren returns true so Idraw can ungroup this Picture.

boolean PictSelection::HasChildren () {
    return Picture::HasChildren();
}

// Propagate must preserve the PictSelection's transformation matrix
// if it has any.

void PictSelection::Propagate () {
    Transformer* original = GetTransformer();
    if (original != nil) {
	original->Reference();
	Selection::Propagate();
	SetTransformer(original);
	delete original;
    } else {
	Selection::Propagate();
    }
}

// WritePicture writes the picture and returns true if the write
// succeeded or false if some IO error occurred.  It omits the
// Postscript prologue and trailer if called with verbose false to
// speed up cutting and pasting pictures between drawings.

boolean PictSelection::WritePicture (FILE* stream, boolean verbose) {
#if !defined(__GNUG__)
    // incompatible with g++, but works around a cfront bug
    filebuf fb(stream);
    ostream to(&fb);
#else
    int fd = fileno(stream);
    ostream to(fd);
#endif
    WritePicture(to, verbose);
    return to.good();
}

// PictSelection knows it's not the outermost PictSelection so it only
// reads data to initialize its graphic state and create its children
// Selections.

PictSelection::PictSelection (istream& from, State* state) : (nil) {
    ReadPictGS(from, state);
    ReadChildren(from, state);
    valid = from.good();
}

// ReadChildren loops determining which kind of Selection follows and
// creating it until it reads "end" which means all of the children
// have been created.

void PictSelection::ReadChildren (istream& from, State* state) {
    while (from.good()) {
	Skip(from);
	Selection* child = nil;
	from >> buf;
	if (strcmp(buf, "BSpl") == 0) {
	    child = new BSplineSelection(from, state);
	} else if (strcmp(buf, "Circ") == 0) {
	    child = new	CircleSelection(from, state);
	} else if (strcmp(buf, "CBSpl") == 0) {
	    child = new	ClosedBSplineSelection(from, state);
	} else if (strcmp(buf, "Elli") == 0) {
	    child = new	EllipseSelection(from, state);
	} else if (strcmp(buf, "Line") == 0) {
	    child = new	LineSelection(from, state);
	} else if (strcmp(buf, "MLine") == 0) {
	    child = new	MultiLineSelection(from, state);
	} else if (strcmp(buf, "Pict") == 0) {
	    child = new	PictSelection(from, state);
	} else if (strcmp(buf, "Poly") == 0) {
	    child = new	PolygonSelection(from, state);
	} else if (strcmp(buf, "Rect") == 0) {
	    child = new	RectSelection(from, state);
	} else if (strcmp(buf, "Text") == 0) {
	    child = new	TextSelection(from, state);
	} else if (strcmp(buf, "eop") == 0) {
	    break;
	} else {
	    fprintf(stderr, "unknown Selection %s, skipping\n", buf);
	    continue;
	}
	if (from.good()) {
	    Append(child);
	} else {
	    delete child;
	}
    }
}

// WritePicture writes the picture's data and Postscript code to print
// it wrapped in Postscript comments that minimally conform to version
// 1.0 of Adobe Systems's structuring conventions for Postscript.  The
// picture must remove itself from its parent if it has a parent to
// prevent the parent's transformation from affecting the picture's
// calculation of its bounding box.

void PictSelection::WritePicture (ostream& to, boolean verbose) {
    Picture* parent = (Picture*) Parent();
    if (parent != nil) {
	parent->SetCurrent(this);
	parent->Remove(this);
    }

    ScaleToPostscriptCoords();
    if (verbose) {
	WriteComments(to);
	WritePrologue(to);
	WriteVersion(to);
	WriteGridSpacing(to);
	WriteDrawing(to);
	WriteTrailer(to);
    } else {
	WriteVersion(to);
	WriteGridSpacing(to);
	WriteDrawing(to);
    }
    ScaleToScreenCoords();

    if (parent != nil) {
	parent->InsertBeforeCur(this);
    }
}

// WriteComments writes information about the picture such as the
// fonts used in it and the smallest bounding box enclosing it.

void PictSelection::WriteComments (ostream& to) {
    to << "%!PS-Adobe-2.0 EPSF-1.2\n";

    to << "%%DocumentFonts:";
    int linelen = strlen("%%DocumentFonts:");
    const int MAXLINELEN = 256;
    IFontList* fontlist = new IFontList;
    CollectFonts(fontlist);
    for (fontlist->First(); !fontlist->AtEnd(); fontlist->Next()) {
	IFont* font = fontlist->GetCur()->GetFont();
	if (linelen + strlen(font->GetPrintFont()) + 2 <= MAXLINELEN) {
	    to << " ";
	    ++linelen;
	} else {
	    to << "\n%%+ ";
	    linelen = strlen("%%+ ");
	}
	to << font->GetPrintFont();
	linelen += strlen(font->GetPrintFont());
    }
    to << "\n";
    delete fontlist;

    to << "%%Pages: 1\n";

    Coord l, b, r, t;
    GetBox(l, b, r, t);
    to << "%%BoundingBox: " << l << " " << b << " " << r << " " << t << "\n";

    to << "%%EndComments\n\n";
    to << "50 dict begin\n\n";
}

// WritePrologue writes definitions of Postscript procedures to draw
// Selections.  You should not rename or delete the "exported"
// capitalized procedures because old drawings rely on them, but you
// can rename or delete the "internal" uncapitalized procedures.

void PictSelection::WritePrologue (ostream& to) {
    to << "/arrowHeight " << ARROWHEIGHT << " def\n";
    to << "/arrowWidth " << ARROWWIDTH << " def\n";
    to << "/none null def\n";
    to << "/numGraphicParameters 17 def\n";
    to << "/stringLimit 65535 def\n\n";
    to << "/Begin {\n";
    to << "save\n";
    to << "numGraphicParameters dict begin\n";
    to << "} def\n\n";
    to << "/End {\n";
    to << "end\n";
    to << "restore\n";
    to << "} def\n\n";
    to << "/SetB {\n";
    to << "dup type /nulltype eq {\n";
    to << "pop\n";
    to << "false /brushRightArrow idef\n";
    to << "false /brushLeftArrow idef\n";
    to << "true /brushNone idef\n";
    to << "} {\n";
    to << "/brushDashOffset idef\n";
    to << "/brushDashArray idef\n";
    to << "0 ne /brushRightArrow idef\n";
    to << "0 ne /brushLeftArrow idef\n";
    to << "/brushWidth idef\n";
    to << "false /brushNone idef\n";
    to << "} ifelse\n";
    to << "} def\n\n";
    to << "/SetCFg {\n";
    to << "/fgblue idef\n";
    to << "/fggreen idef\n";
    to << "/fgred idef\n";
    to << "} def\n\n";
    to << "/SetCBg {\n";
    to << "/bgblue idef\n";
    to << "/bggreen idef\n";
    to << "/bgred idef\n";
    to << "} def\n\n";
    to << "/SetF {\n";
    to << "/printSize idef\n";
    to << "/printFont idef\n";
    to << "} def\n\n";
    to << "/SetP {\n";
    to << "dup type /nulltype eq {\n";
    to << "pop true /patternNone idef\n";
    to << "} {\n";
    to << "/patternGrayLevel idef\n";
    to << "patternGrayLevel -1 eq {\n";
    to << "/patternString idef\n";
    to << "} if\n";
    to << "false /patternNone idef\n";
    to << "} ifelse\n";
    to << "} def\n\n";
    to << "/BSpl {\n";
    to << "0 begin\n";
    to << "storexyn\n";
    to << "newpath\n";
    to << "n 1 gt {\n";
    to << "0 0 0 0 0 0 1 1 true subspline\n";
    to << "n 2 gt {\n";
    to << "0 0 0 0 1 1 2 2 false subspline\n";
    to << "1 1 n 3 sub {\n";
    to << "/i exch def\n";
    to << "i 1 sub dup i dup i 1 add dup i 2 add dup false subspline\n";
    to << "} for\n";
    to << "n 3 sub dup n 2 sub dup n 1 sub dup 2 copy false subspline\n";
    to << "} if\n";
    to << "n 2 sub dup n 1 sub dup 2 copy 2 copy false subspline\n";
    to << "patternNone not brushLeftArrow not brushRightArrow not and and { ";
    to << "ifill } if\n";
    to << "brushNone not { istroke } if\n";
    to << "0 0 1 1 leftarrow\n";
    to << "n 2 sub dup n 1 sub dup rightarrow\n";
    to << "} if\n";
    to << "end\n";
    to << "} dup 0 4 dict put def\n\n";
    to << "/Circ {\n";
    to << "newpath\n";
    to << "0 360 arc\n";
    to << "patternNone not { ifill } if\n";
    to << "brushNone not { istroke } if\n";
    to << "} def\n\n";
    to << "/CBSpl {\n";
    to << "0 begin\n";
    to << "dup 2 gt {\n";
    to << "storexyn\n";
    to << "newpath\n";
    to << "n 1 sub dup 0 0 1 1 2 2 true subspline\n";
    to << "1 1 n 3 sub {\n";
    to << "/i exch def\n";
    to << "i 1 sub dup i dup i 1 add dup i 2 add dup false subspline\n";
    to << "} for\n";
    to << "n 3 sub dup n 2 sub dup n 1 sub dup 0 0 false subspline\n";
    to << "n 2 sub dup n 1 sub dup 0 0 1 1 false subspline\n";
    to << "patternNone not { ifill } if\n";
    to << "brushNone not { istroke } if\n";
    to << "} {\n";
    to << "Poly\n";
    to << "} ifelse\n";
    to << "end\n";
    to << "} dup 0 4 dict put def\n\n";
    to << "/Elli {\n";
    to << "0 begin\n";
    to << "newpath\n";
    to << "4 2 roll\n";
    to << "translate\n";
    to << "scale\n";
    to << "0 0 1 0 360 arc\n";
    to << "patternNone not { ifill } if\n";
    to << "brushNone not { istroke } if\n";
    to << "end\n";
    to << "} dup 0 1 dict put def\n\n";
    to << "/Line {\n";
    to << "0 begin\n";
    to << "2 storexyn\n";
    to << "newpath\n";
    to << "x 0 get y 0 get moveto\n";
    to << "x 1 get y 1 get lineto\n";
    to << "brushNone not { istroke } if\n";
    to << "0 0 1 1 leftarrow\n";
    to << "0 0 1 1 rightarrow\n";
    to << "end\n";
    to << "} dup 0 4 dict put def\n\n";
    to << "/MLine {\n";
    to << "0 begin\n";
    to << "storexyn\n";
    to << "newpath\n";
    to << "n 1 gt {\n";
    to << "x 0 get y 0 get moveto\n";
    to << "1 1 n 1 sub {\n";
    to << "/i exch def\n";
    to << "x i get y i get lineto\n";
    to << "} for\n";
    to << "patternNone not brushLeftArrow not brushRightArrow not and and { ";
    to << "ifill } if\n";
    to << "brushNone not { istroke } if\n";
    to << "0 0 1 1 leftarrow\n";
    to << "n 2 sub dup n 1 sub dup rightarrow\n";
    to << "} if\n";
    to << "end\n";
    to << "} dup 0 4 dict put def\n\n";
    to << "/Poly {\n";
    to << "3 1 roll\n";
    to << "newpath\n";
    to << "moveto\n";
    to << "-1 add\n";
    to << "{ lineto } repeat\n";
    to << "closepath\n";
    to << "patternNone not { ifill } if\n";
    to << "brushNone not { istroke } if\n";
    to << "} def\n\n";
    to << "/Rect {\n";
    to << "0 begin\n";
    to << "/t exch def\n";
    to << "/r exch def\n";
    to << "/b exch def\n";
    to << "/l exch def\n";
    to << "newpath\n";
    to << "l b moveto\n";
    to << "l t lineto\n";
    to << "r t lineto\n";
    to << "r b lineto\n";
    to << "closepath\n";
    to << "patternNone not { ifill } if\n";
    to << "brushNone not { istroke } if\n";
    to << "end\n";
    to << "} dup 0 4 dict put def\n\n";
    to << "/Text {\n";
    to << "ishow\n";
    to << "} def\n\n";
    to << "/idef {\n";
    to << "dup where { pop pop pop } { exch def } ifelse\n";
    to << "} def\n\n";
    to << "/ifill {\n";
    to << "0 begin\n";
    to << "gsave\n";
    to << "patternGrayLevel -1 ne {\n";
    to << "fgred bgred fgred sub patternGrayLevel mul add\n";
    to << "fggreen bggreen fggreen sub patternGrayLevel mul add\n";
    to << "fgblue bgblue fgblue sub patternGrayLevel mul add setrgbcolor\n";
    to << "eofill\n";
    to << "} {\n";
    to << "eoclip\n";
    to << "originalCTM setmatrix\n";
    to << "pathbbox /t exch def /r exch def /b exch def /l exch def\n";
    to << "/w r l sub ceiling cvi def\n";
    to << "/h t b sub ceiling cvi def\n";
    to << "/imageByteWidth w 8 div ceiling cvi def\n";
    to << "/imageHeight h def\n";
    to << "bgred bggreen bgblue setrgbcolor\n";
    to << "eofill\n";
    to << "fgred fggreen fgblue setrgbcolor\n";
    to << "w 0 gt h 0 gt and {\n";
    to << "l b translate w h scale\n";
    to << "w h true [w 0 0 h neg 0 h] { patternproc } imagemask\n";
    to << "} if\n";
    to << "} ifelse\n";
    to << "grestore\n";
    to << "end\n";
    to << "} dup 0 8 dict put def\n\n";
    to << "/istroke {\n";
    to << "gsave\n";
    to << "brushDashOffset -1 eq {\n";
    to << "[] 0 setdash\n";
    to << "1 setgray\n";
    to << "} {\n";
    to << "brushDashArray brushDashOffset setdash\n";
    to << "fgred fggreen fgblue setrgbcolor\n";
    to << "} ifelse\n";
    to << "brushWidth setlinewidth\n";
    to << "originalCTM setmatrix\n";
    to << "stroke\n";
    to << "grestore\n";
    to << "} def\n\n";
    to << "/ishow {\n";
    to << "0 begin\n";
    to << "gsave\n";
    to << "printFont findfont printSize scalefont setfont\n";
    to << "fgred fggreen fgblue setrgbcolor\n";
    to << "/vertoffset printSize neg def {\n";
    to << "0 vertoffset moveto show\n";
    to << "/vertoffset vertoffset printSize sub def\n";
    to << "} forall\n";
    to << "grestore\n";
    to << "end\n";
    to << "} dup 0 3 dict put def\n\n";
    to << "/patternproc {\n";
    to << "0 begin\n";
    to << "/patternByteLength patternString length def\n";
    to << "/patternHeight patternByteLength 8 mul sqrt cvi def\n";
    to << "/patternWidth patternHeight def\n";
    to << "/patternByteWidth patternWidth 8 idiv def\n";
    to << "/imageByteMaxLength imageByteWidth imageHeight mul\n";
    to << "stringLimit patternByteWidth sub min def\n";
    to << "/imageMaxHeight imageByteMaxLength imageByteWidth idiv ";
    to << "patternHeight idiv\n";
    to << "patternHeight mul patternHeight max def\n";
    to << "/imageHeight imageHeight imageMaxHeight sub store\n";
    to << "/imageString imageByteWidth imageMaxHeight mul patternByteWidth ";
    to << "add string def\n";
    to << "0 1 imageMaxHeight 1 sub {\n";
    to << "/y exch def\n";
    to << "/patternRow y patternByteWidth mul patternByteLength mod def\n";
    to << "/patternRowString patternString patternRow patternByteWidth ";
    to << "getinterval def\n";
    to << "/imageRow y imageByteWidth mul def\n";
    to << "0 patternByteWidth imageByteWidth 1 sub {\n";
    to << "/x exch def\n";
    to << "imageString imageRow x add patternRowString putinterval\n";
    to << "} for\n";
    to << "} for\n";
    to << "imageString\n";
    to << "end\n";
    to << "} dup 0 12 dict put def\n\n";
    to << "/min {\n";
    to << "dup 3 2 roll dup 4 3 roll lt { exch } if pop\n";
    to << "} def\n\n";
    to << "/max {\n";
    to << "dup 3 2 roll dup 4 3 roll gt { exch } if pop\n";
    to << "} def\n\n";
    to << "/arrowhead {\n";
    to << "0 begin\n";
    to << "transform originalCTM itransform\n";
    to << "/taily exch def\n";
    to << "/tailx exch def\n";
    to << "transform originalCTM itransform\n";
    to << "/tipy exch def\n";
    to << "/tipx exch def\n";
    to << "/dy tipy taily sub def\n";
    to << "/dx tipx tailx sub def\n";
    to << "/angle dx 0 ne dy 0 ne or { dy dx atan } { 90 } ifelse def\n";
    to << "gsave\n";
    to << "originalCTM setmatrix\n";
    to << "tipx tipy translate\n";
    to << "angle rotate\n";
    to << "newpath\n";
    to << "0 0 moveto\n";
    to << "arrowHeight neg arrowWidth 2 div lineto\n";
    to << "arrowHeight neg arrowWidth 2 div neg lineto\n";
    to << "closepath\n";
    to << "patternNone not {\n";
    to << "originalCTM setmatrix\n";
    to << "/padtip arrowHeight 2 exp 0.25 arrowWidth 2 exp mul add sqrt ";
    to << "brushWidth mul\n";
    to << "arrowWidth div def\n";
    to << "/padtail brushWidth 2 div def\n";
    to << "tipx tipy translate\n";
    to << "angle rotate\n";
    to << "padtip 0 translate\n";
    to << "arrowHeight padtip add padtail add arrowHeight div dup scale\n";
    to << "arrowheadpath\n";
    to << "ifill\n";
    to << "} if\n";
    to << "brushNone not {\n";
    to << "originalCTM setmatrix\n";
    to << "tipx tipy translate\n";
    to << "angle rotate\n";
    to << "arrowheadpath\n";
    to << "istroke\n";
    to << "} if\n";
    to << "grestore\n";
    to << "end\n";
    to << "} dup 0 9 dict put def\n\n";
    to << "/arrowheadpath {\n";
    to << "newpath\n";
    to << "0 0 moveto\n";
    to << "arrowHeight neg arrowWidth 2 div lineto\n";
    to << "arrowHeight neg arrowWidth 2 div neg lineto\n";
    to << "closepath\n";
    to << "} def\n\n";
    to << "/leftarrow {\n";
    to << "0 begin\n";
    to << "y exch get /taily exch def\n";
    to << "x exch get /tailx exch def\n";
    to << "y exch get /tipy exch def\n";
    to << "x exch get /tipx exch def\n";
    to << "brushLeftArrow { tipx tipy tailx taily arrowhead } if\n";
    to << "end\n";
    to << "} dup 0 4 dict put def\n\n";
    to << "/rightarrow {\n";
    to << "0 begin\n";
    to << "y exch get /tipy exch def\n";
    to << "x exch get /tipx exch def\n";
    to << "y exch get /taily exch def\n";
    to << "x exch get /tailx exch def\n";
    to << "brushRightArrow { tipx tipy tailx taily arrowhead } if\n";
    to << "end\n";
    to << "} dup 0 4 dict put def\n\n";
    to << "/midpoint {\n";
    to << "0 begin\n";
    to << "/y1 exch def\n";
    to << "/x1 exch def\n";
    to << "/y0 exch def\n";
    to << "/x0 exch def\n";
    to << "x0 x1 add 2 div\n";
    to << "y0 y1 add 2 div\n";
    to << "end\n";
    to << "} dup 0 4 dict put def\n\n";
    to << "/thirdpoint {\n";
    to << "0 begin\n";
    to << "/y1 exch def\n";
    to << "/x1 exch def\n";
    to << "/y0 exch def\n";
    to << "/x0 exch def\n";
    to << "x0 2 mul x1 add 3 div\n";
    to << "y0 2 mul y1 add 3 div\n";
    to << "end\n";
    to << "} dup 0 4 dict put def\n\n";
    to << "/subspline {\n";
    to << "0 begin\n";
    to << "/movetoNeeded exch def\n";
    to << "y exch get /y3 exch def\n";
    to << "x exch get /x3 exch def\n";
    to << "y exch get /y2 exch def\n";
    to << "x exch get /x2 exch def\n";
    to << "y exch get /y1 exch def\n";
    to << "x exch get /x1 exch def\n";
    to << "y exch get /y0 exch def\n";
    to << "x exch get /x0 exch def\n";
    to << "x1 y1 x2 y2 thirdpoint\n";
    to << "/p1y exch def\n";
    to << "/p1x exch def\n";
    to << "x2 y2 x1 y1 thirdpoint\n";
    to << "/p2y exch def\n";
    to << "/p2x exch def\n";
    to << "x1 y1 x0 y0 thirdpoint\n";
    to << "p1x p1y midpoint\n";
    to << "/p0y exch def\n";
    to << "/p0x exch def\n";
    to << "x2 y2 x3 y3 thirdpoint\n";
    to << "p2x p2y midpoint\n";
    to << "/p3y exch def\n";
    to << "/p3x exch def\n";
    to << "movetoNeeded { p0x p0y moveto } if\n";
    to << "p1x p1y p2x p2y p3x p3y curveto\n";
    to << "end\n";
    to << "} dup 0 17 dict put def\n\n";
    to << "/storexyn {\n";
    to << "/n exch def\n";
    to << "/y n array def\n";
    to << "/x n array def\n";
    to << "n 1 sub -1 0 {\n";
    to << "/i exch def\n";
    to << "y i 3 2 roll put\n";
    to << "x i 3 2 roll put\n";
    to << "} for\n";
    to << "} def\n\n";
    to << "%%EndProlog\n\n";
}

// WriteDrawing writes code to store the picture's transformation
// matrix in a Postscript variable and code to draw the picture.

void PictSelection::WriteDrawing (ostream& to) {
    to << "%%Page: 1 1\n\n";
    to << "Begin\n";
    WritePictGS(to);
    to << "/originalCTM matrix currentmatrix def\n\n";

    for (First(); !AtEnd(); Next()) {
	Selection* s = GetCurrent();
	s->WriteData(to);
    }

    to << "End " << startdata << " eop\n\n";
    to << "showpage\n\n";
}

// WriteData writes the PictSelection's data and its children
// Selections' data with Postscript code to draw them.

void PictSelection::WriteData (ostream& to) {
    to << "Begin " << startdata << " Pict\n";
    WritePictGS(to);
    to << "\n";

    for (First(); !AtEnd(); Next()) {
	Selection* s = GetCurrent();
	s->WriteData(to);
    }

    to << "End " << startdata << " eop\n\n";
}

// WriteTrailer writes clean up code.

void PictSelection::WriteTrailer (ostream& to) {
    to << "%%Trailer\n\n";
    to << "end\n";
}

// ScaleToPostscriptCoords scales the picture to Postscript
// coordinates if screen and Postscript inches are different.

void PictSelection::ScaleToPostscriptCoords () {
    const double postscriptinch = 72.;

    if (inch != postscriptinch) {
	double topostscript = postscriptinch / inch;
	Scale(topostscript, topostscript);
    }
}

// ScaleToScreenCoords scales the picture back to screen coordinates
// if screen and Postscript inches are different.

void PictSelection::ScaleToScreenCoords () {
    const double postscriptinch = 72.;

    if (inch != postscriptinch) {
	double toscreen = inch / postscriptinch;
	Scale(toscreen, toscreen);
    }
}

// CollectFonts adds its font, if it has one, to the list without
// checking if the PictSelection contains any TextSelections.  If it
// doesn't have a font, it collects its children TextSelection's
// fonts.

void PictSelection::CollectFonts (IFontList* fontlist) {
    if (GetFont() != nil) {
	Merge((IFont*) GetFont(), fontlist);
    } else {
	for (First(); !AtEnd(); Next()) {
	    Selection* s = GetCurrent();
	    if (s->HasChildren()) {
		((PictSelection*) s)->CollectFonts(fontlist);
	    } else if (s->IsA(TEXTSELECTION)) {
		Merge((IFont*) s->GetFont(), fontlist);
	    }
	}
    }
}

// Merge merges the print font with the list of all print fonts unless
// the list already has it.

void PictSelection::Merge (IFont* font, IFontList* fontlist) {
    boolean found = false;
    if (font == nil) {
	found = true;
    } else if (fontlist->Find(font)) {
	found = true;
    } else {
	for (fontlist->First(); !fontlist->AtEnd(); fontlist->Next()) {
	    IFont* cmp = fontlist->GetCur()->GetFont();
	    if (strcmp(font->GetPrintFont(), cmp->GetPrintFont()) == 0) {
		found = true;
		break;
	    }
	}
    }
    if (!found) {
	fontlist->Append(new IFontNode(font));
    }
}
