/*
 * File:	ghof.c
 * Author:	P.A.Beskeen
 * Date:	Aug '91
 *
 * Description: Outputs a GHOF (Generic Helios Object Format) File from the
 *		contents of the parse tree. This code interfaces with the
 *		processor specific second pass via the Pass2() function for
 *		a given processor, which then uses the WriteCodeByte/Short/Word
 *		functions to pass the assembled binary instruction back to this
 *		object code formatter.
 *
 * RcsId: $Id: ghof.c,v 1.9 1993/07/12 16:16:55 nickc Exp $
 *
 * (C) Copyright 1991 Perihelion Software Ltd.
 * 
 * $RcsLog$
 *
 */

/* Include Files: */

#include "gasm.h"
#include "y.tab.h"		/* for token and patch type definitions */
#include "ghof.h"


/* Local Data: */

/* stores all code before being flushed */
static unsigned char	codebuffer[CODEBUFFERSIZE];

int	codesize = 0;	/* current amount of code buffered */


/* Internal Functions: */

static void GHOFConstList(ConstList *cl, int size, int pc);
static void GHOFFloatList(FloatList *fl, int size, int pc);
#ifdef __C40TARGET
static void GHOFC40FloatList(FloatList *fl, int size, int pc);
#endif
static void GHOFPatch(Patch *p, int pc);
#ifdef DEBUG
static void DebugGHOFConstList(ConstList *cl, int size, int pc);
static void DebugGHOFPatch(Patch *p, int pc);
#endif
static void GHOFOutputNameList(NameList *nlist, int type);
static int GetCodeWord(void);
static void OutputModuleHeader(void);
static void OutputModuleTailer(void);


/* The parse tree constructed by yyparse() is held as a series of chained */
/* ParseTreeItem structures that specify what will be output in the object */
/* code. Depending on its 'what' type, the parse item may point to other */
/* structures that define the object codes exact contents. */

/* Output a GHOF conformant object file from the contents of the parse tree */
void OutputGHOF(ParseTreeItem *pti)
{
	if (ModuleHeadTail)
		OutputModuleHeader();

	while (pti != NULL) {
		int	type = 0;
		int	pc = pti->logicalPC;

		/* errors to be associated with this line */
		CurLine = pti->linenum;

#ifdef DEBUG
		fprintf(stderr, "line %5d PC %#08x\n", CurLine, pti->logicalPC);
#endif

		switch (pti->what) {

		case SPACE:
			FlushCodeBuffer();
			GHOFEncode(GHOF_SPACE);
			GHOFEncode(pti->type.num);
			break;

		case BYTE:
			GHOFConstList(pti->type.clist, sizeof(char), pc);
			break;

		case SHORT:
			GHOFConstList(pti->type.clist, 2 * sizeof(char), pc);
			break;

		case WORD:
			GHOFConstList(pti->type.clist, sizeof(int), pc);
			break;

		case FLOATY:
			GHOFFloatList(pti->type.flist, 4, pc);
			break;

		case DOUBLE:
			GHOFFloatList(pti->type.flist, 8, pc);
			break;

#ifdef __C40TARGET
		case C40FLOAT:
			GHOFC40FloatList(pti->type.flist, 4, pc);
			break;
#endif

		case MODULE:
			FlushCodeBuffer();
			GHOFEncode(GHOF_MODULE);
			GHOFEncode(Eval(pti->type.expr, pc));
			break;

		case INIT:
			FlushCodeBuffer();
			GHOFEncode(GHOF_INIT);
			break;

		case LABEL:
			if (pti->type.symb == NULL)
				Fatal("NULL symbol table entry in label directive");
			if (pti->type.symb->name == NULL)
				Fatal("Empty string in symbol table");

			FlushCodeBuffer();
			/* @@@ maybe only do this if we explicitly export the label */
			GHOFEncode(GHOF_LABEL);
			ObjWrite(pti->type.symb->name, strlen(pti->type.symb->name)+1);
			break;

		case REF:
			GHOFOutputNameList(pti->type.nlist, GHOF_REF);
			break;

		case EXPORT:
			GHOFOutputNameList(pti->type.nlist, GHOF_EXPORT);
			break;

		case IMPORT:
			GHOFOutputNameList(pti->type.nlist, GHOF_EXPORT);
			break;

		case CODETABLE:
			GHOFOutputNameList(pti->type.nlist, GHOF_CODETABLE);
			break;

		case DATA:
			type = GHOF_DATA;
			goto OutputDataCommon;

		case COMMON:
			type = GHOF_COMMON;
OutputDataCommon:
			if (pti->type.datacommon.name == NULL)
				Fatal("data/common was passed NULL string");

			FlushCodeBuffer();
			GHOFEncode(type);
			GHOFEncode(Eval(pti->type.datacommon.expr, pc));
			ObjWrite(pti->type.datacommon.name, \
				strlen(pti->type.datacommon.name)+1);
			break;

		case PATCHINSTR:
			FlushCodeBuffer();

			/* Assume word sized instruction */
			GHOFEncode(GHOF_WORD);

			/* Output the machine specific patch */
			GHOFEncode(Eval(pti->type.patchinstr.mcpatch, pc));

			/* Output the assembled instruction as the m/c */
			/* patches data */
			Pass2(pti->type.patchinstr.instr, pti->logicalPC);
			/* Steal the assembled instruction back from the */
			/* code buffer */
			GHOFEncode(GetCodeWord());
			codesize = 0;	/* reset the code buffer */

			/* Output the patch that will provide the data */
			/* with which to patch the instruction in the */
			/* manor defined by the obove m/c patch */
			GHOFPatch(pti->type.patchinstr.patch, pc);

			break;

		case INSTRUCTION:
			Pass2(pti->type.instr, pti->logicalPC);
			break;

		default:
			{
				char Err[80];

				sprintf(Err, "Found unknown parse tree item %d", pti->what);
				Fatal(Err);

				break;
			}
		}

		pti = pti->next;	/* move on to next item */
	}

	FlushCodeBuffer();

	if (ModuleHeadTail)
		OutputModuleTailer();
}


