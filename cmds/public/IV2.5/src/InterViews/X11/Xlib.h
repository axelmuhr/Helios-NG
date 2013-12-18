/*
 * C++ interface to standard Xlib.h.
 *
 * We must disable C++ keywords that might be used as identifiers
 * in Xlib.h, and we also must disable function identifiers so we
 * can define their parameter types later.  Finally, we must use
 * different names for X types to avoid conflicts with InterViews
 * (e.g., Font).
 */

#ifndef Xlib_h
#define Xlib_h

#define class CC_class
#define new CC_new
#define delete CC_delete
#define inline CC_inline
#define virtual CC_virtual

#define XLoadQueryFont cc_XLoadQueryFont
#define XQueryFont cc_XQueryFont
#define XGetMotionEvents cc_XGetMotionEvents
#define XOpenDisplay cc_XOpenDisplay
#define XCreate cc_XCreate
#define XFetchBytes cc_XFetchBytes
#define XFetchBuffer cc_XFetchBuffer
#define XGetAtomName cc_XGetAtomName
#define XGetDefault cc_XGetDefault
#define XDisplayName cc_XDisplayName
#define XKeysymToString cc_XKeysymToString
#define XSynchronize cc_XSynchronize
#define XSetAfterFunction cc_XSetAfterFunction
#define XInternAtom cc_XInternAtom
#define XCopyColormapAndFree cc_XCopyColormapAndFree
#define XCreateColormap cc_XCreateColormap
#define XCreatePixmapCursor cc_XCreatePixmapCursor
#define XCreateGlyphCursor cc_XCreateGlyphCursor
#define XCreateFontCursor cc_XCreateFontCursor
#define XLoadFont cc_XLoadFont
#define XCreateGC cc_XCreateGC
#define XGContextFromGC cc_XGContextFromGC
#define XCreatePixmap cc_XCreatePixmap
#define XCreateBitmapFromData cc_XCreateBitmapFromData
#define XCreatePixmapFromBitmapData cc_XCreatePixmapFromBitmapData
#define XCreateSimpleWindow cc_XCreateSimpleWindow
#define XGetSelectionOwner cc_XGetSelectionOwner
#define XGetIconWindow cc_XGetIconWindow
#define XCreateWindow cc_XCreateWindow
#define XListInstalledColormaps cc_XListInstalledColormaps
#define XListFonts cc_XListFonts
#define XListFontsWithInfo cc_XListFontsWithInfo
#define XGetFontPath cc_XGetFontPath
#define XListExtensions cc_XListExtensions
#define XListProperties cc_XListProperties
#define XCreateImage cc_XCreateImage
#define XPutImage cc_XPutImage
#define XGetImage cc_XGetImage
#define XGetSubImage cc_XGetSubImage
#define XListHosts cc_XListHosts
#define XNewModifiermap cc_XNewModifiermap
#define XGetModifierMapping cc_XGetModifierMapping
#define XDeleteModifiermapEntry cc_XDeleteModifiermapEntry
#define XInsertModifiermapEntry cc_XInsertModifiermapEntry
#define XKeycodeToKeysym cc_XKeycodeToKeysym
#define XLookupKeysym cc_XLookupKeysym
#define XGetKeyboardMapping cc_XGetKeyboardMapping
#define XStringToKeysym cc_XStringToKeysym
#define XInitExtension cc_XInitExtension
#define XESetCreateGC cc_XESetCreateGC
#define XESetCopyGC cc_XESetCopyGC
#define XESetFlushGC cc_XESetFlushGC
#define XESetFreeGC cc_XESetFreeGC
#define XESetCreateFont cc_XESetCreateFont
#define XESetFreeFont cc_XESetFreeFont
#define XESetCloseDisplay cc_XESetCloseDisplay
#define XESetError cc_XESetError
#define XESetErrorString cc_XESetErrorString
#define XESetWireToEvent cc_XESetWireToEvent
#define XESetEventToWire cc_XESetEventToWire
#define XRootWindow cc_XRootWindow
#define XDefaultRootWindow cc_XDefaultRootWindow
#define XRootWindowOfScreen cc_XRootWindowOfScreen
#define XDefaultVisual cc_XDefaultVisual
#define XDefaultVisualOfScreen cc_XDefaultVisualOfScreen
#define XDefaultGC cc_XDefaultGC
#define XDefaultGCofScreen cc_XDefaultGCofScreen
#define XBlackPixel cc_XBlackPixel
#define XWhitePixel cc_XWhitePixel
#define XAllPlanes cc_XAllPlanes
#define XBlackPixelOfScreen cc_XBlackPixelOfScreen
#define XWhitePixelOfScreen cc_XWhitePixelOfScreen
#define XNextRequest cc_XNextRequest
#define XLastKnownRequestProcessed cc_XLastKnownRequestProcessed
#define XServerVendor cc_XServerVendor
#define XDisplayString cc_XDisplayString
#define XDefaultColormap cc_XDefaultColormap
#define XDefaultColormapOfScreen cc_XDefaultColormapOfScreen
#define XDisplayOfScreen cc_XDisplayOfScreen
#define XScreenOfDisplay cc_XScreenOfDisplay
#define XDefaultScreenOfDisplay cc_XDefaultScreenOfDisplay
#define XEventMaskOfScreen cc_XEventMaskOfScreen
#define funcs cc_funcs

