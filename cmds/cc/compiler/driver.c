
/* c.driver: copyright (C) A.C. Norman and A. Mycroft */
/* version 0.39 */
/* $Id: driver.c,v 1.17 1994/09/09 16:13:44 nickc Exp $ */

/***********************************************************************/
/*                                                                     */
/*  Driver program - calls parser and then codegenerator.              */
/*                                                                     */
/***********************************************************************/

/* AM 9-Dec-86: include special ARM-oriented scanner if TARGET_IS_ARM */

#include <stddef.h>
#include <stdlib.h>
#include <time.h>
#ifndef COMPILING_ON_ST
#ifndef COMPILING_ON_XPUTER
#include <signal.h>
#define SIGNALS 1
#endif
#endif
#include "cchdr.h"
#include <string.h>
#include "AEops.h"
#include "xrefs.h"
#include "getargs.h"

#ifdef TARGET_IS_XPUTER
#include "cg.h"		/* for debug_notify */
#endif

/* The code to *** exports pp_inclopen() (#include file opener) for pp.c  */
/* Machine dependencies... */

/* The following definitions set up default search paths for #include file */
/* searches.  Note that they are overridden on ARM by implicit -i -j from  */
/* the special arm scanner (whether BBC or MSDOS hosted).                  */
#ifdef COMPILING_ON_ARM
#  define PP_SYSAREA ":MEM.,$.ARM.CLIB."
                               /* default directories for system includes */
#  define PP_USERAREA "@."     /* default directories for user includes   */
/* we should probably provide some way to override the following too.     */
#  define PP_ROOTEDFILE(file) (!isalnum(*file))
#  define PP_DIRCHAR '.'
/* the test in the next line should accept mixed case too */
#  define PP_INCOREFILE(file) \
    (strncmp(file,":MEM.",5) == 0 || strncmp(file,":mem.",5) == 0 ? file+5 : 0)
#endif
#ifdef COMPILING_ON_370
#  define PP_SYSAREA ":,sys"    /* default pds's for system includes is sysh */
#  define PP_USERAREA ","       /* default pds's for user includes   is h    */
#  define PP_DIRCHAR ':'
#  define PP_ROOTEDFILE(file) 0
#  define PP_INCOREFILE(file) (*(file) == ':' ? file+1 : 0)
#endif
#ifdef COMPILING_ON_CLIPPER   /* First approximation of unix behaviour */
#  define PP_SYSAREA ":,sys/"
#  define PP_USERAREA ","
#  define PP_DIRCHAR '/'
#  define PP_ROOTEDFILE(file) (*(file) == '/')
#  define PP_INCOREFILE(file) (*(file) == ':' ? file+1 : 0)
#endif
#ifdef COMPILING_ON_SUN4   /* First approximation of unix behaviour */
#  define PP_SYSAREA "/giga/Norcroft/include/"
#  define PP_USERAREA ","
#  define PP_DIRCHAR '/'
#  define PP_ROOTEDFILE(file) (*(file) == '/')
#  define PP_INCOREFILE(file) (*(file) == ':' ? file+1 : 0)
#endif
#ifdef COMPILING_ON_ST
#  define PP_SYSAREA "c:\\include\\"
#  define PP_USERAREA ","
#  define PP_DIRCHAR '\\'
#  define PP_ROOTEDFILE(file) (*(file) == '\\' || file[2] == ':')
#  define PP_INCOREFILE(file) (*(file) == ':' ? file+1 : 0)
#endif
#ifdef COMPILING_ON_DOS
#  define PP_SYSAREA "c:\\helios\\include\\"
#  define PP_USERAREA ","
#  define PP_DIRCHAR '\\'
#  define PP_ROOTEDFILE(file) (*(file) == '\\' || *(file) == '/' || file[1] == ':')
#  define PP_INCOREFILE(file) (*(file) == ':' ? file+1 : 0)
#endif
#ifdef COMPILING_ON_SUN   /* First approximation of unix behaviour */
#  define PP_SYSAREA "/usr/sol/jim/compilers/c/include/"  /* !!! */
#  define PP_USERAREA ","
#  define PP_DIRCHAR '/'
#  define PP_ROOTEDFILE(file) (*(file) == '/')
#  define PP_INCOREFILE(file) (*(file) == ':' ? file+1 : 0)
#endif
#ifdef COMPILING_ON_XPUTER
#  define PP_SYSAREA "/helios/include/"
#  define PP_USERAREA ","
#  define PP_DIRCHAR '/'
#  define PP_ROOTEDFILE(file) (*(file) == '/')
#  define PP_INCOREFILE(file) (*(file) == ':' ? file+1 : 0)
#endif
#ifdef COMPILING_ON_RS6000   /* First approximation of unix behaviour */
#  define PP_SYSAREA "/dsl/HeliosRoot/Helios/include/"
#  define PP_USERAREA ","
#  define PP_DIRCHAR '/'
#  define PP_ROOTEDFILE(file) (*(file) == '/')
#  define PP_INCOREFILE(file) (*(file) == ':' ? file+1 : 0)
#endif

