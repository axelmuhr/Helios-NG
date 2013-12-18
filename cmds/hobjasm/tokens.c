/* -> tokens/c
 * Title:               Expression element tokenisation
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
/*---------------------------------------------------------------------------*/

#include "constant.h"
#include "errors.h"
#include "extypes.h"
#include "exstore.h"
#include "getline.h"
#include "globvars.h"
#include "llabel.h"
#include "mactypes.h"
#include "nametype.h"
#include "asmvars.h"
#include "occur.h"
#include "p1hand.h"
#include "store.h"
#include "tables.h"
#include "tokens.h"
#include "vars.h"
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*---------------------------------------------------------------------------*/

typedef enum {
              PCWO,
              AtWO,
              OptWO,
              TrueWO,
              FalseWO,
              ModoffWO,         /* Helios special directive */
              DummyWO
             } WordOperand ;

/*---------------------------------------------------------------------------*/

static BOOLEAN initialised = FALSE ;

#ifdef __NCCBUG
Name *operatorTable = NULL ;
Name *operandTable = NULL ;
#else
Name operatorTable[ODummy] ;
Name operandTable[DummyWO] ;
#endif

char charLine[MaxLineLength + 1] ;

#define MAXCARD 0xFFFFFFFF

/*---------------------------------------------------------------------------*/

#if 0   /* useful function no longer used */
/* Read a "C" style number, which may begin "0x", followed by hex */
static CARDINAL CStyleNumber(char *line,CARDINAL *index)
{
 if ((line[*index] == '0') && (line[++(*index)] == 'x'))
  {
   (*index)++ ;
   return HexNumber(line,index) ;
  } ;
 return DecimalNumber(line,index) ;
}
#endif

/*---------------------------------------------------------------------------*/
/* Read a decimal number from the input line, returning the index past the
 * number. Assumes it is currently pointing at a valid decimal digit.
 */
CARDINAL DecimalNumber(char *line,CARDINAL *index)
{
 CARDINAL value = 0 ;
 char     ch = line[*index] ;

 while (isdigit(ch))
  {
   if ((MAXCARD - ch - '0') / 10 < value)
    {
     Warning(DecOver) ;
     return 0 ;
    } ; /* if */
   value = value*10 + ch - '0' ;
   (*index)++ ;
   ch = line[*index] ;
  } ;
 return value ;
} /* End DecimalNumber */

/*---------------------------------------------------------------------------*/
/* Read a hexadecimal number from the input line, returning the index past
 * the number and terminating padding. Assumes the index is currently pointing
 * past the "&".
 */
CARDINAL HexNumber(char *line,CARDINAL *index)
{
 CARDINAL value = 0 ;
 CARDINAL oldIndex = *index ;
 char     ch = line[*index] | 0x20 ;

 while (isxdigit(ch))
  {
   if (value & 0xF0000000)
    {
     Warning(HexOver) ;
     return 0 ;
    } ; /* if */
   value = value * 16 ;
   value += (ch <= '9')  ? ch - '0' : 10 + ch - 'a' ;
   (*index)++ ;
   ch = line[*index] | 0x20 ;
  } ; /* while */

 if (*index == oldIndex)
  Warning(BadHex) ;

 return value ;
} /* End HexNumber */

