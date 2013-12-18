/*
 * various data for dclock
 */

#ifndef data_h
#define data_h

static const int QKEY = 193;

enum TMode { CIVIL, MIL };
enum CMode { SWEPT, PLACED, DEFINED };

extern int Width;		// default width and height of view
extern int Height;
extern int XPos;		// default position of view
extern int YPos;

extern TMode TimeMode;		// CIVIL=12 hour, MIL=24 hour
extern CMode CreateMode;
extern boolean JohnsFlag;	// tail on 9
extern boolean ShowDate;	// date at top
extern boolean ShowBorder;	// visibility of border between date and time
extern boolean ShowTime;	// visibility of time portion
extern int SlantPC;		// 100% = "45 degrees"
extern int ThickPC;		// segment thickness in % of digit size
extern int FadeRate;		// 0 = flash .. 4 = slow fade
extern int FadeStep;		// number of patterns to step when fading

extern boolean SegCode[11][7];  // digit "10" is blank

extern float HTx;		// digit horizontal positions
extern float HUx;
extern float MTx;
extern float MUx;
extern float ALLy;		// digit vertical positions
extern float DigitWidth;
extern float DigitHeight;

struct SegLayout {
    int count;
    float x[6];
    float y[6];
};

struct CharLayout {
    int count;
    float x[12];
    float y[12];
};

struct SegPoints {
    int count;
    Coord x[6];
    Coord y[6];
};

extern SegLayout SegData[7];    	// normalized segment positions
extern SegLayout ColonData[2];    	// normalized colon positions
extern CharLayout AData;		// character position data
extern CharLayout PData;
extern CharLayout MData;

extern void InitData();

#endif
