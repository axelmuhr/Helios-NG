/* -> opcode/c
 * Title:               The opcode detection routine
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
/*---------------------------------------------------------------------------*/

#include "formatio.h"
#include "globvars.h"
#include "nametype.h"
#include "opcode.h"
#include "store.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/*---------------------------------------------------------------------------*/

typedef struct {
                Name       key ;
                CARDINAL   value ;
                OpcodeType type ;
               } OpcodeEntry ;

typedef struct {
                Name     key ;
                CARDINAL value ;
               } CondEntry ;

#define TableSize 512
#define CondSize  64

/*---------------------------------------------------------------------------*/
/* Conditions */
#define CondShift 0x10000000
#define EQCond    0x00000000
#define NECond    0x10000000
#define CSCond    0x20000000
#define CCCond    0x30000000
#define MICond    0x40000000
#define PLCond    0x50000000
#define VSCond    0x60000000
#define VCCond    0x70000000
#define HICond    0x80000000
#define LSCond    0x90000000
#define GECond    0xA0000000
#define LTCond    0xB0000000
#define GTCond    0xC0000000
#define LECond    0xD0000000
#define ALCond    0xE0000000
#define NVCond    0xF0000000

/* Group 1 */
#define ANDCode   0x00000000
#define EORCode   0x00200000
#define RSBCode   0x00600000
#define ADCCode   0x00A00000
#define SBCCode   0x00C00000
#define RSCCode   0x00E00000
#define TSTCode   0x01100000
#define TEQCode   0x01300000
#define CMPCode   0x01500000
#define CMNCode   0x01700000
#define ORRCode   0x01800000
#define BICCode   0x01C00000
#define MVNCode   0x01E00000
#define ANDSCode  0x00100000
#define EORSCode  0x00300000
#define SUBSCode  0x00500000
#define RSBSCode  0x00700000
#define ADDSCode  0x00900000
#define ADCSCode  0x00B00000
#define SBCSCode  0x00D00000
#define RSCSCode  0x00F00000
#define TSTSCode  TSTCode
#define TEQSCode  TEQCode
#define CMPSCode  CMPCode
#define CMNSCode  CMNCode
#define TSTPCode  0x0110F000
#define TEQPCode  0x0130F000
#define CMPPCode  0x0150F000
#define CMNPCode  0x0170F000
#define ORRSCode  0x01900000
#define MOVSCode  0x01B00000
#define BICSCode  0x01D00000
#define MVNSCode  0x01F00000

/* Group 2 */
#define LDRCode   0x04100000
#define STRCode   0x04000000
#define LDRBCode  0x04500000
#define STRBCode  0x04400000
#define LDRTCode  0x04300000
#define STRTCode  0x04200000
#define LDRBTCode 0x04700000
#define STRBTCode 0x04600000

/* Group 3 */
#define SWICode   0x0F000000

/* Group 4 */
#define LDMEDCode 0x09900000
#define STMEDCode 0x08000000
#define LDMFDCode 0x08900000
#define STMFDCode 0x09000000
#define LDMEACode 0x09100000
#define STMEACode 0x08800000
#define LDMFACode 0x08100000
#define STMFACode 0x09800000
#define LDMDBCode LDMEACode
#define STMDBCode STMFDCode
#define LDMDACode LDMFACode
#define STMDACode STMEDCode
#define LDMIBCode LDMEDCode
#define STMIBCode STMFACode
#define LDMIACode LDMFDCode
#define STMIACode STMEACode
#define IncBit    0x00800000
#define PreBit    0x01000000

/* Group 5 */
#define BCode     0x0A000000
#define BLCode    0x0B000000

/* Group 6*/
#define ADRCode   0x020F0000
#define ADRSCode  0x021F0000

/*Group 7*/
#define LDFCode  0x0C100100
#define STFCode  0x0C000100

/*Group 8*/
#define ADFCode  0x0E000100
#define MUFCode  0x0E100100
#define SUFCode  0x0E200100
#define RSFCode  0x0E300100
#define DVFCode  0x0E400100
#define RDFCode  0x0E500100
#define POWCode  0x0E600100
#define RPWCode  0x0E700100
#define RMFCode  0x0E800100
#define FMLCode  0x0E900100
#define FDVCode  0x0EA00100
#define FRDCode  0x0EB00100
#define POLCode  0x0EC00100