/*---------------------------------------------------------------------------*/
/* Get a string from the input line, assuming the opening " has been skipped */
char *GetString(char *line,CARDINAL *index,CARDINAL *length)
{
 char  ch ;
 char *string ;

 *length = 0 ;
 do
  {
   ch = line[*index] ;

   if (ch == CR)
    {
     Warning(MissQuote) ;
     return(charLine) ;
    } ;

   if (ch != Quotes)
    {
#if 1 /* provide C style character constants in strings */
      /*	\"	&22
       *	\'	&27
       *	\b	&08
       *	\t	&09
       *	\n	&0A
       *	\f	&0C
       *	\\	&5C
       *	\r	&0D
       *	\0	&00
       *	\xxx	octal number (exactly 3chars in range '0'..'7')
       * Any other form is ignored (ie. no translation takes place)
       */
     if (ch == '\\')
      {
       switch (line[*index+1])
        {
	 case 'b'  : charLine[*length] = 0x08 ;
                     *index += 2 ;
	             break ;

	 case 't'  : charLine[*length] = 0x09 ;
                     *index += 2 ;
	             break ;

	 case 'n'  : charLine[*length] = 0x0A ;
                     *index += 2 ;
	             break ;

	 case 'f'  : charLine[*length] = 0x0C ;
                     *index += 2 ;
	             break ;

	 case 'r'  : charLine[*length] = 0x0D ;
                     *index += 2 ;
	             break ;

	 case '"'  : charLine[*length] = 0x22 ;
                     *index += 2 ;
	             break ;

	 case '\'' : charLine[*length] = 0x27 ;
                     *index += 2 ;
	             break ;

	 case '\\' : charLine[*length] = 0x5C ;
                     *index += 2 ;
	             break ;

	 case '0'  :
	 case '1'  :
	 case '2'  :
	 case '3'  :
	 case '4'  :
	 case '5'  :
	 case '6'  :
	 case '7'  : /* deal with octal number */
	             {
		      char nch = (ch - '0') ; /* starting point */
		      int  loop ;
	              /* upto three octal digits expected */
		      for (loop = 1; (loop < 4); loop++)
		       {
			char dch = line[*index + 1 + loop] ;
			if ((dch < '0') || (dch > '7'))
			 break ; /* out of the for loop */
			nch = ((nch << 3) | (dch - '0')) ;
		       }
                      charLine[*length] = nch ;
                      *index += 1 + loop ;
		     }
	             break ;

	 default   : charLine[*length] = ch ;
                     (*index)++ ;
	             break ;
        }
      }
     else
      {
       charLine[*length] = ch ;
       (*index)++ ;
      }
#else
     charLine[*length] = ch ;
     (*index)++ ;
#endif
    }
   else
    if (line[*index+1] != Quotes)
     {
      (*index)++ ; /* Point beyond closing quote */
      ALLOCATE(&string,*length) ; /* Get a string to put it in */
      if (*length != 0)
       memcpy(string,charLine,*length) ;
      return(string) ;
     }
    else
     {
      charLine[*length] = Quotes ;
      *index += 2 ;
     }

   (*length)++ ;
  } while (1) ;
} /* End GetString */

/*---------------------------------------------------------------------------*/
/* Read a word type operator, ie. one delimited by colons */
Operator GetWordOperator(char *line,CARDINAL *index,EHeliosPatch *hDIR)
{
 Name     name ;
 CARDINAL value ;

 if (SymbolTest(line,index,&name) && ((line[*index] == Space) || (line[*index] == Colon)) && NameLookup(operatorTable,name,FALSE,&value,ODummy))
  {
   /* ensure the caller knows about Helios directives */
   if ((value >= ModOff) && (value <= MsbOff))
    {
     if (hDIR->type != NoPatch)
      Warning(MultipleHDir) ;
     else
      switch (value)
       {
        case ModOff : hDIR->type = ModOffPatch ;
                      break ;
        case Offset : hDIR->type = OffsetPatch ;
                      break ;
        case LsbOff : hDIR->type = LsbOffPatch ;
                      break ;
        case MidOff : hDIR->type = MidOffPatch ;
                      break ;
        case MsbOff : hDIR->type = MsbOffPatch ;
                      break ;
       }
    }

   (*index)++ ;
   return value ;
  } ;
 Warning(BadOp) ;
 return ODummy ;
} /* End GetWordOperator */

/*---------------------------------------------------------------------------*/

static void PCOperand(EStackEntry *result)
{
 result->type = OperandEST ;
 result->u.operand.operandType = PCRelOT ;
 result->u.operand.u.pLabel = programCounter ;
} /* End PCOperand */

/*---------------------------------------------------------------------------*/

void AtOperand(EStackEntry *result)
{
 CARDINAL i ;

 result->type = OperandEST ;
 switch (variableCounter.type)
  {
   case FixedVCT    :
                      result->u.operand.operandType = ConstantOT ;
                      result->u.operand.u.constant = variableCounter.u.offset ;
                      break ;

   case RelativeVCT :
                      result->u.operand.operandType = RegRelOT ;
                      result->u.operand.u.regRel.offset = variableCounter.u.relativeVC.offset ;
                      for (i = 0; (i <= 15); i++)
                       result->u.operand.u.regRel.registers[i] = 0 ;
                      result->u.operand.u.regRel.registers[variableCounter.u.relativeVC.reg] = 1 ;
  } ; /* case */
} /* End AtOperand */