static void GHOFOutputNameList(NameList *nlist, int type)
{
	if (nlist == NULL)
		Fatal("ref/import/export/codetable was passed NULL name list");

	FlushCodeBuffer();

	while (nlist != NULL) {
		GHOFEncode(type);
		ObjWrite(nlist->name, strlen(nlist->name)+1);

		nlist = nlist->next;
	}
}

static void GHOFConstList(ConstList *cl, int size, int pc)
{
	while (cl) {
		switch (cl->what) {

		case E_Expr:
			{
				int i = Eval(cl->type.expr, pc);

				switch (size) {
				case sizeof(char):
					if (i < -128 || i > 255) {
						char Err[80];

						sprintf(Err, "number %d is to big to fit in a byte", i);
						Error(Err);
					}
					WriteCodeByte(i);
					break;
				case sizeof(char) * 2 :
					if (i < -32768 || i > 65536) {
						char Err[80];

						sprintf(Err, "number %d is to big to fit in a short", i);
						Error(Err);
					}
					WriteCodeShort(i);
					break;
				case sizeof(int):
					WriteCodeWord(i);
					break;
				default:
					Fatal("Unknown constant size given in constant list");
				}
			break;
			}


		/* output non NULL terminated string */
		/* the size of each character equaling the constant size */
		case E_Str: {
			char *s = cl->type.str;

			switch (size) {
			case sizeof(char):
				while (*s)
					WriteCodeByte(*s++);
				break;
			case sizeof(char) * 2:
				while (*s)
					WriteCodeShort(*s++);
				break;
			case sizeof(int):
				while (*s)
					WriteCodeWord(*s++);
				break;
			default:
				Fatal("Unknown constant size given in constant list");
			}
			break;
			}

		/* patch a linker defined value into image file */
		case E_Patch:
			FlushCodeBuffer();

			switch (size) {
			case sizeof(char):
				GHOFEncode(GHOF_BYTE);
				break;
			case sizeof(char) * 2:
				GHOFEncode(GHOF_SHORT);
				break;
			case sizeof(int):
				GHOFEncode(GHOF_WORD);
				break;
			default:
				Fatal("Unknown constant size given in constant list");
			}

			GHOFPatch(cl->type.patch, pc);
			break;

		default:
			Fatal("Unknown type of constant found in constant list");
			break;
		}

		cl = cl->next;
	}
}