#ifdef PASCAL
#  define FILE_EXTENSION 'p'
#else
#  define FILE_EXTENSION 'c'
#endif

static char *sysarea, *userarea;

static bool swapround(s,c)
char *s;
bool c;
{   /* changes abc.def.h to h.abc.def for brazil-like things */
    /* or to h(abc.def for 370 PDS's                         */
    int i = strlen(s);
    if (i >= 3 && s[i-2] == '.' && tolower(s[i-1]) == c)
    {   int j;
        for (j = i-1; j >= 2; j--) s[j] = s[j-2];
        s[0] = c;
        s[1] = PP_DIRCHAR;
        return 1;
    }
    return 0;
}

FILE *pp_inclopen(file,systemheader)
char *file;
bool systemheader;
{   char *area = systemheader ? sysarea : userarea;
    if (PP_ROOTEDFILE(file)) area = ",";  /* go once round loop below */
    while (*area)
    {   char dir[256], ch;
#ifndef NO_INSTORE_FILES
        char *p;
#endif
        int n = 0;
        while ((ch = *area) != 0 && (area++, ch != ','))
          if (n<255) dir[n++] = ch;
        if (n+strlen(file) > 255) continue;
        strcpy(dir+n, file);
        if (debugging(DEBUG_FILES)) fprintf(stderr, "File %s(%s)", dir, dir+n);
#ifndef NO_INSTORE_FILES
        if ((p = PP_INCOREFILE(dir)) != 0)     /* in core file spec        */
        {   if (debugging(DEBUG_FILES)) fprintf(stderr, "incore %s", p);
/* The swapround() on the next line is useful REGARDLESS of what sort      */
/* of file system my computer has on it                                    */
            if (swapround(p, 'h')) p += 2;  /* Lose any final .h suffix */
            if (debugging(DEBUG_FILES)) fprintf(stderr, "(%s)", p);
            for (n=0; builtin_headers[n].name != 0; n++)
              if (strcmp(p, builtin_headers[n].name)==0)  /* found */
              { if (debugging(DEBUG_FILES)) fprintf(stderr, " found\n");
                return _fopen_string_file(builtin_headers[n].content,
                                          strlen(builtin_headers[n].content));
              }
        }
        else
#endif
        {   FILE *fp;
    /* try to swap round a possible .c and .h suffix (or .p for Pascal)  */
            if (!(feature & FEATURE_FILEX))
                if (!swapround(dir+n, 'h'))
                    swapround(dir+n, FILE_EXTENSION);
            if (debugging(DEBUG_FILES)) fprintf(stderr, "=> %s", dir);
            if ((fp = fopen(dir,"r")) != 0)
            { if (debugging(DEBUG_FILES)) fprintf(stderr, " found\n");
              return fp;
            }
        }
        if (debugging(DEBUG_FILES)) fprintf(stderr, " failed\n");
    }
    return 0;    /* tough ched */
}

