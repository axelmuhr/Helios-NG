/* -> tables/c
 * Title:               The symbol tables stuff for all symbols
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
/*---------------------------------------------------------------------------*/

#include "code.h"
#include "errors.h"
#include "globvars.h"
#include "nametype.h"
#include "stats.h"
#include "store.h"
#include "tables.h"
#include "occur.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*---------------------------------------------------------------------------*/

#define APCS_A1 0       /* arg1 */
#define APCS_A2 1       /* arg2 */
#define APCS_A3 2       /* arg3 */
#define APCS_A4 3       /* arg4 */
#define APCS_V1 4       /* variable1 */
#define APCS_V2 5       /* variable2 */
#define APCS_V3 6       /* variable3 */
#define APCS_V4 7       /* variable4 */
#define APCS_V5 8       /* variable5 */
#define APCS_DP 9       /* module table pointer */
#define APCS_SL 10      /* stack limit */
#define APCS_FP 11      /* frame pointer */
#define APCS_IP 12      /* intermediate pointer */
#define APCS_SP 13      /* stack pointer */
#define APCS_LR 14      /* link register */
#define APCS_PC 15      /* program counter */

/*---------------------------------------------------------------------------*/

#ifdef __NCCBUG
SymbolPointer *symbolTable = NULL ;
#else
SymbolPointer symbolTable[MaxSymbol];
#endif

/* Ordering of the above types in Symbol.Status:
 *  bits   0-1: SymbolDefinitionType
 *  bit  11-12: VariableSymbolType (relevant only for variable symbols)
 *  bits 12-13: FixedSymbolType
 *  bits 14-17: FixedSymbolRegister
 *  bits 18-19: SymbolDefinitionStatus
 *  bit     20: SymbolReferenceState
 *  bit     21: SymbolMayForceLiteral
 */

static BOOLEAN initialised = FALSE ;

/*---------------------------------------------------------------------------*/

static SymbolName SymbolHash(Name name)
{
 CARDINAL hash = 0 ;
 CARDINAL j = 5 ;
 int      i ;
 int      end ;

 if (name.length < 5)
  j = name.length ;

 for (i= 0; (i < j); i++)
  hash = hash * 2 + name.key[i] ;

 end = name.length - j ;
 for (i = name.length-1; i >= end; i--)
  hash = hash * 2 + name.key[i] ;

 return((hash + name.length) % MaxSymbol) ;
}

/*---------------------------------------------------------------------------*/

void InitSymbolTable(void)
{
 CARDINAL i ;

#ifdef DEBUG
 printf("DEBUG: InitSymbolTable: entered\n") ;
#endif

#ifdef __NCCBUG
 if (symbolTable == NULL)
  symbolTable = (SymbolPointer *)mymalloc(MaxSymbol * sizeof(SymbolPointer)) ;
#endif

 if (initialised)
  for (i = 0; i < MaxSymbol; i++)
   hRemove(symbolTable[i]) ;

 for (i = 0; i < MaxSymbol; i++)
  symbolTable[i] = NULL ;
  
 initialised = TRUE ;

#ifdef DEBUG
 printf("DEBUG: InitSymbolTable: exiting\n") ;
#endif

 return ;
}

/*---------------------------------------------------------------------------*/

static void InsertSymbol(SymbolPointer *symbolPointer,Name name,Status flags)
{
#ifdef DEBUG
 printf("DEBUG: InsertSymbol: symbolPointer = &%08X\n",(int)symbolPointer) ;
#endif /* DEBUG */

 *symbolPointer = mymalloc(sizeof(**symbolPointer)) ;
 /* Produce a new instance of a symbol */
 (*symbolPointer)->u = flags ;
 (*symbolPointer)->link = NULL ;        /* End of chain */
 (*symbolPointer)->defPtr = NULL ;      /* No cross reference stuff yet */
 (*symbolPointer)->usePtr = NULL ;
 (*symbolPointer)->key.length = name.length ;  /* Length of expected symbol */
 (*symbolPointer)->key.key = mymalloc(name.length) ; /* Get the store for it */
 memcpy((*symbolPointer)->key.key, name.key, name.length) ;
 (*symbolPointer)->length = 0 ;
 (*symbolPointer)->aOFData.symbolId = 0x8000 ; /* Uninitialised */

 return ;
}

