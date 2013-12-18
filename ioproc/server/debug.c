/*------------------------------------------------------------------------
--                                                                      --
--          H E L I O S   I N P U T / O U T P U T   S E R V E R         --
--          ---------------------------------------------------         --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
--      debug.c                                                         --
--                                                                      --
--  Author:  BLV, 12-4-88 based on the original debugger by NHG         --
--               which was debugged by AE                               --
------------------------------------------------------------------------*/
/* RcsId: $Id: debug.c,v 1.4 1994/06/29 13:42:25 tony Exp $ */
/* Copyright (C) 1987, Perihelion Software Ltd.         		*/

/**
*** This module implements the debugging part of the system. It is based on
*** the original stand-alone debugger with all the code altered just enough
*** to get it to work. Hence it is a bit of a mess. It is also transputer
*** specific, so it may be scrapped.
**/
#include "helios.h"
#include <stdlib.h>

#if debugger_incorporated

/**
*** This header file declares various odds and ends needed by module
*** dbdecode.c in the debugger part of the system. I am not sure what they
*** are all for, but they appear to work.
**/

/* external interface functions and variables  */

/* the following tags are chosen to be values which will not usually    */
/* be valid code.                                                       */

#define t_module        0x60f160f1     /* module structure tag         */
#define t_resref        0x60f260f2     /* resident module reference    */
#define t_proc          0x60f360f3     /* procedure entry point        */
#define t_code          0x60f460f4     /* general code symbol          */
#define t_stack         0x60f560f5     /* stack offset symbol          */
#define t_static        0x60f660f6     /* static data symbol           */

struct module {
        Node            node;           /* module list node             */
        char            name[32];       /* module name                  */
        word            base;           /* module base address          */
        word            size;           /* size of module in words      */
        List            procsyms;       /* list of procedures           */
        List            staticsyms;     /* list of static data symbols  */
};

struct proc {
        Node            node;           /* list node                    */
        char            name[32];       /* procedure name               */
        word            value;          /* address of proc entry        */
        List            codesyms;       /* code symbols in proc         */
        List            stacksyms;      /* stack symbols in proc        */
};

struct symb {
        Node            node;           /* list node                    */
        char            name[32];       /* symbol name                  */
        word            value;          /* symbol value                 */
};
typedef struct symb *sptr;
typedef struct proc *pptr;
typedef struct module *mptr;

PRIVATE word fn( isnum,      (void));
PRIVATE word fn( issym,      (void));
PRIVATE word fn( readnumber, (void));
PRIVATE word fn( builtin,    (void));
PRIVATE int  fn( rdch,       (void));
PRIVATE int  fn( char_rdy,   (void));
PRIVATE void fn( dasm,       (void));
PRIVATE void fn( line_pause, (void));
PRIVATE void fn( showframe,  (void));
PRIVATE void fn( getframe,   (void));
PRIVATE void fn( initsym,    (void));
#if 0
PRIVATE void fn( defsyms,    (void));
PRIVATE void fn( defsym,     (STRING, word));
#endif
PRIVATE sptr fn( nearsym,    (List *, word));
PRIVATE void fn( dodisasm,   (void));
PRIVATE uint fn( gbyte,      (void));
PRIVATE void fn( ungbyte,    (void));
PRIVATE void fn( disasm,     (word, word, word));
PRIVATE void fn( dataopd,    (word, word));
PRIVATE void fn( stackopd,   (word, word));
PRIVATE void fn( codeopd,    (word, word));

PRIVATE void  fn( im_reset,   (void));
PRIVATE word  fn( im_next,    (void));

/*=========================================================================*/

PUBLIC List  *Image_List;         /* the system image */
PUBLIC word  isize;

PRIVATE word current;               /* current value of interest */
PRIVATE word framesize;             /* size of display frame     */
PRIVATE word addressbase;           /* offset from which all addresses are caluclated */
PRIVATE int  ch;                    /* current input character   */
PRIVATE UBYTE framebuf[270];        /* buffer for frame (256 + some slush)   */
PRIVATE UBYTE *framep;               /* pointer into start of frame in buffer */
PRIVATE int  toksize;
PRIVATE byte token[128];
PRIVATE word tokval;
PRIVATE word curpos;
PRIVATE word dmode = 0;
PRIVATE jmp_buf err_lev;
PRIVATE List modlist;

#define issize 32
#define ismask (issize-1)

PRIVATE word istack[issize];
PRIVATE word ispos = 0;

#define BS     0x08
#define DEL    0x7f
#define ESC    0x1b

#define probe_value 0x61616161L
#define MAXCHUNK 8192

PRIVATE unsigned long deftrace = MinInt;

/*------------------------------------------------------------------------------
--
-- init_debug()
--
------------------------------------------------------------------------------*/

/**
*** This routine is called once only, from inside main() in module server.c .
*** It is responsible for initialising the debugger, which involves
*** setting up the linked lists used to hold all the symbols and putting the
*** elementary ones in it.
***
*** debug_initialised is used to check that the debugger has actually been
*** initialised, before I try to tidy up uninitialised linked lists.
**/

PRIVATE int debug_initialised = 0;

void init_debug()
{
        initsym();
        debug_initialised++;       /* there is something to tidy up */
}

/*------------------------------------------------------------------------------
--
-- min_fun
--
------------------------------------------------------------------------------*/

/**
*** problems with using a simple macro here on different machines.
**/

PRIVATE int min_fun(a,b)
int a,b;
{
   return( (a < b) ? a : b );
}


/*------------------------------------------------------------------------------
--
-- tidy_debug()
--
------------------------------------------------------------------------------*/

/**
*** Tidy_debug() is called when the system exits. Its main jobs is to free
*** the list of defined symbols. Now, this list consist of elements
*** struct module, each element struct module contains an additional two
*** lists, etc. The nice way to free this mess is using WalkList's.
**/


PRIVATE void delete_proc(node)
pptr node;
{ FreeList(&(node->codesyms));
  FreeList(&(node->stacksyms));
  iofree(Remove(&(node->node)));
}

PRIVATE void delete_module(node)
mptr node;
{ WalkList(&(node->procsyms),   (VoidNodeFnPtr)func(delete_proc));
  FreeList(&(node->staticsyms));
  iofree(Remove(&(node->node)));
}

