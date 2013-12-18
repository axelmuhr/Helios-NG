/*
 * $Header: fbioctl.h,v 1.1 90/01/13 20:12:52 charles Locked $
 * $Source: /server/usr/users/charles/world/drawp/RCS/lochdr/sys/fbioctl.h,v $
 *
 * Copyright (c) 1988 Acorn Computers Ltd., Cambridge, England
 *
 * $Desc$
 * $Log:	fbioctl.h,v $
 * Revision 1.1  90/01/13  20:12:52  charles
 * Initial revision
 * 
 * Revision 1.8  89/12/15  16:33:30  pete
 * File unchanged
 * 
 * Revision 1.7  89/11/03  14:59:04  charles
 * File unchanged
 * 
 * Revision 1.6  89/08/15  13:42:05  charles
 * File unchanged
 * 
 * Revision 1.5  89/08/11  17:46:20  charles
 * File unchanged
 * 
 * Revision 1.4  89/08/10  19:52:33  charles
 * File unchanged
 * 
 * Revision 1.3  89/08/10  19:40:49  charles
 * File unchanged
 * 
 * Revision 1.2  89/08/10  19:00:01  charles
 * File unchanged
 * 
 * Revision 1.1  89/08/10  16:22:31  charles
 * Initial revision
 * 
 * Revision 1.3  89/08/10  15:43:12  charles
 * "just_to_release_lock"
 * 
 * Revision 1.2  89/07/10  18:14:11  charles
 * Just changed so can co-exist with ANSI header files
 * 
 * Revision 1.1  89/07/10  15:25:17  charles
 * Initial revision
 * 
 * Revision 1.3  88/06/17  20:07:03  beta
 * Acorn Unix initial beta version
 * 
 */
/* fbioctl.h */

#ifndef _FBIOCTL_
#define _FBIOCTL_

/* Formats and ioctl defs relating to frame-buffer devices */
    

/*
 * Bitmap descriptions
 *
 * A BitMap is always specified in the orientation that consecutive pixels in
 * memory have the same y offset and consecutive x-offsets, unless the first
 * pixel is at the right-hand end of a raster.
 *
 * The definitions below allow for quite a range of physical bitmap organisations,
 * from 1 bit/pixel simple mono map to e.g. (bits 8-31 out of 32)/pixel colour map,
 * (bits 0-2 out of 8)/pixel overlay plane map.
 */
typedef struct _fbbitmapinfo_
{
    unsigned char  bits_per_pixel;	/* active bits for each pixel */
    unsigned char  pixel_cell_bits;	/* size of cell for a pixel */
    unsigned char  pixel_start_bits;	/* bit offset to start of pixel in cell */
    unsigned int   raster_step_bits;	/* bit distance from <x,y> to <x,y+1> */
    unsigned int   x_pixels;		/* viewable area x dimension */
    unsigned int   y_pixels;		/* viewable area y dimension */
    unsigned int   map_bytes;		/* size in bytes of a full bitmap */
} FBBitMapInfo, *FBBitMapInfoRef;


/*
 * A particular frame buffer device may consist of a number of distinct
 * sub-bitmaps (e.g. a colour display with 1x8bpp colour map + 2x1bpp
 * overlay maps).  Each sub bit-map may or may not be mappable into user 
 * address space.  If it is, the base_address field of a FBBitMap record 
 * will contain the address in user-space.  If not, it will be zero.
 */

typedef struct _fbbitmap_
{
    caddr_t     base_address;			/* #0 => mappable address */
    FBBitMapInfo  info;				/* info on bitmap */
} FBBitMap, *FBBitMapRef;

/*
 * The maximum number of distinct maps supported using this interface, for a
 * given device subtype, is specified as FB_MAXBITMAPS.  This limit is derived
 * from the operation of the ioctl interface (i.e. max 127 bytes in/out) and
 * the sizes of FBBitMap and FBInfo structures.
 */

typedef struct _fbinfo_
{
    int	    type;				/* from list below */
    int	    subtypes;				/* # possible subtypes */
    int	    subtype;				/* current subtype */
    int	    bitmaps;				/* # bitmaps for subtype */
/*
#define FB_MAXBITMAPS	((127-4*sizeof(int))/sizeof(FBBitMap))
*/
#define FB_MAXBITMAPS	4			/* layout-sensitive! */
    FBBitMap  bitmap[FB_MAXBITMAPS];
} FBInfo, *FBInfoRef;


/* Possible frame buffer device types */

#define FB_DISPLAY_SCREEN	0		/* video frame buffer */
#define FB_LASER_PRINTER	1		/* lbp (not mappable!) */
#define FB__TYPE_COUNT		2		/* how many types so far */

/* Ioctl call commands */

#define  FBIOGCURINFO	_IOR('F', 0, FBInfo)	/* get full info on current state */
#define  FBIOGINFO	_IOWR('F', 1, FBInfo)	/* get info on specified subtype */
#define  FBIOSINFO	_IOW('F', 3, FBInfo)	/* set info for (soft) subtype */
#define  FBIOGSUBTYPE	_IOR('F', 4, int)		/* get current subtype */
#define  FBIOSSUBTYPE	_IOW('F', 2, int)		/* set current subtype */


#endif 	/* NORCROFT ANSI 'C' objects to identifier '_FBIOCTL_' placed here */
/* EOF fbioctl.h */
