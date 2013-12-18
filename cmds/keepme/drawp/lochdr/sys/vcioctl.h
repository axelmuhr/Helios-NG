/*
 * $Header: vcioctl.h,v 1.1 90/01/13 20:12:56 charles Locked $
 * $Source: /server/usr/users/charles/world/drawp/RCS/lochdr/sys/vcioctl.h,v $
 *
 * Copyright (c) 1988 Acorn Computers Ltd., Cambridge, England
 *
 * $Desc$
 * $Log:	vcioctl.h,v $
 * Revision 1.1  90/01/13  20:12:56  charles
 * Initial revision
 * 
 * Revision 1.8  89/12/15  16:34:57  pete
 * File unchanged
 * 
 * Revision 1.7  89/11/03  15:00:29  charles
 * File unchanged
 * 
 * Revision 1.6  89/08/15  13:43:22  charles
 * File unchanged
 * 
 * Revision 1.5  89/08/11  17:47:27  charles
 * File unchanged
 * 
 * Revision 1.4  89/08/10  19:53:40  charles
 * File unchanged
 * 
 * Revision 1.3  89/08/10  19:41:56  charles
 * File unchanged
 * 
 * Revision 1.2  89/08/10  19:01:09  charles
 * File unchanged
 * 
 * Revision 1.1  89/08/10  16:24:21  charles
 * Initial revision
 * 
 * Revision 1.3  89/08/10  15:47:00  charles
 * "just_to_release_lock"
 * 
 * Revision 1.2  89/07/10  18:18:15  charles
 * Changed so can co-exist with ANSI files
 * 
 * Revision 1.1  89/07/10  15:26:08  charles
 * Initial revision
 * 
 * Revision 1.8  88/10/22  11:02:01  mark
 * Corrected VCIOCGPARAM from _IOR() to _IOWR().
 * Corrected VC_SCREEN_MODE definition.
 * Added colour support (ioctls VCIOC{S,G}TEXTCOLOURS).
 * 
 * Revision 1.7  88/08/30  17:05:12  mark
 * Corrected definition of VCIOCGPALETTE; was an _IOR(..), but needs to
 * be an _IOWR(..) operation.
 * 
 * Revision 1.6  88/08/22  12:18:02  mark
 * Added new calls VCIOCEVENTCOUNT, VCIOC{G,S}VDUTYPE.
 * Extended description in comment about VCIOC{G,S}PALETTE.
 * 
 * Revision 1.5  88/08/08  14:58:57  mark
 * Added definitions for VCIOCTONE ioctl.
 * Removed out-of-date screen mode codes for VCIOC{G,S}PARAM call.
 * 
 * Revision 1.4  88/08/04  11:36:34  mark
 * Fixed hash-up of spelling of "palette";  old wrong spelling ("pallete")
 * can still be used, however...
 * 
 * Revision 1.3  88/06/17  20:09:28  beta
 * Acorn Unix initial beta version
 * 
 */
/* vcioctl.h
 *
 * Definitions for IOCTL interface to Video Console driver
 *
 * Needs ioctl.h, fbioctl.h.
 */
#ifndef _VCIOCTL_
#define _VCIOCTL_

/*
 * Console/terminal ioctls: (V, 0..19, ...)
 */


/* Structure for getting/setting setup-parameters of VT220-style console */

typedef struct vciocparam
{
    unsigned char param_no;
    unsigned char param_val;
} VCIOCParam, *VCIOCParamRef;

#define VCIOCGPARAM	_IOWR('V', 0, VCIOCParam)
#define VCIOCSPARAM	_IOW('V', 1, VCIOCParam)

/* Values of param_no and param_val in VCIOCParam */

#define VC_SCREEN_MODE	1			/* param no */

/* Lots more to go in here... */

/*
 * Text colour control.
 *
 * This is applicable to any of the virtual terminals.  The 
 * current settings for a terminal may be read (they are all
 * initially grey-scales) and set by the respective commands
 * No check is performed on the sensibility of the colour 
 * relationships (e.g. having all colours the same is 
 * possible!); however the colour values used must be (12+1)-bit,
 * i.e. 4x(R,G,B)+supremacy, as defined below for colour f/b 
 * palette-control ioctls, and any value containing a 1 in a 
 * bit position greater than bit 12 will cause EINVAL to be
 * returned.  Note that two sets of colours are defined: those
 * for "normal" (i.e. "white on black"), and "inverse" (i.e. 
 * "black on white") modes, as defined in the VT200 emulator.
 */