void tidy_debug()
{ if (!debug_initialised) return;
 
  WalkList(&modlist, (VoidNodeFnPtr)func(delete_module));
}

/*------------------------------------------------------------------------------
--
-- debug
--
------------------------------------------------------------------------------*/

/**
*** This is the main loop of the debugger system.
***
*** Returncode is the value returned to main() in module server.c, 0 if the user
*** wants to exit the system completely and non-zero if the user wants to enter
*** the Server. It is set when the user types the quit command etc.
***
*** The main loop of the server checks all the special flags, and then checks
*** to see if a character has been pressed. Routines char_rdy() and rdch()
*** further down in this module deal with all the nasty interaction between
*** the I/O routines needed in the Server and the much higher routines needed
*** by the debugger. Calls to rdch() may only be made after successful calls to
*** char_rdy(), and the character is put into global ch.
***
*** Having got hold of a character it must be processed. Characters which form
*** part of tokens are put into a buffer, otherwise it indicates that there
*** is some action to be taken. First of all it is important to provide basic
*** editing, i.e. backspace and delete. If it is not one of these we continue.
*** By now the user may have typed in a symbol name, a new transputer address,
*** or a built-in command, so I check whether there is a token in the buffer
*** and process it. If not the last key must have been a special command such
*** as comma to go back one frame, and I allow for all the valid combinations.
**/

PRIVATE int returncode;

int debug()
{ word newval;

  ch          = '0';
  addressbase = MinInt;
  framesize   = 16;
  current     = 0;
  returncode  = 1;

  resetlnk();

  (void) setjmp(err_lev);

  toksize = 0;              /* no characters in token buffer */
  
  for(;;)
   { if (!DebugMode)
     { 
       return(returncode);
     }

     if (Special_Exit)
     {
       return(0);
     }

     if (Special_Reboot)
      { Special_Reboot = FALSE;  /* or I would reboot twice */
        DebugMode = 0;           /* to enter Server mode    */
        return(returncode);
      }

     if (!char_rdy()) { goto_sleep(divlong( OneSec, 20L)); continue; }
     rdch();

     ch = ToLower(ch);
     if( (('0' <= ch) && (ch <= '9')) ||
         (('a' <= ch) && (ch <= 'z')) ||
         (ch eq '_') || (ch eq '#') ||
         ((ch eq '.') && (toksize > 0)) )
      { token[toksize++] = (byte) ch;
        continue;
      }

     /* some simple line editing */

     if (ch eq BS && toksize>0)
      { toksize--;
        outputch(' ', &Server_window);
        outputch(BS, &Server_window);
        continue;
      }
     if( ch eq DEL && toksize>0 )
      { toksize++;           /* to wipe out DEL char */
        outputch('\r', &Server_window);
        while( toksize )
         { outputch(' ', &Server_window); toksize--; }
        outputch('\r', &Server_window);
        continue;
      }

     if (ch eq ESC)        /* ESCape is a short-hand for explore */
      { strcpy(token, "EXPLORE");
        toksize = strlen(token);
        output("\rExplore");
      }

     /* when we drop through we have a non-alpha char */
     if( toksize ne 0 )
        { token[toksize] = '\0';
          if( issym() )   current = tokval-addressbase;
          elif( isnum() ) current = tokval;
          elif( !builtin() ) 
           ServerDebug("Warning : Unknown symbol '%s'.",token);
          toksize = 0;
        }

     switch(ch )
      { default   :
        case BS   :
        case DEL  : break;

        case '\r' :                     /* re-display on end of line       */
        case '\n' : 
show:                            /* Horrible GOTO's here !!! */
                    showframe();
                    break;

        case ':':                       /* set frame size                 */
                    newval = readnumber();
                    if (newval > 256 )
                      output("Warning : Maximum frame size is #100 bytes.\r\n");
                    elif (newval < 1)
                      output("Warning : minimum frame size is 1 byte.\r\n");
                    else framesize = newval;
                    goto show;

        case '.':                       /* advance by one frame           */
                    current += framesize;
                    goto show;

        case ',':                       /* back by one frame              */
                    current -= framesize;
                    goto show;

        case '>':                       /* forward to next word boundary  */
                    current = (current+4) & (~3);
                    goto show;

        case '<':                       /* back to prev word boundary     */
                    current = (current-1) & (~3);
                    goto show;

        case '+':                       /* forward n bytes                */
                    current += readnumber();
                    goto show;

        case '-':                       /* backward n bytes               */
                    current -= readnumber();
                    goto show;

        case '=':                       /* alter contents of word         */
                    if( (current+addressbase)&3 )
                      { output("Not at word boundary.\r\n");
                        longjmp(err_lev, 1);
                      }
                    newval = dbrdint(addressbase+current);
                    ServerDebug("\r%8lx: %8lx = %q",current,newval);
                    newval = readnumber();
                    dbwrint((word)(addressbase+current),(word)newval);
                    goto show;

        case '[':                       /* indirect                       */
                    if( (current+addressbase)&3 )
                     { output("Not at word boundary.\r\n");
                       longjmp(err_lev, 1);
                     }
                    istack[ispos++] = current;
                    current = dbrdint(addressbase+current)-addressbase;
                    ispos &= ismask;
                    goto show;

        case ']':                       /* exdirect                       */
                    ispos = (ispos-1) & ismask;
                    current = istack[ispos];
                    goto show;

        case '{':                       /* indirect RPTR                  */
                    if ( (current+addressbase)&3 )
                     { output("Not at word boundary.\r\n");
                       longjmp(err_lev, 1);
                     }                   
                    istack[ispos++] = current;
                    current += dbrdint(addressbase+current);
                    ispos &= ismask;
                    goto show;

        case '\'':                      /* advance by one frame & disasm  */
                    current += framesize;

        case ';':
                    dasm();
                    break;

        }
    }
}

/*------------------------------------------------------------------------------
--
-- issym
-- isnum
-- builtin
--
------------------------------------------------------------------------------*/

/**
*** These routines check whether the token just typed in and stored in a buffer
*** are known.
***
*** The token may be a number, specifying a new address to examine, which is
*** checked for by isnum(). The code is boring - hexadecimal string to integer
*** conversion... To make things a bit more interesting there is an absolute
*** mode.
**/