#define Cursor XCursor
#define Bitmap XBitmap
#define Colormap XColormap
#define Font XFont

#include <X11/Xlib.h>

#undef Cursor
#undef Bitmap
#undef Colormap
#undef Font

#undef XLoadQueryFont
#undef XQueryFont
#undef XGetMotionEvents
#undef XOpenDisplay
#undef XCreate
#undef XFetchBytes
#undef XFetchBuffer
#undef XGetDefault
#undef XGetAtomName
#undef XDisplayName
#undef XKeysymToString
#undef XSynchronize
#undef XSetAfterFunction
#undef XInternAtom
#undef XCopyColormapAndFree
#undef XCreateColormap
#undef XCreatePixmapCursor
#undef XCreateGlyphCursor
#undef XCreateFontCursor
#undef XLoadFont
#undef XCreateGC
#undef XGContextFromGC
#undef XCreatePixmap
#undef XCreateBitmapFromData
#undef XCreatePixmapFromBitmapData
#undef XCreateSimpleWindow
#undef XGetSelectionOwner
#undef XGetIconWindow
#undef XCreateWindow
#undef XListInstalledColormaps
#undef XListFonts
#undef XListFontsWithInfo
#undef XGetFontPath
#undef XListExtensions
#undef XListProperties
#undef XCreateImage
#undef XPutImage
#undef XGetImage
#undef XGetSubImage
#undef XListHosts
#undef XNewModifiermap
#undef XGetModifierMapping
#undef XDeleteModifiermapEntry
#undef XInsertModifiermapEntry
#undef XKeycodeToKeysym
#undef XLookupKeysym
#undef XGetKeyboardMapping
#undef XStringToKeysym
#undef XInitExtension
#undef XESetCreateGC
#undef XESetCopyGC
#undef XESetFlushGC
#undef XESetFreeGC
#undef XESetCreateFont
#undef XESetFreeFont
#undef XESetCloseDisplay
#undef XESetError
#undef XESetErrorString
#undef XESetWireToEvent
#undef XESetEventToWire
#undef XRootWindow
#undef XDefaultRootWindow
#undef XRootWindowOfScreen
#undef XDefaultVisual
#undef XDefaultVisualOfScreen
#undef XDefaultGC
#undef XDefaultGCofScreen
#undef XBlackPixel
#undef XWhitePixel
#undef XAllPlanes
#undef XBlackPixelOfScreen
#undef XWhitePixelOfScreen
#undef XNextRequest
#undef XLastKnownRequestProcessed
#undef XServerVendor
#undef XDisplayString
#undef XDefaultColormap
#undef XDefaultColormapOfScreen
#undef XDisplayOfScreen
#undef XScreenOfDisplay
#undef XDefaultScreenOfDisplay
#undef XEventMaskOfScreen
#undef funcs

#undef class
#undef new
#undef delete
#undef inline
#undef virtual

/*
 * Xlib operations.
 */

Display* XOpenDisplay(const char*);
char* XDisplayName(const char*);
void XCloseDisplay(Display*);