static void GHOFFloatList(FloatList *fl, int size, int pc)
{
	while (fl) {
		Dble d = fl->value;

		switch (size) {

		case 4:	/* (float) */
			WriteCodeFloat(d);
			break;

		case 8: /* (double) */
			WriteCodeDouble(d);
			break;

		default:
			Fatal("Unknown constant size given in float constant list");
		}

		fl = fl->next;
	}
}

#ifdef __C40TARGET
static void GHOFC40FloatList(FloatList *fl, int size, int pc)
{
	while (fl) {
		Dble d = fl->value;

		switch (size) {

		case 4:	/* (float) */
			WriteCodeC40Float(d);
			break;

#if 0		/* @@@ do not support 40 bit internal representation yet */
		case 8: /* (double) */
			WriteCodeDouble(d);
			break;
#endif

		default:
			Fatal("Unknown constant size given in float constant list");
		}

		fl = fl->next;
	}
}
#endif

static void GHOFPatch(Patch *p, int pc)
{
	switch(p->what) {
	case MODNUM:
		GHOFEncode(GHOF_MODNUM);
		break;

	case MODSIZE:
		GHOFEncode(GHOF_MODSIZE);
		break;

	case DATAMODULE:
		GHOFEncode(GHOF_DATAMODULE);
		ObjWrite(p->type.name, strlen(p->type.name)+1);
		break;

	case DATASYMB:
		GHOFEncode(GHOF_DATASYMB);
		ObjWrite(p->type.name, strlen(p->type.name)+1);
		break;

	case CODESYMB:
		GHOFEncode(GHOF_CODESYMB);
		ObjWrite(p->type.name, strlen(p->type.name)+1);
		break;

	case LABELREF:
		GHOFEncode(GHOF_LABELREF);
		ObjWrite(p->type.name, strlen(p->type.name)+1);
		break;

	case CODESTUB:
		GHOFEncode(GHOF_CODESTUB);
		ObjWrite(p->type.name, strlen(p->type.name)+1);
		break;

#ifdef __C40TARGET
	case ADDRSTUB:
		GHOFEncode(GHOF_ADDRSTUB);
		ObjWrite(p->type.name, strlen(p->type.name)+1);
		break;
#endif

	case BYTESWAP:
		GHOFEncode(PATCH_BYTESWAP);
		GHOFPatch(p->type.patch, pc);
		break;

	case SHIFT:
		GHOFEncode(PATCH_SHIFT);
		GHOFEncode(Eval(p->type.shift.expr, pc));
		GHOFPatch(p->type.shift.patch, pc);
		break;

	case P_ADD:
		GHOFEncode(PATCH_ADD);
		GHOFEncode(Eval(p->type.shift.expr, pc));
		GHOFPatch(p->type.shift.patch, pc);
		break;

	case P_OR:
		GHOFEncode(PATCH_OR);
		GHOFEncode(Eval(p->type.shift.expr, pc));
		GHOFPatch(p->type.shift.patch, pc);
		break;

	default:
		Fatal("Unknown patch type found in patch list");
	} /* switch patch */
}