PRIVATE word isnum()
{ word n = 0;
  int i = 0;
  int absolute = 0;

  if (token[i] eq '#')
   { absolute = 1; i++; }

  for(  ; token[i] ; i++ )
   { byte c = token[i];
     if('0' <= c && c <= '9' ) n = (n<<4) + c - '0';
     elif('a' <= c && c <= 'f' ) n = (n<<4) + c - 'a' + 10;
     else return FALSE;
   }

  if (absolute) n -= addressbase;
  tokval = n;
  return TRUE;
}

/**
*** issym() is rather trickier because it involves comparing the token with
*** all the symbols held in the linked list structure.
**/

PRIVATE word issym()
{ mptr m = (mptr)(modlist.head);
  while ( m->node.next != NULL )
   { pptr p = (pptr)(m->procsyms.head);
     while ( p->node.next != NULL )
      { sptr s = (sptr)(p->codesyms.head);
        if ( !mystrcmp(p->name,token) )
         { ServerDebug("%s %8lx.",p->name,p->value);
           tokval = p->value;
           return TRUE;
         }
        while ( s->node.next != NULL )
         { if ( !mystrcmp(s->name,token) )
            { ServerDebug("%s %8lx.",s->name,s->value);
              tokval = s->value;
              return TRUE;
            }
           s = (sptr)(s->node.next);
         }
        p = (pptr)(p->node.next);
      }
     m = (mptr)(m->node.next);
   }
  return FALSE;
}

/**
*** Finally there is a test for built-in commands. I do my usual trick of a
*** variable array of names/functions, which makes life fun and cuts quite a bit
*** off the compiled code, even though I have to use rather a lot more functions
**/

typedef struct built_in_cmd { char       *name;
                              VoidFnPtr  routine;
} built_in_cmd;

PRIVATE void fn( trace,      (void));
PRIVATE void fn( docmp,      (void));
PRIVATE void fn( putsyms,    (void));
PRIVATE void fn( setxp,      (void));
#if 0
PRIVATE void fn( dodefine,   (void));
#endif
PRIVATE void fn( dobase,     (void));
PRIVATE void fn( doreset,    (void));
PRIVATE void fn( doanalyse,  (void));
PRIVATE void fn( doquit,     (void));
PRIVATE void fn( dogo,       (void));
PRIVATE void fn( dobytes,    (void));
PRIVATE void fn( dowords,    (void));
PRIVATE void fn( dosettrace, (void));
PRIVATE void fn( doserver,   (void));
PRIVATE void fn( clear,      (void));
PRIVATE void fn( dump,       (void));
PRIVATE void fn( showdump,   (void));
PRIVATE void fn( doexplore,  (void));

PRIVATE built_in_cmd fntab[] = {
     { "base",       (VoidFnPtr) dobase     }
    ,{ "load",       (VoidFnPtr) loadimage  }
    ,{ "reset",      (VoidFnPtr) doreset    }
    ,{ "analyse",    (VoidFnPtr) doanalyse  }
    ,{ "quit",       (VoidFnPtr) doquit     }    
    ,{ "go",         (VoidFnPtr) dogo       } 
    ,{ "trace",      (VoidFnPtr) trace      } 
    ,{ "bytes",      (VoidFnPtr) dobytes    }
    ,{ "words",      (VoidFnPtr) dowords    }
    ,{ "cmp",        (VoidFnPtr) docmp      }
    ,{ "symbols",    (VoidFnPtr) putsyms    }
    ,{ "settrace",   (VoidFnPtr) dosettrace }
    ,{ "xp",         (VoidFnPtr) setxp      }
    ,{ "server",     (VoidFnPtr) doserver   }
#if 0
    ,{ "define",     (VoidFnPtr) dodefine   }
#endif
    ,{ "clear",      (VoidFnPtr) clear      }
    ,{ "dump",       (VoidFnPtr) dump       }
    ,{ "info",       (VoidFnPtr) showdump   }
    ,{ "explore",    (VoidFnPtr) doexplore  }
    ,{ (char *) NULL, (VoidFnPtr) NULL}
};


PRIVATE word builtin()
{ int fun;
                     /* check down the list of known commands */
  for ( fun = 0 ; fntab[fun].name ne (char *) NULL ; fun++ )
    if ( !mystrcmp(token,fntab[fun].name) )
      { (*fntab[fun].routine)();         /* found it */
        return(TRUE);
      } 
  return FALSE;                         /* not a known command */
}

/**
*** The following bits of code do some of the above built-in commands.
*** They are all pretty straightforward.
**/

PRIVATE void doexplore()
{ xpanalyse(); dump(); showdump();
}

PRIVATE void dobase()
{ 
  addressbase += readnumber();
}

PRIVATE void doreset()
{ xpreset();
  output("Reset.\r\n");
}

PRIVATE void doanalyse()
{ xpanalyse();
  output("Analysed.\r\n");
}

PRIVATE void doquit()
{ DebugMode = 0;      /* This exits tidily */
  Special_Exit = 1;
  returncode = 0;
  longjmp(err_lev,1);
}

PRIVATE void dobytes()
{ dmode = 0;
}

PRIVATE void dowords()
{ dmode = 1;
}

PRIVATE void dosettrace()
{ deftrace = addressbase+readnumber();
}

/**
*** To get back into Server mode just disable the debugger - the next time
*** around the debug() loop this will be noticed.
**/

PRIVATE void doserver()
{ DebugMode = 0;
}

/*------------------------------------------------------------------------------
--
--  dogo()
--
------------------------------------------------------------------------------*/

/**
*** This is used to boot up the transputer and have a look at the first couple
*** of messages going to and from. Booting up the transputer is done by a
*** routine boot_transputer() in module tload.c, and we specify mode debugboot
*** so that the space for the system image is not freed - this avoids reloading
*** if the user wants to do a cmp or a define.
***
*** Once the transputer is booted the debugger enters a simple loop which checks
*** the keyboard for data from the user and the link for data from the
*** transputer. The user can use keys ESCape to exit the loop, 'i' to send
*** system info, 'w' and 'r' to poke and peek a location in memory with a
*** probe value, or 'm' to send a dummy message which should generate an
*** error return message. The data down the link can be peeks or pokes, a Helios
*** message, or the system info message. Some Helios messages such as the
*** debugging message facility and distributed search requests are recognised
*** and acted upon, other messages are just displayed.
**/