/*Group 9*/
#define MVFCode  0x0E008100
#define MNFCode  0x0E108100
#define ABSCode  0x0E208100
#define RNDCode  0x0E308100
#define SQTCode  0x0E408100
#define LOGCode  0x0E508100
#define LGNCode  0x0E608100
#define EXPCode  0x0E708100
#define SINCode  0x0E808100
#define COSCode  0x0E908100
#define TANCode  0x0EA08100
#define ASNCode  0x0EB08100
#define ACSCode  0x0EC08100
#define ATNCode  0x0ED08100

/*Group 10*/
#define CMFCode  0x0E90F110
#define CNFCode  0x0EB0F110
#define CMFECode  0x0ED0F110
#define CNFECode  0x0EF0F110

/*Group 11*/
#define FLTCode  0x0E000110

/*Group 12*/
#define FIXCode  0x0E100110

/*Group 13*/
#define WFSCode  0x0E200110
#define RFSCode  0x0E300110
#define WFCCode  0x0E400110
#define RFCCode  0x0E500110

/*Group 14*/
#define LDCCode   0x0C100000
#define STCCode   0x0C000000
#define LDCLCode  0x0C500000
#define STCLCode  0x0C400000

/*Group 15*/
#define CDPCode   0x0E000000

/*Group 16*/
#define MCRCode   0x0E000010	/* ARM to coproc transfer */
#define MRCCode   0x0E100010	/* coproc to ARM transfer */

/*Group 18*/
#define MULCode   0x00000090
#define MULSCode  0x00100090

/*Group 18*/
#define MLACode   0x00200090
#define MLASCode  0x00300090

/*Group 19*/
#define ADRLCode  0x020F0000

/*Group 20*/
#define SWPCode  0x01000090

/*Extra bits*/
/*For data operations*/
#define SCode 0x00000
#define DCode 0x00080
#define ECode 0x80000
#define PCode 0x20
#define MCode 0x40
#define ZCode 0x60
/*For data transfer*/
#define SingleCode 0x000000
#define DoubleCode 0x008000
#define ExtendCode 0x400000
#define PackedCode 0x408000

/*---------------------------------------------------------------------------*/

static BOOLEAN initialised = FALSE ;

#ifdef __NCCBUG
CondEntry *conditions = NULL ;
OpcodeEntry *opcodes = NULL ;
#else
CondEntry   conditions[CondSize] ;
OpcodeEntry opcodes[TableSize] ;
#endif

/*---------------------------------------------------------------------------*/

static CARDINAL OpcodeHash(Name name)
{
 return ((name.key[0]-0x40) ^ (16*(name.key[1]-0x40)) ^ (8*(name.key[2]-0x40))) & ((TableSize) - 1) ;
} /* End OpcodeHash */

/*---------------------------------------------------------------------------*/

static void Conditional(CARDINAL index,CARDINAL *condition,Name *name,char *opcodeChars)
{
 char     *op = name->key+index ;
 CARDINAL  i = ((32*(*op-0x40) + op[1]-0x40) >> 2) & ((CondSize) - 1) ;

 {
  CondEntry *cond = &(conditions[i]) ;
  if ((cond->key.length == 2) && (*op == *cond->key.key) && (op[1] == cond->key.key[1]))
   {
    *condition = cond->value ;
    /* Only copy opcode if there is a conditional in the middle */
    memcpy(opcodeChars,name->key,index) ;
    memcpy(opcodeChars + index,name->key + index + 2,name->length - index - 2) ;
    name->length -= 2 ;
    name->key = opcodeChars ;
   }
 }
} /* End Conditional */