/*---------------------------------------------------------------------------*/
/* Read a word type operand, ie. one delimited by curly brackets */
static void GetWordOperand(char *line,CARDINAL *index,EStackEntry *result,EHeliosPatch *hDIR)
{
 Name     name ;
 CARDINAL value ;

 if (SymbolTest(line,index,&name) && (line[*index] == CurlyKet) && NameLookup(operandTable,name,FALSE,&value,ODummy))
  {
   (*index)++ ;
   switch (value)
    {
     case PCWO     :
                     PCOperand(result) ;
                     break ;

     case AtWO     :
                     AtOperand(result) ;
                     break ;

     case OptWO    :
                     result->type = OperandEST ;
                     result->u.operand.operandType = ConstantOT ;
                     if ((1 << ListPC) & listStatus)
                      result->u.operand.u.constant = pass - 1 + (2 - pass) * 1024 ;
                     else
                      result->u.operand.u.constant = (pass - 1) * 2 + (2 - pass) * 2048 ;

                     if ((1 << ListCondPC) & listStatus)
                      result->u.operand.u.constant += 4096 ;
                     else
                      result->u.operand.u.constant += 8192 ;

                     if ((1 << ListSETPC) & listStatus)
                      result->u.operand.u.constant += 16 ;
                     else
                      result->u.operand.u.constant += 32 ;

                     if ((1 << ListMacExpPC) & listStatus)
                      result->u.operand.u.constant += 64 ;
                     else
                      result->u.operand.u.constant += 128 ;

                     if ((1 << ListMacCallPC) & listStatus)
                      result->u.operand.u.constant += 256 ;
                     else
                      result->u.operand.u.constant += 512 ;
                     break ;

     case TrueWO   :
     case FalseWO  :
                     result->type = OperandEST ;
                     result->u.operand.operandType = LogicalOT ;
                     result->u.operand.u.bool = TrueWO == value ;
                     break ;

     case ModoffWO : /* always returns 0x00000000 */
                     hDIR->type = ModOffWOPatch ;
                     result->type = OperandEST ;
                     result->u.operand.operandType = ConstantOT ;
                     result->u.operand.u.constant = 0x00000000 ;
                     break ;
    } ; /* case */
  }
 else
  Warning(UnkOp) ;
} /* End GetWordOperand */