PRIVATE void dogo()
{
  word iocport = 0L; 
  word uch = -1L;
  word scport = 0L;

  boot_processor(debugboot);

  for(;;)
   {
    if( char_rdy() )
     { word v;
       int  ch = rdch() & 0x00ff;
       switch ( ch )
        { case ESC: xpanalyse();
                    output("Analysed.\r\n");
                    dump();
                    return;

          case 'i': xpwrint (0xf0f0f0f0L);
                    xpwrword(0x00010100L);
                    xpwrint (0x8000AAAAL);
                    output("Info sent.\r\n");
                    break;

          case 'r': v=dbrdword(MemStart);
                    ServerDebug("Read value = %8lx.",v);
                    break;

          case 'w': dbwrword(MemStart,probe_value);
                    ServerDebug("Probe value %lx written.", probe_value);
                    break;

          case 'm':          /* send a test message */
                    xpwrbyte(2L);
                    xpwrint (0x00010008L);
                    xpwrint (iocport);
                    xpwrint (0x00000000L);
                    xpwrint (0xAAAAAAAAL);
                    xpwrword(0xCCCCCCCCL);
                    xpwrword(0xDDDDDDDDL);
                    xpwrword(0xDDDDDDDDL);
                    break;

           default: if (scport ne 0L)
                      { xpwrbyte(2L);
                        xpwrint(1L);
                        xpwrint(scport);
                        xpwrint(0L);
                        xpwrint(0L);
                        xpwrbyte((word)ch);
                        scport = 0L;
                      }
                    else uch = ch;
                    break;
        }
     }

    if( xprdrdy() )
     { int b;
       word a, v;
       b = (int) xprdbyte();
       switch( b )
        { case 0:        /* write (part of probe) */
                  a=xprdint();
                  v=xprdint();
                  ServerDebug("WRITE: %8lx %8lx.",a,v);
                  break;

          case 1:               /* read command */
                  a=xprdint();  /* address */
                  xpwrword(~probe_value); /* inverted result */
                  ServerDebug("READ : %8lx.",a);
                  break;

          case 2:
                  { word i,h,d,r,f;
                    word csize, dsize;
                    h = xprdint();
                    d = xprdint();
                    r = xprdint();
                    f = xprdint();
                      /* special single character message */
                    if( h eq 1L && f eq 0x22222222L )
                     { int c = (int) xprdbyte();
                       outputch(c, &Server_window);
                       break;
                     }

                    if ( f eq 0x44444444L)
                     { scport = r;
                       break;
                     }

                    if (f eq 0x60002010L)
                     { ServerDebug("Search: %8x %8x %8x %8x.", h, d, r, f);
                       xprdint(); xprdint();
                       if ((dsize = (h & 0x0000ffff)) ne 0)
                        { output("For : ");
                          for (i=0; i < dsize - 1 ;i++)
                            outputch((int) xprdbyte(), &Server_window);
                          xprdbyte();           /* terminating '\0' */
                          output("\r\n");
                        }
                       xpwrbyte(2L);
                       xpwrint(0x00000000L);
                       xpwrint(r);
                       xpwrint(0x8000BBBBL);
                       xpwrint(0x00000000L);
                       break;
                     }
                    ServerDebug("Message      : %8lx %8lx %8lx %8lx", h,d,r,f);
                    if ( (csize=((h & 0x00ff0000L)>>16)) ne 0 )
                      { output("Control vector: ");
                        for( i = 0 ; i < csize ; i++ )
                          ServerDebug("%08lx %q",xprdint());
                        output("\r\n");
                      }
                    if ( (dsize=(h & 0x0000ffffL)) ne 0 )
                      { output("Data vector   : ");
                        for( i = 0 ; i < dsize ; i++ )
                          ServerDebug("%02x %q",(UBYTE)xprdbyte());
                        output("\r\n");
                      }
                    break;
                  }

          case 0xf0:                  /* start of info */
                     xprdbyte();
                     xprdbyte();
                     xprdbyte();      /* rest of sync word */
                     a=xprdint();
                     iocport=xprdint();
                     ServerDebug("INFO : %8lx %8lx.",a,iocport);
                     if( ( a & 0x00ff0000L ) ne 0L )
                      { xpwrint (0xf0f0f0f0L);
                        xpwrint (0x00000100L);
                        xpwrint (0x8000AAAAL);
                        output("Info sent.\r\n");
                      }
                     break;
       }
     }
    goto_sleep(divlong( OneSec, 20L));
  }
}


/*------------------------------------------------------------------------------
--
-- setxp()
--
------------------------------------------------------------------------------*/

/**
*** setxp() is used to debug a remote transputer, by sending the all-singing
*** all-dancing bootstrap program into an adjacent one. A typical use of
*** setxp would be "xp 123" which is used to examine the transputer at the end
*** of link 3 of the transputer at the end of link 2 of the transputer at the
*** end of link 1 of the root transputer. This works by telling the bootstrap
*** program which link to communicate with each time.
**/

PRIVATE void setxp()
{ forever
   if (char_rdy())
    { if (!isspace((int) rdch())) break; }
   else
    goto_sleep(divlong( OneSec, 20L));

   while ( ('0' <= ch) && (ch <= '3') )
    { xpwrbyte(bootsize);               /* bootstrap size */
      xpwrdata(bootstrap,bootsize);     /* bootstrap */

      xpwrbyte((word)ch - '0');

      while (!char_rdy()) goto_sleep(divlong( OneSec, 20L));
      rdch();
    }
}

/**
*** clear() is used to clear some memory, from the end of the bootstrap to the
*** current location. This is very useful when booting a new image into the
*** transputer because it gives you a chance to work out what happened this
*** time instead of the last time.
**/

PRIVATE void clear()
{ output("\r\nSending bootstrap...\r\n");
  xpwrbyte(bootsize);           /* bootstrap size */
  xpwrdata(bootstrap,bootsize); /* bootstrap */

  xpwrbyte(5L);

  xpwrint(addressbase+current);

  while (!xprdrdy());           /* wait for the transputer to finish */
  xprdbyte();

  output("Done\r\n");

  xpreset();
}

/**
*** dump() is used to get information about the state of the transputer when
*** it crashed. It saves the first 400 bytes of the transputer's memory, sends
*** in the bootstrap followed by a byte to tell the bootstrap we want info,
*** and then restores the memory.
**/

PRIVATE word savearea[100];
PRIVATE word dumpdata[26];