/*---------------------------------------------------------------------------*/
/* Do the standard lookup, returning the first matching symbol.
 * If the symbol is not found and insert is TRUE it is inserted into the table
 * with all attributes set to status.
 */
static void Lookup(Name name,SymbolPointer *result,BOOLEAN insert,Status status)
{
 CARDINAL    j ;
 CARDINAL    k ;
 SymbolName  hash = SymbolHash(name) ;
 char       *s1 ;

 *result = symbolTable[hash] ; /* Get the first one */
 if ((*result == NULL) && insert)
  {
   InsertSymbol(&symbolTable[hash],name,status) ;
   *result = symbolTable[hash] ;
   AddAccess(1) ;
   return ;
  }

#ifdef DEBUG
 printf("Lookup: *result = &%08X\n",(int)*result) ;
#endif

 j = 0 ;
 k = name.length ;
 s1 = name.key ;
 do
  {
   /* Something back to front here? */  /* CHECK OUT FULLY SOME-TIME */
   if (*result == NULL)
    {
#ifdef DEBUG
     printf("Lookup: *result == NULL\n") ;
#endif
     return ;
     AddAccess(j) ;     /***** CHECK... maybe should be in front *****/
    } ;
   /* See if the test name is the same as the current symbol, and chain if not */
   j++ ;
   if ((k == (*result)->key.length) && (memcmp(s1,(*result)->key.key,k) == 0))
    {
#ifdef DEBUG
     printf("Lookup: returning *result = &%08X\n",(int)*result) ;
#endif
     return ;
    }
   if (((*result)->link == NULL) && insert)
    {
     InsertSymbol(&(*result)->link,name,status) ;
     *result = (*result)->link ;
     AddAccess(j + 1) ;
     return ;
    } ; /* if */
   *result = (*result)->link ; /* Step to next in chain */
  } while (1) ; /* loop */

 /* never reached */
 return ;
}

/*---------------------------------------------------------------------------*/

SymbolPointer DefineReg(Name name)
{
 SymbolPointer result ;
 Status        status ;

 /* Lookup and insert if not there */
 status.status = 0 ;
 Lookup(name,&result,FALSE,status) ;
 if (result != NULL)
  return NULL ;

 status.s.sdt = RegisterNameSDT ;
 status.s.sds = DefinedSDS ;
 status.s.rt = IntRT ;
 Lookup(name,&result,TRUE,status) ;
 if (result->u.s.sdt == RegisterNameSDT)
  {
   result->length = 0 ;
   return(result) ;
  }
 else
  {
   /* Symbol was there but wrong type */
   return(NULL) ;
  }

 /* never reached */
 return(NULL) ;
}

/*---------------------------------------------------------------------------*/

SymbolPointer DefineFPReg(Name name)
{
 SymbolPointer result ;
 Status        status ;

 /* Lookup and insert if not there */
 status.status = 0 ;
 Lookup(name,&result,FALSE,status) ;
 if (result != NULL)
  return(NULL) ;

 status.s.sdt = RegisterNameSDT ;
 status.s.sds = DefinedSDS ;
 status.s.rt = RealRT ;
 Lookup(name,&result,TRUE,status) ;
 if (result->u.s.sdt == RegisterNameSDT)
  {
   result->length = 0 ;
   return(result) ;
  }
 else
  {
   /* Symbol was there but wrong type */
   return(NULL) ;
  }
 /* never reached */
 return(NULL) ;
}

/*---------------------------------------------------------------------------*/