/*---------------------------------------------------------------------------*/
/* The result is TRUE if the name is an opcode */
BOOLEAN Opcode(Name name,OpcodeType *opcodeType,CARDINAL *opcodeValue)
{
 CARDINAL conditionalPart = ALCond ;
 CARDINAL index ;
 CARDINAL i ;
 char     opcodeChars[7] ;
 char     ch ;

 switch (name.length)
  {
   case 1  : if (*name.key == 'B')
              {
               *opcodeValue = BCode + conditionalPart ;
               *opcodeType = Branch ;
               return(TRUE) ;
              }
             return(FALSE) ;

   case 2  : if ((*name.key == 'B') && (name.key[1] == 'L'))
              {
               *opcodeValue = BLCode + conditionalPart ;
               *opcodeType = Branch ;
               return(TRUE) ;
              }
             return(FALSE) ;

   case 3  : if (*name.key == 'B')
              Conditional(1,&conditionalPart,&name,opcodeChars) ;
             break ;

   case 4  : if ((*name.key == 'B') && (name.key[1] == 'L')) 
              Conditional(2,&conditionalPart,&name,opcodeChars) ;
             break ;

   case 5  :
   case 6  :
   case 7  : Conditional(3,&conditionalPart,&name,opcodeChars) ;
             break ;

   default : return(FALSE) ; /* not an opcode */
  } /* switch */

 /* Now we are ready to look up the opcode without conditional parts */
 switch (name.length)
  {
   case 1  : if (*name.key == 'B')
              {
               *opcodeValue = BCode + conditionalPart ;
               *opcodeType = Branch ;
               return TRUE ;
              }

   case 2  : if ((*name.key == 'B') && (name.key[1] == 'L'))
              {
               *opcodeValue = BLCode + conditionalPart ;
               *opcodeType = Branch ;
               return TRUE ;
              }

   case 3  :
   case 4  :
   case 5  : index = OpcodeHash(name) ;
             {
              OpcodeEntry *opc = &opcodes[index] ;
              if ((opc->key.length != 0) && (memcmp(name.key,opc->key.key,3) == 0))
               {
                *opcodeValue = opc->value + conditionalPart ;
                *opcodeType = opc->type ;
                switch (name.length)
                 {
                  case 3  : switch (opc->type)
                             {
                              case DataProcessing :
                              case DataTransfer   :
                              case SWI            :
                              case Adr            :
                              case FPCompare      :
                              case FPFloat        :
                              case FPFix          :
                              case FPStatus       :
                              case CPRT           :
                              case CPDT           :
                              case CPDO           :
                              case MUL            :
                              case MLA            :
                              case SWP            :
                                                    break ;

                              case Branch         :
                              case BlockData      :
                              case FPDataTransfer :
                              case FPDyadic       :
                              case FPMonadic      :
                              case ADRL           :
                                                    return FALSE ;
                             } /* switch */
                            break ;

                  case 4  : ch = (name.key[3]) ;
                            switch (opc->type)
                             {
                              case DataProcessing : if (ch == 'S')
                                                     *opcodeValue |= ANDSCode ;
                                                    else
                                                     if ((ch == 'P') && (opc->value >= TSTCode) && (opc->value <= CMNCode))
                                                      *opcodeValue += TSTPCode-TSTCode ;
                                                     else
                                                      return FALSE ;
                                                    break ;

                              case DataTransfer   :
                              case SWP            : if (ch == 'B')
                                                     *opcodeValue += LDRBCode-LDRCode ;
                                                    else
                                                     if ((ch == 'T') && (opc->type != SWP))
                                                      *opcodeValue += LDRTCode-LDRCode ;
                                                     else
                                                      return FALSE ;
                                                    break ;

                              case Adr            : if (ch == 'L')
                                                     {
                                                      *opcodeValue = ADRLCode + conditionalPart ;
                                                      *opcodeType = ADRL ;
                                                     }
                                                    else
                                                     return FALSE ;
                                                    break ;

                              case MUL            :
                              case MLA            : if (ch == 'S')
                                                     *opcodeValue += MULSCode-MULCode ;
                                                    else
                                                     return FALSE ;
                                                    break ;

                              case FPDataTransfer : if (ch == 'S')
                                                     *opcodeValue += SingleCode ;
                                                    else
                                                     if (ch == 'D')
                                                      *opcodeValue += DoubleCode ;
                                                     else
                                                      if (ch == 'E')
                                                       *opcodeValue += ExtendCode ;
                                                      else
                                                       if (ch == 'P')
                                                        *opcodeValue += PackedCode ;
                                                       else
                                                        return FALSE ;
                                                    break ;

                              case FPDyadic       :
                              case FPMonadic      :
                              case FPFloat        :
                              case FPFix          : switch (ch)
                                                     {
                                                      case 'M' : *opcodeValue += MCode ;
                                                                 break ;
  
                                                      case 'P' : *opcodeValue += PCode ;
                                                                 break ;
  
                                                      case 'Z' : *opcodeValue += ZCode ;
                                                                 break ;

                                                      case 'S' : *opcodeValue += SCode ;
                                                                 break ;

                                                      case 'D' : *opcodeValue += DCode ;
                                                                 break ;

                                                      case 'E' : *opcodeValue += ECode ;
                                                                 break ;

                                                      default  : return FALSE ;
                                                     } /* switch */
                                                    break ;

                              case FPCompare      : if (ch == 'E')
                                                     *opcodeValue += CMFECode-CMFCode ;
                                                    else
                                                     return FALSE ;
                                                    break ;

                              case CPDT           : if (ch == 'L')
                                                     *opcodeValue += LDCLCode-LDCCode ;
                                                    else
                                                     return FALSE ;
                                                    break ;

                              default             : return FALSE ;
                             } /* switch */
                            break ;
        
                  case 5  : switch (opc->type)
                             {
                              case DataTransfer   : if ((name.key[3] == 'B') && (name.key[4] == 'T'))
                                                     *opcodeValue += LDRBTCode-LDRCode ;
                                                    else
                                                     return FALSE ;
                                                    break ;
          
                              case BlockData      : ch = name.key[4] ;
                                                    switch (name.key[3])
                                                     {
                                                      case 'D' : if (ch == 'B')
                                                                  *opcodeValue += PreBit ;
                                                                  else
                                                                   if (ch != 'A')
                                                                    return FALSE ;
                                                                 break ;

                                                      case 'E' : i = PreBit ;
                                                                 if (ch == 'D')
                                                                  i += IncBit ;
                                                                 else
                                                                  if (ch != 'A')
                                                                   return FALSE ;
                                                                 if (opc->value == STMDACode)
                                                                  i ^= PreBit | IncBit ;
                                                                 *opcodeValue += i ;
                                                                 break ;

                                                      case 'F' : i = 0 ;
                                                                 if (ch == 'D')
                                                                  i += IncBit ;
                                                                 else
                                                                  if (ch != 'A')
                                                                   return FALSE ;
                                                                 if (opc->value == STMDACode)
                                                                  i ^= PreBit | IncBit ;
                                                                 *opcodeValue += i ;
                                                                 break ;

                                                      case 'I' : *opcodeValue += IncBit ;
                                                                 if (ch == 'B')
                                                                  *opcodeValue += PreBit ;
                                                                 else
                                                                  if (ch != 'A')
                                                                   return FALSE ;
                                                                 break ;

                                                      default  : return FALSE ;
                                                     } /* switch */
                                                    break ;
          
                              case FPDyadic       :
                              case FPMonadic      :
                              case FPFloat        :
                              case FPFix          : switch (name.key[3])
                                                     {
                                                      case 'D' : *opcodeValue += DCode ;
                                                                 break ;

                                                      case 'E' : *opcodeValue += ECode ;
                                                                 break ;

                                                      case 'S' :
                                                                 break ;

                                                      default  : return FALSE ;
                                                     } /* switch */
                                                    switch (name.key[4])
                                                     {
                                                      case 'M' : *opcodeValue += MCode ;
                                                                 break ;

                                                      case 'P' : *opcodeValue += PCode ;
                                                                 break ;

                                                      case 'Z' : *opcodeValue += ZCode ;
                                                                 break ;

                                                      default  : return FALSE ;
                                                     } /* switch */
                                                    break ;

                              default             : return FALSE ;
                             } /* switch */
        
                  } /* switch */
                 if ((*opcodeType <= Adr) || allowFP || (*opcodeType >= CPDT))
                  {
                   if ((*opcodeType >= FPDataTransfer) && (*opcodeType <= FPStatus))
                    hadFP = TRUE ;
                   return TRUE ;
                  }
               }
    }
  } /* switch */
 return(FALSE) ;
} /* End Opcode */

