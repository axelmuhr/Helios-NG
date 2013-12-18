/*
 * layout data and initializations for dclock
 */

#include "dclock.h"
#include "data.h"
#include "segment.h"

int Width = 175;
int Height = 30;
int XPos = 815;
int YPos = 65;

TMode TimeMode = CIVIL;		// initial (default) values for options
CMode CreateMode = SWEPT;
boolean JohnsFlag = false;
boolean Invert = false;
boolean ShowDate = true;
boolean ShowBorder = false;
boolean ShowTime = true;

int SlantPC = 30;
int ThickPC = 15;
int FadeRate = 4;
int FadeStep = 1;

boolean SegCode[11][7] = {	// segments to form various digits
    {1,1,1,1,1,1,0},		// digit 0
    {0,1,1,0,0,0,0},
    {1,1,0,1,1,0,1},
    {1,1,1,1,0,0,1},
    {0,1,1,0,0,1,1},
    {1,0,1,1,0,1,1},
    {1,0,1,1,1,1,1},
    {1,1,1,0,0,0,0},
    {1,1,1,1,1,1,1},
    {1,1,1,0,0,1,1},		// digit 9
    {0,0,0,0,0,0,0}		// blank digit
};

float HTx, HUx, MTx, MUx;	// normalized digit x offsets
float COLx;			// normalized colon x offset
float ALLy;			// normalized digit y offsets
float DigitWidth;		// normalized digit width with slant
float DigitHeight;		// normalized digit height

SegLayout SegData[7];		// normalized segments, slanted, unshifted
SegLayout ColonData[2];		// normalized colon, slanted, shifted

CharLayout AData = {		// character A
    11,				//count
    {0.0,0.1,0.15,0.25,0.2,0.18,0.07,0.08,0.17,0.125,0.05},	// x
    {0.0,0.3,0.3,0.0,0.0,0.05,0.05,0.1,0.1,0.22,0.0}		// y
};
CharLayout PData = {		// character P
    12,
    {0.0,0.0,0.2,0.25,0.25,0.2,0.05,0.05,0.2,0.2,0.05,0.05},
    {0.0,0.3,0.3,0.25,0.15,0.1,0.1,0.15,0.15,0.25,0.25,0.0}
};
CharLayout MData = {		// character M
    12,
    {0.0,0.0,0.05,0.2,0.35,0.4,0.4,0.35,0.35,0.2,0.05,0.05},
    {0.0,0.3,0.3,0.1,0.3,0.3,0.0,0.0,0.2,0.0,0.2,0.0}
};

// these are variables so that a meta-view can potentially change them

float BorderWidth = 0.0;	// digit relative border width
float LMargin = 0.3;		// ... left margin
float RMargin = 0.3;		// ... right margin
float TMargin = 0.1;		// ... top margin
float BMargin = 0.1;		// ... bottom margin
float DigitGap = 0.2;		// ... interdigit spacing
float SepGap = 1.0;		// ... separator spacing
float VThick = 0.2;		// ... thickness of vertical segments
float HThick = 0.15;		// ... thickness of horizontal segments
float Slant = 0.3;		// ... rightwards slant of digit top

float width, height;    	// total dimensions, digit relative

void MakeSeg( Seg s, SegLayout& base, float Xoff, float Yoff ) {
    for ( int i=0; i<base.count; i++ ) {
	SegData[s].y[i] = base.y[i]+Yoff;
	SegData[s].x[i] = base.x[i]+Xoff+SegData[s].y[i]*Slant/width;
    }
    SegData[s].count = base.count;
}