/* *** end of #include file opener */


static void disable_warnings(s)
char *s;
{   for (;;) switch (*s++)
    {  case 'a': case 'A': suppress |= D_ASSIGNTEST;   break;
       case 'f': case 'F': var_warn_implicit_fns = 0; break;
       case 'd': case 'D': var_warn_deprecated = 0;   break;
       case 's': case 'S': suppress |= D_SHORTWARN;    break;
       case 'v': case 'V': suppress |= D_IMPLICITVOID; break;
       case 0: return;
    }
}

static void disable_errors(s)
char *s;
{   for (;;) switch (*s++)
    {  case 'c': case 'C': var_warn_implicit_casts = 0;  break;
       case 'p': case 'P': suppress |= D_PPALLOWJUNK;     break;
#ifndef NO_VALOF_BLOCKS
       case 'v': case 'V': suppress |= D_VALOFBLOCKS;     break;
#endif
       case 'z': case 'Z': suppress |= D_ZEROARRAY;       break;
       case 0: return;
    }
}

static void feature_set(s)
char *s;
{   for (;;) switch (*s++)
    {  case 'f': case 'F': feature &= ~FEATURE_SAVENAME; break;
       case 'a': case 'A': feature |= FEATURE_ANOMALY;   break;
       case 'v': case 'V': feature |= FEATURE_NOUSE;     break;
       case 'm': case 'M': feature |= FEATURE_PPNOUSE;   break;
       case 'h': case 'H': feature |= FEATURE_PREDECLARE; break;
       case 'r': case 'R': feature |= FEATURE_RELOCATE;  break;
       case 's': case 'S': feature |= FEATURE_ANNOTATE;  break;
       case 'x': case 'X': feature &= ~FEATURE_FILEX;     break;
       case 0: return;
    }
}

static void pragma_set(s)
char *s;
{   char p = tolower(*s++);
    if (islower(p))
    {   int n = -1;
        if (isdigit(*s))
        {   n = 0;
            do n = n*10 + *s++ - '0'; while (isdigit(*s));
        }
        pp_pragmavec[p-'a'] = n;
    }
}

static void debug_set(s)
char *s;
{   for (;;) switch (*s++)
    {   case 0: return;
#ifdef ENABLE_AETREE
        case 'a': case 'A':
            _debugging |= DEBUG_AETREE;
            break;
#endif
#ifdef ENABLE_BIND
        case 'b': case 'B':
            _debugging |= DEBUG_BIND;
            break;
#endif
#ifdef ENABLE_DATA
        case 'd': case 'D':
            _debugging |= DEBUG_DATA;
            break;
#endif
#ifdef ENABLE_FNAMES
        case 'f': case 'F':
            _debugging |= DEBUG_FNAMES;
            break;
#endif
#ifdef ENABLE_CG
        case 'g': case 'G':
            _debugging |= DEBUG_CG;
            break;
#endif
#ifdef ENABLE_SPILL
        case 'h': case 'H':
            _debugging |= DEBUG_SPILL;
            break;
#endif
#ifdef ENABLE_FILES
        case 'i': case 'I':
            _debugging |= DEBUG_FILES;
            break;
#endif
#ifdef ENABLE_LEX
        case 'l': case 'L':
            _debugging |= DEBUG_LEX;
            break;
#endif
#ifdef ENABLE_MAPSTORE
        case 'm': case 'M':
            _debugging |= DEBUG_MAPSTORE;
            break;
#endif
#ifdef ENABLE_OBJ
        case 'o': case 'O':
            _debugging |= DEBUG_OBJ;
            break;
#endif
#ifdef ENABLE_PP
        case 'p': case 'P':
            _debugging |= DEBUG_PP;
            break;
#endif
#ifdef ENABLE_Q
        case 'q': case 'Q':
            _debugging |= DEBUG_Q;
            break;
#endif
#ifdef ENABLE_REGS
        case 'r': case 'R':
            _debugging |= DEBUG_REGS;
            break;
#endif
#ifdef ENABLE_SYN
        case 's': case 'S':
            _debugging |= DEBUG_SYN;
            break;
#endif
#ifdef ENABLE_TYPE
        case 't': case 'T':
            _debugging |= DEBUG_TYPE;
            break;
#endif
#ifdef ENABLE_STORE
        case 'u': case 'U':
            _debugging |= DEBUG_STORE;
            break;
#endif
#ifdef ENABLE_2STORE
        case 'w': case 'W':
            _debugging |= DEBUG_2STORE;
            break;
#endif
#ifdef ENABLE_X
        case 'x': case 'X':
            _debugging |= DEBUG_X;
            break;
#endif
#ifdef ENABLE_LOOP
        case 'y': case 'Y':
            _debugging |= DEBUG_LOOP;
            break;
#endif
/* -Qz to modify syserr behaviour is always available */
        case 'z': case 'Z':
            syserr_behaviour++;
#ifdef SIGNALS
            signal(SIGINT, SIG_DFL);
#endif
            break;
    }
}