SymbolPointer DefineCoprocReg(Name name)
{
 SymbolPointer result ;
 Status        status ;

 /* Lookup and insert if not there */
 status.status = 0 ;
 Lookup(name,&result,FALSE,status) ;
 if (result != NULL)
  return(NULL) ;

 status.s.sdt = RegisterNameSDT ;
 status.s.sds = DefinedSDS ;
 status.s.rt = CopRRT ;
 Lookup(name,&result,TRUE,status) ;
 if (result->u.s.sdt == RegisterNameSDT)
  {
   result->length = 0 ;
   return result ;
  }
 else
  {
   /* Symbol was there but wrong type */
   return(NULL) ;
  }
 /* never reached */
 return(NULL) ;
}

/*---------------------------------------------------------------------------*/

SymbolPointer DefineCoprocName(Name name)
{
 SymbolPointer result ;
 Status        status ;

 /* Lookup and insert if not there */
 status.status = 0 ;
 Lookup(name,&result,FALSE,status) ;
 if (result != NULL)
  return(NULL) ;

 status.s.sdt = RegisterNameSDT ;
 status.s.sds = DefinedSDS ;
 status.s.rt = CopNRT ;
 Lookup(name,&result,TRUE,status) ;
 if (result->u.s.sdt == RegisterNameSDT)
  {
   result->length = 0 ;
   return(result) ;
  }
 else
  {
   /* Symbol was there but wrong type */
   return(NULL) ;
  }
 /* never reached */
 return(NULL) ;
}

/*---------------------------------------------------------------------------*/

SymbolPointer LookupFixed(Name name,BOOLEAN reference)
{
 SymbolPointer result ;
 Status        status ;

 /* Lookup and insert if not there */
 status.status = 0 ;
 status.s.sdt = FixedSDT ;
 Lookup(name,&result,TRUE,status) ;
 if (result->u.s.sdt == FixedSDT)
  {
   if (reference)
    result->u.s.srs = ReferencedSRS ;
   return result ;
  }
 else
  return NULL ; /* Symbol was there but wrong type */
}

/*---------------------------------------------------------------------------*/
/* Lookup a reference without inserting into the table */

SymbolPointer LookupRef(Name name,BOOLEAN reference)
{
 SymbolPointer result ;
 Status        status ;

#ifdef DEBUG
 printf("LookupRef: entered with \"") ;
 PrintSymbol(name) ;
 printf("\"\n") ;
#endif

 status.status = 0 ;
 Lookup(name,&result,FALSE,status) ;

#ifdef DEBUG
 printf("LookupRef: result = &%08X \"",(int)result) ;
 PrintSymbol(result->key) ;
 printf("\"\n") ;
#endif

 if (reference && (result != NULL))
  result->u.s.srs = ReferencedSRS ;

 return(result) ;
} /* End LookupRef */

/*---------------------------------------------------------------------------*/

SymbolPointer LookupExternal(Name name)
{
 SymbolPointer result ;
 Status        status ;

 status.status = 0 ;
 status.s.sdt = ExternalSDT ;
 status.s.at = ExportAT ;       /* we are IMPORTing this symbol */
 Lookup(name,&result,TRUE,status) ;
 result->length = 0 ;
 result->value.card = 0 ;
 return result ;
} /* End LookupExternal */

SymbolPointer DefineGlobalA(Name name)
{
  SymbolPointer result;
  Status        status;

  /*Lookup and insert if not there*/
  status.s.sdt = VariableSDT;
  status.s.sds = DefinedSDS;
  status.s.vst = ArithmeticVST;
  Lookup(name, &result, TRUE, status);
  if ((result->u.s.sdt == VariableSDT) && (result->u.s.vst == ArithmeticVST)) {
     result->length = 0;
     result->value.card = 0;
     return result;
  } else {
     /*Symbol was there but wrong type*/
     return NULL;
  };
} /* End DefineGlobalA */

/*---------------------------------------------------------------------------*/

SymbolPointer DefineGlobalL(Name name)
{
 SymbolPointer result ;
 Status        status ;

 /* Lookup and insert if not there */
 status.status = 0 ;
 status.s.sdt = VariableSDT ;
 status.s.sds = DefinedSDS ;
 status.s.vst = LogicalVST ;
 Lookup(name,&result,TRUE,status) ;
 if ((result->u.s.sdt == VariableSDT) && (result->u.s.vst == LogicalVST))
  {
   result->length = 0 ;
   result->value.bool = FALSE ;
   return result ;
  }
 else
  {
   /* Symbol was there but wrong type */
   return NULL ;
  } ;
} /* End DefineGlobalL */

