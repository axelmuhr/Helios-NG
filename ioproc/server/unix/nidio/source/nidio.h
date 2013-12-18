
#ifndef _NIDIO_H

#define _NIDIO_H

/*
 * I/O controls
 */
#define	NIDIO_RESET		(('n'<<8)|0)	/* Reset site */
#define	NIDIO_INPUT_PENDING	(('n'<<8)|1)	/* Is input pending */
#define NIDIO_TIMEOUT		(('n'<<8)|2)	/* Set timeout */
#define NIDIO_OUTPUT_READY	(('n'<<8)|3)	/* Ready to output */

#endif
