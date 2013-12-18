/* -> extypes/h
 * Title:      Types for expressions
 * Author:     J.G.Thackray
 *             Copyright (C) 1989 Acorn Computers Limited
 */
/*---------------------------------------------------------------------------*/

#ifndef exprtypes_h
#define exprtypes_h

#include "nametype.h"

#include "tables.h"

/*---------------------------------------------------------------------------*/

typedef enum Operator {
                       STBot,
                       STTop,
                       Bra,
                       Ket,
                       LAnd,
                       LOr,
                       LEor,
                       Less,
                       OpEquals,
                       LessEquals,
                       Greater,
                       NotEquals,
                       GreaterEquals,
                       Plus,
                       Minus,
                       Ror,
                       Rol,
                       Shr,
                       Shl,
                       BAnd,
                       BOr,
                       BEor,
                       Left,
                       Right,
                       Cc,
                       Star,
                       Slash,
                       Mod,
                       UPlus,
                       UMinus,
                       LNot,
                       BNot,
                       Len,
                       Chr,
                       Str,
                       Base,
                       Index,
                       ModOff,
                       Offset,
                       LsbOff,
                       MidOff,
                       MsbOff,
                       ODummy
                      } Operator ;


/* These are the values returned from "Expression" */
typedef enum EHeliosPatchType {
                               NoPatch,
                               ModOffPatch,
                               OffsetPatch,
                               LsbOffPatch,
                               MidOffPatch,
                               MsbOffPatch,
                               ModOffWOPatch
                              } EHeliosPatchType ;
typedef struct {
                EHeliosPatchType type ;
                SymbolPointer    symbol ;
               } EHeliosPatch ;

typedef enum EStackType {OperatorEST,OperandEST} EStackType ;

typedef enum OperandType {
                          UnDefOT,
                          ConstantOT,
                          StringOT,
                          PCRelOT,
                          RegRelOT,
                          LogicalOT,
                          DPRelOT          /* helios module table relative */
                         } OperandType ;

typedef CARDINAL PCRelOperand ;

typedef struct RegRelOperand {
                              CARDINAL registers[16] ;
                              CARDINAL offset ;
                             } RegRelOperand ;

typedef struct Operand {
                        OperandType operandType ;
                        union {
                               /* case ConstantOT, UnDefOT */
                               CARDINAL      constant ;
                               Name          string ;
                               PCRelOperand  pLabel ;
                               RegRelOperand regRel ;
                               BOOLEAN       bool ;
                               SymbolPointer hSymbol ;
                              } u; /* case */
                       } Operand ; /* record */

typedef struct EStackEntry {
                            EStackType type ;
                            union {
                                   /* case OperatorEST */
                                   Operator operator ;
                                   Operand  operand ;
                                  } u ;
                           } EStackEntry;

#define EStackLimit 256 /* Maximum number of items on the expression stack */

#define MaxStringSize 256 /* Maximum size a string may become internally */

extern CARDINAL priorities[ODummy] ; /* The operator precedences */

#endif

/* End extypes/h */
