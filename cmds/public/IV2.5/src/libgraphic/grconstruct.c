/*
 * Implementation of GraphicConstruct object construction function.
 */

#include <InterViews/Graphic/base.h>
#include <InterViews/Graphic/ellipses.h>
#include <InterViews/Graphic/instance.h>
#include <InterViews/Graphic/grclasses.h>
#include <InterViews/Graphic/grconstruct.h>
#include <InterViews/Graphic/label.h>
#include <InterViews/Graphic/lines.h>
#include <InterViews/Graphic/picture.h>
#include <InterViews/Graphic/polygons.h>
#include <InterViews/Graphic/rasterrect.h>
#include <InterViews/Graphic/splines.h>
#include <InterViews/Graphic/stencil.h>

Persistent* GraphicConstruct (ClassId id) {
    switch (id) {
	case BOXOBJ:		return new BoxObj;
	case BSPLINE:		return new BSpline;
	case CIRCLE:		return new Circle;
	case CLOSEDBSPLINE:	return new ClosedBSpline;
	case ELLIPSE:		return new Ellipse;
	case FILLBSPLINE:	return new FillBSpline;
	case FILLCIRCLE:	return new FillCircle;
	case FILLELLIPSE:	return new FillEllipse;
	case FILLPOLYGON:	return new FillPolygon;
	case FILLPOLYGONOBJ:	return new FillPolygonObj;
	case FILLRECT:		return new FillRect;
	case FULL_GRAPHIC:	return new FullGraphic;
	case GRAPHIC:		return new Graphic;
	case INSTANCE:		return new Instance;
	case LABEL:		return new Label;
	case LINE:		return new Line;
	case LINEOBJ:		return new LineObj;
	case MULTILINE:		return new MultiLine;
	case MULTILINEOBJ:	return new MultiLineObj;
	case PBRUSH:		return new PBrush;
	case PCOLOR:		return new PColor;
	case PFONT:		return new PFont;
	case PICTURE:		return new Picture;
	case POINT:		return new Point;
	case POINTOBJ:		return new PointObj;
	case POLYGON:		return new Polygon;
	case PPATTERN:		return new PPattern;
        case RASTERRECT:        return new RasterRect;
	case RECT:		return new Rect;
        case STENCIL:           return new Stencil;

	default:		return nil;
    }
}