SymbolPointer DefineGlobalS(Name name)
{
  SymbolPointer result;
  Status        status;

  /*Lookup and insert if not there*/
  status.status = 0;
  Lookup(name, &result, FALSE, status);
  if (result == NULL) {
    status.s.sdt = VariableSDT;
    status.s.sds = DefinedSDS;
    status.s.vst = StringVST;
    Lookup(name, &result, TRUE, status);
    result->length = 0;
    result->value.ptr = mymalloc(sizeof(*result->value.ptr));
    result->value.ptr->length = 0;
    result->value.ptr->key = NULL;
    result->value.ptr->maxLength = 0;
    return result;
  } else if ((result->u.s.sdt == VariableSDT) &&
    (result->u.s.vst == StringVST)) {
    result->length = 0;
    result->value.ptr->length = 0;
    return result;
  } else {
   /*Symbol was there but wrong type*/
   return NULL;
   };
} /* End DefineGlobalS */

/*---------------------------------------------------------------------------*/

SymbolPointer LookupGlobalA(Name name)
{
  SymbolPointer result;
  Status        status;

  status.status = 0;
  Lookup(name, &result, FALSE, status);
  if ((result != NULL) && (result->u.s.sdt == VariableSDT) &&
     (result->u.s.vst == ArithmeticVST)) {
     result->length = 0;
     return result;
  } else {
     /*Symbol was there but wrong type*/
     return NULL;
  }; /* if */
} /* End LookupGlobalA */

/*---------------------------------------------------------------------------*/

SymbolPointer LookupGlobalL(Name name)
{
  SymbolPointer result;
  Status        status;

  status.status = 0;
  Lookup(name, &result, FALSE, status);
  if ((result != NULL) && (result->u.s.sdt == VariableSDT) &&
     (result->u.s.vst == LogicalVST)) {
     result->length = 0;
     return result;
  } else {
     /*Symbol was there but wrong type*/
     return NULL;
  }; /* if */
} /* End LookupGlobalL */

/*---------------------------------------------------------------------------*/

SymbolPointer LookupGlobalS(Name name)
{
  SymbolPointer result;
  Status        status;

  status.status = 0;
  Lookup(name, &result, FALSE, status);
  if ((result != NULL) && (result->u.s.sdt == VariableSDT) &&
     (result->u.s.vst == StringVST)) {
     result->length = 0;
     return result;
  } else {
     /*Symbol was there but wrong type*/
     return NULL;
  }; /* if */
} /* End LookupGlobalS */

/*---------------------------------------------------------------------------*/

#if 0 /* JGS : I do not believe this is required anymore */
/* In "as" mode, mark all undefined symbols as externals */
void AddImports(void)
{
 int i ;
 for (i = 0; (i < MaxSymbol); i++)
  {
   SymbolPointer symbol_pointer = symbolTable[i] ;
   while (symbol_pointer != NULL)
    {
     if ((symbol_pointer->u.s.sdt == FixedSDT) && (symbol_pointer->u.s.sds == UndefinedSDS))
      {
       symbol_pointer->u.s.sdt = ExternalSDT ;
#if 1
       symbol_pointer->value.bool = FALSE ; /* not EXCEPTION */
#else
       symbol_pointer->value.bool = FALSE ; /* Not weak */
#endif
       if (symbol_pointer->aOFData.symbolId == 0x8000)
        {
         AddSymbol(symbol_pointer) ;
        } /* if */
      } /* if */
     symbol_pointer = symbol_pointer->link ;
    } /* while */
  } /* for */
 return ;
} /* End AddImports */
#endif

/*---------------------------------------------------------------------------*/