char* XGetDefault(Display*, const char* prog, const char* param);
Atom XInternAtom(Display*, const char*, int);
XColormap* XListInstalledColormaps();
char* XGetAtomName(Display*, Atom);

void XSetScreenSaver(
    Display*, int timeout, int interval, int blanking, int exposures
);
void XForceScreenSaver(Display*, int mode);
void XActivateScreenSaver(Display*);
void XResetScreenSaver(Display*);
void XGetScreenSaver(
    Display*, int* timeout, int* interval, int* blanking, int* exposures
);

void XAddHost(Display*, XHostAddress*);
void XAddHosts(Display*, XHostAddress[], int);
XHostAddress* XListHosts(Display*, int* n, Bool*);
void XRemoveHost(Display*, XHostAddress*);
void XRemoveHosts(Display*, XHostAddress[], int);

Window XCreateWindow(
    Display*, Window parent, int x, int y,
    unsigned int width, unsigned int height,
    unsigned int bwidth, int depth,
    unsigned int window_class, Visual*, unsigned long, XSetWindowAttributes*
);
Window XCreateSimpleWindow(
    Display*, Window, int x, int y,
    unsigned int width, unsigned int height, unsigned int bwidth,
    unsigned long border, unsigned long background
);
void XDestroyWindow(Display*, Window);
void XDestroySubwindows(Display*, Window);
void XMapWindow(Display*, Window);
void XMapRaised(Display*, Window);
void XMapSubwindows(Display*, Window);
void XUnmapWindow(Display*, Window);
void XUnmapSubwindows(Display*, Window);
void XConfigureWindow(Display*, Window, unsigned int, XWindowChanges*);
void XMoveWindow(Display*, Window, int x, int y);
void XResizeWindow(
    Display*, Window, unsigned int width, unsigned int height
);
void XMoveResizeWindow(
    Display*, Window, int x, int y, unsigned int width, unsigned int height
);
void XSetWindowBorderWidth(Display*, Window, unsigned int width);
void XRaiseWindow(Display*, Window);
void XLowerWindow(Display*, Window);
void XCirculateSubwindows(Display*, Window, int direction);
void XCirculateSubwindowsUp(Display*, Window);
void XCirculateSubwindwosDown(Display*, Window);
void XRestackWindows(Display*, Window[], int nwindows);
void XChangeWindowAttributes(
    Display*, Window, unsigned int, XSetWindowAttributes*
);
void XSetWindowBackground(Display*, Window, unsigned long);
void XSetWindowBackgroundPixmap(Display*, Window, Pixmap);
void XSetWindowBorder(Display*, Window, unsigned long);
void XSetWindowBorderPixmap(Display*, Window, Pixmap);

XCursor XCreateFontCursor(Display*, int shape);
XCursor XCreatePixmapCursor(
    Display*, Pixmap src, Pixmap mask, XColor* fg, XColor* bg, int x, int y
);
XCursor XCreateGlyphCursor(
    Display*, XFont src, XFont mask, unsigned int schar, unsigned int mchar,
    XColor* fg, XColor* bg
);
void XRecolorCursor(Display*, XCursor, XColor* fg, XColor* bg);
void XFreeCursor(Display*, XCursor);
void XQueryBestCursor(
    Display*, Drawable, unsigned int width, unsigned int height,
    unsigned int* rwidth, unsigned int* rheight
);
void XDefineCursor(Display*, Window, XCursor);
void XUndefineCursor(Display*, Window);

Status XQueryTree(
    Display*, Window, Window* root, Window* parent,
    Window** children, int* nchildren
);
Status XGetWindowAttributes(Display*, Window, XWindowAttributes*);
Status XGetGeometry(
    Display*, Drawable, Drawable* root, int* x, int* y,
    unsigned int* width, unsigned int* height,
    unsigned int* bwidth, unsigned int* depth
);
int XTranslateCoordinates(
    Display*, Window src, Window dst, int srcx, int srcy,
    int* dstx, int* dsty, Window* child
);