static time_t tmuse_front = 0, tmuse_back = 0;

#ifdef TARGET_IS_ARM
static int innermain(argc,argv)
int argc;
char *argv[];
           /* compile special CLI on ARM only */
#else
int main(argc,argv)
int argc;
char *argv[];
#endif
{
    char *s;
    int i=1, nfiles = 0;
    ArgStack *argstack = NULL;

#ifdef SIGNALS
    signal(SIGINT, exit);
#else
#ifdef COMPILING_ON_ST
    freopen("CON:","w",stderr);
#endif
#endif
#ifdef NHG
    fprintf(stderr, "Helios C *TEST* 6/3/89\n");
#else
    fprintf(stderr, "Helios C 2.09.03 09/09/94 \n");
#endif
    fprintf(stderr, "(c) Copyright 1988-94 Perihelion Software Ltd.\n");
    fprintf(stderr, "All rights reserved.\n");

#ifndef TARGET_IS_XPUTER
    {
	char s[4];
	strcpy(s,"1\0\0");
	lsbitfirst = (*((int *)&s) & 255);  /* !=0 iff lsb in lowest addr */
    }
#else
	lsbitfirst = 1;
#endif

#ifdef TARGET_IS_ARM
    normal_sp_sl = 1;
#endif

    _debugging = 0;
    suppress = 0;
    feature = FEATURE_SAVENAME | FEATURE_FILEX;
    /* compile function names in (for debugger), and do name.h as header */
    syserr_behaviour = 0;
#ifdef EXPERIMENTAL_DEBUGGER
    dbgstream = 0;
#endif
    asmstream = objstream = 0;
    alloc_init();
    sysarea = PP_SYSAREA, userarea = PP_USERAREA;
    pp_init();      /* for pp_predefine() option and pragma on command line */
/* must init_sym_tab here if pp shares its symbol tables */

    while ((i<argc) || (argstack != NULL)) {
     if (i>=argc) {
       popargs(&argstack, &argc, &argv);
       i = 0;
       continue;
     }

     if (*argv[i] == '@') {
       char **argfile;
       int c;

       argfile = getargs(argv[i]+1, &c);
       if (argfile == NULL) {
         fprintf(stderr, "Unable to open or create arguments from %s\n", argv[i]+1);
         exit(1);
       } else {
         /* argv+1 is pushed so we return to the next argv */
         pushargs(&argstack, argc-i-1, &argv[i+1]);
         argv = argfile;
         argc = c;
         i = 0;
       }

       continue;
     }

     switch(s = argv[i++], *s++)
     {

/* The following macro allows for flags like -d which can be followed by   */
/* an argument either as -dXXX or as -d XXX.                               */
#define _ARG_ if (*s==0 && i<argc && argv[i][0] != '-') s = argv[i++];

case '-':
        switch (*s++)
        {
/* lsbitfirst can ONLY be changed here as a startup option - it is not   */
/* something that can be altered by #pragma.                             */
            case 'b': case 'B': lsbitfirst = !lsbitfirst; break;
            case 'd': case 'D': _ARG_;
                                pp_predefine(s);
                                break;
            case 'e': case 'E': _ARG_;
                                disable_errors(s);
                                break;
            case 'w': case 'W': _ARG_;
                                disable_warnings(s);
                                break;
            case 'f': case 'F': _ARG_;
                                feature_set(s);
                                break;
#ifdef EXPERIMENTAL_DEBUGGER
            case 'g': case 'G':
/* NB for this version -G needs an arg - eventually it will put its output */
/* in a standard place.                                                    */
                _ARG_;
                if ((dbgstream = fopen(s, "w")) == 0)
                    fprintf(stderr, "Couldn't write file '%s'\n", s),
                    exit(1);
                break;
#endif
            case 'i': case 'I': _ARG_;
                                userarea = s;
                                break;
            case 'j': case 'J': _ARG_;
                                sysarea = s;
                                break;
            case 'k':           var_profile_option = 1; break;
            case 'K':           var_profile_option = 2; break;
#ifdef TARGET_IS_XPUTER
	    case 'l': case 'L':
	    		{
	    			extern int libfile;
	    			libfile = 1;
	    			break;
	    		}

	    case 'm': case 'M':
		{
				extern int nomodule;
				nomodule = 1;
				break;
		}
	    case 'r': case 'R':
		{
				extern int nomodule;
				extern int nodata;
				nomodule = nodata = 1;
				break;
		}
	    case 'g': case 'G':
	    		{
	    			extern int export_statics;
		        	var_dump_info = 1;
		        	debug_notify = 5;
		        	var_sort_locals = 0;
		        	export_statics = TRUE;
		        	break;
		        }
#endif
/* Also put here a case to conform to Acorn's documentation.             */
            case 'o': case 'O': fprintf(stderr, "-O is no longer supported\n");
                break;
            case 'p': case 'P': _ARG_;
                                pragma_set(s);
                                break;
            case 'q': case 'Q': _ARG_;
                                debug_set(s);
                                break;
            case 's': case 'S':
                _ARG_;
#ifdef NO_ASSEMBLER_OUTPUT
                eprintf("This version of the compiler does not support -s\n");
                break;
#else
                if ((asmstream = fopen(s, "w")) == 0)
                    fprintf(stderr, "Couldn't write file '%s'\n", s),
                    exit(1);
                break;
#endif
#ifdef TARGET_IS_XPUTER
	    case 't': case 'T': _ARG_;
#ifdef NEVER
				switch( *s )
				{
				case '8':
					floatingHardware = TRUE;
					dupAllowed = TRUE;
					popAllowed = FALSE;
					break;
				case '5':
					floatingHardware = FALSE;
					dupAllowed = TRUE;
					popAllowed = TRUE;
					break;
				case '4':
				default:
					floatingHardware = FALSE;
					dupAllowed = FALSE;
					popAllowed = FALSE;
					break;
				}
#else
				pragma_set( s - 1 );
#endif
				break;
#endif
#ifdef TARGET_IS_ARM
/* By special request I provide an option (see armgen.c) that can cause  */
/* SP and SL to be swapped around so that it is easier to generate code  */
/* that can run in SVC mode. This option needs a special version of the  */
/* library to go with it.                                                */
            case 'x': case 'X': normal_sp_sl = 0;
                break;
#endif
            default:
                s--;
                fprintf(stderr, "Unrecognized option '%c': ignored\n", *s++);
                break;
        }
        break;
default:    s--;
            switch (++nfiles)
            {   case 1: if (!freopen(s, "r", stdin))
                            fprintf(stderr, "Couldn't read file '%s'\n", s),
                            exit(1);
                        pp_cisname = s;
                        break;
                case 2: if ((objstream = fopen(s, "wb")) == 0)
                            fprintf(stderr, "Couldn't write file '%s'\n", s),
                            exit(1);
                        break;
                default: fprintf(stderr, "Too many file args"); exit(1);
            }
            continue;
     }
    }

    if (!asmstream && !objstream)
        asmstream = stdout, feature |= FEATURE_ANNOTATE; /* simple test use */
#ifdef PROFILE
    for (i=0; i<16; i++) profile[i]=0;
    for (i=0; i<16; i++) aprofile[i]=0;
#endif

    bind_init();
    lex_init();         /* sets curlex.sym to s_nothing */
    builtin_init();     /* change to setup from syn_init? */
    sem_init();
    syn_init();
    cg_init();
    obj_init();

    if (objstream) obj_header();
#ifndef NO_ASSEMBLER_OUTPUT
    if (asmstream) asm_header();
#endif
    mcdep_init();             /* code for system dependent module header */
    show_entry(bindsym_(codesegment), xr_code+xr_defloc);  /* nasty here */
    show_code(bindsym_(codesegment));                      /* nasty here */
    initstaticvar(datasegment, 1);                         /* nasty here */
    drop_local_store();       /* required for alloc_reinit()             */

    tmuse_front = 0, tmuse_back = 0;
    while ((curlex.sym==s_nothing ? nextsym() : curlex.sym) != s_eof)
    {   TopDecl *d;
        clock_t t0;
        phasename = "reinit";
        lex_beware_reinit();      /* preserve needed things over reinit */
        drop_local_store();       /* in case nextsym() above read #if   */
        alloc_reinit();
        lex_reinit();
        codebuf_reinit();         /* must be done BEFORE parsing */
        t0 = clock();
        phasename = "parse";
        d = rd_topdecl();
        alloc_noteAEstoreuse();
        tmuse_front += clock() - t0;
        if (debugging(DEBUG_AETREE)) pr_topdecl(d);
        t0 = clock();
        phasename = "jopcode";
        cg_topdecl(d);
        tmuse_back += clock() - t0;
        drop_local_store();
    }

#ifdef PASCAL   /* is there a nicer way to do this? */
    syn_tidy();
#endif

  /* worry about interactions in the next two lines - e.g. dreverse */
    if (objstream)
    {   obj_trailer();
        if (ferror(objstream) ||
            fclose(objstream)) cc_fatalerr("I/O error on object stream");
    }
#ifndef NO_ASSEMBLER_OUTPUT
    if (asmstream)
    {   asm_trailer();
        if (asmstream != stdout)
        {   if (ferror(asmstream) ||
                fclose(asmstream)) cc_fatalerr("I/O error on -s stream");
        }
    }
#endif
    bind_cleanup();
#ifdef EXPERIMENTAL_DEBUGGER
    if (dbgstream && dbgstream != stdout)
    {   if (ferror(dbgstream) ||
            fclose(dbgstream)) cc_fatalerr("I/O error on -g stream");
    }
#endif
    pp_tidyup();
    if (debugging(DEBUG_STORE))
    {   fprintf(stderr, "Time: %dcs front-end %dcs back-end\n",
                tmuse_front, tmuse_back);
        show_store_use();
    }
    cg_tidy();    /* currently only extra store usage figures */
#ifdef PROFILE
    printf("Summary of number of register variables used\n");
    for (i=0; i<8; i++) printf("%2d : %d\n", i, profile[i]);
    printf("Summary of number words of arguments passed\n");
    for (i=0; i<16; i++) printf("%2d : %d\n", i, aprofile[i]);
#endif
    summarise();
    if (debugging(DEBUG_MAPSTORE)) _mapstore();
    alloc_dispose();

    exit ((errorcount) ? 1 : 0);
}

/* end of driver.c */

