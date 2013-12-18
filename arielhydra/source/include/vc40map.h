/* vc40map.h */

#include "vc40drvmap.h"
#include "coff.h"

/*
 * User configurable parameters
 */

/*
 * VME Device names
 */
#define	VME_A32D32	"/dev/vme32d32"
#define	VME_A24D32	"/dev/vme24d32"
#define	VME_A16D32	"/dev/vme16d32"

/*
 * Address spaces
 */
#define	MCR_SPACE	A24	/* can be A16, A24, or A32 */

/*
 * Maximum number of Hydras supported
 */
#define	MAX_HYDRA	4

void vc40map();
int get_property(/* struct vc40_vic *vic, int prop_id, u_long *prop_val */);

typedef struct vc40_units_struct   VC40DSP;

extern VC40DSP hydras[];
int vc40_cmd_interrupt();
int c40_write_long();
int c40_read_long();
int c40_get_long();
int c40_put_long();
int c40_get_float();
int c40_put_float();
int c40_get_dsp_float();
int c40_put_dsp_float();
int c40_run();
u_long ieee2dsp();
float dsp2ieee();