static void OutputModuleHeader(void)
{
	/* Output a Helios module header */
	/* See Module structure defined in Helios <module.h> */

	/* Define module number. */
	GHOFEncode(GHOF_MODULE);
	GHOFEncode(ModuleNumber);

	/* Output module header. */
	WriteCodeWord(0x60f160f1);	/* T_Module */
	FlushCodeBuffer();
	GHOFEncode(GHOF_WORD);
	GHOFEncode(GHOF_MODSIZE);

	/* Write 32 bytes of module name. */
	if (*ModuleName == 0) {
		/* if not set by -n, default the name */
		if (OutputFileName == NULL)
			strcpy(ModuleName, "module");
		else {
			strncpy(ModuleName, OutputFileName, 31);
			ModuleName[31] = '\0';
		}
	}
	{
		int i;

		for(i = 0; i < 32;  i++)
			WriteCodeByte(ModuleName[i]);
	}
	FlushCodeBuffer();

	/* Module's number */
	GHOFEncode(GHOF_WORD);
	GHOFEncode(GHOF_MODNUM);

	/* Module's version */
	WriteCodeWord(ModuleVersion);
	FlushCodeBuffer();

	/* Module's static data area size */
	GHOFEncode(GHOF_WORD);
	GHOFEncode(GHOF_DATASYMB);
	ObjWrite(".MaxData", 9);
	FlushCodeBuffer();

	/* Module's data / code table initialisation chain root */
	GHOFEncode(GHOF_INIT);

	/* Module's code pointer table size */
	/* Only used in Split Module Table based systems (SMT) */
	GHOFEncode(GHOF_WORD);
	GHOFEncode(GHOF_CODESYMB);
	ObjWrite(".MaxCodeP", 10);
	FlushCodeBuffer();
}


static void OutputModuleTailer(void)
{
	/* Output tail information for this module */

	/* Output final data item, its offset is synonymous with */
	/* the static data size for the module. The Module header includes */
	/* this information via patching by the linker. */
	GHOFEncode(GHOF_DATA);
	GHOFEncode(0);
	ObjWrite(".MaxData", 9);

	/* Output final code pointer item, its offset is synonymous */
	/* with the code table size for the module */
	/* Only used in Split Module Table based systems (SMT) */
	GHOFEncode(GHOF_CODETABLE);
	ObjWrite(".MaxCodeP", 10);
}


/* Pass2 uses these WriteCodeXXXX fns to transfer the assembled binary code */
/* back to the object code formatter. The functions simply insert the code */
/* produced into a code buffer. The buffer being flushed when it is full, */
/* or when another directive need to be output */

void WriteCodeWord(int w)
{
	if (codesize + 4 >= CODEBUFFERSIZE)
		FlushCodeBuffer();

#ifdef __BIGENDIAN /* target processor */
	codebuffer[codesize++] = (w >> 24) & 0xff;
	codebuffer[codesize++] = (w >> 16) & 0xff;
	codebuffer[codesize++] = (w >> 8 ) & 0xff;
	codebuffer[codesize++] = w & 0xff;
#else
	codebuffer[codesize++] = w & 0xff;
	codebuffer[codesize++] = (w >> 8 ) & 0xff;
	codebuffer[codesize++] = (w >> 16) & 0xff;
	codebuffer[codesize++] = (w >> 24) & 0xff;
#endif
}

void WriteCodeShort(int s)
{
	if (codesize + 2 >= CODEBUFFERSIZE)
		FlushCodeBuffer();

#ifdef __BIGENDIAN /* target processor */
	codebuffer[codesize++] = (s >> 8) & 0xff;
	codebuffer[codesize++] = s & 0xff;
#else
	codebuffer[codesize++] = s & 0xff;
	codebuffer[codesize++] = (s >> 8) & 0xff;
#endif
}

void WriteCodeByte(char b)
{
	if (codesize + 1 >= CODEBUFFERSIZE)
		FlushCodeBuffer();

	codebuffer[codesize++] = b & 0xff;
}


void WriteCodeFloat(Dble d)
{
	union {
		long	w;	/* expect 32 bit int */
#ifdef HOST_SUPPORTS_IEEE
		float	f;	/* assume host supports 32 bit IEEE FP */
#else
		FloatBin f ;
#endif
	} fw;

	if (codesize + 4 >= CODEBUFFERSIZE)
		FlushCodeBuffer();

/*
-- IEEE 64 bit --> IEEE 32 bit representation (with rounding)
*/
#ifdef HOST_SUPPORTS_IEEE
	fw.f = (float)d;
#else
	(void) fltrep_narrow_round (&d, &fw.f) ;
#endif

#ifdef __BIGENDIAN /* target processor */
	codebuffer[codesize++] = (fw.w >> 24) & 0xff;
	codebuffer[codesize++] = (fw.w >> 16) & 0xff;
	codebuffer[codesize++] = (fw.w >> 8 ) & 0xff;
	codebuffer[codesize++] = fw.w & 0xff;
#else 
	codebuffer[codesize++] = (unsigned char) (fw.w & 0xff);
	codebuffer[codesize++] = (unsigned char) ((fw.w >> 8 ) & 0xff);
	codebuffer[codesize++] = (unsigned char) ((fw.w >> 16) & 0xff);
	codebuffer[codesize++] = (unsigned char) ((fw.w >> 24) & 0xff);
#endif
}

