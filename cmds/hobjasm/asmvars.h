/* -> asmvars/h
 * Title:               The code production variables
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
/*---------------------------------------------------------------------------*/

#ifndef asmvars_h
#define asmvars_h

#include "constant.h"
#include "helios.h"

extern BOOLEAN  noInitArea ;
extern BOOLEAN  keepingAll ;
extern HOF_header heliosHdr ;

extern CARDINAL stringOffset ;

extern int symbolId ;

typedef enum {
              CodeST,
              DataST,
              BssST
             } SegmentType ;

extern BOOLEAN     area_is_code ;
extern SegmentType segment_type ;
extern CARDINAL    code_size ;
extern CARDINAL    data_size ;
extern CARDINAL    bss_size ;

#endif

/*---------------------------------------------------------------------------*/
/* EOF asmvars/h */
