/* -> errors/c
 * Title:               Error handling
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
/*---------------------------------------------------------------------------*/

#include "osdepend.h"
#include "errors.h"
#include "asm.h"
#include "conds.h"
#include "constant.h"
#include "formatio.h"
#include "getline.h"
#include "globvars.h"
#include "listing.h"
#include "asmvars.h"

#ifdef Cstyle
#include "store.h"
#endif

#include <stdio.h>
#include <stdlib.h>

/*---------------------------------------------------------------------------*/

BOOLEAN  errorFound ;
BOOLEAN  linePrinted ;

CARDINAL totalErrors ;

static BOOLEAN errorStreamExists = FALSE ;

static FILE *errorStream ;

/*---------------------------------------------------------------------------*/
/* Give the error line and file name */

#ifdef Cstyle

typedef struct macstr {
                       struct macstr *last ;	/* last element */
		       CARDINAL	      line ;	/* associated line */
                       char          *text ;	/* MACRO name */
                      } macstr ;

static void ErrorLine(void)
{
 CARDINAL               i ;
 CARDINAL               line ;
 StructureStackElement  s ;
 macstr                *mptr = NULL ;

 if (inFile)
  {
   InitErrorAccess() ; /* Get ready to interrogate the structure stack */
   line = lineNumber ;

   ErrorFile(TRUE) ;

   /* display MACRO nesting and line numbers withn the file that defined
    * the MACRO (rather than relative to the MACRO definition).
    */
   while (NextMacroElement(&s))
    {
     macstr *new ;
     new = (macstr *)mymalloc(sizeof(macstr)) ;
     new->last = mptr ;
     new->line = line ;
     new->text = mymalloc(s.u.macro.name.length + 1) ;
     for (i = 0; (i <= (s.u.macro.name.length - 1)); i++)
      new->text[i] = s.u.macro.name.key[i] ;
     new->text[i] = '\0' ; /* terminate the name */
     mptr = new ;
     line = s.u.macro.lineNumber ;
    }

   WriteCh(Space) ;
   WriteCardinal(line) ;
   if (errorStreamExists)
    fprintf(errorStream," %u",line) ;
   WriteChs(": ") ;
   if (errorStreamExists)
    fprintf(errorStream,": ") ;

   while (mptr != NULL)
    {
     macstr *old = mptr ;
     mptr = old->last ;

     WriteChs("line ") ;
     WriteCardinal(old->line) ;
     WriteChs(" in macro ") ;
     WriteChs(old->text) ;
     if (errorStreamExists)
      fprintf(errorStream,"line %u in macro %s",old->line,old->text) ;
     if (mptr != NULL)
      {
       WriteChs(", ") ;
       if (errorStreamExists)
	fprintf(errorStream,", ") ;
      }

     free(old->text) ;
     free(old) ;
    }
   WriteCh(Space) ;
   if (errorStreamExists)
    fprintf(errorStream,"%c",Space) ;
  }
 else
  {
   ErrorFile(FALSE) ;
   WriteCh(Space) ;
   if (errorStreamExists)
    fprintf(errorStream,"%c",Space) ;
  }
} /* End ErrorLine */
#else
static void ErrorLine(void)
{
 CARDINAL              i ;
 CARDINAL              line ;
 StructureStackElement s ;

 if (inFile)
  {
   InitErrorAccess() ; /* Get ready to interrogate the structure stack */
   line = lineNumber ;
   WriteCh(Space) ;
   if (errorStreamExists)
    fprintf(errorStream,"%c",Space) ;
   while (NextMacroElement(&s))
    {
     WriteChs("at line ") ;
     WriteCardinal(line) ;
     WriteChs(" in macro ") ;
     if (errorStreamExists)
      {
       fprintf(errorStream,"at line ") ;
       fprintf(errorStream,"%u",line) ;
       fprintf(errorStream," in macro ") ;
      } ; /* if */
     for (i = 0; (i <= s.u.macro.name.length - 1); i++)
      {
       WriteCh(s.u.macro.name.key[i]) ;
       if (errorStreamExists)
        fprintf(errorStream,"%c",s.u.macro.name.key[i]) ;
      } ; /* for */
     WriteChs("\\N") ;
     if (errorStreamExists)
      fprintf(errorStream,"\n") ;
     line = s.u.macro.lineNumber ;
    } ;
   WriteChs("at line ") ;
   WriteCardinal(line) ;
   if (errorStreamExists)
    fprintf(errorStream," at line %u",line) ;
   ErrorFile(TRUE) ;
  } ;/* if */
} /* End ErrorLine */
#endif