/* In "as" export all the symbols that were undefined. Called end of pass 2 */
void PutImplicitImports(void)
{
 int i;

 for (i = 0; i < MaxSymbol; i++)
  {
   SymbolPointer symbol_pointer = symbolTable[i] ;
   while (symbol_pointer != NULL)
    {
     if ((symbol_pointer->u.s.sdt == ExternalSDT) && (symbol_pointer->u.s.at == NoneAT))
      {
       AddSymbolToTable(symbol_pointer,symbol_pointer->key,TRUE,TRUE) ;
       symbol_pointer->u.s.at = ExportedAT ;
      } ; /* if */
     symbol_pointer = symbol_pointer->link ;
    } ; /* while */
  } ; /* for */
} /* End PutImplicitImports */

/*---------------------------------------------------------------------------*/

static void SetImplicitCPUReg(char *name, CARDINAL value)
{
Name          reg_name;
SymbolPointer sym;
reg_name.key = name;
reg_name.length = strlen(name);
sym = DefineReg(reg_name);
if (sym == NULL) AssemblerError("Implicit AS style register already exists\n");
sym->value.card = value;
}

/*---------------------------------------------------------------------------*/

static void SetImplicitFPReg(char *name, CARDINAL value)
{
Name          reg_name;
SymbolPointer sym;
reg_name.key = name;
reg_name.length = strlen(name);
sym = DefineFPReg(reg_name);
if (sym == NULL) AssemblerError("Implicit AS style register already exists\n");
sym->value.card = value;
}

/*---------------------------------------------------------------------------*/

static void SetImplicitCoprocReg(char *name, CARDINAL value)
{
Name          reg_name;
SymbolPointer sym;
reg_name.key = name;
reg_name.length = strlen(name);
sym = DefineCoprocReg(reg_name);
if (sym == NULL) AssemblerError("Implicit AS style register already exists\n");
sym->value.card = value;
}

/*---------------------------------------------------------------------------*/

static void SetImplicitCoprocName(char *name, CARDINAL value)
{
Name          reg_name;
SymbolPointer sym;
reg_name.key = name;
reg_name.length = strlen(name);
sym = DefineCoprocName(reg_name);
if (sym == NULL) AssemblerError("Implicit AS style register already exists\n");
sym->value.card = value;
}

/*---------------------------------------------------------------------------*/