/*---------------------------------------------------------------------------*/
/* Get a token in the general form returning it in "result" */
void Token(char *line,CARDINAL *index,EStackEntry *result,BOOLEAN *defined,EHeliosPatch *hDIR)
{
 char          ch ;
 char          topDigit ;
 CARDINAL      i ;
 CARDINAL      length ;
 CARDINAL      base ;
 Name          name ;
 Name          nname ;
 char          nametext[MaxStringSize] ;
 NamePointer   namePointer ;
 SymbolPointer symbolPointer ;

 nname.key = nametext ;

 *defined = TRUE ; /* Assume will be defined */
 result->type = OperatorEST ; /* Assume we're getting an operator */
 if (AllComment(line, index))
  {
   result->u.operator = STTop ;
   return ;
  } ;

 ch = line[*index] ;
 if (isdigit(ch))
  {
   /* Handle number */
   result->type = OperandEST ;
   result->u.operand.operandType = ConstantOT ;
   if (line[*index+1] == UnderLine)
    {
     /* Handle based integer */
     topDigit = ch ;
     base = ch - '0' ;
     *index += 2 ;
     ch = line[*index] ;
     result->u.operand.u.constant = 0 ;
     if ((ch < '0') || (ch >= topDigit))
      {
       Warning(BadBaseNum) ;
       return ;
      } ; /* if */
     do
      {
       (*index)++ ;
       if ((MAXCARD - (ch - '0')) / base < result->u.operand.u.constant)
        {
         Warning(NumOver) ;
         return ;
        } ; /* if */
       result->u.operand.u.constant = result->u.operand.u.constant*base + ch - '0' ;
       ch = line[*index] ;
      } while ((ch >= '0') && (ch < topDigit)) ; /* repeat */
    }
   else
    {
     result->u.operand.u.constant = DecimalNumber(line,index) ;
     /* Now be a bit careful about as style local label references */
    } ; /* if */
  }
 else /* deal with non-numeric values */
  if ((isalpha(ch) || (ch == UnderLine) || (ch == Bar)) && SymbolTest(line,index,&name))
   {
    /* deal with "symbols" */
    result->type = OperandEST ;

    if (hDIR->type != NoPatch)
     {
#if 1
      nametext[0] = 128 ;
#else
      /* use the "_" prefixed label */
      nametext[0] = '_' ;
#endif
      memcpy(&nametext[1],name.key,name.length) ;
      nametext[1 + name.length] = '\0' ;
      nname.length = strlen(nametext) ;
      symbolPointer = LookupLocal(nname) ;

      if (symbolPointer == NULL)
       symbolPointer = LookupRef(nname,TRUE) ;

      if (symbolPointer == NULL)
       symbolPointer = LookupFixed(nname,TRUE) ;
     }
    else
     {
      symbolPointer = LookupLocal(name) ;

      if (symbolPointer == NULL)
       symbolPointer = LookupRef(name,TRUE) ;

      if (symbolPointer == NULL)
       symbolPointer = LookupFixed(name,TRUE) ;
     }

    AddUse(symbolPointer) ;

    /* Treat IMPORTed symbols as special cases when used with the special
     * Helios directives.
     */

    if ((hDIR->type != NoPatch) && ((symbolPointer->u.s.at == ExportAT) || (symbolPointer->u.s.at == ExportedAT) || (symbolPointer->u.s.at == HDataAT) || (symbolPointer->u.s.at == HCodeAT) || (symbolPointer->u.s.at == HDataExportAT) || (symbolPointer->u.s.at == HDataExportedAT) || (symbolPointer->u.s.at == HCodeExportAT) || (symbolPointer->u.s.at == HCodeExportedAT)))
     {
      hDIR->symbol = symbolPointer ;
      *defined = TRUE ;
      result->u.operand.operandType = DPRelOT ;
     }
    else
     {

    if (symbolPointer->u.s.sds != DefinedSDS)
     {
      /* Specified symbol is not defined, see if we have a "_" prefixed
       * "HDataAT" or "HCodeAT" symbol of the same name.
       */
#ifdef DEBUG
      printf("Token: \"") ;
      PrintSymbol(name) ;
      printf("\" not defined\n") ;
#endif

#if 1
      nametext[0] = 128 ;
#else
      nametext[0] = '_' ;
#endif
      memcpy(&nametext[1],name.key,name.length) ;
      nametext[1 + name.length] = '\0' ;
      nname.length = strlen(nametext) ;
      symbolPointer = LookupLocal(nname) ;

      if (symbolPointer == NULL)
       symbolPointer = LookupRef(nname,TRUE) ;

      if (symbolPointer == NULL)
       symbolPointer = LookupFixed(nname,TRUE) ;

      if ((symbolPointer->u.s.at != NoneAT) && ((symbolPointer->u.s.sdt == ExternalSDT) || ((symbolPointer->u.s.sdt== FixedSDT) && (symbolPointer->u.s.fst == ModuleFST))))
       {
        *defined = TRUE ;
        result->u.operand.operandType = DPRelOT ;
        result->u.operand.u.hSymbol = symbolPointer ;
       }
      else
       {
        /* Cannot find a symbol close to the desired one */
        *defined = FALSE ;
        result->u.operand.operandType = UnDefOT ;
       }
     }
    else
     {
      switch (symbolPointer->u.s.sdt)
       {
        case FixedSDT : switch (symbolPointer->u.s.fst)
                         {
                          case RelocatableFST :
                                                result->u.operand.operandType = PCRelOT ;
                                                result->u.operand.u.pLabel = symbolPointer->value.card ;
                                                break ;

                          case ModuleFST      : /* data area label */
                                                result->u.operand.operandType = DPRelOT ;
                                                result->u.operand.u.hSymbol = symbolPointer ;
                                                break ;

                          case AbsoluteFST    : result->u.operand.operandType = ConstantOT ;
                                                result->u.operand.u.constant = symbolPointer->value.card ;
                                                break ;

                          case RegisterRelativeFST : result->u.operand.operandType = RegRelOT ;
                                                     for (i = 0; i <= 15; i++)
                                                      result->u.operand.u.regRel.registers[i] = 0 ;
                                                     result->u.operand.u.regRel.registers[symbolPointer->u.s.fsr] = 1 ;
                                                     result->u.operand.u.regRel.offset = symbolPointer->value.card ;

                         } ; /* case */
                        break ;

        case ExternalSDT : Warning(ExtNotVal) ;
                           *defined = FALSE ;
                           break ;

        case VariableSDT : switch (symbolPointer->u.s.vst)
                            {
                             case ArithmeticVST : result->u.operand.operandType = ConstantOT ;
                                                  result->u.operand.u.constant = symbolPointer->value.card ;
                                                  break ;

                             case LogicalVST    : result->u.operand.operandType = LogicalOT ;
                                                  result->u.operand.u.bool = symbolPointer->value.bool ;
                                                  break ;

                             case StringVST     : result->u.operand.operandType = StringOT ;
                                                  namePointer = symbolPointer->value.ptr ;
                                                  if ((namePointer != NULL) && (namePointer->length != 0))
                                                   {
                                                    result->u.operand.u.string.length = namePointer->length ;
                                                    ALLOCATE(&result->u.operand.u.string.key, namePointer->length) ;
                                                    memcpy(result->u.operand.u.string.key, namePointer->key,namePointer->length) ;
                                                    /* String variables are not very efficient! */
                                                   }
                                                  else
                                                   {
                                                    result->u.operand.u.string.length = 0 ; /* The null string */
                                                    result->u.operand.u.string.key = NULL ;
                                                   } ; /* if */
                            } ; /* case */
                           break ;

        case RegisterNameSDT : result->u.operand.operandType = ConstantOT ;
                               result->u.operand.u.constant = symbolPointer->value.card ;
       } ; /* case */
     }
     }
   }
  else
   {
    (*index)++ ;
    switch (ch)
     {
      case '"'   : result->type = OperandEST ;
                   result->u.operand.operandType = StringOT ;
                   result->u.operand.u.string.key = GetString(line,index,&length) ;
                   result->u.operand.u.string.length = length ;
                   break ;

      case Quote : result->type = OperandEST ;
                   result->u.operand.operandType = ConstantOT ;
                   result->u.operand.u.constant = *index ;
                   if ((line[*index] == CR) || (line[++(*index)] != Quote))
                    Warning(MissQuote) ;
                   else
                    (*index)++ ;
                   break ;

      case '%'   :
                     {
                      result->type = OperandEST ;
                      {
                       CARDINAL area ;
                       result->u.operand.u.pLabel = LabelUse(line, index, defined, &area) ;
                       result->u.operand.operandType = PCRelOT ;
                      }
                     } ; /* if */
                    break ;

       case '&'   :
                   result->type = OperandEST ;
                   result->u.operand.operandType = ConstantOT ;
                   result->u.operand.u.constant = HexNumber(line,index) ;
                   break ;

       case '('  : result->u.operator = Bra ;
                   break ;

       case ')'  : result->u.operator = Ket ;
                   break ;

       case '*'  : result->u.operator = Star ;
                   break ;

       case '+'  : result->u.operator = Plus ; /*Could be unary*/
                   break ;

       case '-'  : result->u.operator = Minus ; /*Could be unary*/
                   break ;

       case '/'  : if (line[*index] == '=')
                    {
                     (*index)++ ;
                     result->u.operator = NotEquals ;
                    }
                   else
                    result->u.operator = Slash ;
                   break ;

       case ':'  : result->u.operator = GetWordOperator(line,index,hDIR) ;
                   break ;

       case '<'  : ch = line[*index] ;
                   if (ch == '=')
                    {
                     result->u.operator = LessEquals ;
                     (*index)++ ;
                    }
                   else
                    if (ch == '>')
                     {
                      result->u.operator = NotEquals ;
                      (*index)++ ;
                     }
                    else
                     if (ch == '<')
                      {
                       result->u.operator = Shl ;
                       (*index)++ ;
                      }
                     else
                      result->u.operator = Less ;
                   break ;

       case '='  : result->u.operator = OpEquals ;
                   break ;

       case '>'  : ch = line[*index] ;
                   if (ch == '<')
                    {
                     (*index)++ ;
                     result->u.operator = NotEquals ;
                    }
                   else
                    if (ch == '=')
                     {
                      (*index)++ ;
                      result->u.operator = GreaterEquals ;
                     }
                    else
                     if (ch == '>')
                      {
                       result->u.operator = Shr ;
                       (*index)++ ;
                      }
                     else
                      result->u.operator = Greater;
                   break ;

       case '?'  : result->type = OperandEST ;
                   result->u.operand.operandType = ConstantOT ;
                   result->u.operand.u.constant = 0 ;
                   if (!SymbolTest(line, index, &name))
                    {
                     Warning(NoSym) ;
                     return ;
                    } ; /* if */
                   symbolPointer = LookupLocal(name) ;
                   if (symbolPointer == NULL)
                    symbolPointer = LookupRef(name,TRUE) ;
                   if (symbolPointer == NULL)
                    symbolPointer = LookupFixed(name,TRUE) ;
                   AddUse(symbolPointer) ;
                   if (symbolPointer->u.s.sds != DefinedSDS)
                    *defined = FALSE ;
                   else
                    result->u.operand.u.constant = symbolPointer->length ;
                   break ;

       case '{'  : GetWordOperand(line,index,result,hDIR) ;
                   break ;

       case '.'  : PCOperand(result) ;
                   break ;

       case '@'  : AtOperand(result) ;
                   break ;

       case '|'  : result->u.operator = BOr ;
                   break ;

       case '~'  : result->u.operator = BNot ;
                   break ;

       default   : result->u.operator = STTop ;
                   (*index)-- ;
      } ; /* case */
    } ; /* if */
} /* End Token */