/*---------------------------------------------------------------------------*/
/* Force remainder (possibly all) of line to be listed */
static void ForceListing(void)
{
 if (inFile && !linePrinted)
  {
   if (!(((1 << ListPC) & listStatus) || printState))
    {
     InitLineList() ;
     PrintLineNumber() ;
     PrintAddress() ;
    } ; /* if */
   PrintLine() ;
  } ; /* if */
} /* End ForceListing */

/*---------------------------------------------------------------------------*/
#ifdef Cstyle
void AssemblerError(char *error)
{
 totalErrors++ ;
 ErrorLine() ;
 WriteChs("Error: ") ;
 WriteChs(error) ;
 if (errorStreamExists)
  fprintf(errorStream,"Error: %s\nAssembly terminated\n",error) ;
 ForceListing() ;
 TellErrors() ;
 exit(8) ;
} /* End AssemblerError */
#else
void AssemblerError(char *error)
{
 totalErrors++ ;
 WriteChs("Assembler error: ") ;
 if (errorStreamExists)
  fprintf(errorStream,"Assembler error: %s\nAssembly terminated\n",error) ;
 WriteChs(error) ;
 ErrorLine() ;
 WriteChs("\\NAssembly terminated\\N") ;
 ForceListing() ;
 TellErrors() ;
 exit(8) ;
} /* End AssemblerError */
#endif

/*---------------------------------------------------------------------------*/
/* To allow partial error construction */
void WarningChs(char *error)
{
 WriteChs(error) ;
 if (errorStreamExists)
  fprintf(errorStream,error) ;
} /* End WarningChs */

/*---------------------------------------------------------------------------*/

#ifdef Cstyle
void WarningReport(char *error)
{
 ErrorLine() ;
 if (inFile)
  WriteChs("Error: ") ;
 WriteChs(error) ;
 if (errorStreamExists)
  fprintf(errorStream,"%s%s",((inFile) ? "Error: " : ""),error) ;
 WriteChs("\\N") ;
 if (errorStreamExists)
  fprintf(errorStream,"\n") ;
 totalErrors++ ;
 errorFound = TRUE ;
 ForceListing() ;
} /* End WarningReport */
#else
void WarningReport(char *error)
{
 WriteChs(error) ;
 if (errorStreamExists)
  fprintf(errorStream,error) ;
 ErrorLine() ;
 WriteChs("\\N") ;
 if (errorStreamExists)
  fprintf(errorStream,"\n") ;
 totalErrors++ ;
 errorFound = TRUE ;
 ForceListing() ;
} /* End WarningReport */
#endif

/*---------------------------------------------------------------------------*/

#ifdef Cstyle
void NotifyReport(char *error)
{
 if (pass == 2) /* don't bother displaying the message during pass 1 */
  {
   ErrorLine() ;
   WriteChs("Warning: ") ;
   WriteChs(error) ;
   if (errorStreamExists)
    fprintf(errorStream,"Warning: %s",error) ;
   WriteChs("\\N") ;
   if (errorStreamExists)
    fprintf(errorStream,"\n") ;
  }
 return ;
}
#else
static void NotifyReport(char *error)
{
 if (pass == 2) /* don't bother displaying the message during pass 1 */
  {
   WriteChs("Warning: ") ;
   WriteChs(error) ;
   if (errorStreamExists)
    fprintf(errorStream,"Warning: %s",error) ;
   ErrorLine() ;
   WriteChs("\\N") ;
   if (errorStreamExists)
    fprintf(errorStream,"\n") ;
  }
 return ;
}
#endif