#define VC_COLOUR_BITS		(3*4)
#define VC_COLOUR_MASK		((1 << VC_COLOUR_BITS) - 1)
#define VC_SUPREMACY		(1 << 12)
#define VC_COLOUR_VMASK		(VC_SUPREMACY | VC_COLOUR_MASK)

#define VC_TCS__ENTRIES	10
typedef struct _vctextcolourset_
{
    unsigned int tcs_entry[VC_TCS__ENTRIES];
} VCTextColourSet, *VCTextColourSetRef;
#define VC_TCS_BORDER	0			/* round the edge */
#define tcs_border	tcs_entry[VC_TCS_BORDER]	
#define VC_TCS_DISPLAY	1			/* background colour */
#define tcs_display	tcs_entry[VC_TCS_DISPLAY] 
#define VC_TCS_CURSOR	2			/* normal cursor colour */
#define tcs_cursor	tcs_entry[VC_TCS_CURSOR] 
#define VC_TCS_ICURSOR	3			/* cursor over inverse char */
#define tcs_icursor	tcs_entry[VC_TCS_ICURSOR] 
#define VC_TCS_NORMAL1	4			/* normal char colour 1 */
#define tcs_normal1	tcs_entry[VC_TCS_NORMAL1]	
#define VC_TCS_BOLD1	5			/* bold attribute chars 1 */
#define tcs_bold1	tcs_entry[VC_TCS_BOLD1]		
#define VC_TCS_NORMAL2	6			/* normal char colour 2 */
#define tcs_normal2	tcs_entry[VC_TCS_NORMAL2]	
#define VC_TCS_BOLD2	7			/* bold attribute chars 2 */
#define tcs_bold2	tcs_entry[VC_TCS_BOLD2]		
#define VC_TCS_NORMAL3	8			/* normal char colour 3 */
#define tcs_normal3	tcs_entry[VC_TCS_NORMAL3]	
#define VC_TCS_BOLD3	9			/* bold attribute chars 3 */
#define tcs_bold3	tcs_entry[VC_TCS_BOLD3]		


typedef struct _vctextcolours_
{
    VCTextColourSet tc_tcs[2];
} VCTextColours, *VCTextColoursRef;
#define VC_TC_W_ON_B	0
#define tc_w_on_b	tc_tcs[VC_TC_W_ON_B]
#define VC_TC_B_ON_W	1
#define tc_b_on_w	tc_tcs[VC_TC_B_ON_W]

#define VCIOCGTEXTCOLOURS	_IOR('V', 4, VCTextColours)
#define VCIOCSTEXTCOLOURS	_IOW('V', 4, VCTextColours)


/*
 * Event-device (kbd, mouse) ioctls: (V, 20..39, ...)
 */

#define VCIOCMOUSELINK	_IOW('V', 20, int)	/* set/reset keyboard-mouse event link */

typedef struct _vciocrepeat_
{
    unsigned char start_gap_csec;		/* 0 -> no auto-repeat */
    unsigned char repeat_gap_csec;
} VCIOCRepeat, *VCIOCRepeatRef;

#define VCIOCGAUTORPT	_IOR('V', 21, VCIOCRepeat) /* get auto-repeat parameters */
#define VCIOCSAUTORPT	_IOW('V', 22, VCIOCRepeat) /* set auto-repeat parameters */
#define VCIOCKBDTYPE	_IOR('V', 23, int)	/* read keyboard type */

#define VC_KBD_TYPE_UNKNOWN	(-1)		/* initial value(!?) */
#define VC_KBD_TYPE_A500	0		/* A500 */
#define VC_KBD_TYPE_AXXX_UK	1		/* A3xx/4xx/6xx UK */
#define VC_KBD_TYPE__COUNT	2

typedef struct _vcioceventcount_
{
    unsigned int kbd_events;
    unsigned int mouse_events;
} VCIOCEventCount, *VCIOCEventCountRef;

/*
 * Get address of user-readable struct holding event count.
 * This call can be used on the kbd and mouse devices.
 */
#define VCIOCEVENTCOUNT	_IOR('V', 24, VCIOCEventCountRef)

/*
 * Frame Buffer (screen) device ioctls: (V, 40..59, ...)
 */

