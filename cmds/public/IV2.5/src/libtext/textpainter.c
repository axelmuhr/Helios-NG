/*
 * TextPainter - styled text
 */

#include <InterViews/Text/textpainter.h>
#include <InterViews/paint.h>

TextPainter::TextPainter( Painter * p ) : ( p ) {
    bold = underlined = boxed = inverted = transparent = greyed = false;
}

void TextPainter::Transparent() {
    if ( ! transparent ) {
	transparent = true;
	FillBg( false );
    }
}

void TextPainter::NotTransparent() {
    if ( transparent ) {
	transparent = false;
	FillBg( true );
    }
}

void TextPainter::Inverted() {
    if ( ! inverted ) {
	inverted = true;
	SetColors( GetBgColor(), GetFgColor() );
    }
}

void TextPainter::NotInverted() {
    if ( inverted ) {
	inverted = false;
	SetColors( GetBgColor(), GetFgColor() );
    }
}

void TextPainter::Text( Canvas * c, const char * s, int l ) {
    Coord startx;
    Coord endx;
    Coord y;
    if ( bold || underlined || boxed || greyed ) {
	GetPosition( startx, y );
    }
    Painter::Text( c, s, l );
    if ( bold || underlined || boxed || greyed ) {
	GetPosition( endx, y );
    }
    if ( bold ) {
	MoveTo( startx-1, y );
	if ( ! transparent ) {
	    FillBg( false );
	    Painter::Text( c, s, l );
	    FillBg( true );
	} else {
	    Painter::Text( c, s, l );
	}
	MoveTo( endx, y );
    }
    if ( underlined ) {
	Line( c, startx, y, endx-1, y );
    }
    if ( boxed ) {
	Rect( c, startx, y, endx-1, y+GetFont()->Height()-1 );
    }
    if ( greyed ) {	// deliberately SLOW!!!
	SetColors( GetBgColor(), GetFgColor() );
	if ( ! transparent ) {
	    SetPattern( new Pattern( 0xa5a5 ) );
	    FillBg( false );
	    FillRect( c, startx, y, endx-1, y+GetFont()->Height()-1 );
	    FillBg( true );
	    SetPattern( solid );
	} else {
	    FillRect( c, startx, y, endx-1, y+GetFont()->Height()-1 );
	}
	SetColors( GetBgColor(), GetFgColor() );
    }
}
