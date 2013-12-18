/* -> code/h
 * Title:               Code output
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
#ifndef code_h
#define code_h

#include "constant.h"
#include "helios.h"
#include "globvars.h"
#include "nametype.h"
#include "asmvars.h"
#include "tables.h"

#include <stdio.h>

void RelocInit(void);

void RelocEnd(void);

void CodeByte(char byte);

void CodeWord(CARDINAL word);

void CodeInit(void);

void CodeEnd(void);

void AddSymbol(SymbolPointer symbolPointer);

void AddSymbolToTable(SymbolPointer symbolPointer,Name name,BOOLEAN external,BOOLEAN nonimp);

void P1InitAreas(void);

void P2InitAreas(void);

void DumpSymbolTable(void);

void DumpStringTable(void);

void DumpHeader(void);

void DumpAreaDecs(void);

extern CARDINAL               totalAreas;

void write_dotlabel(int type,char *text,int length) ;
void write_purelabel(int type,char *text,int length) ;
void write_label(int type,char *text,int length) ;
void write_patch(int type,CARDINAL instruction) ;
void write_bss(int size) ;
void write_init(void) ;
void write_staticarea(int type,int value,SymbolPointer sp) ;
void write_labeldir(SymbolPointer sp) ;
void write_labeldirname(Name *name) ;
void write_imagesize(void) ;
void write_modnum(void) ;
void write_modnum1(SymbolPointer sp) ;
void write_modnum2(SymbolPointer sp) ;
void write_modnum4(void) ;
void write_modnum5(void) ;
void write_dataref(SymbolPointer sp) ;
void write_datarefname(Name *name) ;
void write_coderef(SymbolPointer sp) ;
void write_coderefname(Name *name) ;

#endif

/* End code/h */