/*---------------------------------------------------------------------------*/
/* Initialise an entry in an opcode table */

void InitTable(char *chars,CARDINAL value,OpcodeType type,OpcodeEntry *table)
{
 CARDINAL index ;
 Name     name ;

 name.length = strlen(chars) ;
 name.key = chars ;
 index = OpcodeHash(name) ;
 while (table[index].key.length != 0)
  {
   WriteChs("Non-first time hash on ") ;
   WriteChs(chars) ;
   WriteChs("\\N") ;
   index++ ;
   if (index >= TableSize)
    index = 0 ;
  }
 table[index].key.length = strlen(chars) ;
 table[index].key.key = mymalloc(table[index].key.length) ;
 memcpy(table[index].key.key,chars,table[index].key.length) ;
 table[index].value = value ;
 table[index].type = type ;
} /* End InitTable */

/*---------------------------------------------------------------------------*/

static void InitCond(char *chars,CARDINAL condValue)
{
 CARDINAL index = ((32*(chars[0]-0x40) + chars[1]-0x40) >> 2) & (CondSize -1) ;
 if (conditions[index].key.length != 0)
  {
   WriteChs("Conditional clash at ") ;
   WriteChs(chars) ;
   WriteChs("\\N") ;
  }
 conditions[index].value = condValue ;
 conditions[index].key.length = 2 ;
 conditions[index].key.key = mymalloc(2) ;
 conditions[index].key.key[0] = chars[0] ;
 conditions[index].key.key[1] = chars[1] ;
} /* End InitCond */