PRIVATE void dump()
{ int i;

  output("\r\nSaving low memory...\r\n");

  for(i = 0 ; i < 100 ; i++ )
    savearea[i] = dbrdword(0x80000000L+(i<<2));

  output("Sending bootstrap...\r\n");
  xpwrbyte(bootsize);           /* bootstrap size */
  xpwrdata(bootstrap,bootsize); /* bootstrap */

  xpwrbyte(6L);

  for( i = 0 ; i < 26 ; i++ ) dumpdata[i] = xprdint();

  xpanalyse();

  output("Restoring low memory...\r\n");

  for(i = 0 ; i < 100 ; i++ )
    dbwrword(0x80000000L+(i<<2),savearea[i]);
}

/**
*** showdump() is used to examine the information obtained by the last
*** routine.
**/

PRIVATE void showdump()
{
  ServerDebug("Iptr         : %8lx",dumpdata[0]);
  ServerDebug("Wptr         : %8lx",dumpdata[1]);
  ServerDebug("BootLink     : %8lx",dumpdata[2]);
  ServerDebug("%s",dumpdata[3]?"Analysed":"Reset");
  ServerDebug("Output links : %8lx %8lx %8lx %8lx",dumpdata[4],dumpdata[5],dumpdata[6],dumpdata[7]);
  ServerDebug("Input  links : %8lx %8lx %8lx %8lx",dumpdata[8],dumpdata[9],dumpdata[10],dumpdata[11]);
  ServerDebug("Event channel: %8lx",dumpdata[12]);
  ServerDebug("Timer Queues : hi %8lx lo %8lx",dumpdata[13],dumpdata[14]);
  ServerDebug("Save Area    : W %8lx I %8lx A %8lx B %8lx C %8lx\r\n               S %8lx E %8lx",
    dumpdata[15],dumpdata[16],dumpdata[17],dumpdata[18],dumpdata[19],
    dumpdata[20],dumpdata[21]);
  ServerDebug("Hi Pri Queue : head %8lx tail %8lx",dumpdata[22],dumpdata[23]);
  ServerDebug("Lo Pri Queue : head %8lx tail %8lx",dumpdata[24],dumpdata[25]);
}

#if 0
/*------------------------------------------------------------------------------
--
-- Dodefine()
--
------------------------------------------------------------------------------*/

/**
*** This is used to examine the system image, which is loaded if necessary, and
*** store all the symbols in the linked list. It should only be done once to
*** avoid confusing the list, and anyway there can only be one system image so
*** only one set of symbols unless you exit the system. Routine defsyms() is in
*** module dbdecode.c .
**/

PRIVATE void dodefine()
{ PRIVATE int symbols_defined = 0;

  if (symbols_defined) return;

  if (!loadimage()) return;

  defsyms();

  output("Symbols defined.\r\n");

  symbols_defined++;
}
#endif

/*------------------------------------------------------------------------------
--
-- Showframe()
--
------------------------------------------------------------------------------*/

/**
*** This is one of the most popular routines in the debugger, used to display
*** a frame of the transputer's memory. It has to do lots of nasty formatted
*** outputted depending on whether we are in words or bytes mode, etc.
*** Actually getting the data from the transputer is done by getframe() below.
**/

#define linesize 8

PRIVATE void showframe()
{ word linebase = 0;
  if( !xpwrrdy() ) { output("Warning : Transputer not ready.\r\n"); return; }
  getframe();
  output("\r\n");
  while( linebase < framesize )
   { int todo = min_fun((int)linesize,(int)(framesize-linebase));
     int j;
     ServerDebug("%8lx: %q",addressbase+current+linebase);
     if (dmode eq 0 )
      { for ( j = 0 ; j < todo ; j++ )
         ServerDebug("%s%02x %q",j%4?"":" ",framep[linebase+j]);
      }
     else
      { word w = 0;
        for( j = 0 ; j < 4 ; j++ ) w |= (word)framep[linebase+j]<<(j*8);
        ServerDebug("%08lx %q",w);
        w = 0;
        for ( j = 4 ; j < linesize ; j++ ) w |= (word)framep[linebase+j]<<((j-4)*8);
          ServerDebug("%08lx %q",w);
      }

     for( ; j < linesize ; j++ )
      ServerDebug("%s   %q",j%4?"":" ");

     for( j = 0 ; j < todo ; j++ )
      { UBYTE b = framep[linebase+j];
        UBYTE ctl = ' ', c = b;
        if( b < ' ') { ctl = '^'; c = b+'@'; }
        if( b > '~' ) c = '.';
        ServerDebug("%c%c%q",ctl,c);
      }
     linebase += todo;
     output("\r\n");
  }
}

PRIVATE void getframe()
{ int i;
  word lwb = current&(~3);
  word upb = (current+framesize+3L)&(~3L);
  int fsize = (int) ((upb-lwb)>>2);
  word *f = (word *)framebuf;
 
  for( i = 0 ; i < fsize ; i++ )
    f[i] = (word) dbrdword((word)(addressbase+lwb+(i<<2)));

  framep = framebuf + (current&3);
}



/*------------------------------------------------------------------------------
--
-- Readnumber()
--
------------------------------------------------------------------------------*/

/**
*** This routine is used to read a hexadecimal number from the keyboard, which
*** is returned. The number is typed in after a prompt, so there may be some
*** space characters still buffered. Converting the number is easy, even
*** allowing for some simple editing. readnumber() is also used by the
*** base and settrace commands to specify an offset, in which case an
*** absolute mode is required : the dobase and dosettrace routines will add
*** the addressbase, so readnumber() should subtract it to give an absolute
*** mode.
**/

PRIVATE word readnumber()
{ word n = 0L;
  int  absolute = 0;

  forever
   if (char_rdy())
     { if (!isspace((int) rdch())) break; }
   else
    goto_sleep(divlong(OneSec, 20L));

  if (ch eq '#')
   { absolute = 1;
     while (!char_rdy()) goto_sleep(divlong(OneSec, 20L));
     rdch();
   }

  forever
   { if (ch eq ESC )
      { output("\r\n\n"); longjmp(err_lev,(int)1); }
     if (ch eq DEL)         /* convert to backspace */
      { outputch(BS,  &Server_window);
        outputch(' ', &Server_window);
        outputch(BS,  &Server_window);
        outputch(BS,  &Server_window);
        ch = BS;
      }
     if (ch eq BS)
      { outputch(' ', &Server_window);
        outputch(BS,  &Server_window);
        n = (n>>4) & 0x0fffffffL;
      }
     elif('0' <= ch && ch <= '9' )
        n = (n<<4) + ch - '0';
     elif('a' <= ch && ch <= 'f' )
        n = (n<<4) + ch - 'a' + 10;
     else break;

     while(!char_rdy()) goto_sleep(divlong(OneSec, 20L));
     rdch();
     ch = ToLower(ch);
   }

 if (absolute)
  return(n - addressbase);
 else
  return(n);
}


