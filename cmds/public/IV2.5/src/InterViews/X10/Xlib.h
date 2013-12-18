/*
 * C++ interface to standard Xlib.h.
 *
 * We must disable C++ keywords that might be used as identifiers
 * in Xlib.h, and we also must disable function identifiers so we
 * can define their parameter types later.
 */

#ifndef Xlib_h
#define Xlib_h

#define class CC_class
#define new CC_new
#define delete CC_delete
#define inline CC_inline
#define virtual CC_virtual

#define XOpenDisplay cc_XOpenDisplay
#define XFetchBytes cc_XFetchBytes
#define XFetchBuffer cc_XFetchBuffer
#define XErrDescrip cc_XErrDescrip
#define XLookupMapping cc_XLookupMapping
#define XFontWidths cc_XFontWidths
#define XOpenFont cc_XOpenFont
#define XGetDefault cc_XGetDefault
#define XCharBitmap cc_XCharBitmap
#define XStoreBitmap cc_XStoreBitmap
#define XMakePixmap cc_XMakePixmap
#define XMakeTile cc_XMakeTile
#define XStorePixmapXY cc_XStorePixmapXY
#define XStorePixmapZ cc_XStorePixmapZ
#define XPixmapSave cc_XPixmapSave
#define XCreateCursor cc_XCreateCursor
#define XStoreCursor cc_XStoreCursor
#define XCreate cc_XCreate
#define XCreateTerm cc_XCreateTerm
#define XCreateTransparency cc_XCreateTransparency
#define XCreateWindow cc_XCreateWindow
#define XGetIconWindow cc_XGetIconWindow
#define XGetFont cc_XGetFont
#define XFetchName cc_XFetchName
#define XGetColorCells cc_XGetColorCells
#define XGetColor cc_XGetColor
#define XGetHardwareColor cc_XGetHardwareColor
#define XGetResizeHint cc_XGetResizeHint
#define XGrabButton cc_XGrabButton
#define XGrabMouse cc_XGrabMouse
#define XInterpretLocator cc_XInterpretLocator
#define XParseColor cc_XParseColor
#define XPixmapGetXY cc_XPixmapGetXY
#define XPixmapGetZ cc_XPixmapGetZ
#define XQueryMouseButtons cc_XQueryMouseButtons
#define XQueryFont cc_XQueryFont
#define XQueryMouse cc_XQueryMouse
#define XQueryTree cc_XQueryTree
#define XQueryWindow cc_XQueryWindow
#define XReadBitmapFile cc_XReadBitmapFile
#define XUpdateMouse cc_XUpdateMouse
#define XCreateAssocTable cc_XCreateAssocTable

#include <X/Xlib.h>

#undef XOpenDisplay
#undef XFetchBytes
#undef XFetchBuffer
#undef XErrDescrip
#undef XLookupMapping
#undef XFontWidths
#undef XOpenFont
#undef XGetDefault
#undef XCharBitmap
#undef XStoreBitmap
#undef XMakePixmap
#undef XMakeTile
#undef XStorePixmapXY
#undef XStorePixmapZ
#undef XPixmapSave
#undef XCreateCursor
#undef XStoreCursor
#undef XCreate
#undef XCreateTerm
#undef XCreateTransparency
#undef XCreateWindow
#undef XGetIconWindow
#undef XGetFont
#undef XFetchName
#undef XGetColorCells
#undef XGetColor
#undef XGetHardwareColor
#undef XGetResizeHint
#undef XGrabButton
#undef XGrabMouse
#undef XInterpretLocator
#undef XParseColor
#undef XPixmapGetXY
#undef XPixmapGetZ
#undef XQueryMouseButtons
#undef XQueryFont
#undef XQueryMouse
#undef XQueryTree
#undef XQueryWindow
#undef XReadBitmapFile
#undef XUpdateMouse
#undef XCreateAssocTable

#undef class
#undef new
#undef delete
#undef inline
#undef virtual

#endif