int XGetWindowProperty(
    Display*, Window, Atom, long offset, long len, Bool del,
    Atom req_type, Atom* actual_type, int* actual_format, unsigned long* n,
    long* bytes_after, unsigned char** prop
);
Atom* XListProperties(Display*, Window, int* nprop);
void XChangeProperty(
    Display*, Window, Atom prop, Atom type, int format, int mode,
    unsigned char* data, int nelements
);
void XRotateWindowProperties(Display*, Window, Atom[], int n, int npos);
void XDeleteProperty(Display*, Window, Atom);

void XSetSelectionOwner(Display*, Atom, Window, Time);
Window XGetSelectionOwner(Display*, Atom);
void XConvertSelection(
    Display*, Atom selection, Atom target, Atom prop,
    Window requestor, Time
);

void XStoreName(Display*, Window, const char*);
Status XFetchName(Display*, Window, char**);
void XSetIconName(Display*, Window, const char*);
Status XGetIconName(Display*, Window, char**);
void XSetCommand(Display*, Window, char* argv[], int argc);

XColormap XCreateColormap(Display*, Window, Visual*, int alloc);
XColormap XCopyColormapAndFree(Display*, XColormap);
void XFreeColormap(Display*, XColormap);
void XSetWindowColormap(Display*, Window, XColormap);
Status XAllocColor(Display*, XColormap, XColor*);
Status XAllocNamedColor(Display*, XColormap, const char*, XColor*, XColor*);
Status XLookupColor(Display*, XColormap, const char*, XColor*, XColor*);
void XStoreColors(Display*, XColormap, XColor[], int);
void XStoreColor(Display*, XColormap, XColor*);
Status XAllocColorCells(
    Display*, XColormap, Bool, unsigned long plane[], unsigned long nplanes,
    unsigned long[], unsigned long npixels
);
Status XAllocColorPlanes(
    Display*, XColormap, Bool, unsigned long[], unsigned long npixels,
    int nreds, int ngreens, int nblues,
    unsigned long* rmask, unsigned long* gmask, unsigned long* bmask
);
void XStoreNamedColor(Display*, XColormap, const char*, unsigned long, int);
void XFreeColors(
    Display*, XColormap, unsigned long[], int, unsigned long nplanes
);
Status XQueryColor(Display*, XColormap, XColor*);
Status XQueryColors(Display*, XColormap, XColor[], int ncolors);

XFont XLoadFont(Display*, const char*);
XFontStruct* XQueryFont(Display*, XFont);
char** XListFontsWithInfo(
    Display*, char*, int maxnames, int count, XFontStruct**
);
XFontStruct* XLoadQueryFont(Display*, const char*);
void XFreeFont(Display*, XFontStruct*);
Bool XGetFontProperty(XFontStruct*, Atom, unsigned long*);
char** XListFonts(Display*, char*, int maxnames, int* count);
void XFreeFontNames(char**);

void XSetFontPath(Display*, char**, int);
char** XGetFontPath(Display*, int*);
void XFreeFontPath(char**);

int XTextWidth(XFontStruct*, const char*, int);
int XTextWidth16(XFontStruct*, const unsigned short*, int);
void XTextExtents(
    XFontStruct*, const char*, int,
    int* direction, int* ascent, int* descent, XCharStruct* overall
);
void XTextExtents16(
    XFontStruct*, const unsigned short*, int, int* direction,
    int* ascent, int* descent, XCharStruct* overall
);
void XQueryTextExtents(
    Display*, XFont, const char*, int,
    int* direction, int* ascent, int* descent, XCharStruct* overall
);
void XQueryTextExtents16(
    Display*, XFont, const XChar2b*, int,
    int* direction, int* ascent, int* descent, XCharStruct* overall
);

void XDrawText(Display*, Drawable, GC, int x, int y, XTextItem[], int);
void XDrawText16(Display*, Drawable, GC, int x, int y, XTextItem16[], int);
void XDrawString16(
    Display*, Drawable, GC, int x, int y, const XChar2b*, int
);
void XDrawImageString16(
    Display*, Drawable, GC, int x, int y, const XChar2b*, int
);