void SetImplicitRegisters(void)
/* Setup as style register names */
{
SetImplicitCPUReg("r0", 0);
SetImplicitCPUReg("r1", 1);
SetImplicitCPUReg("r2", 2);
SetImplicitCPUReg("r3", 3);
SetImplicitCPUReg("r4", 4);
SetImplicitCPUReg("r5", 5);
SetImplicitCPUReg("r6", 6);
SetImplicitCPUReg("r7", 7);
SetImplicitCPUReg("r8", 8);
SetImplicitCPUReg("r9", 9);
SetImplicitCPUReg("r10", 10);
SetImplicitCPUReg("r11", 11);
SetImplicitCPUReg("r12", 12);
SetImplicitCPUReg("r13", 13);
SetImplicitCPUReg("r14", 14);
SetImplicitCPUReg("r15", 15);

/* The APCS register cpu names */

SetImplicitCPUReg("a1", APCS_A1);
SetImplicitCPUReg("a2", APCS_A2);
SetImplicitCPUReg("a3", APCS_A3);
SetImplicitCPUReg("a4", APCS_A4);
SetImplicitCPUReg("v1", APCS_V1);
SetImplicitCPUReg("v2", APCS_V2);
SetImplicitCPUReg("v3", APCS_V3);
SetImplicitCPUReg("v4", APCS_V4);
SetImplicitCPUReg("v5", APCS_V5);
SetImplicitCPUReg("dp", APCS_DP);
SetImplicitCPUReg("sl", APCS_SL);
SetImplicitCPUReg("fp", APCS_FP);
SetImplicitCPUReg("ip", APCS_IP);
SetImplicitCPUReg("sp", APCS_SP);
SetImplicitCPUReg("lr", APCS_LR);
SetImplicitCPUReg("pc", APCS_PC);

/* Floating pointer coproc register names */
SetImplicitFPReg("f0", 0);
SetImplicitFPReg("f1", 1);
SetImplicitFPReg("f2", 2);
SetImplicitFPReg("f3", 3);
SetImplicitFPReg("f4", 4);
SetImplicitFPReg("f5", 5);
SetImplicitFPReg("f6", 6);
SetImplicitFPReg("f7", 7);

/* Predefined co-processor names */
SetImplicitCoprocName("p0", 0);
SetImplicitCoprocName("p1", 1);
SetImplicitCoprocName("p2", 2);
SetImplicitCoprocName("p3", 3);
SetImplicitCoprocName("p4", 4);
SetImplicitCoprocName("p5", 5);
SetImplicitCoprocName("p6", 6);
SetImplicitCoprocName("p7", 7);
SetImplicitCoprocName("p8", 8);
SetImplicitCoprocName("p9", 9);
SetImplicitCoprocName("p10", 10);
SetImplicitCoprocName("p11", 11);
SetImplicitCoprocName("p12", 12);
SetImplicitCoprocName("p13", 13);
SetImplicitCoprocName("p14", 14);
SetImplicitCoprocName("p15", 15);

/*Predefinedco-processorregisternames*/
SetImplicitCoprocReg("c0", 0);
SetImplicitCoprocReg("c1", 1);
SetImplicitCoprocReg("c2", 2);
SetImplicitCoprocReg("c3", 3);
SetImplicitCoprocReg("c4", 4);
SetImplicitCoprocReg("c5", 5);
SetImplicitCoprocReg("c6", 6);
SetImplicitCoprocReg("c7", 7);
SetImplicitCoprocReg("c8", 8);
SetImplicitCoprocReg("c9", 9);
SetImplicitCoprocReg("c10", 10);
SetImplicitCoprocReg("c11", 11);
SetImplicitCoprocReg("c12", 12);
SetImplicitCoprocReg("c13", 13);
SetImplicitCoprocReg("c14", 14);
SetImplicitCoprocReg("c15", 15);
} /* End SetImplicitRegisters */

/*---------------------------------------------------------------------------*/

static void RemoveContext(OccPtr occ)
/* Clear out the context information for one symbol occurrence */
{
  OccPtr next;
  while (occ != NULL) {
    if ((occ->context == File) && (occ->u.file.length != 0)) {
      free(occ->u.file.key);
    }; /* if */
    next = occ->newContext;
    free(occ);
    occ = next;
  }; /* while */
} /* End RemoveContext */

/*---------------------------------------------------------------------------*/

static void RemoveOcc(OccStartPtr occ)
/* Clear out a chain of occurrences for a symbol */
{
  OccStartPtr next;
  while (occ != NULL) {
    RemoveContext(occ->occ);
      /* Get rid of the context chain for the occurrence*/
    next = occ->next;
    free(occ);
    occ = next;
  }; /* while */
} /* End RemoveOcc */

/*---------------------------------------------------------------------------*/

void hRemove(SymbolPointer symbolPointer)
/*Clear out remains of previous assembly*/
{
  SymbolPointer temp;
  while (symbolPointer != NULL) {
    /*Get rid of pointed to object*/
    temp = symbolPointer->link;

    /*Get rid of reference chains*/
    RemoveOcc(symbolPointer->defPtr);
    RemoveOcc(symbolPointer->usePtr);

    /*Now get rid of current object*/

    /*First the Name part*/
    free(symbolPointer->key.key);

    /*Now possible string part*/
    if ((symbolPointer->u.s.sdt == VariableSDT) &&
       (symbolPointer->u.s.vst == StringVST)) {
       if (symbolPointer->value.ptr->maxLength != 0) {
         free(symbolPointer->value.ptr->key);
       }; /* if */
       free(symbolPointer->value.ptr);
    }; /* if */

    /*Now the record*/
    free(symbolPointer);
    symbolPointer = temp;
  }; /* while */
} /* End hRemove */

/*---------------------------------------------------------------------------*/
/* EOF tables/c */