/*------------------------------------------------------------------------------
--
-- Console input
--
------------------------------------------------------------------------------*/

/**
*** Console input is slightly tricky because on the ST, at least, the keyboard
*** interrupt vector has been zapped. Module terminal.c contains routine
*** window_getchar() which may be used for all that is required, i.e. check
*** whether there is a character in the buffer and supply it.
***
*** Before reading any character it is essential that char_rdy() has returned
*** success. char_rdy() is responsible for calling the main system polling
*** routine, checking the special flags, and extracting a character from the
*** buffer. This character is stored in a static where it can be obtained
*** and echoed by rdch().
**/

extern int  fn (window_getchar, (Window *));

PRIVATE int current_char;

PRIVATE int char_rdy()
{ Now = clock();

#if multi_tasking
  Multiwait();
#endif

  poll_the_devices();        /* So that data ends up in the buffer */
 
  if (!DebugMode || Special_Reboot || Special_Exit) /* exit debugger */
   longjmp(err_lev, 1);

  current_char = window_getchar(&Server_window);

  return ((current_char ne 0) ? 1 : 0);
}

PRIVATE int rdch()
{
  outputch(current_char, &Server_window);
  return (ch = current_char);
}

/*------------------------------------------------------------------------------
--
-- docmp()
--
------------------------------------------------------------------------------*/

/**
*** This routine is responsible for comparing the true system image with what
*** is currently in memory. It has proved particularly useful to detect
*** dodgy memory hardware.
**/

PRIVATE void docmp()
{ INT i, j;

  if (!loadimage()) return;     /* make sure image is in memory */
                im_reset();

  j = 0;
  for( i = 0 ; i < isize ; i+=4 )
   { word xpw, bfw;
     if ((xpw=dbrdword(LoadBase+i)) != (bfw = im_next()) )
      { ServerDebug("%8lx: %8lx != %8lx.",LoadBase+i,xpw,bfw);
        j++;
        if (j eq 15)
         { line_pause(); j = 0; }
      }
   }
  output("Comparison finished.\r\n");
}

/*------------------------------------------------------------------------------
--
-- dasm()
--
------------------------------------------------------------------------------*/

/**
*** This routine is used to disassemble the current frame. The frame is loaded
*** into memory by a call to getframe() above, and disassembled by dodisasm()
*** in module dbdecode.c .
**/

PRIVATE word fpos;
PRIVATE jmp_buf end_lev;

PRIVATE void dasm()
{ framesize+=8;         /* make sure we complete the last instruction */
  getframe();
  framesize-=8;

  output("\r\n");

  fpos = 0;
  curpos = addressbase+current;

  if (setjmp(end_lev) == 0 )
   while( fpos < framesize ) dodisasm();
}


/*------------------------------------------------------------------------------
--
-- trace()
--
------------------------------------------------------------------------------*/

/**
*** There is a little-known routine in the Helios kernel, trace(), which puts
*** information into an area of memory known as the trace vector. Details of
*** this trace vector, e.g. where it is, are known only to NHG, but he has
*** provided the following code which should find it. The trace vector is
*** examined element by element, with a facility for the user to escape if there
*** are too many elements.
**/

PRIVATE void trace()
{ word dbvec;
  word upb;
  word i;
  int lines = 1;

  if ( deftrace ne MinInt )
    dbvec = deftrace;
  else
   { dbvec = dbrdint(0x80001000L);
     dbvec = dbrdint(dbvec+0x80001000L+0x54);
   }

  upb = dbrdint(dbvec);
 
  for ( i = 4; i < upb; i+=4)
   { if (lines > 20) { line_pause(); lines = 1; }

     switch( (int) (dbrdint(dbvec+i) & 0x00FFFF)  )
      { case 0x1111: ServerDebug("Regs: T= %08lx W= %08lx I= %08lx %q",
                            dbrdint(dbvec+i+4),
                            dbrdint(dbvec+i+8),
                            dbrdint(dbvec+i+12) );
                     codeopd(0L, dbrdint(dbvec+i+12) );
                     ServerDebug("\r\n      A= %08lx B= %08lx C= %08lx",
                            dbrdint(dbvec+i+16),
                            dbrdint(dbvec+i+20),
                            dbrdint(dbvec+i+24) );
                     i += 24; lines += 2;
                     break;

        case 0x2222: ServerDebug("Mark: T= %08lx W= %08lx I= %08lx %q",
                            dbrdint(dbvec+i+4),
                            dbrdint(dbvec+i+8),
                            dbrdint(dbvec+i+12) );
                     codeopd(0L, dbrdint(dbvec+i+12) );
                     output("\r\n");
                     i += 12; lines += 1;
                     break;

        default:     ServerDebug("????: %08x",dbrdint(dbvec+i) );
                     lines += 1;
                     break;
      }
   }
}

/*------------------------------------------------------------------------------
--
-- putsyms()
--
------------------------------------------------------------------------------*/

/**
*** This routine is used to display all the known symbols.
**/

PRIVATE void line_pause()
{ output("more...");
  while (!char_rdy()) goto_sleep(divlong(OneSec, 20L));
  rdch();
  output("\r         \r");
  if (ch eq ESC || ch eq 'q' || ch eq 'Q')
    { output("\r\n\n"); longjmp(err_lev, 1); }
}

