/* -> errors/h
 * Title:               Error handling
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */

#ifndef errors_h
#define errors_h

/*---------------------------------------------------------------------------*/

#define Cstyle	/* C compiler style error/warning messages */

/*---------------------------------------------------------------------------*/

#include "constant.h"

typedef enum {
   ADirMissing 		/* Area directive missing */,
   SKSymMissing         /* Symbol missing */,
   SKSizeMissing        /* Size missing */,
   BadLDR               /* LDR cannot be used with r9 */,
   BadOpUse             /* Operator can only be used with ARM instruction */,
   BadOpInstruction     /* NOT data transfer or data processing instruction */,
   MultipleHDir         /* Multiple Helios patch directives in expression */,
   BadPatchUse          /* Patch type cannot be applied to this instruction */,
   NoStubFN             /* STUB function label cannot be found */,
   BadExtern            /* Bad external symbol definition */,
   BadExternType        /* Bad external symbol type */,
   CommaMissing /*Missing comma*/,
   BadSymType /*Bad symbol type*/,
   MulDefSym /*Multiply defined symbol*/,
   BadExprType /*Bad expression type*/,
   TooLateOrg /*Too late to set origin now*/,
   RegSymExists /*Register symbol already defined*/,
   NoMacro /*No current macro expansion*/,
   BadMEND /*MEND not allowed within conditionals*/,
   BadGlob /*Bad global name*/,
   GlobExists /*Global name already exists*/,
   LocNotAllowed /*Locals not allowed outside macros*/,
   BadLoc /*Bad local name*/,
   LocExists /*Local name already exists*/,
   WrongSy /*Unknown or wrong type of global/local symbol*/,
   BadAlign /*Bad alignment boundary*/,
   BadImport /*Bad imported name*/,
   ImportExists /*Imported name already exists*/,
   BadExport /*Bad exported name*/,
   BadExportType /*Bad exported symbol type*/,
   NoAreaName /*Area name missing*/,
   BadAttr /*Bad area attribute or alignment*/,
   EntryExists /*Entry address already set*/,
   BadEOL /*Unexpected characters at end of line*/,
   StringShort /*String too short for operation*/,
   StringOver /*String overflow*/,
   BadOpType /*Bad operand type*/,
   UnDefExp /*Undefined exported symbol*/,
   CantOpenCode /*Unable to open code file*/,
   CantCloseCode /*Unable to close code file*/,
   BadShift /*Bad shift name*/,
   UnkShift /*Unknown shift name*/,
   ShiftOpt /*Shift option out of range*/,
   BadSym /*Bad symbol*/,
   BadReg /*Bad register name symbol*/,
   UnExpOp /*Unexpected operator*/,
   UnDefSym /*Undefined symbol*/,
   UnExpOpnd /*Unexpected operand*/,
   UnExpUnOp /*Unexpected unary operator*/,
   BraMiss /*Missing open bracket*/,
   SynAfterDir /*Syntax error following directive*/,
   IllLineStart /*Illegal line start, should be blank*/,
   LabMiss /*Label missing from line start*/,
   BadLocNum /*Bad local label number*/,
   SynAfterLocLab /*Syntax error following local label definition*/,
   WrongRout /*Incorrect routine name*/,
   UnkOpc /*Unknown opcode*/,
   TooManyParms /*Too many actual parameters*/,
   BadOpcSym /*Bad opcode symbol*/,
   SynAfterLab /*Syntax error following label*/,
   InvLineStart /*Invalid line start */,
   BadTrans /*Translate not allowed in pre-indexed form*/,
   MissSqKet /*Missing close square bracket*/,
   ImmValRange /*Immediate value out of range*/,
   KetMiss /*Missing close bracket*/,
   BadBrOp /*Invalid operand to branch instruction*/,
   BadRot /*Bad rotator*/,
   DatOff /*Data transfer offset out of range*/,
   BadRegRange /*Bad register range*/,
   BrOff /*Branch offset out of range*/,
   DecOver /*Decimal overflow*/,
   HexOver /*Hexadecimal overflow*/,
   BadHex /*Bad hexadecimal number*/,
   MissQuote /*Missing close quote*/,
   BadOp /*Bad operator*/,
   BadBaseNum /*Bad based number*/,
   NumOver /*Numeric overflow*/,
   ExtAreaSym /*External area relocatable symbol used*/,
   ExtNotVal /*Externals not valid in expressions*/,
   NoSym /*Symbol missing*/,
   CodeInDataArea /*Code generated in data area*/,
   BadMacroParms /*Error in macro parameters*/,
   RegRange /*Register number out of range*/,
   HashMissing /*Missing hash*/,
   FPRegRange /*Floating point register number out of range*/,
   CPRegRange /*Coprocessor register number out of range*/,
   CPNameRange /*Coprocessor number out of range*/,
   FPOver /*Floating point overflow*/,
   FPNoNum /*Floating point number not found*/,
   FPTooLate /*Too late to ban floating point*/,
   UnkOp /*Unknown operand*/,
   CPOpRange /*Coprocessor operation out of range*/,
   BadMul /*Multiply destination equals first source*/,
   StructErr /*Structure mismatch*/,
   SubstLong /*Substituted line too long*/,
   IllLabParm /*Illegal label parameter start in macro prototype*/,
   BadMacParmDef /*Bad macro parameter default value*/,
   BadRegInBD /*Register occurs multiply in LDM/STM list*/,
   BadIMAttr /*Bad or unknown attribute*/,
   BadFPCon /*Bad floating point constant*/,
   BadADRL /*ADRL can't be used with PC*/,
   BadNoInit /*Non-zero data within uninitialised area*/,
   MissBra /*Missing open square bracket*/,
   DivZero /*Division by zero*/,
   TooLateStyle /* Too late to change output format */,
   BadWeak /*Weak symbols not permitted in a.out */,
   BadOrg /*Origin illegal for a.out */,
   BadStrong /* STRONG directive not suported by a.out */,
   BadAttrs /* Illegal combiation of code and zero initialised */,
   TooManyData /* Too many data areas for a.out */,
   TooManyCode /* Too many code areas for a.out */,
   TooMany0Init /* Too many bss areas for a.out */
   } WarningNumber;

void AssemblerError(char *error) ;

void NotifyReport(char *error) ;

void WarningReport(char *error) ;

void WarningChs(char *error) ;

void TellErrors(void) ;

void ErrorFile(BOOLEAN lower) ;

void OpenErrorStream(char *fileName) ;

void CloseErrorStream(void) ;

void Notification(WarningNumber warn) ;

void Warning(WarningNumber warn) ;

extern BOOLEAN  errorFound,
                linePrinted;

extern CARDINAL totalErrors;

#endif

/* End errors/h */
