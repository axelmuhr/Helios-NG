/* -> literals/h
 * Title:               Literal handling
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
#ifndef literals_h
#define literals_h

#include "tables.h"

typedef enum {
  ConstantLT,
  AddressLT,
  ExternalLT,
  FPSingleLT,
  FPDoubleLT
  } LiteralType;

typedef struct {
  unsigned short symbolId; /* ARMObjFmt.SymbolId */
  CARDINAL       offset;
  SymbolPointer  symbolPointer;
} ExternalLiteral; /* record */

typedef struct {
  CARDINAL value1,
           value2;
} FPDoubleLiteral; /* record */

typedef struct Literal *LiteralPointer;

typedef struct Literal {
  LiteralType type;
  union {
    /* case ConstantLT */
    /* case FPSingleLT */
    CARDINAL constant;

    /* case AddressLT */
    CARDINAL offset;

    /* case ExternalLT */
    ExternalLiteral ext;

    /* case FPDoubleLT */
    FPDoubleLiteral fpdouble;
  } u; /* case */
  CARDINAL       address;
  BOOLEAN        adrSet;
  LiteralPointer link;
  BOOLEAN        status;
} Literal;

typedef struct LiteralBlock *LiteralBlockPointer;

typedef struct LiteralBlock {
  CARDINAL address,  /*The address of the start of the pool for this file*/
           size;     /*A multiple of four*/
  LiteralBlockPointer link;
} LiteralBlock;

CARDINAL AddLiteral(BOOLEAN status, CARDINAL value);

CARDINAL AddAddressLiteral(BOOLEAN status, CARDINAL value);

CARDINAL AddExternalLiteral(SymbolPointer SymbolPointer, CARDINAL value);

CARDINAL AddFPLiteralSingle(CARDINAL value);

CARDINAL AddFPLiteralDouble(CARDINAL value1, CARDINAL value2);

void LiteralFileStart(void);

void LiteralAsmStart(void);

void LiteralFileEnd(void);

void LiteralOrg(void);

#endif

/* End literals/h */
