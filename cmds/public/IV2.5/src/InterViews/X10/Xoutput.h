/*
 * Output related definitions from <X/Xlib.h>.
 * Copyright (c) 1985 Massachusetts Institute of Technology
 */

#ifndef Xoutput_h
#define Xoutput_h

#include <InterViews/X10/Xdefs.h>

#define XMakePattern(pattern, patlen, patmul)\
	((XPattern)(((patmul) << 20) | (((patlen) - 1) << 16) | (pattern) ))
#define AllPlanes (~0)

#define	GXclear			0x0		/* 0 */
#define GXand			0x1		/* src AND dst */
#define GXandReverse		0x2		/* src AND NOT dst */
#define GXcopy			0x3		/* src */
#define GXandInverted		0x4		/* NOT src AND dst */
#define	GXnoop			0x5		/* dst */
#define GXxor			0x6		/* src XOR dst */
#define GXor			0x7		/* src OR dst */
#define GXnor			0x8		/* NOT src AND NOT dst */
#define GXequiv			0x9		/* NOT src XOR dst */
#define GXinvert		0xa		/* NOT dst */
#define GXorReverse		0xb		/* src OR NOT dst */
#define GXcopyInverted		0xc		/* NOT src */
#define GXorInverted		0xd		/* NOT src OR dst */
#define GXnand			0xe		/* NOT src OR NOT dst */
#define GXset			0xf		/* 1 */

/* Used in X_TileMode */

#define TileModeAbsolute	0
#define TileModeRelative	1

/* Used in X_ClipMode */

#define ClipModeClipped		0
#define ClipModeDrawThru	1

/* Used in X_Draw */

#define	DrawSolidLine		0
#define DrawDashedLine		1
#define DrawPatternedLine	2

/* Used in X_Draw and X_DrawFilled */

typedef struct _Vertex {
	short x, y;
	unsigned short flags;
} XVertex;

/* The meanings of the flag bits.  If the bit is 1 the predicate is true */

#define VertexRelative		0x0001		/* else absolute */
#define VertexDontDraw		0x0002		/* else draw */
#define VertexCurved		0x0004		/* else straight */
#define VertexStartClosed	0x0008		/* else not */
#define VertexEndClosed		0x0010		/* else not */
#define VertexDrawLastPoint	0x0020		/* else don't */

/* Used in X_StoreColors */

typedef struct _ColorDef {
	unsigned short pixel;
	unsigned short red, green, blue;
} XColorDef;

/* Used in X_PixmapBitsPut and X_StorePixmap */

#define XYFormat		0
#define ZFormat			1

/* size in bytes of a bitmap */
#define BitmapSize(width, height) (((((width) + 15) >> 3) &~ 1) * (height))
/* size in bytes of a pixmap in XYFormat */
#define XYPixmapSize(width, height, planes) (BitmapSize(width, height) * (planes))
/* size in bytes of a pizmap in ZFormat for 9 to 16 bit planes */
#define WZPixmapSize(width, height) (((width) * (height)) << 1)
/* size in bytes of a pixmap in ZFormat for 2 to 8 bit planes */
#define BZPixmapSize(width, height) ((width) * (height))

/* Used in X_QueryShape */

#define CursorShape		0
#define TileShape		1
#define BrushShape		2

/* 
 * Data returned by XQueryFont.
 */
typedef struct _FontInfo {
	XFont id;
	short height, width, baseline, fixedwidth;
	unsigned char firstchar, lastchar;
	short* widths;		/* pointer to width array in OpenFont */
} XFontInfo;


/*
 * Data structure used by color operations; ints rather than shorts
 * to keep 16 bit protocol limitation out of the library.
 */
typedef struct _Color {
	int pixel;
	unsigned short red, green, blue;
} XColor;


/*
 * Data structure use by XCreateTiles.
 */
typedef struct _TileFrame {
	int pixel;		/* Pixel color for constructing the tile. */
	XPixmap pixmap;		/* Pixmap id of the pixmap, filled in later. */
} XTileFrame;

/*
 * Line pattern related definitions for the library.
 */
typedef long XPattern;

#define DashedLine XMakePattern(0xf0f0, 16, 1)
#define DottedLine XMakePattern(0xaaaa, 16, 1)
#define DotDashLine XMakePattern(0xf4f4, 16, 1)
#define SolidLine  XMakePattern(1,1,1)