GC XCreateGC(Display*, Drawable, unsigned int, XGCValues*);
void XCopyGC(Display*, GC src, unsigned int, GC dst);
void XChangeGC(Display*, GC, unsigned int, XGCValues*);
void XFreeGC(Display*, GC);
void XSetState(
    Display*, GC, unsigned long fg, unsigned long bg, int func, unsigned long
);
void XSetFunction(Display*, GC, int);
void XSetPlaneMask(Display*, GC, unsigned long);
void XSetForeground(Display*, GC, unsigned long);
void XSetBackground(Display*, GC, unsigned long);
void XSetLineAttributes(
    Display*, GC, unsigned int width, int style, int cap, int join
);
void XSetDashes(Display*, GC, int offset, char* dash, int n);
void XSetFillStyle(Display*, GC, int);
void XSetFillRule(Display*, GC, int);
void XQueryBestSize(
    Display*, int, Drawable, unsigned int width, unsigned int height,
    unsigned int* rwidth, unsigned int* rheight
);
void XQueryBestTile(
    Display*, Drawable, unsigned int width, unsigned int height,
    unsigned int* rwidth, unsigned int* rheight
);
void XQueryBestStipple(
    Display*, Drawable, unsigned int width, unsigned int height,
    unsigned int* rwidth, unsigned int* rheight
);
void XSetTile(Display*, GC, Pixmap);
void XSetStipple(Display*, GC, Pixmap);
void XSetTSOrigin(Display*, GC, int x, int y);

void XSetFont(Display*, GC, XFont);
void XSetClipOrigin(Display*, GC, int x, int y);
void XSetClipMask(Display*, GC, Pixmap);
void XSetClipRectangles(
    Display*, GC, int x, int y, XRectangle[], int n, int ordering
);
void XSetArcMode(Display*, GC, int);
void XSetSubwindowMode(Display*, GC, int);
void XSetGraphicsExposures(Display*, GC, Bool);

Pixmap XCreatePixmap(
    Display*, Drawable,
    unsigned int width, unsigned int height, unsigned int depth
);
Pixmap XCreateBitmapFromData(
    Display*, Drawable, void*, unsigned int, unsigned int
);
void XFreePixmap(Display*, Pixmap);

void XClearArea(
    Display*, Window, int x, int y, unsigned int width, unsigned int height,
    Bool exposures
);
void XClearWindow(Display*, Window);
void XCopyArea(
    Display*, Drawable src, Drawable dst, GC, int srcx, int srcy,
    unsigned int width, unsigned int height, int dstx, int dsty
);
void XCopyPlane(
    Display*, Drawable src, Drawable dst, GC, int srcx, int srcy,
    unsigned int width, unsigned int height, int dstx, int dsty,
    unsigned long plane
);

void XDrawPoint(Display*, Drawable, GC, int x, int y);
void XDrawPoints(Display*, Drawable, GC, XPoint[], int, int mode);
void XDrawLine(Display*, Drawable, GC, int x1, int y1, int x2, int y2);
void XDrawLines(Display*, Drawable, GC, XPoint[], int n, int mode);
void XDrawSegments(Display*, Drawable, GC, XSegment[], int);
void XDrawRectangle(
    Display*, Drawable, GC, int x, int y,
    unsigned int width, unsigned int height
);
void XDrawRectangles(Display*, Drawable, GC, struct XRectangle[], int);
void XDrawArc(
    Display*, Drawable, GC, int x, int y,
    unsigned int width, unsigned int height,
    int angle1, int angle2
);
void XDrawArcs(Display*, Drawable, GC, XArc[], int);
void XFillRectangle(
    Display*, Drawable, GC, int x, int y,
    unsigned int width, unsigned int height
);
void XFillRectangles(Display*, Drawable, GC, XRectangle[], int);
void XFillPolygon(
    Display*, Drawable, GC, XPoint[], int, int shape, int mode
);
void XFillArc(
    Display*, Drawable, GC, int x, int y,
    unsigned int width, unsigned int height,
    int angle1, int angle2
);
void XFillArcs(Display*, Drawable, GC, XArc[], int);

void XDrawString(Display*, Drawable, GC, int x, int y, const char*, int);
void XDrawImageString(
    Display*, Drawable, GC, int x, int y, const char*, int
);