void InitData() {
    FadeStep = FadeRate==0 ? 16 : 1;
    SegCode[9][SegD] = (JohnsFlag)? true : false;
    Slant = min( max( 0,SlantPC ), 100 )/100.0;
    HThick = min( max( 5,ThickPC ), 25 )/100.0;
    VThick = min( max( 5,ThickPC ), 25 )/100.0 * 3.0/2.0;

    Width = Coord( min( max( 100,Width ), 1024));
    Height = Coord( min( max( 25,Height ), 865));
    YPos = YPos - Height + 1;// ypos is the TOP of clock; we need the bottom

    width = 2*BorderWidth+LMargin+4.0+2*DigitGap+SepGap+RMargin+Slant;
    height = 2*BorderWidth+BMargin+1.0+TMargin;

    HTx = (BorderWidth + LMargin)/width;	// hours tens digit x offset
    HUx = HTx + (1.0 + DigitGap)/width;		// hours units ...
    MTx = HUx + (1.0 + SepGap)/width;		// minutes tens ...
    MUx = MTx + (1.0 + DigitGap)/width;		// minutes units ...
    COLx = HUx + (1.0 + SepGap/2.0)/width;	// colon ...
    ALLy = (BorderWidth + BMargin)/height;
    DigitWidth = (1.0 + Slant)/width;
    DigitHeight = 1.0/height;

    SegLayout Colon;				// generic normalized colon dot

    Colon.count = 6;
    Colon.x[0] = 0.0;
    Colon.x[1] = ( - VThick/2.0)/width;
    Colon.x[2] = ( - VThick/2.0)/width;
    Colon.x[3] = 0.0;
    Colon.x[4] = VThick/2.0/width;
    Colon.x[5] = VThick/2.0/width;
    Colon.y[0] = 0.0;
    Colon.y[1] = 0.5*HThick/height;
    Colon.y[2] = 1.0*HThick/height;
    Colon.y[3] = 1.5*HThick/height;
    Colon.y[4] = 1.0*HThick/height;
    Colon.y[5] = 0.5*HThick/height;

    ColonData[0].count = Colon.count;
    ColonData[1].count = Colon.count;
    for ( int i=0; i< Colon.count; i++ ) {
	ColonData[0].y[i] = Colon.y[i]+(0.5-2.25*HThick)/height+ALLy;
	ColonData[0].x[i] = Colon.x[i]+COLx+ColonData[0].y[i]*Slant/width;
	ColonData[1].y[i] = Colon.y[i]+(0.5+0.25*HThick)/height+ALLy;
	ColonData[1].x[i] = Colon.x[i]+COLx+ColonData[1].y[i]*Slant/width;
    }

    for ( i=0; i<12; i++ ) {			// shift, scale, slant A, P, M
	AData.y[i] = (AData.y[i]+0.7)/height + ALLy;
	AData.x[i] = (AData.x[i])/width+HTx+AData.y[i]*Slant/width-0.15/width;
	PData.y[i] = (PData.y[i]+0.7)/height + ALLy;
	PData.x[i] = (PData.x[i])/width+HTx+PData.y[i]*Slant/width-0.15/width;
	MData.y[i] = (MData.y[i]+0.7)/height + ALLy;
	MData.x[i] = (MData.x[i]+0.3)/width+HTx+MData.y[i]*Slant/width-0.15/width;
    }

    SegLayout HSeg, VSeg;	// generic normalized horiz and vert segments

    HSeg.count = 6;
    HSeg.x[0] = VThick/2.0/width;
    HSeg.x[1] = VThick/width;
    HSeg.x[2] = (1.0 - VThick)/width;
    HSeg.x[3] = (1.0 - VThick/2.0)/width;
    HSeg.x[4] = (1.0 - VThick)/width;
    HSeg.x[5] = VThick/width;
    HSeg.y[0] = HThick/2.0/height;
    HSeg.y[1] = HThick/height;
    HSeg.y[2] = HThick/height;
    HSeg.y[3] = HThick/2.0/height;
    HSeg.y[4] = 0.0;
    HSeg.y[5] = 0.0;

    VSeg.count = 6;
    VSeg.x[0] = VThick/2.0/width;
    VSeg.x[1] = 0.0;
    VSeg.x[2] = 0.0;
    VSeg.x[3] = VThick/2.0/width;
    VSeg.x[4] = VThick/width;
    VSeg.x[5] = VThick/width;
    VSeg.y[0] = HThick/2.0/height;
    VSeg.y[1] = HThick/height;
    VSeg.y[2] = (0.5 - HThick/2.0)/height;
    VSeg.y[3] = 0.5/height;
    VSeg.y[4] = (0.5 - HThick/2.0)/height;
    VSeg.y[5] = HThick/height;

    MakeSeg( SegA, HSeg, 0.0, (1.0-HThick)/height );
    MakeSeg( SegB, VSeg, (1.0-VThick)/width, (0.5-HThick/2.0)/height );
    MakeSeg( SegC, VSeg, (1.0-VThick)/width, 0.0 );
    MakeSeg( SegD, HSeg, 0.0, 0.0 );
    MakeSeg( SegE, VSeg, 0.0, 0.0 );
    MakeSeg( SegF, VSeg, 0.0, (0.5-HThick/2.0)/height );
    MakeSeg( SegG, HSeg, 0.0, (0.5-HThick/2.0)/height );
}
