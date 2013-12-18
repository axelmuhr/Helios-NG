
#ifndef _HEPCIO_H

#define _HEPCIO_H

/*
 * I/O controls
 */
#define	HEPC_RESET		(('k'<<8)|0)	/* Reset site */
#define	HEPC_ENABLE_ERRORS	(('k'<<8)|1)	/* Abort i/o on error */
#define	HEPC_DISABLE_ERRORS	(('k'<<8)|2)	/* Ignore errors */
#define	HEPC_ERROR		(('k'<<8)|3)	/* Is error flag set? */
#define	HEPC_INPUT_PENDING	(('k'<<8)|4)	/* Is input pending */
#define	HEPC_DMA		(('k'<<8)|5)	/* DMA setup */
#define HEPC_TIMEOUT		(('k'<<8)|6)	/* Set timeout */
#define HEPC_OUTPUT_READY	(('k'<<8)|7)	/* Ready to output */

#endif