PRIVATE void putsyms()
{ mptr m = (mptr)(modlist.head);
  int lines_count = 0;

  while( m->node.next ne NULL )
   { pptr p = (pptr)(m->procsyms.head);
     ServerDebug("Module:\t%s %4x %4x",m->name,m->base,m->size);
     if (++lines_count eq 15) { line_pause(); lines_count = 0; }
     while( p->node.next ne NULL )
      { sptr s = (sptr)(p->codesyms.head);
        ServerDebug("Proc:\t\t%s %lx",p->name,p->value);
        if (++lines_count eq 15) { line_pause(); lines_count = 0; }
        while( s->node.next ne NULL )
         { ServerDebug("Code:\t\t\t%s %lx",s->name,s->value);
           if (++lines_count eq 15) { line_pause(); lines_count = 0; }
           s = (struct symb *)(s->node.next);
         }
        p = (pptr)(p->node.next);
      }
     m = (mptr)(m->node.next);
   }
}

/*  -- End of debug.c -- */

/* dbdecode.c */

/**
*** This module forms part of the debugger. It has two main parts. First there
*** is code for disassembling bits of transputer memory, accessible via
*** routine dodisasm(), and I have no idea how that works. Secondly there is
*** code for manipulating symbols, InitSym() to set things up and Defsyms()
*** which goes through an entire system image loaded in host memory and
*** extracts the symbols. I do not know how that bit works either, or even
*** whether it works. Therefore the documentation for this module is rather
*** brief.
*** Here are the mnemonics for the transputer opcodes, they used to be in a
*** header file optab.h but this is the only module in which they are used.
**/
        /* direct functions */
PRIVATE STRING directfns[] = {
        "j    ",
        "ldlp ",
        "pfix ",
        "ldnl ",
        "ldc  ",
        "ldnlp",
        "nfix ",
        "ldl  ",
        "adc  ",
        "call ",
        "cj   ",
        "ajw  ",
        "eqc  ",
        "stl  ",
        "stnl ",
        "opr  "
        };

PRIVATE STRING oper[] = {
        /* one byte operations */
        "rev",
        "lb",
        "bsub",
        "endp",
        "diff",
        "add",
        "gcall",
        "in",
        "prod",
        "gt",
        "wsub",
        "out",
        "sub",
        "startp",
        "outbyte",
        "outword",


        /* two byte operations */
        "seterr",
        0,
        "resetch",
        "csub0",
        0,
        "stopp",
        "ladd",
        "stlb",
        "sthf",
        "norm",
        "ldiv",
        "ldpi",
        "stlf",
        "xdble",
        "ldpri",
        "rem",

        "ret",
        "lend",
        "ldtimer",
        0,
        0,
        0,
        0,
        0,
        0,
        "testerr",
        "testpranl",
        "tin",
        "div",
        0,
        "dist",
        "disc",

        "diss",
        "lmul",
        "not",
        "xor",
        "bcnt",
        "lshr",
        "lshl",
        "lsum",
        "lsub",
        "runp",
        "xword",
        "sb",
        "gajw",
        "savel",
        "saveh",
        "wcnt",

        "shr",
        "shl",
        "mint",
        "alt",
        "altwt",
        "altend",
        "and",
        "enbt",
        "enbc",
        "enbs",
        "move",
        "or",
        "csngl",
        "ccnt1",
        "talt",
        "ldiff",

        "sthb",
        "taltwt",
        "sum",
        "mul",
        "sttimer",
        "stoperr",
        "cword",
        "clrhalterr",
        "sethalterr",
        "testhalterr",
        "dup",
        "move2dinit",
        "move2dall",
        "move2dnonzero",
        "move2dzero",
        0,

        0,
        0,
        0,
        "unpacksn",
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        "postnormsn",
        "roundsn",
        0,
        0,

        0,
        "ldinf",
        "fmul",
        "cflerr",
        "crcword",
        "crcbyte",
        "bitcnt",
        "bitrevword",
        "bitrevnbits",
        0,
        0,
        0,
        0,
        0,
        0,
        0,

        0,
        "wsubdb",
        "fpldnldbi",
        "fpchkerr",
        "fpstnldb",
        0,
        "fpldnlsni",
        "fpadd",
        "fpstnlsn",
        "fpsub",
        "fpldnldb",
        "fpmul",
        "fpdiv",
        0,
        "fpldnlsn",
        "fpremfirst",

        "fpremstep",
        "fpnan",
        "fpordered",
        "fpnotfinite",
        "fpgt",
        "fpeq",
        "fpi32tor32",
        0,
        "fpi32tor64",
        0,
        "fpb32tor64",
        0,
        "fptesterr",
        "fprtoi32",
        "fpstnli32",
        "fpldzerosn",

        "fpldzerodb",
        "fpint",
        0,
        "fpdup",
        "fprev",
        0,
        "fpldnladddb",
        0,
        "fpldnlmuldb",
        0,
        "fpldnladdsn",
        "fpentry",
        "fpldnlmulsn",
        0,
        0,
        0
        };

#ifdef NEVER

        /* three byte operations, not used at present */
PRIVATE STRING oper3[] = {
        0,
        "fpusqrtfirst",
        "fpusqrtstep",
        "fpusqrtlast",
        "fpurp",
        "fpurm",
        "fpurz",
        "fpur32tor64",
        "fpur64tor32",
        "fpuexpdec32",
        "fpuexpinc32",
        "fpuabs",
        0,
        "fpunoround",
        "fpuchki32",
        "fpuchki64"
        };
#endif /* NEVER */

/**
*** Useful macro
**/
#define New(_type) (_type *)malloc(sizeof(_type))

/**
*** The various transputer opcodes
**/

#define f_pfix          0x2
#define f_nfix          0x6
#define f_call          0x9
#define f_j             0x0
#define f_cj            0xa
#define f_ldc           0x4
#define f_ldnl          0x3
#define f_ldnlp         0x5
#define f_stnl          0xe
#define f_ldl           0x7
#define f_stl           0xd
#define f_ldlp          0x1

PRIVATE word function;
PRIVATE word operand;
PRIVATE UBYTE ivec[8];           /* buffer for decoded instructions */

/****************************************************************/
/* Procedure: decode                                            */
/*                                                              */
/* Decode bytes into function and operand                       */
/****************************************************************/


PRIVATE void decode()
{ word i = 0;
  uint a_byte;
  operand = 0;
  forever
   { a_byte = gbyte();
     ivec[i++] = (UBYTE) a_byte;
     function = a_byte>>4;
     operand = (operand << 4) | (a_byte & 0xf);
     switch ( (int) function )
      { case f_nfix: operand = ~operand;
        case f_pfix: break;
        default    : return;
      }
   }

}

