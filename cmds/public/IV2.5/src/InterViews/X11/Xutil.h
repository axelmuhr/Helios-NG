#ifndef Xutil_h
#define Xutil_h

#include <InterViews/X11/Xlib.h>

#define class cc_class
#define new cc_new
#define delete cc_delete
#define virtual cc_virtual

#define XSaveContext cc_XSaveContext
#define XFindContext cc_XFindContext
#define XDeleteContext cc_XDeleteContext
#define XGetWMHints cc_XGetWMHints
#define XCreateRegion cc_XCreateRegion
#define XPolygonRegion cc_XPolygonRegion
#define XGetVisualInfo cc_XGetVisualInfo
#define XCreateImage cc_XCreateImage
#define XReadBitmapFile cc_XReadBitmapFile

#define Colormap XColormap

#include <X11/Xutil.h>

#undef Colormap

#undef XSaveContext
#undef XFindContext
#undef XDeleteContext
#undef XGetWMHints
#undef XCreateRegion
#undef XPolygonRegion
#undef XGetVisualInfo
#undef XCreateImage
#undef XReadBitmapFile

#undef class
#undef new
#undef delete
#undef virtual

int XSaveContext(Window, XContext, const void*);
int XFindContext(Display*, Window, XContext, void**);
int XDeleteContext(Window, XContext);

unsigned XParseGeometry(const char*, Coord*, Coord*, int*, int*);

void XSetClassHint(Display*, Window, XClassHint*);
Status XGetClassHint(Display*, Window, XClassHint*);
void XSetNormalHints(Display*, Window, XSizeHints*);
Status XGetNormalHints(Display*, Window, XSizeHints*);
void XSetSizeHints(Display*, Window, XSizeHints*, Atom);
Status XGetSizeHints(Display*, Window, XSizeHints*, Atom);
void XSetStandardProperties(
    Display*, Window, const char*, const char*, Pixmap,
    char*[], int, XSizeHints*
);
void XSetTransientForHint(Display*, Window, Window);
Status XGetTransientForHint(Display*, Window, Window*);
void XSetWMHints(Display*, Window, XWMHints*);
XWMHints* XGetWMHints(Display*, Window);

Region XCreateRegion(), XPolygonRegion();

#undef XDestroyImage
#undef XGetPixel
#undef XPutPixel
#undef XSubImage
#undef XAddPixel

inline int XDestroyImage (XImage* i) {
    typedef int (*PF)(XImage*);
    return (*((PF)i->f.destroy_image))(i);
}

inline unsigned long XGetPixel (XImage* i, int x, int y) {
    typedef unsigned long (*PF)(XImage*, int, int);
    return (*((PF)i->f.get_pixel))(i, x, y);
}

inline int XPutPixel (XImage* i, int x, int y, unsigned long pixel) {
    typedef int (*PF)(XImage*, int, int, unsigned long);
    return (*((PF)i->f.put_pixel))(i, x, y, pixel);
}

inline XImage* XSubImage (
    XImage* i, int x, int y, unsigned int width, unsigned int height
) {
    typedef XImage* (*PF)(XImage*, int, int, unsigned int, unsigned int);
    return (*((PF)i->f.sub_image))(i, x, y, width, height);
}

inline int XAddPixel (XImage* i, long value) {
    typedef int (*PF)(XImage*, long);
    return (*((PF)i->f.add_pixel))(i, value);
}

Status XReadBitmapFile(
    Display*, Drawable, const char*, int*, int*, Pixmap*,
    int* x_hot, int* y_hot
);

typedef void XHandler(Display*, XErrorEvent*);
void XSetErrorHandler(XHandler*);
void XGetErrorText(Display*, int, char*, int);

#endif
