/* $Header: /dsl/HeliosARM/nick/RCS/result.h,v 1.1 1991/03/03 22:15:36 paul Exp $ */
/* $Source: /dsl/HeliosARM/nick/RCS/result.h,v $ */
/************************************************************************/ 
/* result.h - Macros for building ABC result and error codes for Helios	*/
/*									*/
/* Copyright (C) 1989 Active Book Company Ltd., Cambridge, England	*/
/*									*/
/* Author: Nick Reeves,  October 1989					*/
/************************************************************************/

#ifndef RESULT_H
#define RESULT_H

#include <codes.h>	/* for Helios error codes */

/* Result codes are 32 bits long. Results >=0 indicate success and can
 * be allocated at the discretion of the particular sub-system. Pointers
 * are assumed to have bit 31 clear, and thus can be returned as results.
 *
 * Result codes <0 indicate errors. These must conform to the Helios
 * error code standard, and are created using the macro ERROR. This
 * error code contains 4 fields, the error class, the sub system number,
 * the generic error code, and the specific sub system error identifier.
 */

typedef int Result;

/* As Perehelion have only reserved 5 bits for sub system number, use
 * last one to split among all non nucleus 'software units'. Re-use the 16
 * specific error bits to distinguish between the software units and the 8
 * general error bits to give the specific error. Where the specific error
 * matches a general error use that value, otherwise use a value at high
 * end of range.
 */

#define RESULT_SOFTUNITSUBSYS SS_Mask

#define RESULT_SOFTUNITMASK EO_Mask
#define RESULT_SOFTUNITSHIFT EO_Shift

#define RESULT_SUBERRORMASK EG_Mask
#define RESULT_SUBERRORSHIFT EG_Shift

/* general error declaration macro */
#define GERROR( softUnit, name, class ) ( EC_##class )\
| RESULT_SOFTUNITSUBSYS | ( EG_##name ) | ( softUnit##_ID )

/* specific error declaration macro */
#define SERROR( softUnit, number, class ) ( EC_##class )\
| RESULT_SOFTUNITSUBSYS | ( number << RESULT_SUBERRORSHIFT )\
| ( softUnit##_ID )

/*
 * SOFTWARE UNIT NUMBERS
 */
 
#define RESULT_SOFTUNIT( id ) ( id << RESULT_SOFTUNITSHIFT )

#endif