/*---------------------------------------------------------------------------*/
/* Report total number of errors on given stream */
void TellErrors(void)
{
 WriteCardinal(totalErrors) ;
 WriteChs(" error") ;
 if (totalErrors != 1)
  WriteCh('s') ;
 WriteChs(" found\\N") ;
 if (errorStreamExists)
  fprintf(errorStream,"%uerror%sfound\n",totalErrors,(totalErrors != 1) ? "s" : "") ;
} /* End TellErrors */

/*---------------------------------------------------------------------------*/

#ifdef Cstyle
void ErrorFile(BOOLEAN lower)
{
 CARDINAL index ;

 if (!lower)
  {
   WriteChs("File ") ;
   if (errorStreamExists)
    fprintf(errorStream,"File ") ;
  }

 index = 0 ;
 while ((currentFileName[index] > Space) && (currentFileName[index] < Del))
  {
   WriteCh(currentFileName[index]) ;
   if (errorStreamExists)
    fprintf(errorStream,"%c",currentFileName[index]) ;
   index++ ;
  } ; /* while */

 if (lower)
  {
   WriteChs(": ") ;
   if (errorStreamExists)
    fprintf(errorStream,": ") ;
  }

 return ;
}
#else
/* Write out the file name. "lower == TRUE" file to begin with lower case */
void ErrorFile(BOOLEAN lower)
{
 CARDINAL index ;
 if (lower)
  {
   WriteChs(" in file \"") ;
   if (errorStreamExists)
    fprintf(errorStream," in file \"") ;
  }
 else
  {
   WriteChs("File \"") ;
   if (errorStreamExists)
    fprintf(errorStream,"File \"") ;
  } ; /* if */
 index = 0 ;
 while ((currentFileName[index] > Space) && (currentFileName[index] < Del))
  {
   WriteCh(currentFileName[index]) ;
   if (errorStreamExists)
    fprintf(errorStream,"%c",currentFileName[index]) ;
   index++ ;
  } ; /* while */
 WriteCh(Quotes) ;
 if (errorStreamExists)
  fprintf(errorStream,"%c",Quotes) ;
} /* End ErrorFile */
#endif

/*---------------------------------------------------------------------------*/
/* Open the error stream for assembly error output */
void OpenErrorStream(char *fileName)
{
 CloseErrorStream() ;
 errorStream = fopen(fileName,"w") ;
 if ((errorStream == NULL) || ferror(errorStream))
  {
   WarningReport("Unable to open error file") ;
   return ;
  } ;
 errorStreamExists = TRUE ;
} /* End OpenErrorStream */

/*---------------------------------------------------------------------------*/
/* Close the error stream for assembly error output */
void CloseErrorStream(void)
{
 if (errorStreamExists)
  {
   errorStreamExists = FALSE ;
   fclose(errorStream) ;
  } ; /* if */
} /* End CloseErrorStream */

/*---------------------------------------------------------------------------*/
/* Just notify the user that you are unhappy with what he is doing */
void Notification(WarningNumber warn)
{
 switch (warn)
  {
   case BadLDR  : NotifyReport("Possible corruption of r9 \"dp\" in LDR") ;
                  break ;
   case BadADRL : NotifyReport("Possible corruption of r9 \"dp\" in ADR") ;
                  break ;
  }
 return ;
}

/*---------------------------------------------------------------------------*/