/*---------------------------------------------------------------------------*/
/* Initialise the word token table */

void InitTokens(void)
{
 Operator    i ;
 WordOperand j ;
 if (!initialised)
  {
#ifdef DEBUG
   printf("DEBUG: InitTokens: Initialising operatorTable and operandTable\n") ;
#endif

#ifdef __NCCBUG
   operatorTable = (Name *)mymalloc(ODummy * sizeof(Name)) ;
   operandTable = (Name *)mymalloc(DummyWO * sizeof(Name)) ;
#endif

   for (i = STBot; (i <= ODummy); i++)
    operatorTable[i].length = 0 ;

   CopyName(LAnd,    "LAND",   operatorTable) ;
   CopyName(LOr,     "LOR",    operatorTable) ;
   CopyName(LEor,    "LEOR",   operatorTable) ;
   CopyName(Ror,     "ROR",    operatorTable) ;
   CopyName(Rol,     "ROL",    operatorTable) ;
   CopyName(Shr,     "SHR",    operatorTable) ;
   CopyName(Shl,     "SHL",    operatorTable) ;
   CopyName(BAnd,    "AND",    operatorTable) ;
   CopyName(BEor,    "EOR",    operatorTable) ;
   CopyName(BOr,     "OR",     operatorTable) ;
   CopyName(Left,    "LEFT",   operatorTable) ;
   CopyName(Right,   "RIGHT",  operatorTable) ;
   CopyName(Cc,      "CC",     operatorTable) ;
   CopyName(Mod,     "MOD",    operatorTable) ;
   CopyName(LNot,    "LNOT",   operatorTable) ;
   CopyName(BNot,    "NOT",    operatorTable) ;
   CopyName(Len,     "LEN",    operatorTable) ;
   CopyName(Chr,     "CHR",    operatorTable) ;
   CopyName(Str,     "STR",    operatorTable) ;
   CopyName(Base,    "BASE",   operatorTable) ;
   CopyName(Index,   "INDEX",  operatorTable) ;
   CopyName(ModOff,  "MODOFF", operatorTable) ;
   CopyName(Offset,  "OFFSET", operatorTable) ;
   CopyName(LsbOff,  "LSBOFF", operatorTable) ;
   CopyName(MidOff,  "MIDOFF", operatorTable) ;
   CopyName(MsbOff,  "MSBOFF", operatorTable) ;
   CopyName(ODummy,  "||",    operatorTable) ;

   for (j = PCWO; (j <= DummyWO); j++)
    operandTable[j].length = 0 ;

   CopyName(PCWO,    "PC",    operandTable) ;
   CopyName(AtWO,    "VAR",   operandTable) ;
   CopyName(OptWO,   "OPT",   operandTable) ;
   CopyName(TrueWO,  "TRUE",  operandTable) ;
   CopyName(FalseWO, "FALSE", operandTable) ;
   CopyName(ModoffWO,"MODOFF",operandTable) ;
   CopyName(DummyWO, "||",    operandTable) ;
   initialised = TRUE ;
  } ; /* if */
} /* End InitTokens */

/*---------------------------------------------------------------------------*/
/* EOF tokens/c */