void XGrabButton(
    Display*, unsigned int, unsigned int, Window, Bool,
    unsigned int, int ptrmode, int kbdmode, Window confined, XCursor
);
void XUngrabButton(Display*, unsigned int, unsigned int, Window);
int XGrabPointer(
    Display*, Window, Bool, unsigned int, int ptrmode, int kbdmode,
    Window confined, XCursor, Time
);
void XUngrabPointer(Display*, Time);
void XChangeActivePointerGrab(Display*, unsigned int, XCursor, Time);
void XGrabKeyboard(Display*, Window, Bool, int ptrmode, int kbdmode, Time);
void XUngrabKeyboard(Display*, Time);
void XGrabKey(
    Display*, int, unsigned int, Window, Bool, int ptrmode, int kbdmode
);
void XUngrabKey(Display*, int key, unsigned int, Window);
void XAllowEvents(Display*, int, Time);

void XGrabServer(Display*);
void XUngrabServer(Display*);

void XWarpPointer(
    Display*, Window src, Window dst, int srcx, int srcy,
    unsigned int width, unsigned int height, int dstx, int dsty
);
void XSetInputFocus(Display*, Window, int revert, Time);
void XGetInputFocus(Display*, Window*, int* revert);

void XChangePointerControl(
    Display*, Bool has_acc, Bool has_thresh, int acc1, int acc2, int thresh
);
void XGetPointerControl(Display*, int* acc1, int* acc2, int* thresh);

void XChangeKeyboardControl(Display*, unsigned long, XKeyboardControl*);
void XGetKeyboardControl(Display*, XKeyboardState*);
void XAutoRepeatOn(Display*);
void XAutoRepeatOff(Display*);
void XBell(Display*, int percent);
int XSetPointerMapping(Display*, unsigned char[], int);
int XGetPointerMapping(Display*, unsigned char[], int);

void XQueryPointer(
    Display*, Window, Window* root, Window* child, int* x0, int* y0,
    int* wx, int* wy, unsigned int*
);

void XChangeKeyboardMapping(Display*, int first, int percode, KeySym[], int);
void XSetModifierMapping(Display*, XModifierKeymap*);
void XGetModifierMapping(Display*, XModifierKeymap*);

void XSelectInput(Display*, Window, long);
void XFlush(Display*);
void XSync(Display*, int);
int XPending(Display*);
void XNextEvent(Display*, XEvent*);
void XPeekEvent(Display*, XEvent*);

typedef Bool (*XPredicate)(Display*, XEvent*, char*);

void XIfEvent(Display*, XEvent*, XPredicate, char*);
int XCheckIfEvent(Display*, XEvent*, XPredicate, char*);
void XPeekIfEvent(Display*, XEvent*, XPredicate, char*);
void XPutBackEvent(Display*, XEvent*);
void XWindowEvent(Display*, Window, unsigned int, XEvent*);
int XCheckWindowEvent(Display*, Window, unsigned int, XEvent*);
void XMaskEvent(Display*, unsigned int, XEvent*);
int XCheckMaskEvent(Display*, unsigned int, XEvent*);
XTimeCoord* XGetMotionEvents(
    Display*, Window, Time start, Time stop, int* nevents
);

void XSendEvent(Display*, Window, Bool, unsigned int, XEvent*);

KeySym XLookupKeysym(XKeyEvent*, int);
void XRefereshKeyboardMapping(XMappingEvent*);
int XLookupString(XKeyEvent*, char*, int, KeySym*, struct _XComposeStatus*);
void XRebindKeySym(
    Display*, KeySym, KeySym[], int, unsigned char*, int nbytes
);

XImage* XCreateImage(
    Display*, Visual*, int depth, int format, int offset,
    void* data, int width, int height, int bitpad, int bytes_per_line
);

void XPutImage(
    Display*, Drawable, GC, XImage*, int srcx, int srcy,
    int dstx, int dsty, unsigned int width, unsigned int height
);

XImage* XGetImage(
    Display*, Drawable, int x, int y,
    unsigned int width, unsigned int height,
    unsigned long, int format
);

XImage* XSubGetImage(
    Display*, Drawable, int x, int y,
    unsigned int width, unsigned int height,
    unsigned long, int format,
    XImage* dest, int dstx, int dsty
);

#endif
