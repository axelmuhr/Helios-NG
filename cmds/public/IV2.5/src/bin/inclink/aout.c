/* 
 * Access to Unix object files.
 */

#include "aout.h"
#include "defs.h"
#include <stdio.h>

#ifdef sun
char zero[PAGSIZ];
#endif
#ifdef vax
char zero[1024];
#endif

static FILE* dumpFile = stdout;

void WriteSym(unsigned int, int, int, int);

boolean IsObjectFile (void* b) {
    Exec* e = (Exec*)b;
    return !e->BadMagic();
}

void Exec::Dump () {
    fprintf(dumpFile,"0000 Magic Number    %04o\n", a_magic);
    fprintf(dumpFile,"0004 Text Segment    %4d bytes\n", a_text);
    fprintf(dumpFile,"0008 Init Data       %4d bytes\n", a_data);
    fprintf(dumpFile,"000c Uninit Data     %4d bytes\n", a_bss);
    fprintf(dumpFile,"0010 Symbol Table    %4d bytes\n", a_syms);
    fprintf(dumpFile,"0014 Entry point     %04x\n", a_entry);
    fprintf(dumpFile,"0018 Text Relocation %4d bytes\n", a_trsize);
    fprintf(dumpFile,"001c Data Relocation %4d bytes\n", a_drsize);
}

boolean NList::IsVar () {
    int t;
    t = Type();
    if (t == N_DATA || t == N_BSS || 
        (t == N_UNDF && Global() && n_value != 0)
    ) {
	return true;
    } else {
	return false;
    }
}


void NList::Dump () {
    fprintf(dumpFile,"%04x %02x|%02x|%04x %08x: ", 
	Index(), n_type, n_other, n_desc, n_value
    );
/*    fprintf(dumpFile," \"%s\", ", strTab->IdxToStr(Index()));*/
    WriteSym(n_desc, n_type, n_value, n_other);
}

void RelocInfo::Dump () {
    if (r_extern) {
	fprintf(dumpFile,"IMPORTED ");
	fprintf(dumpFile,"relocation info name dump unimplemented\n");
    } else {
	fprintf(dumpFile,"stype ");
	if (r_symbolnum & N_EXT) 
	    fprintf(dumpFile,"GLOBAL ");
	switch (r_symbolnum & ~N_EXT) {
	    case N_UNDF: fprintf(dumpFile,"UNDEFINED; "); break;
	    case N_ABS: fprintf(dumpFile,"ABS; "); break;
	    case N_TEXT: fprintf(dumpFile,"TEXT; "); break;
	    case N_DATA: fprintf(dumpFile,"DATA; "); break;
	    case N_BSS: fprintf(dumpFile,"BSS; "); break;
	    case N_COMM: fprintf(dumpFile,"COMMON; "); break;
	    case N_FN: fprintf(dumpFile,"FUNCTION; "); break;
	    default: fprintf(dumpFile,"???? (%d); ", r_symbolnum & ~N_EXT);
	}
    }

    if (r_pcrel) {
	fprintf(dumpFile," PCRel ");
    }
    switch (r_length) {
	case 0x0: fprintf(dumpFile,"byte\n"); break;
	case 0x1: fprintf(dumpFile,"word\n"); break;
	case 0x2: fprintf(dumpFile,"long\n"); break;
	default: fprintf(dumpFile,"Unknown (%d)\n", r_length);
    }
}

/*
 * Output the symbol information in a form readable by the simulator.
 */
void WriteSym (unsigned int /* sd */, int st, int sv, int so) {
    if (so) {
	fprintf(dumpFile, "WOW! A non-zero \"other\" field!?");
    }
    if (st & N_EXT) {
	fprintf(dumpFile, "Global ");
    }
    switch (st & 0xfffffffe) {
	case N_ABS:   fprintf(dumpFile,"abs = 0x%x = %d", sv, sv);
		      break;
	case N_TEXT:  fprintf(dumpFile,"text = 0x%x = %d", sv, sv);
		      break;
	case N_DATA:  fprintf(dumpFile,"data = 0x%x = %d", sv, sv);
		      break;
	case N_BSS:   fprintf(dumpFile,"bss = 0x%x = %d", sv, sv);
		      break;
	case N_UNDF:  if (sv != 0) {
			  fprintf(dumpFile,"BSS size = %04d",sv);
		      } else {
			  fprintf(dumpFile,"undefined");
		      }			  
		      break;
	default:      fprintf(dumpFile,"UNKNOWN (%d)", sv);	break;
    }
    fprintf(dumpFile,"\n");
}
