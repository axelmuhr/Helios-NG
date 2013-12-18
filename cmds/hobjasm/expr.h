/* -> expr/h
 * Title:               Expression parsing and evaluation
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
/*---------------------------------------------------------------------------*/

#ifndef expression_h
#define expression_h

#include "extypes.h"
#include "nametype.h"
#include "tables.h"

/*---------------------------------------------------------------------------*/
/* This actually returns a 12bit value suitable for ORing into the opcode */

CARDINAL ShiftedRegister(char *line,CARDINAL *index,BOOLEAN allowRegisterShift) ;

CARDINAL AnyRegisterExpr(char *line,CARDINAL *index,BOOLEAN *Int) ;

CARDINAL RegisterExpr(char *line,CARDINAL *index) ;

CARDINAL FPRegisterExpr(char *line,CARDINAL *index) ;

CARDINAL CPRegisterExpr(char *line,CARDINAL *index) ;

CARDINAL CPNameExpr(char *line,CARDINAL *index) ;

BOOLEAN LogicalExpr(char *line,CARDINAL *index) ;

CARDINAL FPConstant(char *line,CARDINAL *index) ;

BOOLEAN AssertExpr(char *line,CARDINAL *index,BOOLEAN allowUnd,BOOLEAN *defined) ;

CARDINAL ConstantExpr(char *line,CARDINAL *index,BOOLEAN allowUnd,BOOLEAN *defined,EHeliosPatch *hDIR) ;

CARDINAL ConstantOrAddressExpr(char *line,CARDINAL *index,OperandType *type,BOOLEAN allowUnd,BOOLEAN *defined) ;

CARDINAL NotStringExpr(char *line,CARDINAL *index,CARDINAL *reg,OperandType *type,BOOLEAN allowUnd,BOOLEAN *defined) ;

CARDINAL AddressExpr(char *line,CARDINAL *index,BOOLEAN allowUnd,BOOLEAN *defined) ;

void StringExpr(char *line,CARDINAL *index,Name *string);

CARDINAL RegisterRelExpr(char *line,CARDINAL *index,CARDINAL *reg,OperandType *type,BOOLEAN allowUnd,BOOLEAN *defined) ;

CARDINAL StringOrConstantExpr(char *line,CARDINAL *index,BOOLEAN allowUnd,OperandType *type) ;

SymbolPointer ExternalExpr(char *line,CARDINAL *index,CARDINAL *offset) ;

BOOLEAN ArgSyntaxCheck(Operand arg1,Operand arg2,Operator op) ;

void ShuffleDown(CARDINAL pointerLo,CARDINAL pointerHi,Operator *op) ;

void Init_Expression(void) ;

#endif

/*---------------------------------------------------------------------------*/
/* EOF expr/h */
