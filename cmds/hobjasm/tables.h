/* -> tables/h
 * Title:               The symbol tables stuff for all symbols
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
/*---------------------------------------------------------------------------*/

#ifndef tables_h
#define tables_h

#include "nametype.h"
#define MaxSymbol 2048

/*Bits for the symbol status*/
typedef enum SymbolDefinitionType {
  FixedSDT,        /*Label type*/
  ExternalSDT,     /* Externally defined */
  VariableSDT,     /*Assembly time variable*/
  RegisterNameSDT  /*Name of a register*/
  } SymbolDefinitionType;

typedef enum VariableSymbolType {
  ArithmeticVST,/*Standard numerical value*/
  LogicalVST,   /*Boolean value*/
  StringVST     /*String of characters type*/
  } VariableSymbolType;

typedef enum FixedSymbolType {
   /*These types are also applicable to externals*/
  RelocatableFST,       /* Program labels etc. */
  ModuleFST,            /* module data area labels etc. */
  AbsoluteFST,          /* Fixed constants */
  RegisterRelativeFST   /* DSECT types */
  } FixedSymbolType;

typedef int FixedSymbolRegister; /* [0..14] Register to relocate via*/

typedef enum SymbolDefinitionStatus {
  UndefinedSDS,  /*Seen but no definition attempted yet*/
  PartDefinedSDS,/*Definition attempted but failed for undefined symbols*/
  UndefinableSDS,/*Definition failed for syntax error etc.*/
  DefinedSDS     /*Value known up to relocation/registerbase*/
  } SymbolDefinitionStatus;

typedef enum SymbolReferenceState {
  UnreferencedSRS,/*Never used*/
  ReferencedSRS   /*A use has been made of this symbol*/
  } SymbolReferenceState;

typedef enum SymbolMayForceLiteral {
  HasNotForcedSMFL,/*No unnecessary literal table entry allocated*/
  HasForcedSMFL    /*Possible unnecessary literal table entry allocated*/
  } SymbolMayForceLiteral;

typedef enum RegisterType {
  IntRT,  /*Ordinary ARM register*/
  RealRT, /*Floating point register*/
  CopNRT, /*Coprocessor number*/
  CopRRT  /*Coprocessor register number*/
  } RegisterType;

typedef enum AOFType {
  NoneAT,
  HDataAT,              /* static data area symbol */
  HCodeAT,              /* static function pointer area symbol */
  HDataExportAT,        /* static data area symbol to be exported */
  HDataExportedAT,      /* static data area symbol that has been exported */
  HCodeExportAT,        /* static function pointer symbol to be exported */
  HCodeExportedAT,      /* static function pointer that has been exported */
  KeptAT,
  ExportAT,
  ExportedAT
  } AOFType;

/*
Ordering of the above types in Symbol.Status:
bits   0-1: SymbolDefinitionType
bits  2-10: Unused
bits 11-12: VariableSymbolType
bits 13-14: FixedSymbolType
bits 15-18: FixedSymbolRegister
bits 19-20: SymbolDefinitionStatus
bit     21: SymbolReferenceState
bit     22: SymbolMayForceLiteral
bits 23-24: Register type (Coproc number, coproc register, FP or ordinary)
bits 25-28: AOF type
*/

/*
CONST SDTShift  = 1;

CONST VSTShift  = 800H;

CONST FSTShift  = 2000H;

CONST FSRShift  = 8000H;

CONST SDSShift  = 80000H;

CONST SRSShift  = 200000H;

CONST SMFLShift = 400000H;

CONST RTShift   = 800000H;

CONST AOFTShift = 2000000H;
*/

typedef int SymbolName; /* [0..MaxSymbol-1] */

typedef enum ValueType { CardVT, BoolVT, PtrVT } ValueType;

typedef union {
/*
   CASE ValueType
   OF
*/
  /* case CardVT */
  CARDINAL card;

  /* case BoolVT */
  BOOLEAN  bool;

  /* case PtrVT */
  NamePointer ptr;

} Value;

typedef struct AOFData {
                        CARDINAL stringPosition ; /* offset from the start of the string table */
                        int      symbolId ;
                       } AOFData ; /* record */

/* Stuff for cross reference work */

typedef enum Context { Macro, File } Context;

typedef struct OccStart *OccStartPtr;

typedef struct Occurrence *OccPtr;

typedef struct Occurrence {
  Context context;

  union {
    /* Case File */
    Name file;

    /* Case Macro */
    Name macro;
  } u; /* case */
  CARDINAL line; /* The line number */
  OccPtr newContext;
}; /* record */

typedef struct OccStart {
  OccPtr      occ;
  OccStartPtr next;
}; /* record */

typedef struct Symbol *SymbolPointer;

typedef union status {
  CARDINAL status;/*A mixed bag of bits containing the attributes from MOD*/

  struct {
  unsigned int sdt:2, /* bits   0-1: SymbolDefinitionType */
               pad:9,
               vst:2, /* bits 11-12: VariableSymbolType */
               fst:2, /* bits 13-14: FixedSymbolType */
               fsr:4, /* bits 15-18: FixedSymbolRegister */
               sds:2, /* bits 19-20: SymbolDefinitionStatus */
               srs:1, /* bit     21: SymbolReferenceState */
               smfl:1,/* bit     22: SymbolMayForceLiteral */
               rt:2,  /* bits 23-24: Register type
                         (Coproc number, coproc register, FP or ordinary) */
               at:4;  /* bits 25-28: symbol type */
  } s;
} Status;

typedef struct Symbol {
                       Name          key ; /* symbol name and string length */
                       Status        u ;
                       Value         value ;
                       CARDINAL      length ; /* The length attribute */
                       AOFData       aOFData ;
                       SymbolPointer link ;
                       OccStartPtr   defPtr ;
                       OccStartPtr   usePtr     ;
                      } Table_Symbol ; /* record */

#ifdef __NCCBUG
extern SymbolPointer *symbolTable ;
#else
extern SymbolPointer symbolTable[MaxSymbol] ;
#endif

void InitSymbolTable(void);

SymbolPointer LookupFixed(Name name, BOOLEAN reference);

SymbolPointer LookupRef(Name name, BOOLEAN reference);

SymbolPointer DefineReg(Name name);

SymbolPointer DefineFPReg(Name name);

SymbolPointer DefineCoprocName(Name name);

SymbolPointer DefineCoprocReg(Name name);

SymbolPointer DefineGlobalA(Name name);

SymbolPointer DefineGlobalL(Name name);

SymbolPointer DefineGlobalS(Name name);

SymbolPointer LookupGlobalA(Name name);

SymbolPointer LookupGlobalL(Name name);

SymbolPointer LookupGlobalS(Name name);

SymbolPointer LookupExternal(Name name);

void AddImports(void);
/* In as mode mark all undefined symbols as externals. Called end of pass 1 */

void PutImplicitImports(void);
/* In as export all the symbols that were undefined. Called end of pass 2 */

void SetImplicitRegisters(void);
/* Setup as style register names */

void hRemove(SymbolPointer symbolPointer);
/*Clear out remains of previous assembly*/

#endif

/*---------------------------------------------------------------------------*/
/* End tables/h */