/*
 * Palette description structure: defines up to 20 entries in an
 * arbitary sized word-wide device palette.  This is for use with the 
 * frame buffer device.  The "start" field of the VCIOCPalette struct
 * defines which entry of the real device palette corresponds to 
 * "value[0]" in the struct.  The "count" field specifies how many
 * contiguous real palette entries (up to a maximum of 20) are to 
 * be written to (VCIOCSPALETTE) or read from (VCIOCGPALETTE).
 * With the current fb devices, entries are specified according to
 * attached monitor type, as indicated below.
 *
 * Low and medium resolution monitors
 *   The entries are indexed as:
 *	4 bits/pixel modes:
 *         0-15: main palette
 *         16:   border colour
 *      1 bit/pixel modes:
 *         0-1:  main palette
 *         2:    border colour
 *   Each entry is constructed as 13 bits:
 *	bits 0..3  red level (0-15)
 *           4..7  green level (0-15)
 *           8..11 blue level (0-15)
 *      bit 12     supremacy bit (for external video mixing)
 *   Current hardware does not make use of the supremacy bit, which
 *   will normally be set to 0.
 *
 * High resolution monitors
 *   The entries are indexed as:
 *     0: background colour
 *     1: foreground colour
 *     2: border pattern
 *
 *   Each colour entry is constructed as for the low/medium resolution
 *   monitor case, however there are only 2 displayable "colours",
 *   black and white.  The sum of the three nominal colour levels
 *   is used to decide which displayable colour will be used: if
 *   the sum is greater than (15*3) div 2, i.e. > 22, white will be
 *   used, otherwise black.  The most obvious scheme is to use the
 *   value 0x000 for black, 0xFFF for white.  For the border pattern
 *   entry, the least significant 4 bits of the entry are replicated
 *   horizontally throughout the border area, giving variously solid
 *   black, vertical stripe patterns or solid white, according to the
 *   value: 0 gives black, 15 white, interim values stripes.
 */

typedef struct _vciocpalette_
{
    unsigned int  start;			/* initial entry # */
    unsigned int  count;			/* how many to do */
#define VCIOCPALETTE_MAX	20
    unsigned int  value[VCIOCPALETTE_MAX];	/* entry settings */
} VCIOCPalette, *VCIOCPaletteRef;

#define VCIOCGPALETTE	_IOWR('V', 20, VCIOCPalette) /* get palette entries */
#define VCIOCSPALETTE	_IOW('V', 21, VCIOCPalette) /* set palette entries */

/* 
 * The following junk is a historical bungle: "palette" used to be
 * spelled "pallete" (!!!)   PLEASE USE THE RIGHT SPELLING!
 */
typedef struct _vciocpallete_
{
    unsigned int  start;			/* initial entry # */
    unsigned int  count;			/* how many to do */
#define VCIOCPALLETE_MAX	20
    unsigned int  value[VCIOCPALLETE_MAX];	/* entry settings */
} VCIOCPallete, *VCIOCPalleteRef;

#define VCIOCGPALLETE	_IOWR('V', 20, VCIOCPallete) /* get pallete entries */
#define VCIOCSPALLETE	_IOW('V', 21, VCIOCPallete) /* set pallete entries */


/* ioctls (V, 60..79, ...) */

/* Generic ioctls: (V, 80..99, ...) */


/*
 * Make a simple tone (at a fixed pitch, around 473Hz): four amplitude
 * levels (faint,quiet,medium,loud) are possible; the length is specified
 * in msec (but is resolved in units of about 8ms).
 */
typedef struct _vcioctone_
{
    unsigned int amplitude;			/* 0..3 */
    unsigned int length;			/* 0..9999 msec */
} VCIOCTone, *VCIOCToneRef;

#define VCIOCTONE	_IOW('V', 80, VCIOCTone)

/*
 * Setting of VDU type:  the call
 *   int vdutype;
 *   ioctl (fd, VCIOCGVDUTYPE, &vdutype);
 * returns the current vdu type code.  The call:
 *   ioctl (fd, VCIOCSVDUTYPE, &vdutype); 
 * when used with superuser privilege will set the CMOS RAM flag for
 * vdu type and reconfigure the VC driver accordingly.
 */
#define VCIOCGVDUTYPE	_IOR('V', 82, int)
#define VCIOCSVDUTYPE	_IOW('V', 83, int)
#ifndef KERNEL
#define  VDU_TYPE_LOW_RES	0		/* low res only */
#define  VDU_TYPE_MULTISCAN	1		/* low/medium res */
#define  VDU_TYPE_HIGH_RES	2		/* high res only */
#define  VDU_TYPE_VGA		3		/* medium res only */
#endif

#endif /* NORCROFT ANSI 'C' objects to identifier '_VCIOCTL_' placed here */
/* EOF vcioctl.h */
