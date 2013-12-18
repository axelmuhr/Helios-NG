
#ifndef _HEPCIO_H

#define _HEPCIO_H

/*
 * I/O controls
 */
#define	HEPC_RESET		(('k'<<8)|0)	/* Reset site */
#define	HEPC_INPUT_PENDING	(('k'<<8)|1)	/* Is input pending */
#define HEPC_TIMEOUT		(('k'<<8)|2)	/* Set timeout */
#define HEPC_OUTPUT_READY	(('k'<<8)|3)	/* Ready to output */

#endif