void XLine(
    XWindow, int x1, int y1, int x2, int y2, int width, int height,
    int pixel, int func, int planes
);
void XDraw(
    XWindow, XVertex v[], int, int width, int height,
    int pixel, int func, int planes
);
void XDrawPatterned(
    XWindow, XVertex v[], int, int width, int height,
    int pixel, int altpix, XPattern pat, int func, int planes
);
void XDrawDashed(
    XWindow, XVertex v[], int, int width, int height,
    int pixel, XPattern pat, int func, int planes
);
void XDrawTiled(
    XWindow, XVertex v[], int, XPixmap, int func, int planes
);
void XDrawFilled(
    XWindow, XVertex v[], int, int pixel, int func, int planes
);
void XQueryBrushShape (int width, int height, int *cwidth, int *cheight);

void XClear(XWindow);
void XPixSet(XWindow, int x, int y, int width, int height, int pixel);
void XPixFill(
    XWindow, int x, int y, int width, int height, int pixel,
    XBitmap clipmask, int func, int planes
);
void XPixmapPut(
    XWindow, int sx, int sy, int dx, int dy, int width, int height,
    XPixmap, int func, int planes
);
void XQueryTileShape(int width, int height, int *rwidth, int *rheight);
void XTileSet(XWindow, int x, int y, int width, int height, XPixmap);
void XTileFill(
    XWindow, int x, int y, int width, int height, XPixmap,
    XBitmap clipmask, int func, int planes
);

void XMoveArea(XWindow, int sx, int sy, int dx, int dy, int width, int height);
void XCopyArea(
    XWindow, int sx, int sy, int dx, int dy, int width, int height,
    int func, int planes
);

void XPixmapBitsPutXY(
    XWindow, int x, int y, int width, int height, short* data, XBitmap mask,
    int func, int planes
);
void XPixmapBitsPutZ(
    XWindow, int x, int y, int width, int height, void* data, XBitmap mask,
    int func, int planes
);
void XBitmapBitsPut(
    XWindow, int x, int y, int width, int height, short* data, int fg, int bg,
    XBitmap mask, int func, int planes
);
XPixmap XPixmapSave(XWindow, int x, int y, int width, int height);
void XPixmapGetXY(XWindow, int x, int y, int width, int height, void* data);
void XPixmapGetZ(XWindow, int x, int y, int width, int height, void* data);

XPixmap XStorePixmapXY(int width, int height, void* data);
XPixmap XStorePixmapZ(int width, int height, void* data, int format);

XBitmap XStoreBitmap(int width, int height, void* data);
XPixmap XMakePixmap(XBitmap, int fg, int bg);
XPixmap XMakeTile(int pixel);
void XFreePixmap(XPixmap);
void XFreeBitmap(XBitmap);
XBitmap XCharBitmap(XFont, int ch);

XCursor XStoreCursor(
    XBitmap cursor, XBitmap mask, int xoff, int yoff, int fg, int bg, int func
);
void XQueryCursorShape(int width, int height, int* rwidth, int* rheight);
void XFreeCursor(XCursor);
XCursor XCreateCursor(
    int width, int height, short* cursor, short* mask, int xoff, int yoff,
    int fg, int bg, int func
);
void XDefineCursor(XWindow, XCursor);
void XUndefineCursor(XWindow);

XStatus XGetHardwareColor(XColor* );
XStatus XGetColorCells(
    int contig, int ncolors, int nplanes, int* planes, int pixels[]
);
XStatus XGetColor(const char*, XColor* hard_def, XColor* exact_def);
void XFreeColors(int pixels[], int);
void XStoreColors(int, XColor*);
void XStoreColor(XColor*);
XStatus XQueryColor(XColor*);
void XQueryColors(XColor defs[], int);
XStatus XParseColor(const char*, XColor*);

XFontInfo* XOpenFont(const char*);
void XCloseFont(XFontInfo*);
XFont XGetFont(const char*);
XStatus XQueryFont(XFont, XFontInfo*);
void XFreeFont(XFont);
XStatus XCharWidths(const char*, int, XFont, short*);
short* XFontWidths(XFont);
int XQueryWidth(const char*, XFont);
int XStringWidth(const char*, XFontInfo*, int charpad, int spacepad);

void XText(XWindow, int x, int y, const char*, int, XFont, int fg, int bg);
void XTextPad(
    XWindow, int x, int y, const char*, int, XFont,
    int charpad, int spacepad, int fg, int bg, int func, int planes
);
void XTextMask(XWindow, int x, int y, const char*, int, XFont, int fg);
void XTextMaskPad(
    XWindow, int x, int y, const char*, int, XFont,
    int charpad, int spacepad, int fg, int func, int planes
);

#endif