void WriteCodeDouble(Dble d)
{
	union {
		struct {
			long	w1;	/* 64 bit representation */
			long	w2;
		} ws;
		Dble	d;
	} dw;

	if (codesize + 8 >= CODEBUFFERSIZE)
		FlushCodeBuffer();

	dw.d = d;

#ifdef __BIGENDIAN /* target processor */
	codebuffer[codesize++] = (dw.ws.w1 >> 24) & 0xff;
	codebuffer[codesize++] = (dw.ws.w1 >> 16) & 0xff;
	codebuffer[codesize++] = (dw.ws.w1 >> 8 ) & 0xff;
	codebuffer[codesize++] = dw.ws.w1 & 0xff;

	codebuffer[codesize++] = (dw.ws.w2 >> 24) & 0xff;
	codebuffer[codesize++] = (dw.ws.w2 >> 16) & 0xff;
	codebuffer[codesize++] = (dw.ws.w2 >> 8 ) & 0xff;
	codebuffer[codesize++] = dw.ws.w2 & 0xff;
#else
	codebuffer[codesize++] = (unsigned char) (dw.ws.w1 & 0xff);
	codebuffer[codesize++] = (unsigned char) ((dw.ws.w1 >> 8 ) & 0xff);
	codebuffer[codesize++] = (unsigned char) ((dw.ws.w1 >> 16) & 0xff);
	codebuffer[codesize++] = (unsigned char) ((dw.ws.w1 >> 24) & 0xff);

	codebuffer[codesize++] = (unsigned char) (dw.ws.w2 & 0xff);
	codebuffer[codesize++] = (unsigned char) ((dw.ws.w2 >> 8 ) & 0xff);
	codebuffer[codesize++] = (unsigned char) ((dw.ws.w2 >> 16) & 0xff);
	codebuffer[codesize++] = (unsigned char) ((dw.ws.w2 >> 24) & 0xff);
#endif
}


#ifdef __C40TARGET
void WriteCodeC40Float(Dble d)
{
	int	w;

	if (codesize + 4 >= CODEBUFFERSIZE)
		FlushCodeBuffer();

/*
-- IEEE 64 bit --> C40 32 bit representation
*/
	w = (int) IEEE_64ToC40_32(d);

#ifdef __BIGENDIAN /* target processor */
	codebuffer[codesize++] = (w >> 24) & 0xff;
	codebuffer[codesize++] = (w >> 16) & 0xff;
	codebuffer[codesize++] = (w >> 8 ) & 0xff;
	codebuffer[codesize++] = w & 0xff;
#else
	codebuffer[codesize++] = w & 0xff;
	codebuffer[codesize++] = (w >> 8 ) & 0xff;
	codebuffer[codesize++] = (w >> 16) & 0xff;
	codebuffer[codesize++] = (w >> 24) & 0xff;
#endif
}
#endif

/* Used by instruction patches to retrieve a binary instruction that it */
/* needs to apply a patch to. instruction is always in first word of code */
/* buffer as it is explicitly flushed before the instructions 2nd pass. */

static int GetCodeWord(void)
{
	int w = 0;

#ifdef __BIGENDIAN /* target processor */
	w = codebuffer[0];
	w = (w << 8) | codebuffer[1];
	w = (w << 8) | codebuffer[2];
	w = (w << 8) | codebuffer[3];
#else
	w = codebuffer[3];
	w = (w << 8) | codebuffer[2];
	w = (w << 8) | codebuffer[1];
	w = (w << 8) | codebuffer[0];
#endif

	return w;
}