/*---------------------------------------------------------------------------*/

void InitOpcode(void)
{
 CARDINAL i ;

 if (!initialised)
  {
#ifdef __NCCBUG
   opcodes = (OpcodeEntry *)mymalloc(TableSize * sizeof(OpcodeEntry)) ;
   conditions = (CondEntry *)mymalloc(CondSize * sizeof(CondEntry)) ;
#endif

   for (i = 0; i < TableSize; i++)
    opcodes[i].key.length = 0 ;
   for (i = 0; i < CondSize; i++)
    conditions[i].key.length = 0 ;

   /* Initialise conditions */
   InitCond("EQ",EQCond) ;
   InitCond("NE",NECond) ;
   InitCond("CS",CSCond) ;
   InitCond("CC",CCCond) ;
   InitCond("MI",MICond) ;
   InitCond("PL",PLCond) ;
   InitCond("VS",VSCond) ;
   InitCond("VC",VCCond) ;
   InitCond("HI",HICond) ;
   InitCond("LS",LSCond) ;
   InitCond("GE",GECond) ;
   InitCond("LT",LTCond) ;
   InitCond("GT",GTCond) ;
   InitCond("LE",LECond) ;
   InitCond("AL",ALCond) ;
   InitCond("NV",NVCond) ;
   InitCond("HS",CSCond) ;
   InitCond("LO",CCCond) ;

   /* Initialise opcodes */
   InitTable("AND",ANDCode,  DataProcessing,opcodes) ;
   InitTable("EOR",EORCode,  DataProcessing,opcodes) ;
   InitTable("SUB",SUBCode,  DataProcessing,opcodes) ;
   InitTable("RSB",RSBCode,  DataProcessing,opcodes) ;
   InitTable("ADD",ADDCode,  DataProcessing,opcodes) ;
   InitTable("ADC",ADCCode,  DataProcessing,opcodes) ;
   InitTable("SBC",SBCCode,  DataProcessing,opcodes) ;
   InitTable("RSC",RSCCode,  DataProcessing,opcodes) ;
   InitTable("TST",TSTCode,  DataProcessing,opcodes) ;
   InitTable("TEQ",TEQCode,  DataProcessing,opcodes) ;
   InitTable("CMP",CMPCode,  DataProcessing,opcodes) ;
   InitTable("CMN",CMNCode,  DataProcessing,opcodes) ;
   InitTable("ORR",ORRCode,  DataProcessing,opcodes) ;
   InitTable("MOV",MOVCode,  DataProcessing,opcodes) ;
   InitTable("BIC",BICCode,  DataProcessing,opcodes) ;
   InitTable("MVN",MVNCode,  DataProcessing,opcodes) ;
   InitTable("LDR",LDRCode,  DataTransfer,  opcodes) ;
   InitTable("STR",STRCode,  DataTransfer,  opcodes) ;
   InitTable("SWI",SWICode,  SWI,           opcodes) ;
   InitTable("ADR",ADRCode,  Adr,           opcodes) ;
   InitTable("LDC",LDCCode,  CPDT,          opcodes) ;
   InitTable("STC",STCCode,  CPDT,          opcodes) ;
   InitTable("CDP",CDPCode,  CPDO,          opcodes) ;
   InitTable("MCR",MCRCode,  CPRT,          opcodes) ;
   InitTable("MRC",MRCCode,  CPRT,          opcodes) ;
   InitTable("MUL",MULCode,  MUL,           opcodes) ;
   InitTable("MLA",MLACode,  MLA,           opcodes) ;
   InitTable("STM",STMDACode,BlockData,     opcodes) ;
   InitTable("LDM",LDMDACode,BlockData,     opcodes) ;
   InitTable("LDF",LDFCode,  FPDataTransfer,opcodes) ;
   InitTable("STF",STFCode,  FPDataTransfer,opcodes) ;
   InitTable("ADF",ADFCode,  FPDyadic,      opcodes) ;
   InitTable("MUF",MUFCode,  FPDyadic,      opcodes) ;
   InitTable("SUF",SUFCode,  FPDyadic,      opcodes) ;
   InitTable("RSF",RSFCode,  FPDyadic,      opcodes) ;
   InitTable("DVF",DVFCode,  FPDyadic,      opcodes) ;
   InitTable("RDF",RDFCode,  FPDyadic,      opcodes) ;
   InitTable("POW",POWCode,  FPDyadic,      opcodes) ;
   InitTable("RPW",RPWCode,  FPDyadic,      opcodes) ;
   InitTable("RMF",RMFCode,  FPDyadic,      opcodes) ;
   InitTable("FML",FMLCode,  FPDyadic,      opcodes) ;
   InitTable("FDV",FDVCode,  FPDyadic,      opcodes) ;
   InitTable("FRD",FRDCode,  FPDyadic,      opcodes) ;
   InitTable("POL",POLCode,  FPDyadic,      opcodes) ;
   InitTable("MVF",MVFCode,  FPMonadic,     opcodes) ;
   InitTable("MNF",MNFCode,  FPMonadic,     opcodes) ;
   InitTable("ABS",ABSCode,  FPMonadic,     opcodes) ;
   InitTable("RND",RNDCode,  FPMonadic,     opcodes) ;
   InitTable("SQT",SQTCode,  FPMonadic,     opcodes) ;
   InitTable("LOG",LOGCode,  FPMonadic,     opcodes) ;
   InitTable("LGN",LGNCode,  FPMonadic,     opcodes) ;
   InitTable("EXP",EXPCode,  FPMonadic,     opcodes) ;
   InitTable("SIN",SINCode,  FPMonadic,     opcodes) ;
   InitTable("COS",COSCode,  FPMonadic,     opcodes) ;
   InitTable("TAN",TANCode,  FPMonadic,     opcodes) ;
   InitTable("ASN",ASNCode,  FPMonadic,     opcodes) ;
   InitTable("ACS",ACSCode,  FPMonadic,     opcodes) ;
   InitTable("ATN",ATNCode,  FPMonadic,     opcodes) ;
   InitTable("CMF",CMFCode,  FPCompare,     opcodes) ;
   InitTable("CNF",CNFCode,  FPCompare,     opcodes) ;
   InitTable("FLT",FLTCode,  FPFloat,       opcodes) ;
   InitTable("FIX",FIXCode,  FPFix,         opcodes) ;
   InitTable("WFS",WFSCode,  FPStatus,      opcodes) ;
   InitTable("RFS",RFSCode,  FPStatus,      opcodes) ;
   InitTable("WFC",WFCCode,  FPStatus,      opcodes) ;
   InitTable("RFC",RFCCode,  FPStatus,      opcodes) ;
   InitTable("SWP",SWPCode,  SWP,           opcodes) ;

   initialised = TRUE ;
  }
} /* End InitOpcode */

/*---------------------------------------------------------------------------*/
/* EOF opcode/c */
