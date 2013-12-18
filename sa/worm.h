
#include <helios.h>

typedef struct WormLoadData {
	word		parentsid;	/* parent's node number	*/
	word		parentslink;	/* parents link to me	*/
	word		myid;		/* my node number	*/
	word		mylink;		/* my link to parent	*/
} WormLoadData;

typedef struct WormLinkData {
	word		proc;		/* processor through link */
	word		link;		/* his link to me	*/
} WormLinkData;

typedef struct WormInfo {
	word		type;		/* processor type	*/
	word		memsize;	/* memory size		*/
	word		memok;		/* memtest result	*/
	WormLinkData	linkarray[4];	/* link connectivity	*/
} WormInfo;

#define ProbeNeighbour	(MinInt+0x100)
#define BootNeighbour	(MinInt+0x200)
#define LoadDataTag	(MinInt+0x300)
#define ReturnControl	(MinInt+0x400)
#define AlreadyLoaded	(MinInt+0x500)
#define Synchronise	(MinInt+0x600)
#define PeekValue	(MinInt+0x700)
#define LinkId		(MinInt+0x800)
#define NetworkData	(MinInt+0x900)
#define NoMoreData	(MinInt+0xa00)

#define Unattached	(-1)
#define TimeOut		(-2)
#define TokenError	(-3)

#define PeekAddress	(MinInt+0x70)
