/* -> opcode/h
 * Title:               The opcode detection routine
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
/*---------------------------------------------------------------------------*/

#ifndef opcode_h
#define opcode_h

#include "nametype.h"

#define SUBCode 0x00400000
#define ADDCode 0x00800000
#define MOVCode 0x01A00000

typedef enum {
              DataProcessing,
              DataTransfer,
              SWI,
              BlockData,
              Branch,
              Adr,
              /* Now the floating point ones */
              FPDataTransfer,
              FPDyadic,
              FPMonadic,
              FPCompare,
              FPFloat,
              FPFix,
              FPStatus,
              /* Now the generic coprocessor ones */
              CPDT,
              CPDO,
              CPRT,
              /* Now MUL and MLA */
              MUL,
              MLA,
              /* Now ADRL */
              ADRL,
              /* Now SWP */
              SWP
             } OpcodeType ;

/*---------------------------------------------------------------------------*/

/* The result is TRUE if the name is an opcode */
BOOLEAN Opcode(Name name,OpcodeType *opcodeType,CARDINAL *opcodeValue) ;

void InitOpcode(void) ;

#endif

/*---------------------------------------------------------------------------*/
/* EOF opcode/h */