/*
 * FlushCodeBuffer
 *
 * This function writes out the contents of the codebuffer to the object
 * file. It is used when the code buffer is full, or we need to output
 * an GHOF object code directive into the object stream.
 */

void FlushCodeBuffer(void)
{
	if (codesize == 0)
		/* no code to output */
		return;

	/* write GHOF code directive and code buffer contents */
	GHOFEncode(GHOF_CODE);
	GHOFEncode(codesize);
	ObjWrite((char *)codebuffer, codesize);

	codesize = 0;
}



/* Write a block of data to the object file */

void ObjWrite(char *buf, int size)
{
	if (fwrite(buf, sizeof(char), size, OutputFile) < size) {
		Fatal("write to object file failed");
	}
}


/* Write a byte to the object file */
void ObjWriteByte(char b)
{
	if (fputc(b, OutputFile) == EOF)
		Fatal("write to object file failed");
}



#ifdef DEBUG

/* Debugging versions of GHOF code output */
/* - just print object code to be generated */

void DebugOutputGHOF(ParseTreeItem *pti)
{
	while (pti != NULL) {
		int pc = pti->logicalPC;

		/* errors to be associated with this line */
		CurLine = pti->linenum;

		fprintf(stderr, "line %5d PC %6d ", CurLine, pti->logicalPC);

		switch (pti->what) {

		case SPACE:
			fprintf(stderr, "space %d (%#x)\n", pti->type.num, pti->type.num);

			break;

		case BYTE:
			fprintf(stderr, "byte : ");
			DebugGHOFConstList(pti->type.clist, sizeof(char), pc);
			fprintf(stderr, "\n");
			break;

		case SHORT:
			fprintf(stderr, "short: ");
			DebugGHOFConstList(pti->type.clist, 2 * sizeof(char), pc);
			fprintf(stderr, "\n");
			break;

		case WORD:
			fprintf(stderr, "short: ");
			DebugGHOFConstList(pti->type.clist, sizeof(int), pc);
			fprintf(stderr, "\n");
			break;

		case LABEL:
			if (pti->type.symb == NULL)
				Fatal("NULL symbol table entry in label directive");
			if (pti->type.symb->name == NULL)
				Fatal("Empty string in symbol table");

			fprintf(stderr, "label: %s\n", pti->type.symb->name);
			break;

		case MODULE:
			fprintf(stderr, "module: %d (%#x)\n", Eval(pti->type.expr, pc), Eval(pti->type.expr, pc));
			break;

		case REF:
		{
			NameList *nlist = pti->type.nlist;

			if (nlist == NULL)
				Fatal("ref was passed NULL name list");

			fprintf(stderr, "ref: ");

			while (nlist != NULL) {
				fprintf(stderr, "%s ", nlist->name);
				nlist = nlist->next;
			}
	
			fprintf(stderr, "\n");
			break;
		}

		case EXPORT:
		{
			NameList *nlist = pti->type.nlist;

			if (nlist == NULL)
				Fatal("export was passed NULL name list");

			fprintf(stderr, "export: ");

			while (nlist != NULL) {
				fprintf(stderr, "%s ", nlist->name);
				nlist = nlist->next;
			}
	
			fprintf(stderr, "\n");
			break;
		}

		case CODETABLE:
		{
			NameList *nlist = pti->type.nlist;

			if (nlist == NULL)
				Fatal("codetable was passed NULL name list");

			fprintf(stderr, "codetable: ");

			while (nlist != NULL) {
				fprintf(stderr, "%s ", nlist->name);
				nlist = nlist->next;
			}
	
			fprintf(stderr, "\n");
			break;
		}

		case DATA:
			if (pti->type.datacommon.name == NULL)
				Fatal("data was passed NULL string");

			fprintf(stderr, "data: %s %d (%#08.8x)\n", \
				pti->type.datacommon.name, \
				Eval(pti->type.datacommon.expr, pc),
				Eval(pti->type.datacommon.expr, pc)
				);
			break;

		case COMMON:
			if (pti->type.datacommon.name == NULL)
				Fatal("common was passed NULL string");

			fprintf(stderr, "common: %s %d (%#x)\n", \
				pti->type.datacommon.name, \
				Eval(pti->type.datacommon.expr, pc),
				Eval(pti->type.datacommon.expr, pc)
				);
			break;

		case PATCHINSTR:
			fprintf(stderr, "patchinstr: %d (%#x)", \
				Eval(pti->type.patchinstr.mcpatch, pc), \
				Eval(pti->type.patchinstr.mcpatch, pc)
				);

			fprintf(stderr, " INSTRUCTION: %#08.8x [%s]\n", pti->type.patchinstr.instr->opcode, itoabin(NULL, 32, pti->type.patchinstr.instr->opcode));

			DebugGHOFPatch(pti->type.patchinstr.patch, pc);
			fprintf(stderr,"\n");
			break;

		case INIT:
			fprintf(stderr, "init: ");
			fprintf(stderr,"\n");
			break;

		case INSTRUCTION:
#ifdef __C40TARGET
			fprintf(stderr, "INSTRUCTION: %#08.8x [%s]\n", pti->type.instr->opcode, itoabin(NULL, 32, pti->type.instr->opcode));

			if (pti->type.instr->combine) {
				fprintf(stderr, "\toptexpr %d, combine %d\n", \
					(pti->type.instr->optexpr) ? \
					Eval(pti->type.instr->optexpr, pc) : 0, \
					pti->type.instr->combine \
				);

				if (pti->type.instr->combine2) {
					fprintf(stderr, "\toptexpr2 %d, combine2 %d\n", \
						(pti->type.instr->optexpr2) ? \
						Eval(pti->type.instr->optexpr2, pc) : 0, \
						pti->type.instr->combine2 \
					);
				}
			}
#else
			fprintf(stderr, "INSTRUCTION item\n");
#endif
			break;

		default:
			Fatal("Found unknown parse tree item");
			break;
		}

		pti = pti->next;	/* move on to next item */
	}
}