void Warning(WarningNumber warn)
{
 switch (warn)
  {
   case ADirMissing:
    WarningReport("Area directive missing");
    break;

   case SKSymMissing:
    WarningReport("Symbol missing") ;
    break ;

   case SKSizeMissing:
    WarningReport("Object size definition missing") ;
    break ;

   case BadLDR:
    WarningReport("LDR cannot be used with r9") ;
    break;

   case BadOpUse:
    WarningReport("Operator can only be used with an ARM instruction") ;
    break ;

   case BadOpInstruction:
    WarningReport("Operator can only be used with a data processing or data transfer instruction") ;
    break ;

   case MultipleHDir:
    WarningReport("Multiple HELIOS patch directives in expression") ;
    break ;

   case BadPatchUse:
    WarningReport("The Helios patch directive cannot be applied to this instruction") ;
    break ;

   case NoStubFN:
    WarningReport("Missing STUB function for external function call") ;
    break ;

   case BadExtern:
    WarningReport("Bad external symbol definition") ;
    break ;

   case BadExternType:
    WarningReport("Bad external symbol type") ;
    break ;

   case CommaMissing:
    WarningReport("Missing comma");
    break;

   case BadSymType:
    WarningReport("Bad symbol type");
    break;

   case MulDefSym:
    WarningReport("Multiply or incompatibly defined symbol");
    break;

   case BadExprType:
    WarningReport("Bad expression type");
    break;

   case TooLateOrg:
    WarningReport("Too late to set origin now");
    break;

   case RegSymExists:
    WarningReport("Register symbol already defined");
    break;

   case NoMacro:
    WarningReport("No current macro expansion");
    break;

   case BadMEND:
    WarningReport("MEND not allowed within conditionals");
    break;

   case BadGlob:
    WarningReport("Bad global name");
    break;

   case GlobExists:
    WarningReport("Global name already exists");
    break;

   case LocNotAllowed:
    WarningReport("Locals not allowed outside macros");
    break;

   case BadLoc:
    WarningReport("Bad local name");
    break;

   case LocExists:
    WarningReport("Local name already exists");
    break;

   case WrongSy:
    WarningReport("Unknown or wrong type of global/local symbol");
    break;

   case BadAlign:
    WarningReport("Bad alignment boundary");
    break;

   case BadImport:
    WarningReport("Bad imported name");
    break;

   case ImportExists:
    WarningReport("Imported name already exists");
    break;

   case BadExport:
    WarningReport("Bad exported name");
    break;

   case BadExportType:
    WarningReport("Bad exported symbol type");
    break;

   case NoAreaName:
    WarningReport("Area name missing");
    break;

   case BadAttr:
    WarningReport("Bad area attribute or alignment");
    break;

   case EntryExists:
    WarningReport("Entry address already set");
    break;

   case BadEOL:
    WarningReport("Unexpected characters at end of line");
    break;

   case StringShort:
    WarningReport("String too short for operation");
    break;

   case StringOver:
    WarningReport("String overflow");
    break;

   case BadOpType:
    WarningReport("Bad operand type");
    break;

   case UnDefExp:
    WarningReport("Undefined exported symbol");
    break;

   case CantOpenCode:
    WarningReport("Unable to open code file");
    break;

   case CantCloseCode:
    WarningReport("Unable to close code file");
    break;

   case BadShift:
    WarningReport("Bad shift name");
    break;

   case UnkShift:
    WarningReport("Unknown shift name");
    break;

   case ShiftOpt:
    WarningReport("Shift option out of range");
    break;

   case BadSym:
    WarningReport("Bad symbol");
    break;

   case BadReg:
    WarningReport("Bad register name symbol");
    break;

   case UnExpOp:
    WarningReport("Unexpected operator");
    break;

   case UnDefSym:
    WarningReport("Undefined symbol");
    break;

   case UnExpOpnd:
    WarningReport("Unexpected operand");
    break;

   case UnExpUnOp:
    WarningReport("Unexpected unary operator");
    break;

   case BraMiss:
    WarningReport("Missing open bracket");
    break;

   case SynAfterDir:
    WarningReport("Syntax error following directive");
    break;

   case IllLineStart:
    WarningReport("Illegal line start should be blank");
    break;

   case LabMiss:
    WarningReport("Label missing from line start");
    break;

   case BadLocNum:
    WarningReport("Bad local label number");
    break;

   case SynAfterLocLab:
    WarningReport("Syntax error following local label definition");
    break;

   case WrongRout:
    WarningReport("Incorrect routine name");
    break;

   case UnkOpc:
    WarningReport("Unknown opcode");
    break;

   case TooManyParms:
    WarningReport("Too many actual parameters");
    break;

   case BadOpcSym:
    WarningReport("Bad opcode symbol");
    break;

   case SynAfterLab:
    WarningReport("Syntax error following label");
    break;

   case InvLineStart:
    WarningReport("Invalid line start ");
    break;

   case BadTrans:
    WarningReport("Translate not allowed in pre-indexed form");
    break;

   case MissSqKet:
    WarningReport("Missing close square bracket");
    break;

   case ImmValRange:
    WarningReport("Immediate value out of range");
    break;

   case KetMiss:
    WarningReport("Missing close bracket");
    break;

   case BadBrOp:
    WarningReport("Invalid operand to branch instruction");
    break;

   case BadRot:
    WarningReport("Bad rotator");
    break;

   case DatOff:
    WarningReport("Data transfer offset out of range");
    break;

   case BadRegRange:
    WarningReport("Bad register range");
    break;

   case BrOff:
    WarningReport("Branch offset out of range");
    break;

   case DecOver:
    WarningReport("Decimal overflow");
    break;

   case HexOver:
    WarningReport("Hexadecimal overflow");
    break;

   case BadHex:
    WarningReport("Bad hexadecimal number");
    break;

   case MissQuote:
    WarningReport("Missing close quote");
    break;

   case BadOp:
    WarningReport("Bad operator");
    break;

   case BadBaseNum:
    WarningReport("Bad based number");
    break;

   case NumOver:
    WarningReport("Numeric overflow");
    break;

   case ExtAreaSym:
    WarningReport("External area relocatable symbol used");
    break;

   case ExtNotVal:
    WarningReport("Externals not valid in expressions");
    break;

   case NoSym:
    WarningReport("Symbol missing");
    break;

   case CodeInDataArea:
    WarningReport("Code generated in data area");
    break;

   case BadMacroParms:
    WarningReport("Error in macro parameters");
    break;

   case RegRange:
    WarningReport("Register value out of range");
    break;

   case HashMissing:
    WarningReport("Missing hash");
    break;

   case FPRegRange:
    WarningReport("Floating point register number out of range");
    break;

   case CPRegRange:
    WarningReport("Coprocessor register number out of range");
    break;

   case CPNameRange:
    WarningReport("Coprocessor number out of range");
    break;

   case FPOver:
    WarningReport("Floating point overflow");
    break;

   case FPNoNum:
    WarningReport("Floating point number not found");
    break;

   case FPTooLate:
    WarningReport("Too late to ban floating point");
    break;

   case UnkOp:
    WarningReport("Unknown operand");
    break;

   case CPOpRange:
    WarningReport("Coprocessor operation out of range");
    break;

   case BadMul:
    WarningReport("Multiply destination equals first source");
    break;

   case StructErr:
    WarningReport("Structure mismatch");
    break;

   case SubstLong:
    WarningReport("Substituted line too long");
    break;

   case IllLabParm:
    WarningReport("Illegal label parameter start in macro prototype");
    break;

   case BadMacParmDef:
    WarningReport("Bad macro parameter default value");
    break;

   case BadRegInBD:
    WarningReport("Register occurs multiply in LDM/STM list");
    break;

   case BadIMAttr:
    WarningReport("Bad or unknown attribute");
    break;

   case BadFPCon:
    WarningReport("Bad floating point constant");
    break;

   case BadADRL:
    WarningReport("ADRL cannot be used with PC or r9") ;
    break;

   case BadNoInit:
    WarningReport("Non-zero data within uninitialised area");
    break;

   case MissBra:
    WarningReport("Missing open square bracket");
    break;

   case DivZero:
    WarningReport("Division by zero");
    break;

   case TooLateStyle:
    WarningReport("Too late to change output format");
    break;

   case BadWeak:
    WarningReport("Weak symbols not permitted in a.out");
    break;

   case BadOrg:
    WarningReport("Origin illegal for a.out");
    break;

   case BadStrong:
    WarningReport("STRONG directive not suported by a.out");
    break;

   case BadAttrs:
    WarningReport("Illegal combination of code and zero initialised");
    break;

   case TooManyData:
    WarningReport("Too many data areas for a.out");
    break;

   case TooManyCode:
    WarningReport("Too many code areas for a.out");
    break;

   case TooMany0Init:
    WarningReport("Too many bss areas for a.out");
    break;

  } /* switch */
} /* End Warning */

/*---------------------------------------------------------------------------*/
/* EOF errors/c */