/****************************************************************/
/* dodisam                                                      */
/*                                                              */
/* disassemble a single instruction                             */
/****************************************************************/

PRIVATE void dodisasm()
{ word loc = curpos;
  word i;
  word ilen = 0;
  mptr m = (mptr) nearsym(&modlist,loc);
  pptr p = m==NULL?NULL: (pptr) nearsym(&m->procsyms,loc);
  sptr s = p==NULL?NULL: (sptr)nearsym(&p->codesyms,loc);
  word nextlab;

  function = 0;

        /* see if we have a label here */
  if ( s ne NULL ) nextlab = s->value;

  while (nextlab eq loc )
   { output("                      ");
     ServerDebug("%s:",s->name);
     s = (sptr)(s->node.next);
     if( s eq (sptr)NULL )
      { p = (pptr)(p->node.next);
        if( p eq (pptr)NULL )
         { m = (mptr)(m->node.next);
           if( m eq (mptr)NULL ) break;
           p = (pptr)(m->procsyms.head);
         }
        s = (sptr)(p->codesyms.head);
      }
     nextlab = s->value;
   }

        /* try to concatenate any NOPs */
  if (ilen eq 0 )
   { while ((ivec[ilen++] = (UBYTE) gbyte()) eq 0x20 )
      { if( ilen eq 8 ) goto lab1;
        function = -1;
      }
     ungbyte(); ilen--;
   }

lab1:
        /* and finally decode the instruction */
  if( ilen eq 0 ) { decode(); ilen = curpos-loc; }

  ServerDebug("%8lx: %q",loc);

  for ( i = 0 ; i < min_fun((int)ilen,(int)4) ; i++ )
   ServerDebug("%02x %q",ivec[i]);

  for ( ; i < 4 ; i++ ) output("   ");

  output("        ");

  switch( (int) function )
   { case -1: output("nop");
              break;

     default: disasm(curpos, function, operand);
   }

  output("\r\n");

  if( i < ilen )
   { output("        : ");
     for ( ; i < ilen ; i++ )
      ServerDebug("%02x %q",ivec[i]);
     output("\r\n");
   }
}

/****************************************************************/
/* Procedure: disasm                                            */
/*                                                              */
/* Generate the text of the instruction plus its argument (if   */
/* any) to standard output.                                     */
/****************************************************************/

PRIVATE void disasm( loc, op, opd )
word loc, op,opd;
{
  if( (0L <= op) && (op <= 0xe) )
   { ServerDebug("%s %8lx    %q",directfns[op],opd);
     switch( (int)op )
      { case f_call  :
        case f_j     :
        case f_cj    : codeopd( opd, loc );
                       break;
        case f_ldnl  :
        case f_stnl  :
        case f_ldnlp : dataopd( opd, loc );
                       break;
        case f_ldl   :
        case f_stl   :
        case f_ldlp  : stackopd( opd, loc );
                       break;
        case f_ldc   : codeopd( opd, loc );
                       outputch(' ', &Server_window);
                       dataopd( opd, loc );
                       outputch(' ', &Server_window);
                       stackopd( opd, loc );
                       break;
      }
   }
  elif((0L <= opd) && (opd <= 0x00acL) && (oper[opd] ne 0L))
   ServerDebug("%s%q",oper[opd]);
  else
   ServerDebug("UNKNOWN %2lx %8lx%q",op,opd);
}

PRIVATE void codeopd( opd, loc )
word opd, loc;
{ word dest = loc+opd;
  mptr m = (mptr) nearsym(&modlist,dest);
  pptr p = m eq NULL ? NULL : (pptr) nearsym(&m->procsyms,dest);
  sptr s = p eq NULL ? NULL : (sptr) nearsym(&p->codesyms,dest);
  if( s ne NULL )
   { word diff = dest-s->value;
     if (diff eq 0L )
       ServerDebug("%s%q",s->name);
     else
       ServerDebug("%s+%lx%q",s->name,diff);
   }
  else
   ServerDebug("%8lx%q",dest);
}

PRIVATE void dataopd( opd, loc )
word opd, loc;
{ mptr m = (mptr) nearsym(&modlist,loc);
  sptr s = m eq NULL ? NULL : (sptr) nearsym(&m->staticsyms,opd);
  if ( s ne NULL )
   { word diff = opd-s->value;
     if (diff eq 0 )
      ServerDebug("%s%q",s->name);
     else
      ServerDebug("%s+%lx%q",s->name,diff);
   }
}

PRIVATE void stackopd( opd, loc )
word opd, loc;
{ mptr m = (mptr) nearsym(&modlist,loc);
  pptr p = m eq NULL ? NULL : (pptr) nearsym(&m->procsyms,loc);
  sptr s = p eq NULL ? NULL : (sptr) nearsym(&p->stacksyms,opd);
  if (s ne NULL )
   { word diff = opd-s->value;
     if (diff eq 0 )
      ServerDebug("%s%q",s->name);
     else
      ServerDebug("%s+%lx%q",s->name,diff);
   }
}


PRIVATE unsigned int gbyte()
{ curpos++;
  return framep[fpos++];
}

PRIVATE void ungbyte()
{ curpos--;
  fpos--;
}

/****************************************************************/
/* symbol table stuff                                           */
/*                                                              */
/****************************************************************/

PRIVATE void initsym()
{
  InitList(&modlist);
}

PRIVATE sptr nearsym(list,value)
List *list;
word value;
{ Node *node = list->tail;
  while( node->prev ne NULL )
   { if (((sptr)node)->value <= value )
       return (sptr)node;
     node = node->prev;
   }
  return NULL;
}

/**
*** Code to read the system image, held in a linked list.
**/

PRIVATE GenData *CurData;
PRIVATE int     cur_left;
PRIVATE word    *cur_ptr;

PRIVATE void im_reset()
{ CurData  = (GenData *) Image_List->head;
  cur_left      = CurData->size;
  cur_ptr  = (word *) &(CurData->data[0]);
}

       /* Get the next word in the system image, moving on */
PRIVATE word im_next()
{ word result = *cur_ptr++;
  cur_left -= 4;
  if (cur_left <= 0)
   { CurData  = (GenData *) CurData->node.next;
     cur_left = CurData->size;
     cur_ptr  = (word *) &(CurData->data[0]);
   }
  return(result);
}

/*  -- End of dbdecode.c -- */


#endif   /* debugger_incorporated */