static void DebugGHOFConstList(ConstList *cl, int size, int pc)
{
	while (cl) {
		switch (cl->what) {
		case E_Expr:
			fprintf(stderr, "%#x, ", Eval(cl->type.expr, pc));
			break;
		case E_Str: {
			char *s = cl->type.str;
			while (*s)
				fprintf(stderr, "%#x, ", *s++);
			break;
			}
		case E_Patch:
			DebugGHOFPatch(cl->type.patch, pc);
			break;
		default:
			Fatal("Unknown type of constant found in constant list");
			break;
		}

		cl = cl->next;
	}
}


static void DebugGHOFPatch(Patch *p, int pc)
{
	switch(p->what) {
	case MODNUM:
		fprintf(stderr,"MODNUM, ");
		break;
	case MODSIZE:
		fprintf(stderr,"MODSIZE, ");
		break;
	case DATAMODULE:
		fprintf(stderr,"DATAMODULE(%s), ",p->type.name);
		break;
	case DATASYMB:
		fprintf(stderr,"DATASYMB(%s), ",p->type.name);
		break;
	case CODESYMB:
		fprintf(stderr,"CODESYMB(%s), ",p->type.name);
		break;
	case LABELREF:
		fprintf(stderr,"LABELREF(%s), ",p->type.name);
		break;
	case CODESTUB:
		fprintf(stderr,"CODESTUB(%s), ",p->type.name);
		break;
	case BYTESWAP:
		fprintf(stderr,"BYTESWAP(");
		DebugGHOFPatch(p->type.patch, pc);
		fprintf(stderr,"), ");
		break;
	case SHIFT:
		fprintf(stderr,"SHIFT(%d, ", Eval(p->type.shift.expr, pc));
		DebugGHOFPatch(p->type.shift.patch, pc);
		fprintf(stderr,"), ");
		break;
	default:
		Fatal("Unknown patch type found in patch list");
	} /* switch patch */
}
#endif

/* ghof.c */
