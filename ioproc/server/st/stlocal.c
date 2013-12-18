/*------------------------------------------------------------------------
--                                                                      --
--          H E L I O S   I N P U T / O U T P U T   S E R V E R	        --
--          ---------------------------------------------------      	--
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                       All Rights Reserved.                           --
--                                                                      --
--     stlocal.c                                                        --
--                                                                      --
--                This is the "local" module for the ST. For each       --
--                                                                      --
--     implementation of the server it is necessary to provide a number --
--                                                                      --
--     of routines to do actual IO, rather than just translate Helios IO--
--                                                                      --
--     into local stuff, and other odds and ends like coroutines. This  --
--                                                                      --
--     is provided by the local modules.                                --
--									--
--	Author:  BLV 05/02/88						--
--									--
------------------------------------------------------------------------*/
/* SccsId: 3.7 14/4/89\ Copyright (C) 1987, Perihelion Software Ltd.	*/

#define Local_Module
#include <helios.h>
#include <xbios.h>

/**
*** This module comes in three bits. First there are the memory management
*** routines. Next come some very hardware-dependant routines, which you
*** do not need to worry about excessively. Finally, there are the documented
*** routines which must be supplied for every implementation of the server.
**/

#if use_own_memory_management
/*------------------------------------------------------------------------
--
-- The memory management routines
--
------------------------------------------------------------------------*/

/**
*** On the ST I do not trust either the system or the C library's memory
*** allocation routines, so I provide my own. This involves a fair bit of
*** work. First, I grab all available system memory apart from about 75K for
*** TOS and the C library's internal allocation, so that all the memory is mine
*** and I do not have to worry about grabbing more space when I run out. I
*** divide this memory into two lots : about 25K for small bits of data like
*** list nodes, and the remainder for large bits like coroutine stacks and
*** message data buffers.
***
*** To keep track of all the memory I use linked lists, surprise surprise.
*** There are two separate linked lists, small_pool for the small bits of data
*** and big_pool for the large chunks.
**/


typedef struct memory_node { Node     node;
                             WORD     size;
} memory_node;

List small_pool, big_pool;
PRIVATE char *get_small(), *get_big();
#define memory_big    0x66000000L
#define memory_small  0x77000000L
#define memory_mask   0xFF000000L

void initialise_memory()
{ WORD available = Malloc(-1L);
  char *all_of_memory;
  memory_node *current_node;
  WORD temp;

  if (available < (200L * 1024L))
    { printf("There is insufficient memory to run the server.\n");
      printf("Please dispose of any Ram disks etc.\n");
      exit(1);
    }

  available    -= (75L * 1024L);
  all_of_memory = (char *) Malloc(available);
  if (all_of_memory eq (char *) NULL)
    { printf("The server was unable to obtain enough memory from TOS.\n");
      exit(1);
    }

  InitList(&small_pool); InitList(&big_pool);

  current_node = (memory_node *) all_of_memory;
  current_node->size = (100L * 1024L);
  AddTail(current_node, &small_pool);

  temp = current_node->size + 200L;
  current_node = (memory_node *) ((char *)current_node + temp);
  current_node->size = available - temp - 200L;
  AddTail(current_node, &big_pool);
}

/**
*** This routine implements "malloc" by calling additional
*** routines below.
**/
char *get_mem(size)
uint size;
{ if (size < 250)
    return(get_small((WORD) size));
  else
    return(get_big((WORD) size));
}


/**
*** free'ing memory is not as easy as adding the memory back to the appropriate
*** linked list because I want to do continuous garbage collection, i.e. 
*** whenever a bit is freed I try to merge it with adjacent free bits. This
*** involves keeping the linked lists sorted, which is no big deal as they
*** tend to be very small in practice. The first step is to work out
*** which pool the memory belongs to (assuming it is valid) and going down
*** the linked list until I find the nearest bits of free memory. Then I
*** try to merge the newly-freed bit with the previous lot, and with the
*** next lot, making sure that I do not generate invalid list nodes. Finally
*** I can put the data back into the free pool, either at the end of the list
*** or just before the next node so that the list remains sorted.
**/

void free_mem(a_ptr)
memory_node *a_ptr;
{ memory_node *current_node = a_ptr - 1, *prev_node, *next_node, *expected;
  List *memory_list;

  if ((current_node->size & memory_mask) eq memory_small)
    memory_list = &small_pool;
  elif ((current_node->size & memory_mask) eq memory_big)
    memory_list = &big_pool;
  else
   { printf("Error : free'ing memory that has not been allocated.\n");
     return;
   }
  current_node->size &= ~memory_mask;

  expected = (memory_node *) ((char *)a_ptr + current_node->size);

  for ( next_node = (memory_node *)memory_list->head,
        prev_node = (memory_node *)NULL;

        (next_node->node.next ne (Node *) NULL) &&
        (next_node < expected);

        prev_node = next_node,
        next_node = (memory_node *)next_node->node.next);

/**
*** At this stage I have :
*** prev_pointer is NULL or the previous element in the free list.
*** next_pointer is dummy or the next element in the free list.
**/

  if (prev_node ne NULL)
    if (((char *)prev_node + (WORD)sizeof(memory_node) + prev_node->size) eq
         ((char *)current_node) )
       { prev_node->size += (WORD)sizeof(memory_node) + current_node->size;
         Remove(prev_node);
         current_node = prev_node;
       }

  if (((char *)current_node + (WORD)sizeof(memory_node) + current_node->size) eq
      ((char *)next_node) )
     { current_node->size += (WORD)sizeof(memory_node) + next_node->size;
       Remove(next_node);
       next_node = next_node->node.next;
     }

  if (next_node->node.next eq (Node *) NULL)
    AddTail(current_node, memory_list);
  else
    PreInsert(next_node, current_node);
}

/**
*** This is used to get some memory from the small pool. The amount requested
*** is rounded to the nearest multiple of 4 so that memory is always allocated
*** in words rather than bytes. Then I go through the list of available memory
*** finding a suitable chunk. I do not try any fancy searching - the first bit
*** of free memory that is large enough will suffice. I may use this bit
*** as a lump if it is not too much bigger than what I want, or I may split
*** it up into two lumps. If the end of the list is reached then I have run out
*** so I return NULL, and all my code is careful to check for NULL (I hope).
**/
PRIVATE char *get_small(amount)
WORD amount;
{ memory_node *current_node = small_pool.head, *new_node;
  WORD actual, difference;

  amount = 4L * ((amount + 3L) / 4L);

  for (; current_node->node.next ne (Node *) NULL;
         current_node = current_node->node.next )
   { difference = current_node->size - amount;
     if (difference < 0L) continue;    /* current node is too small */

     if (difference < 50L) /* Is it small enough not to worry about splitting */
       { current_node->size += memory_small;
         Remove(current_node);
         return( (char *) (current_node + 1));
       }

     actual   = current_node->size;
     new_node = (memory_node *) ((char *) current_node + sizeof(memory_node)  
                                  + amount);
     new_node->size =  actual - (amount + (WORD) sizeof(memory_node));

     AddTail(new_node, &small_pool);
     Remove(current_node);

     current_node->size = memory_small + amount;     
     return( (char *) (current_node + 1)) ;        
   }

  return(NULL);
}

/**
*** This code is virtually a copy of the above, the only difference being that
*** memory is allocated from the pool of big chunks instead of small ones and
*** I use a looser definition of what constitutes a close match.
**/
PRIVATE char *get_big(amount)
WORD amount;
{ memory_node *current_node = big_pool.head, *new_node;
  WORD actual, difference;

  amount = 4L * ((amount + 3L) / 4L);

  for (; current_node->node.next ne (Node *) NULL;
         current_node = current_node->node.next )
   { difference = current_node->size - amount;
     if (difference < 0L) continue;    /* current node is too small */

     if (difference < 3000L) /* Is it small enough not to worry about splitting */
       { current_node->size += memory_big;
         Remove(current_node);
         return( (char *) (current_node + 1));
       }

     actual   = current_node->size;
     new_node = (memory_node *) ((char *) current_node + sizeof(memory_node)  
                                  + amount);
     new_node->size =  actual - (amount + (WORD) sizeof(memory_node));

     AddTail(new_node, &big_pool);
     Remove(current_node);

     current_node->size = memory_big + amount;
     return( (char *) (current_node + 1)) ;        
   }

  return(NULL);
}

/**
*** This code provides another debugging option, Memory or -x, which gives
*** the number and total size of the bits of free memory.
**/
PRIVATE void add_mem(node, a_ptr, size)
memory_node *node;
WORD *a_ptr, *size;
{ *a_ptr += 1L;
  *size += node->size;
}

void memory_map()
{ WORD small = 0L, small_total = 0L, big = 0L, big_total = 0L;
 
  WalkList(&small_pool, add_mem, &small, &small_total);
  WalkList(&big_pool, add_mem, &big, &big_total);

  ServerDebug("In the small memory pool there are %ld nodes, giving %ld bytes.",
         small, small_total);
  ServerDebug("In the big   memory pool there are %ld nodes, giving %ld bytes.",
         big, big_total);
}

#endif /* use_own_memory_management */

/*------------------------------------------------------------------------
--
-- The initialisation and restoring routines.
--
------------------------------------------------------------------------*/

PRIVATE WORD critic_event;       /* the old critical event handler */
PRIVATE int  old_conterm;
PRIVATE int critical_handler();
extern WORD peekl(), pokel();
PRIVATE struct	kbdvbase *base;
PRIVATE int old_delay, old_repeat;

int floppy_errno;
int RS232_errno;
int Centronics_errno;
int Printer_errno;


/**
*** ST_initialise_devices() is called once only when the system starts up.
*** It is responsible for setting up my private scancode tables described above
*** and for changing the interrupt vectors.
***
***    Fsetdta() is a slightly funny routine needed for the ST. When
*** searching for a file or through a directory, the information obtained
*** is stored into a static buffer area and unfortunately I do not know
*** where this buffer is. Fsetdta() specifies that the search information
*** should go into my own buffer.
**/

void ST_initialise_devices()
{ int temp = Kbrate(-1, -1);
  WORD new_delay  = get_int_config("Repeat_Delay");
  WORD new_repeat = get_int_config("Repeat_Interval");

  old_delay  = (temp >> 8) & 0x00FF;
  old_repeat = (temp & 0x00FF);
  if (new_delay  eq Invalid_config) new_delay  = (WORD) old_delay;
  if (new_repeat eq Invalid_config) new_repeat = (WORD) old_repeat;
  (void) Kbrate( (int) new_delay, (int) new_repeat);

  base = Kbdvbase();
	
/**
*** Here are some important bits and pieces
**/
                  /* I want to zap the system's critical event handler which */
                  /* affects operations with floppies */
  critic_event = peekl(0x404L);
  (void) pokel(0x404L, (WORD) &critical_handler);

                  /* the server should always disable keyclick because */
                  /* I cannot stand it. Also, having a click going on  */
                  /* when the raw keyboard is enabled may be disastrous */
  old_conterm = peekb(0x0484L);
  (void)pokeb(0x0484L, old_conterm & 0xFFFE);

                  /* I want the cursor to appear */
  (void) Cursconf(1, 0);
  (void) Cursconf(3, 0);   /* but I do not want it to blink */

  Fsetdta(&searchbuffer);  /* Set this up for Fsfirst and Fsnext calls */
}

/**
*** ST_restore_devices() is called once only, when the system exits, and is
*** responsible for undoing all the harm done by ST_initialise_devices()
*** above.
**/

void ST_restore_devices()
{ (void) Kbrate(old_delay, old_repeat);
  pokel(0x404L, critic_event);  
  pokeb(0x484L, old_conterm);
}

/*------------------------------------------------------------------------
--
-- The following routines handle interrupts.
--
------------------------------------------------------------------------*/

/**
*** The critical error handler is called when something goes wrong with the
*** floppy. The argument is a small negative number indicating the error,
*** and these errors appear to correspond to the errors for Floprd as
*** described in Atari ST internals. I convert these to the corresponding
*** Helios floppy errors defined in servinc/ioaddon.h, and set the floppy error
*** flag accordingly. The function returns without doing anything else, since
*** the error must be dealt with at a higher level.
**/
PRIVATE int critical_handler(x)
int x;
{
#if floppies_available
  if (x eq -13)
    floppy_errno = floppy_protected;
  else
    floppy_errno = floppy_invalid;
#endif

  return(-1);     /* That seems to be what TOS wants, but it is not documented */
}

/*------------------------------------------------------------------------------
--
-- The mouse device
--
------------------------------------------------------------------------------*/

#if mouse_supported

/**
*** I need to provide the usual local routines initialise_mouse(),
*** tidy_mouse(), start_mouse() and stop_mouse(). Start_mouse() does an
*** Initmous() using a data vector set up previously, installing my own
*** mouse event handler. Unfortunately this mouse handler does not get
*** button events, which have to be dealt with by the keyboard routines
*** instead. I need a flag mouse_active to allow the keyboard to work out
*** whether mouse button events should be discarded or used.
**/

PRIVATE WORD mouse_vec;
extern  void mouse_int();

extern  void new_mouse();
typedef struct {
        BYTE  topmode;
        BYTE  buttons;
        BYTE  xscale;
        BYTE  yscale;
        SHORT xmax;
        SHORT ymax;
        SHORT xstart;
        SHORT ystart;
} MOUSE;
PRIVATE MOUSE mouse, sysmouse;
PRIVATE int mouse_active;
PRIVATE int mouse_x, mouse_y;
PRIVATE int mouse_divisor;

void initialise_mouse()
{ WORD mouse_res   = get_int_config("mouse_resolution");
  WORD mouse_div   = get_int_config("mouse_divisor");
  int  scale;

  if ((mouse_div eq Invalid_config) || (mouse_div <= 0L) ) mouse_div = 1L;
  if ((mouse_res eq Invalid_config) || (mouse_div <= 0L) ) mouse_res = 1L;

  scale = ((int) mouse_res) * ((int) mouse_div);
  mouse_divisor = (int) mouse_div;

  mouse_vec        = (WORD) base->kb_mousevec;
  mouse_x = 16384; mouse_y = 16384; mouse_active = 0;

  mouse.topmode    = 0;
  mouse.buttons    = 0x04;
  mouse.xscale     = scale;
  mouse.yscale     = scale;

  sysmouse.topmode    = 0;
  sysmouse.buttons    = 0x00;
  sysmouse.xscale     = 1;
  sysmouse.yscale     = mouse.xscale;

}

void tidy_mouse()
{ if (mouse_active)
   { Initmous(1, (BYTE *) &sysmouse, mouse_vec);
     mouse_active = 0;
   }
  base->kb_mousevec = (VoidFnPtr) mouse_vec;
}

void start_mouse()
{ mouse_active = 1;
  Initmous(1, (BYTE *) &mouse, mouse_int);
}

void stop_mouse()
{ Initmous(1, (BYTE *) &sysmouse, mouse_vec);
  mouse_active = 0;
}

/**
*** Mouse_events() is called by the assembler interrupt routine in ST/Events.s
*** The mouse interrupt routine only deals with mouse movements, which are
*** relative movements and which I have to convert to absolute coordinates.
*** Mouse coordinates are restricted to the range 0-32767 but they wrap around.
**/
 
void mouse_event(x, y)
int x,y;
{ x /= mouse_divisor; y /= mouse_divisor;
  mouse_x = (mouse_x + x) & 0x7FFF;
  mouse_y = (mouse_y + y) & 0x7FFF;
  new_mouse(mouse_x, mouse_y, Buttons_Unchanged);
} 

#else   /* mouse not supported, but routine needed to link with assembler */

void mouse_event()
{
}

#endif

/*------------------------------------------------------------------------
--
-- Next the keyboard. This is complicated because the keyboard chip
-- interrupt can come from a number of devices.
--
------------------------------------------------------------------------*/

#if keyboard_supported

PRIVATE WORD keyboard_vec;
extern  void keyboard_int();
extern  void new_keyboard();
int shift, control;
int keyboard_active;

void initialise_keyboard()
{ keyboard_vec = (WORD) base->kb_kbdsys;
  keyboard_active = 0;  
}

void tidy_keyboard()
{ if (keyboard_active)
   { base->kb_kbdsys = (VoidFnPtr) keyboard_vec; keyboard_active = 0; }
}

void start_keyboard()
{ keyboard_active = 1; shift = 0; control = 0;
  base->kb_kbdsys = keyboard_int;
}

void stop_keyboard()
{ base->kb_kbdsys = (VoidFnPtr) keyboard_vec;
  keyboard_active = 0;
}

/**
*** handle_key() is called when the keyboard chip interrupt routine discovers
*** that the interrupt was caused by a key-up or key-down. The routine is
*** provided with the scancode of the key, and the top bit of this indicates
*** the type of event.
***
*** There are a couple of special cases, because for some strange reason on the
*** ST pressing a mouse button generates a keyboard event. These are tested
*** for and dealt with by calling new_mouse() above. Pressing the shift, control
*** and ALT keys also generate events like these, as does caps-lock, and I have
*** to process all of these here. Depending on the current shift status I
*** extract an ASCII value or similar for the key by using my scan code tables,
*** which have been set up to generate recognisable values for special keys.
*** I test for all the shift keys and alter my shift status appropriately.
*** Then if both the control and shift keys are down I assume that the user is
*** trying to access one of the system's special facilities, and I process
*** it appropriately. If only the control key has been pressed I must convert 
*** ASCII characters to control characters, e.g. G goes to ctrl-G the bell.
*** Finally I can put the event in the keyboard buffer.
**/

static void handle_key(scancode)
unsigned int scancode;
{ WORD event;
  
  scancode = scancode & 0x00FF;		/* stop sign extending */

  if (scancode & 0x0080)	/* distinguish between key up and key down */
   event = Keys_KeyUp;
  else
   event = Keys_KeyDown;
  scancode &= 0x007F;

  if ((scancode eq 0x0074) || (scancode eq 0x0075))
#if mouse_supported
   { if (!mouse_active) return;
     if (scancode eq 0x0074)
      new_mouse(mouse_x, mouse_y, 
       (event eq Keys_KeyUp) ? Buttons_Button0_Up : Buttons_Button0_Down);
     else
      new_mouse(mouse_x, mouse_y,
       (event eq Keys_KeyUp) ? Buttons_Button1_Up : Buttons_Button1_Down);
     return;
   }
#else
   return;
#endif /* mouse_supported */


  if ((scancode eq 0x2A) || (scancode eq 0x36))
   { if (event eq Keys_KeyDown)
      shift = 1;
     else
      shift = 0;
     new_keyboard(event, (WORD) scancode);
     return;
   }
  if (scancode eq 0x1D)
   { if (event eq Keys_KeyDown)
      control = 1;
     else
      control = 0;
     new_keyboard(event, (WORD) scancode);
     return;
   }

  if ((control ne 0) && (shift ne 0))		/* debugging options */
   { if (event eq Keys_KeyUp) return;
     switch(scancode)
      {
#if debugger_incorporated
        case 0x0041 : DebugMode = 1 - DebugMode; break;
#endif
        case 0x0042 : Special_Status = TRUE;          break;
        case 0x0043 : Special_Exit   = TRUE;          break;
        case 0x0044 : Special_Reboot = TRUE;          break;

        case 0x0010 : debugflags ^= Quit_Flag;        break;
        case 0x0011 : debugflags ^= Write_Flag;       break;
        /* 0x0012, 'e' unused */
        case 0x0013 : debugflags ^= Read_Flag;        break;
        case 0x0014 : debugflags ^= Timeout_Flag;     break;
        case 0x0015 : debugflags |= ListAll_Flag;     break;
        case 0x0016 : debugflags |= Nopop_Flag;       break;
        case 0x0017 : debugflags ^= Init_Flag;        break;
        case 0x0018 : debugflags ^= Open_Flag;        break;
        case 0x0019 : debugflags ^= Close_Flag;       break;
        /* 0x001E, 'a' is all */
        case 0x001F : debugflags ^= Search_Flag;      break;
        case 0x0020 : debugflags ^= Delete_Flag;      break;
        case 0x0021 : debugflags ^= FileIO_Flag;      break;
        case 0x0022 : debugflags ^= Graphics_Flag;    break;
        case 0x0023 : debugflags ^= HardDisk_Flag;    break;
        case 0x0024 : debugflags ^= Directory_Flag;   break;
        case 0x0025 : debugflags ^= Keyboard_Flag;    break;
        case 0x0026 : debugflags |= Log_Flag;         break;
        case 0x002C : debugflags |= Reconfigure_Flag; break;
        case 0x002D : debugflags |= Memory_Flag;      break;
        case 0x002E : debugflags ^= Com_Flag;         break;
        case 0x002F : debugflags ^= OpenReply_Flag;   break;
        case 0x0030 : debugflags ^= Boot_Flag;        break;
        case 0x0031 : debugflags ^= Name_Flag;        break;
        case 0x0032 : debugflags ^= Message_Flag; break;

        case 0x001E : if (debugflags eq 0L)
                       debugflags = All_Debug_Flags;
                      else
                       debugflags = 0L;
                      break;
      }
     return;
   }

			/* If I have got this far the event must be meant for */
			/* the transputer, so I put it in the event list.     */
  new_keyboard(event, (WORD) scancode);
}

/*------------------------------------------------------------------------
--
-- The following code is called from assembler. It is a simplified port
-- of the BIOS listing.
--
------------------------------------------------------------------------*/

/**
*** Nobody really wants to understand the workings of the ST's BIOS....
**/

#define normal	0
#define	statks	1
#define amouse	2
#define	rmouse	3
#define	stclock	4
#define	joyall	5
#define	joy0	6
#define joy1	7
#define statdex	7
#define amdex	5
#define	rmdex	3
#define	clkdex	6
#define	joyadex	7
#define	joydex	1

static int keyboard_state	=	normal;
static int keyboard_index	= 	0;
static BYTE record[10];		/* plenty of space */
static BYTE *current_rec;
static VoidFnPtr current_interrupt;

			/* this is called after any keyboard chip interrupt */
void keyboard_event(key)
unsigned int key;
{ key = key & 0x00FF;		/* Stop sign extend !! */

  if (keyboard_state ne normal)  /* partway through processing one device's */
    { *current_rec++ = key;      /* IO, so add it to the buffer */
      if (--keyboard_index eq 0)
        { (*current_interrupt)(record);  /* and call that device's interrupt */
	  keyboard_state = normal;       /* handler */
        }
      return;
    }

    /* A new event. This may be the start of a transaction for one particular */
    /* device or it may be a key. All new transactions are introduced by a    */
    /* special byte. */

  if (key < 0x00F6)      /* Is it just a key */
   { handle_key(key);
     return;
   }

  current_rec	= record;
                          /* No, start of new transaction */
 switch(key)
  { case 0x00F6	: current_interrupt		= base->kb_statvec;
		  keyboard_state		= statks;
		  keyboard_index		= statdex;
		  break;

    case 0x00F7	: current_interrupt		= base->kb_mousevec;
		  keyboard_state		= amouse;
		  keyboard_index		= amdex;
		  break;

    case 0x00F8	:
    case 0x00F9	:
    case 0x00FA :
    case 0x00FB	: current_interrupt		= base->kb_mousevec;
		  keyboard_state		= rmouse;
		  keyboard_index		= rmdex-1;
		  *current_rec++		= (BYTE) key;
		  break;

    case 0x00FC	: current_interrupt		= base->kb_clockvec;
		  keyboard_state		= stclock;
		  keyboard_index		= clkdex;
		  break;

    case 0x00FD   : current_interrupt		= base->kb_joyvec;
		  keyboard_state		= joyall;
		  keyboard_index		= joyadex-1;
		  *current_rec++		= (BYTE) key;
		  break;

    case 0x00FE   : current_interrupt		= base->kb_joyvec;
		  keyboard_state		= joy0;
		  keyboard_index		= joydex;
		  *current_rec++		= (BYTE) key;
		  break;

    case 0x00FF   : current_interrupt		= base->kb_joyvec;
		  keyboard_state		= joy1;
		  keyboard_index		= joydex;
		  *current_rec++		= (BYTE) key;
		  break;

  }
}

void keyboard_reset()
{ keyboard_state = normal;
}

#else    /* keyboard not supported, I need some dummy routines for linking */

void keyboard_event()
{
}

void keyboard_reset()
{
}

#endif   /* Keyboard supported */

/*------------------------------------------------------------------------
--
-- The following code deals with finding out which drives are available,
-- and which are floppies.  
--
------------------------------------------------------------------------*/

/**
*** get_drives() returns a mask indicating which drives are available. The
*** drives available are physical floppies, and not sets of logical floppies
*** which map onto the same physical ones, hard disk partitions, and ram disks.
*** Bit 0 of the mask corresponds to drive A, bit 1 to B, etc. In addition
*** I need to know which of the drives are floppies, so I am passed a pointer
*** to a suitable word and turn this into another mask.
***
*** On the ST finding out which drives are connected is easy, because there is
*** a routine Drvmap() which does it for me. However, finding out whether drives
*** A and B map onto the same floppy drive or different ones is non-trivial.
*** I do it by peeking one of the system variables, _nflops, using a
*** Mark Williams C routine which enters supervisor mode thus bypassing the
*** hardware protection on low memory. This variable contains 0, 1, or 2
*** depending on how many physical drives there are.
**/

WORD get_drives(floppies)
WORD *floppies;
{ WORD result = Drvmap();
  int  temp;
  extern int peekw();

  *floppies = 0L;

  temp = peekw(0x4A6L);

  if (temp eq 0)
    result &= 0xFFFFFFFCL;
  elif (temp eq 1)
    { result &= 0xFFFFFFFDL;           /* floppy A only */
      *floppies = 0x01L;
    }
  else
    *floppies = 0x03L;

  return(result); 
}


/*------------------------------------------------------------------------------
--
-- ST specific utilities
--
------------------------------------------------------------------------------*/

		/* this routine takes an MSdos time stamp and converts it to */
		/* Unix seconds since 1970 */
static word monthlen[12] = { 0L, 31L, 28L, 31L, 30L, 31L, 30L, 31L, 31L, 30L,
							 31L, 30L };

PRIVATE word unixtime(MSdate, MStime)
int MStime, MSdate;
{ int  years, months;
  word days, hours, minutes, seconds;
  word count;
  int i;

  years   = ( MSdate >> 9) & 0x7F;	/* since 1980 */
  months  = ( MSdate & 0x01E0) >> 5;
  days    = ( MSdate & 0x001F);
  hours	  = ( MStime >> 11) & 0x1F;
  minutes = ( MStime & 0x07E0) >> 5;
  seconds = ( MStime & 0x001F) * 2;
  
  count = (((word)years + 10L) * 365L) + 2L;  /* days since 1970, 2 leapyears */
                                              /* between 1970 and 1980 */
  for (i=0; i < years; i++)
    if (i%4 eq 0)	count++;		/* leap years since 1980 */
									/* year 2000 ??? */
  for (i=1; i < months; i++)
	count = count + monthlen[i];				/* add months */

  if ((years%4 eq 0) && (months > 2))	/* is year a leap year ? */
	count++;

  count += days-1L;			/* this should be days since 1970 */

  count = (24L * count) + hours;
  count = (60L * count) + minutes;
  count = (60L * count) + seconds;
  return(count);
}

PRIVATE int MSdate_stamp(unix)
WORD unix;
{ int days = unix / (24L * 60L * 60L);
  int month = 1, curyear = 1970;
  int result;

  while (days > ((curyear % 4) ? 365 : 366))      /* get to right year */
   days -= ((curyear++ % 4) ? 365 : 366);

  if (curyear < 1980) return(0);

  monthlen[2] = (curyear % 4) ? 28l : 29L;     /* alter length of February */

  while (days > (int) monthlen[month])             /* get to right month */
    days -= (int) monthlen[month++];

  monthlen[2] = 28L;              /* restore February */

  result = ((curyear - 1980) << 9) + (month << 5) + days + 1;
  return(result); 
}

PRIVATE int MStime_stamp(unix)
WORD unix;
{ int seconds, minutes, hours, result;

  seconds = (int) (unix % 60L);
  unix    = unix / 60L;
  minutes = (int) (unix % 60L);
  unix    = unix / 60L;
  hours   = (int) (unix % 24L);

  result = (hours << 11) + (minutes << 5) + (seconds >>  1);
  return(result);
}

/*******************************************************************************
********************************************************************************
******** Routines needed by all the local modules ******************************
********************************************************************************
*******************************************************************************/

/**
*** Get the current system time, as a Unix time stamp. This is needed all over
*** the place.
**/
WORD get_unix_time()
{ int MStime, MSdate;

  MStime = Tgettime(); MSdate = Tgetdate();
  return(unixtime(MSdate, MStime));
}

/**
*** Set the current system time to the one specified. This is not possible on
*** all configurations, e.g. under Unix only the superuser is allowed to set
*** the system time. If so, there are some #if's in fundefs.h and devices.c
*** to handle that, and you do not need to worry about this routine.
**/
void set_current_time(unixstamp)
WORD unixstamp;
{ int MSdate, MStime;
										
  MSdate = MSdate_stamp(unixstamp);
  MStime = MStime_stamp(unixstamp);

  Tsettime(MStime); Tsetdate(MSdate);
}

void goto_sleep(delay)
WORD delay;
{ delay = delay;
}

/*******************************************************************************
*** Window and console keyboard support
*******************************************************************************/

#if multiple_windows
#define Server_handle 1
PRIVATE WORD next_handle = Server_handle;
WORD current_handle;
PRIVATE WORD Helios_handle;

#if gem_supported
#define Gem_handle    -1
PRIVATE WORD Gem_saved_handle = Server_handle;
#endif

void switch_window();
extern  void redraw_screen();

WORD create_a_window(name)
char *name;
{ current_handle = next_handle;

  if (next_handle eq Server_handle)
   { Bconout(2, 0x1B); Bconout(2, 'w'); }

  return(next_handle++);
  use(name)
}

void close_window(handle)
WORD handle;
{ if ((handle eq current_handle) && (handle ne Server_handle))
   { if (Special_Exit)
      { current_handle = Server_handle;
        redraw_screen(&Server_window);
      }
     else
       switch_window(0);
   }
}

void window_size(handle, x, y)
WORD handle;
WORD *x, *y;
{ *x = 80L; *y = 25L;
  use(handle)
}

/**
*** This routine is used to check whether output is actually going to the
*** current window or not. It is slightly subtle : if the current window
*** is gem then the output will never be displayed; if the output is
*** destined for the Server's window then that is brought to the front.
*** Otherwise the output must not be displayed.
**/
int check_window(handle)
WORD handle;
{ 
  if (handle ne current_handle)
   { if (Server_windows_nopop) return(0);

#if gem_supported
     if (current_handle eq Gem_handle)
       return(0);
#endif

     if (handle eq Server_handle)
      { Helios_handle  = current_handle;
        current_handle = Server_handle;
        redraw_screen(&Server_window);
        return(1);
      }
     else
      return(0);   /* Discard output to this window */
   }
  else
   return(1);
}

void switch_window(direction)
int direction;
{ Window *window;

#if gem_supported
  if (current_handle eq Gem_handle)
    return;
#endif


  if (current_handle eq Server_handle)  /* switch to first or last window */
   { if (direction)
      window = (Window *) Window_List.list.head;
     else
      window = (Window *) Window_List.list.tail;
   }
  else    /* find current window */
   { for (window = (Window *) Window_List.list.head;
          window->node.node.next ne (Node *) NULL;
          window = (Window *) window->node.node.next )
      if (window->handle eq current_handle)
          break;
     if (direction)  /* switch to next or previous */
      { window = (Window *) window->node.node.next;
        if (window->node.node.next eq (Node *) NULL)
         window = &Server_window;
      }
     else
      { window = (Window *) window->node.node.prev;
      	if (window->node.node.prev eq (Node *) NULL)
         window = &Server_window;
      }
   }

  if (window ne &Server_window)
   if ( (window->node.node.next eq (Node *) NULL) ||
        (window->node.node.prev eq (Node *) NULL) )
    window = &Server_window;

  current_handle = window->handle;
  redraw_screen(window);
}

/**
*** Special case for switching windows tidily. The routine is called from
*** module gem.c, with TRUE or FALSE.
**/
#if gem_supported
void switch_to_gem(flag)
{ if (flag)
   { Gem_saved_handle = current_handle;
     current_handle   = Gem_handle;
   }
  else
   { if (Gem_saved_handle eq Server_handle)
      { current_handle = Server_handle;
        redraw_screen(&Server_window);
      }
     else
      { Window *window;
        for (window = (Window *) Window_List.list.head;
             window->node.node.next ne (Node *) NULL;
             window = (Window *) window->node.node.next )
          if (window->handle eq Gem_saved_handle)
            break;
        if (window->node.node.next eq (Node *) NULL)
          window = &Server_window;
        current_handle = window->handle;
        redraw_screen(window);
      }
   }
}
#endif /* gem_supported */

#else   /* Windows not supported */

int check_window()
{ return(1);
}
  
#endif /* multiple_windows */

#define not_a_thing 0
#define k_f1        1
#define k_f2        2
#define k_f3        3
#define k_f4        4
#define k_f5        5 
#define k_f6        6
#define k_f7        7
#define k_f8        8  
#define k_f9        9
#define k_f10      10
#define k_tilde    11
#define k_sright   12
#define k_sleft    13
#define k_undo     14
PRIVATE int next_ch = 0, state = not_a_thing;

#define key_f1     0xe1
#define key_f2     0xe2
#define key_f3     0xe3
#define key_f4     0xe4
#define key_f5     0xe5
#define key_f6     0xe6
#define key_f7     0xe7
#define key_f8     0xe8
#define key_f9     0xe9
#define key_f10    0xea
#define key_help   0xeb
#define key_undo   0xec
#define key_insert 0xed
#define key_home   0xee
#define key_up     0xef
#define key_down   0xf0
#define key_left   0xf1
#define key_right  0xf2

#define CSI        0x009B

int read_char_from_keyboard(handle)
WORD handle;
{ WORD x, shiftstat;
  int temp, shift, alt, contrl;

#if multiple_windows
  if (handle ne current_handle)
   {
#if gem_supported
     if ((current_handle ne Gem_handle) || (handle ne Gem_saved_handle))
#endif
       return(-1);
   }
#endif

  if (next_ch ne 0)
   { temp = next_ch; next_ch = 0; return(temp); }

  if (state ne not_a_thing)
   switch (state)
    { case k_f1     :
      case k_f2     :
      case k_f3     :
      case k_f4     :
      case k_f5     :  
      case k_f6     :
      case k_f7     :
      case k_f8     :
      case k_f9     :
      case k_f10    : temp = state - k_f1 + '0'; state = k_tilde; return(temp);
      case k_tilde  : state = not_a_thing; return('~');
      case k_sright : state = k_tilde; return('@'); 
      case k_sleft  : state = k_tilde; return('A');
      case k_undo   : state = not_a_thing; return('z');
      default       : state = not_a_thing;
    }

  if (Bconstat(2) eq 0)
    return(-1);

  x = Bconin(2);
  shiftstat = Getshift(-1);
  shift     = shiftstat & (GS_LSH + GS_RSH);
  alt       = shiftstat & GS_ALT;
  contrl    = shiftstat & GS_CTRL;

  temp = (int) (x & 0x00FFL);
  if (shift)                  /* Deal with funny shifted cursor keys */
   if ((temp >= '0') && (temp <= '9'))
    { x &= 0xFFFFFF00L; temp = 0; }

  if (temp eq 0)        /* If it is a special key, convert the scancode */
   { temp = (int) ((x >> 16) & 0x00FFL);
     if ((temp >= 0x54) && (temp <= 0x5D)) temp = temp + 0x3b - 0x54;
     switch (temp)
      { case 0x3b : temp = key_f1;     break;
        case 0x3c : temp = key_f2;     break; 
        case 0x3d : temp = key_f3;     break;
        case 0x3e : temp = key_f4;     break;
        case 0x3f : temp = key_f5;     break;
        case 0x40 : temp = key_f6;     break;
        case 0x41 : temp = key_f7;     break;
        case 0x42 : temp = key_f8;     break;
        case 0x43 : temp = key_f9;     break;
        case 0x44 : temp = key_f10;    break;
        case 0x62 : temp = key_help;   break;
        case 0x61 : temp = key_undo;   break;
        case 0x52 : temp = key_insert; break;
        case 0x47 : temp = key_home;   break;
        case 0x48 : temp = key_up;     break;
        case 0x4b : temp = key_left;   break;
        case 0x50 : temp = key_down;   break;
        case 0x4d : temp = key_right;  break;
        default   : return(-1);
      }
   }



#if multiple_windows
 if (((temp eq key_f1) || (temp eq key_f2)) && alt)
   { Window_Testfun(&shiftstat);
     if (shiftstat)
 				 {
       if (shift)
        { if (current_handle eq Server_handle)
           { Window *window;
             for (window = (Window *) Window_List.list.head;
                  window->node.node.next ne (Node *) NULL;
                  window = (Window *) window->node.node.next )
         	    if (window->handle eq Helios_handle)
               { current_handle = Helios_handle;
                 redraw_screen(window);
               }
           }
          else
           { Helios_handle  = current_handle;
             current_handle = Server_handle;
             redraw_screen(&Server_window);
           }
        }
       else
        switch_window((temp eq key_f1) ? 1 : 0);
       return(-1);
      }
   }
#endif  /* multiple_windows */

                         /* Server-specific debugging sequence */
  if (contrl && shift)
   { if ((temp >= key_f7) && (temp <= key_f10))
      { switch(temp)
         {
#if debugger_incorporated
           case key_f7  : DebugMode = 1 - DebugMode; break;
#endif
           case key_f8  : Special_Status = TRUE; break;
           case key_f9  : Special_Exit   = TRUE; break;
           case key_f10 : Special_Reboot = TRUE; break;
         }
      }     

     if (temp <= 0x1A)                       /* could be a debugging flag */
      { int flagchar = temp + 'a' - 1;    /* convert to flag */
        int i;
        if (flagchar eq 'a')
         { if (debugflags eq 0L)
            debugflags = All_Debug_Flags;
           else
            debugflags = 0L;
           return(-1);
         }
        for (i = 0; options_list[i].flagchar ne '\0'; i++)
         if (options_list[i].flagchar eq flagchar)
          { debugflags ^= options_list[i].flag;
            return(-1);
          }
      }

     return(-1);     
   }

  if (temp >= key_f1)
   { if (temp <= key_f10)
      { if (alt)
         { if (shift)
            next_ch = '3';
           else
            next_ch = '2';
           state = (temp - key_f1 + k_f1);
           return(CSI);
         }
        elif (shift)
         { next_ch = '1';
           state = (temp - key_f1 + k_f1);
           return(CSI);
         }
        state   = k_tilde;
        next_ch = '0' + temp - key_f1;
        return(CSI);
      }
     switch(temp)
      { case key_up    : if (shift)
                          { next_ch = 'T'; state = k_tilde; return(CSI); }
                         else
                          { next_ch = 'A'; return(CSI); }
        case key_down  : if (shift)
                          { next_ch = 'S'; state = k_tilde; return(CSI); }
                         else
                          { next_ch = 'B'; return(CSI); }
        case key_left  : if (shift)
                          { next_ch = ' '; state = k_sleft; return(CSI); }
                         else
                          { next_ch = 'D'; return(CSI); }
        case key_right : if (shift)
                          { next_ch = ' '; state = k_sright; return(CSI); }
                         else
                          { next_ch = 'C'; return(CSI); }

        case key_help   : next_ch = '?'; state = k_tilde; return(CSI);
        case key_undo   : next_ch = '1'; state = k_undo; return(CSI);
        case key_home   : next_ch = 'H'; return(CSI);
        case key_insert : next_ch = '@'; return(CSI);
        default         : return(-1);
      }
   }

  return((int) x);
}

/**
*** RS232 support - these routines are only required if you have specified
*** RS232_supported in defines.h . The routines needed are described in detail
*** in devices.c, in the section on communication ports.
**/

#define overflow_max 256
#define RS232_inXOFF           0x01
#define RS232_outXOFF          0x02
#define RS232_BreakDetected    0x04
#define RS232_UseHardware      0x08
#define RS232_ClearToWrite     0x10
#define RS232_NeedToXoff       0x20
#define RS232_UseInXoff        0x40
#define RS232_UseOutXoff       0x80
#define RS232_StripIn          0x100
#define RS232_IgnorePar        0x200
#define RS232_ParMark          0x400
#define RS232_ParCheck         0x800
#define RS232_NeedToXon        0x1000
#define RS232_TXEmpty          0x2000
#define RS232_CTS              0x4000
#define RS232_RingDetected     0x8000

#define XON                    0x11
#define XOFF                   0x13

             /* hardware handshaking support */
#define RTS                    0x02
#define DTR                    0x01
#define CTS                    0x30
#define Ring                   0x40

typedef struct RS232 { 
                       BYTE       *inbuf;
                       BYTE       *outbuf;
                       Port       breakint;
                       Port       modemint;
                       uint       flags;
                       uint       incount;
                       uint       inmax;
                       uint       outcount;
                       uint       outmax;
                       uint       overflow_count;
                       BYTE       overflow_buf[overflow_max];
} RS232;

PRIVATE RS232 rs232_port;
PRIVATE ComsPort RS232_ComsPort;

PRIVATE void RS232_newcha(), RS232_sendchar();

extern  void RS232_enable_interrupts(), RS232_disable_interrupts();
extern  void RS232_setattrib(), RS232_writech(), RS232_dobreak();
extern  void RS232_checkCTS();
extern  int  RS232_arg1, RS232_arg2;
PRIVATE void RS232_setDTR(), RS232_setRTS();

PRIVATE WORD fn( RS232_send,          (ComsPort *, WORD, UBYTE *));
PRIVATE WORD fn( RS232_pollwrite,     (ComsPort *));
PRIVATE WORD fn( RS232_abortwrite,    (ComsPort *));
PRIVATE WORD fn( RS232_receive,       (ComsPort *, WORD, UBYTE *));
PRIVATE WORD fn( RS232_pollread,      (ComsPort *));
PRIVATE WORD fn( RS232_abortread,     (ComsPort *));
PRIVATE void fn( RS232_configure,     (ComsPort *));
PRIVATE void fn( RS232_done,          (ComsPort *));
extern  void fn( RS232_error_handler, (void));

WORD RS232_initlist(list, port)
List *list;
ComsPort **port;
{ *port = &RS232_ComsPort;

  RS232_ComsPort.error_fn      = func(RS232_error_handler);
  RS232_ComsPort.done_fn       = func(RS232_done);
  RS232_ComsPort.configure_fn  = func(RS232_configure);
  RS232_ComsPort.send_fn       = func(RS232_send);
  RS232_ComsPort.pollwrite_fn  = func(RS232_pollwrite);
  RS232_ComsPort.abortwrite_fn = func(RS232_abortwrite);
  RS232_ComsPort.receive_fn    = func(RS232_receive);
  RS232_ComsPort.pollread_fn   = func(RS232_pollread);
  RS232_ComsPort.abortread_fn  = func(RS232_abortread);


  rs232_port.flags          = RS232_TXEmpty;

  rs232_port.incount        = 0;
  rs232_port.inmax          = 0;
  rs232_port.inbuf          = (BYTE *) NULL;
  rs232_port.outcount       = 0;
  rs232_port.outmax         = 0;
  rs232_port.outbuf         = (BYTE *) NULL;
  rs232_port.breakint       = NullPort;
  rs232_port.modemint       = NullPort;
  rs232_port.overflow_count = 0;

  Supexec(RS232_enable_interrupts);              /* This is now safe */
  RS232_setDTR(0);
  RS232_setRTS(0);
  Supexec(RS232_checkCTS);
  return(0L);
  use(list)
}

PRIVATE WORD RS232_send(comsport, amount, buffer)
ComsPort *comsport;
WORD     amount;
UBYTE    *buffer;
{ RS232 *port = &rs232_port;

  if (port->outbuf ne (BYTE *) NULL)         /* Port in use ? */
   return(FALSE);

  port->outmax   = (uint) amount;         /* Set up the buffer */
  port->outbuf   = buffer;
  port->outcount = 0;

  if (port->flags & RS232_UseHardware)    /* Hardware handshaking ? */
   RS232_setRTS(1);

  if (port->flags & RS232_ClearToWrite)
   RS232_sendchar(port);

  return(TRUE);
  use(comsport)
}

PRIVATE WORD RS232_pollwrite(comsport)
ComsPort *comsport;
{ RS232 *port = &rs232_port;

  if (debugflags & Com_Flag)
   ServerDebug("RS232 write : written %d of %d bytes, flag = %x",
             port->outcount, port->outmax, port->flags);

  if (port->outmax eq -1)      /* Break occurred */
    { if (port->flags & RS232_UseHardware)
       RS232_setRTS(0);
      return((WORD) (port->outcount));
    }
 
  if (port->outcount >= port->outmax)    /* All the chars have been written */
   { port->outbuf  = (BYTE *) NULL;
     port->outcount = 0;
     port->outmax   = 0;
     if (port->flags & RS232_UseHardware)
      RS232_setRTS(0);
     return(-1L);
   }

  Supexec(RS232_checkCTS);         /* the CTS interrupts are dodgy... */

  if (port->flags & RS232_TXEmpty)
   if (port->flags & RS232_UseHardware)
    { if (port->flags & RS232_CTS)
       { port->flags |= RS232_ClearToWrite;
         RS232_sendchar(port);
       }
      else
       port->flags &= ~RS232_ClearToWrite;
    }
   else
    { port->flags |= RS232_ClearToWrite;
      RS232_sendchar(port);
    }

  return(-2L);                      /* Indicate still busy */  
  use(comsport)
}

PRIVATE WORD RS232_abortwrite(comsport)
ComsPort *comsport;
{ RS232 *port = &rs232_port;
  unsigned int   temp;

  port->outbuf   = (BYTE *) NULL;
  port->outmax   = 0;
  temp           = port->outcount;
  port->outcount = 0;

  if (port->flags & RS232_UseHardware)
   RS232_setRTS(0);

  return((WORD) temp);
  use(comsport)
}

/**
*** The code for reading is almost identical to that for writing.
**/

PRIVATE WORD RS232_receive(comsport, amount, buffer)
ComsPort *comsport;
WORD     amount;
UBYTE *  buffer;
{ RS232 *port = &rs232_port;

  if (port->overflow_count > 0)
   { if (port->overflow_count > (uint) amount)
      { memcpy(buffer, port->overflow_buf, (uint) amount);
        memmove(port->overflow_buf, &(port->overflow_buf[(uint) amount]),
               (port->overflow_count - (uint) amount) );
        port->overflow_count -= (uint) amount;
        port->inmax   = (uint) amount;
        port->incount = (uint) amount;
        return(TRUE);
      }
    else
      { port->inmax   = (uint) amount;
        port->incount = port->overflow_count;
        port->inbuf   = &(buffer[port->overflow_count]);
        memcpy(buffer, port->overflow_buf, port->overflow_count);
      }
   }
  else
   { port->inmax    = (uint) amount;
     port->incount  = 0;
     port->inbuf    = buffer;
   }

  port->overflow_count = 0;

  if (port->flags & RS232_UseHardware)   /* Support hardware handshaking */
   RS232_setDTR(1);

  if (port->flags & RS232_inXOFF)       /* Did I XOFF ? */
   { 
     port->flags |= RS232_NeedToXon;
     if (port->flags & RS232_ClearToWrite)
      RS232_sendchar(port);
   }

  return(TRUE);
  use(comsport)
}

PRIVATE WORD RS232_pollread(comsport)
ComsPort *comsport;
{ RS232 *port = &rs232_port;

  if (debugflags & Com_Flag)
   ServerDebug("RS232 read : read %d of %d bytes, flag = %x",
             port->incount, port->inmax, port->flags);

  if (port->inmax eq -1)      /* Break occurred */
    { if (port->flags & RS232_UseHardware)
       RS232_setDTR(0);
      return((WORD) (port->incount));
    }
 
  if (port->incount >= port->inmax)
   { port->inbuf   = (BYTE *) NULL;
     port->inmax   = 0;
     port->incount = 0;
     if (port->flags & RS232_UseHardware)
       RS232_setDTR(0);
     return(-1L);
   } 

  return(-2L);
  use(comsport)
}

PRIVATE WORD RS232_abortread(comsport)
ComsPort *comsport;
{ RS232 *port = &rs232_port;
  WORD  temp;

  port->inbuf   = (BYTE *) NULL;
  port->inmax   = 0;
  temp          = (WORD) port->incount;
  port->incount = 0;
  if (port->flags & RS232_UseHardware)
   RS232_setDTR(0);

  return(temp);
  use(comsport)
}

#define UCR_Even   0x02
#define UCR_Parity 0x04
#define UCR_1stop  0x08
#define UCR_2stop  0x18
#define UCR_5bits  0x60
#define UCR_6bits  0x40
#define UCR_7bits  0x20
#define UCR_8bits  0x00
#define b_19200    0x101
#define b_9600     0x102
#define b_4800     0x104
#define b_2400     0x108
#define b_1800     0x10b
#define b_1200     0x110
#define b_600      0x120
#define b_300      0x140
#define b_200      0x160
#define b_150      0x180
#define b_134      0x18F
#define b_110      0x1AF
#define b_75       0x240
#define b_50       0x260

PRIVATE void RS232_configure(comsport)
ComsPort *comsport;
{ RS232 *port = &rs232_port;
  Attributes *attr = &(comsport->attr);
  int   UCR = 0x0080;         /* Top bit must be set */
  int   baud = 0;
  int   do_break = 0;

  switch((int) (GetInputSpeed(attr)))
   { case RS232_B50    : baud = b_50; break;
     case RS232_B75    : baud = b_75; break;
     case RS232_B110   : baud = b_110; break;
     case RS232_B134   : baud = b_134; break;
     case RS232_B150   : baud = b_150; break;
     case RS232_B200   : baud = b_200; break;
     case RS232_B300   : baud = b_300; break;
     case RS232_B600   : baud = b_600; break;
     case RS232_B1200  : baud = b_1200; break;
     case RS232_B1800  : baud = b_1800; break;
     case RS232_B2400  : baud = b_2400; break;
     case RS232_B4800  : baud = b_4800; break;
     case RS232_B19200 : baud = b_19200; break;

     case RS232_B0     : do_break++;
     case RS232_B9600  :         /* This is the default baud rate */
     default           :
     case RS232_B38400 : baud = b_9600;
                         SetInputSpeed(attr, (WORD) RS232_B9600);
                         break;
   }

  if (IsAnAttribute(attr, RS232_Csize_5))
    UCR += UCR_5bits;
  elif (IsAnAttribute(attr, RS232_Csize_6))
    UCR += UCR_6bits;
  elif (IsAnAttribute(attr, RS232_Csize_7))
    UCR += UCR_7bits;
  else
    UCR += UCR_8bits;

  if (IsAnAttribute(attr, RS232_Cstopb))
    UCR += UCR_2stop;
  else
    UCR += UCR_1stop;

  if (IsAnAttribute(attr, RS232_ParEnb))
    UCR += UCR_Parity;

  if (!IsAnAttribute(attr, RS232_ParOdd))
    UCR += UCR_Even;

  if (IsAnAttribute(attr, RS232_IgnPar))
   port->flags |= RS232_IgnorePar;
  else
   port->flags &= ~RS232_IgnorePar;

  if (IsAnAttribute(attr, RS232_ParMrk))
   port->flags |= RS232_ParMark;
  else
   port->flags &= ~RS232_ParMark;

  if (IsAnAttribute(attr, RS232_InPck))
   port->flags |= RS232_ParCheck;
  else
   port->flags &= ~RS232_ParCheck;

           /* Now check for the handshaking protocol, this may leave me */
           /* clear to send a message */
           /* First, see whether hardware handshaking is to be used */
  if (!IsAnAttribute(attr, RS232_CLocal))
   { port->flags |= RS232_UseHardware;
     if ((port->flags & RS232_TXEmpty) && (port->flags & RS232_CTS))
      port->flags |= RS232_ClearToWrite;
     else
      port->flags &= ~RS232_ClearToWrite;
   }
  else
   { port->flags &= ~RS232_UseHardware;
     if (port->flags & RS232_TXEmpty)
      port->flags |= RS232_ClearToWrite;
     else
      port->flags &= ~RS232_ClearToWrite;
   }
         /* Next check input XON/XOFF */
         /* This involves checking whether I have sent an XOFF already, which */
         /* must be disabled ASAP */
  if (IsAnAttribute(attr, RS232_IXOFF))
   port->flags |= RS232_UseInXoff;
  else
   { port->flags &= ~RS232_UseInXoff;
     if (port->flags & RS232_inXOFF)    /* Have I sent an XOFF ? */
      { port->flags |= RS232_NeedToXon;
        port->flags &= ~RS232_inXOFF;
      }
   }

             /* Now check for output Xon/Xoff control */
  if (IsAnAttribute(attr,  RS232_IXON))
   port->flags |= RS232_UseOutXoff;
  else
   { port->flags &= ~RS232_UseOutXoff;
     if (port->flags & RS232_outXOFF)   /* Have I received an XOFF ? */
      port->flags &= ~RS232_outXOFF;    /* Yes, forget about it      */
   }

  if (do_break)
   Supexec(RS232_dobreak);

  RS232_arg1 = UCR; RS232_arg2 = baud;
  Supexec(RS232_setattrib);

  if (port->flags & RS232_ClearToWrite)
    RS232_sendchar(port);
}

/**
*** The done function is used to send a break signal when the stream is closed
*** assuming that the relevant attribute is set.
**/
PRIVATE void RS232_done(comsport)
ComsPort *comsport;
{ RS232 *port = &rs232_port;
  Attributes *attr = &(comsport->attr);

  if (IsAnAttribute(attr, RS232_HupCl))    /* Send a break signal */
   { SetInputSpeed(attr, (WORD) RS232_B0);
     RS232_configure(comsport);
   }
         /* Clear all the flags that are likely to give problems */
  port->flags &= ~(RS232_inXOFF + RS232_outXOFF + RS232_BreakDetected +
                   RS232_NeedToXoff + RS232_NeedToXon);
  port->breakint = NullPort;
  port->modemint = NullPort;
  Supexec(RS232_disable_interrupts);
}

void RS232_disable_events(comsport)
ComsPort *comsport;
{ RS232 *port = &rs232_port;

  port->breakint = NullPort;
  port->modemint = NullPort;
  use(comsport)
}

void RS232_enable_events(comsport, mask, event_port)
ComsPort *comsport;
WORD mask, event_port;
{ RS232 *port = &rs232_port;

  if (mask & Event_SerialBreak)
   port->breakint = event_port;
  else
   port->modemint = event_port;
  use(comsport)
}

void RS232_check_events()
{ RS232 *port = &rs232_port;
  extern void RS232_send_event();

  if ((port->flags & RS232_BreakDetected) && (port->breakint ne NullPort))
    RS232_send_event(port->breakint, Event_SerialBreak);
  elif ((port->flags & RS232_RingDetected) && (port->modemint ne NullPort))
    RS232_send_event(port->modemint, Event_ModemRing);
  port->flags &= ~(RS232_BreakDetected + RS232_RingDetected);
}

/**
*** Here are the routines called from interrupt code.
**/

#define LSR_OverRun      0x0040
#define LSR_Framing      0x0010
#define LSR_Parity       0x0020
#define LSR_Break        0x0008

void RS232_error(code)
int code;
{ RS232 *port = &rs232_port;
  Attributes *attr = &(RS232_ComsPort.attr);

  if (code & LSR_OverRun)
   { RS232_newcha(port, 0x00FF); RS232_newcha(port, 1);  
   }
  if (code & (LSR_Framing + LSR_Parity))
   { if (IsAnAttribute(attr, RS232_InPck))
      { if (IsAnAttribute(attr, RS232_IgnPar))
         { RS232_newcha(port, 0x00FF); RS232_newcha(port, 0);
           RS232_newcha(port, 0);
         } 
        else
         RS232_newcha(port, 0);
      }
   }
  if (code & LSR_Break)
   { if (IsAnAttribute(attr, RS232_IgnoreBreak))
      return;
     if (IsAnAttribute(attr, RS232_BreakInterrupt))
      { if (port->breakint ne NullPort)
         { port->flags |= RS232_BreakDetected;
           RS232_errno = 1;
         }
        if (port->inbuf ne (BYTE *) NULL)
         { port->inmax = -1; port->inbuf = (BYTE *) NULL; }
        if (port->outbuf ne (BYTE *) NULL)
         { port->outmax = -1; port->outbuf = (BYTE *) NULL; }
      }
    else
     RS232_newcha(port, 0);
   }

}
 
void RS232_gotcha(ch)
int ch;
{ RS232 *port = &rs232_port;

  if (((ch eq XON) || (ch eq XOFF)) && (port->flags & RS232_UseOutXoff))
   { if (ch eq XON)
      { port->flags &= ~RS232_outXOFF;
        if (port->flags & RS232_ClearToWrite) 
          RS232_sendchar();
      }
     elif (ch eq XOFF)
      port->flags |= RS232_outXOFF;
   }
  else
   { if (port->flags & RS232_StripIn)       /* Strip off bit 8 ? */
      RS232_newcha(port, (ch & 0x7F));
     elif (ch eq 0x00ff)                    /* Byte 0377, used for errors ? */
      { if (port->flags & RS232_IgnorePar)
          { RS232_newcha(port, 0x00FF); RS232_newcha(port, 0x00FF); }
        else
         RS232_newcha(port, 0x00FF);
      }
     else
       RS232_newcha(port, ch);
   }
}

/**
*** RS232_sendcha() is called when the transmit register is empty. It sets
*** the appropriate flags, allowing for hardware handshaking, and calls
*** RS232_sendchar().
**/

void RS232_sendcha()
{ RS232 *port = &rs232_port;

  if (port->flags & RS232_UseHardware)
   { port->flags |= RS232_TXEmpty;
     if (port->flags & RS232_CTS)
      { port->flags |= RS232_ClearToWrite;
        RS232_sendchar(port);
      }
   }
  else
   { port->flags |= (RS232_ClearToWrite | RS232_TXEmpty);
     RS232_sendchar(port);
   }
}

/**
*** This routine is called after a modem status register interrupt. The
*** cases I am interested in are clear-to-send and ring-indicator.
**/
void RS232_modem()
{ RS232 *port = &rs232_port;

  if (port->modemint ne NullPort)
   { port->flags |= RS232_RingDetected;
     RS232_errno = 1;
   }
}

void RS232_gotCTS(flag)
int flag;
{ RS232 *port = &rs232_port;

  if (flag)
   { port->flags |= RS232_CTS;
     if (port->flags & RS232_UseHardware)
     if (port->flags & RS232_TXEmpty)
      { port->flags |= RS232_ClearToWrite;
        RS232_sendchar(port);
      }
   }
  else
   { port->flags &= ~RS232_CTS;
     if (port->flags & RS232_UseHardware)
      port->flags &= ~RS232_ClearToWrite;
   }
}   


/**
*** RS232_newcha() is called when a character has been received, or when an
*** error has occurred that should generate input. No processing is performed
*** on the character. If I have a buffer I put the character into it, checking
*** for the case of a full buffer. Otherwise I try to put it into the
*** overflow buffer if there is any space. If the overflow buffer is filling
*** up, I check whether it is desirable to send an XOFF.
**/

PRIVATE void RS232_newcha(port, ch)
RS232 *port;
int ch;
{                                /* Can I fit into the existing buffer ? */
  if (port->inbuf ne (BYTE *) NULL)
   { *(port->inbuf)++ = ch;
     if ((++port->incount) >= port->inmax)
      port->inbuf = (BYTE *) NULL;
   }
  else                   /* No, try the overflow buffer */
   { if (port->overflow_count < overflow_max)             /* Any space ? */
      { port->overflow_buf[port->overflow_count++] = ch;  /* Yes         */
        if (port->overflow_count > (overflow_max / 2))    /* Should I XOFF ? */
         if ((port->flags & RS232_UseInXoff) &&           /* Xon/Xoff in use ? */
             !(port->flags & RS232_inXOFF))               /* And not XOFFed already ? */ 
          { port->flags |= RS232_NeedToXoff;
            if (port->flags & RS232_ClearToWrite)
             RS232_sendchar(port);
         }
      }
   }
}

/**
*** RS232_sendchar() is called when it is legal to write a character, and
*** there might be data to write. This data include XON/XOFF characters.
*** If the other side has XOFF'ed then the routine just returns.
**/

PRIVATE void RS232_sendchar(port)
RS232 *port;
{ int ch;


  if (port->flags & RS232_NeedToXon)
   { port->flags &= ~(RS232_NeedToXon + RS232_ClearToWrite + RS232_TXEmpty +
                       RS232_inXOFF);
     RS232_arg1 = XON;
     RS232_writech();
   }
  elif (port->flags & RS232_NeedToXoff)
   { port->flags &= ~(RS232_NeedToXoff + RS232_ClearToWrite + RS232_TXEmpty);
     port->flags |= RS232_inXOFF;
     RS232_arg1 = XOFF;
     RS232_writech();
   }
  elif (port->flags & RS232_outXOFF) /* Do not send if the other side */
    return;                          /* Has Xoff'ed */
  elif (port->outbuf ne (BYTE *) NULL)
   { ch = *(port->outbuf)++;
     if ((++port->outcount) >= port->outmax)
      port->outbuf = (BYTE *) NULL;
     port->flags &= ~(RS232_TXEmpty + RS232_ClearToWrite);
     RS232_arg1 = ch;
     RS232_writech();
   }
}

PRIVATE void RS232_setDTR(flag)
int flag;
{ if (!flag)
   Ongibit(0x10);
  else
   Offgibit(~0x10);
}

PRIVATE void RS232_setRTS(flag)
int flag;
{ if (!flag)
   Ongibit(0x08);
  else
   Offgibit(~0x08);
}


/**
*** The Centronics device - this is very similar to the RS232 one, but the
*** Centronics device cannot be read and there are no attributes for it.
**/

PRIVATE UBYTE *Centronics_write_buffer;
PRIVATE WORD  Centronics_written, Centronics_towrite;
PRIVATE ComsPort Centronics_ComsPort;
#define centronics 0

PRIVATE void fn( Centronics_done,          (ComsPort *));
PRIVATE void fn( Centronics_configure,     (ComsPort *));
PRIVATE WORD fn( Centronics_send,          (ComsPort *, WORD, UBYTE *));
PRIVATE WORD fn( Centronics_pollwrite,     (ComsPort *));
PRIVATE WORD fn( Centronics_abortwrite,    (ComsPort *));
PRIVATE WORD fn( Centronics_pollread,      (ComsPort *));
extern  void fn( Centronics_error_handler, (void));

PRIVATE WORD Centronics_null()
{ return(FALSE);
}

WORD Centronics_initlist(list, comsport)
List *list;
ComsPort **comsport;
{ Centronics_write_buffer = (UBYTE *) NULL;
  *comsport = &Centronics_ComsPort;

  Centronics_ComsPort.error_fn      = func(Centronics_error_handler);
  Centronics_ComsPort.done_fn       = func(Centronics_done);
  Centronics_ComsPort.configure_fn  = func(Centronics_configure);
  Centronics_ComsPort.send_fn       = func(Centronics_send);
  Centronics_ComsPort.pollwrite_fn  = func(Centronics_pollwrite);
  Centronics_ComsPort.abortwrite_fn = func(Centronics_abortwrite);
  Centronics_ComsPort.receive_fn    = func(Centronics_null);
  Centronics_ComsPort.pollread_fn   = func(Centronics_null);
  Centronics_ComsPort.abortread_fn  = func(Centronics_null);

  return(0L);
  use(list)
}

WORD Centronics_send(comsport, amount, buffer)
ComsPort *comsport;
WORD     amount;
UBYTE    *buffer;
{ if (Centronics_write_buffer ne (UBYTE *) NULL)
   return(FALSE);
  Centronics_write_buffer = buffer;
  Centronics_written      = 0L;
  Centronics_towrite      = amount;
  Centronics_errno        = 0L;
  return(TRUE);
  use(comsport)
}

WORD Centronics_pollwrite(comsport)
ComsPort *comsport;
{ if (Centronics_errno)
   { Centronics_write_buffer = (UBYTE *) NULL;
     return(Centronics_written);
   }

  for ( ; Centronics_written < Centronics_towrite ;)
    { if (Bcostat(centronics))           /* can I send some data ? */
       { Bconout(centronics, *Centronics_write_buffer++);   /* yes */
         if (Centronics_errno)
          { Centronics_write_buffer = (UBYTE *) NULL;
            return(Centronics_written);
          }
	 Centronics_written++;
       }
      else
       return(-2L);                /* Cannot send so return to loop */
    }
                                   /* All written */
  Centronics_write_buffer = (UBYTE *) NULL;
  return(-1L);                      /* Indicate success */  
  use(comsport)
}

WORD Centronics_abortwrite(comsport)
ComsPort *comsport;
{ Centronics_write_buffer = (UBYTE *) NULL;
  return(Centronics_written);
  use(comsport)
}

PRIVATE void Centronics_done(comsport)
ComsPort *comsport;
{ use(comsport)
}

PRIVATE void Centronics_configure(comsport)
ComsPort *comsport;
{ use(comsport)
}

/**
*** The Printer device. This contains three entries on the PC - default, serial
*** and parallel. Default maps onto either serial or parallel, depending on the
*** configuration file. Serial and parallel are done in terms of the routines
*** above.
**/
extern void fn( add_port_node, (List *, char *, ComsPort *));

WORD Printer_initlist(list, comsport)
List *list;
ComsPort **comsport;
{ 
  char   *temp = get_config("default_printer");

                              /* Work out what the default printer is */
  if (temp eq (char *) NULL)
    *comsport = &Centronics_ComsPort;
  else
   { if (!mystrcmp(temp, "SERIAL"))
      *comsport = &RS232_ComsPort;
     elif (!mystrcmp(temp, "PARALLEL"))
      *comsport = &Centronics_ComsPort;
     else
      { ServerDebug("Warning - invalid default_printer %s in host.con file.",
                temp);
        ServerDebug("Valid defaults are SERIAL and PARALLEL.");
        *comsport = &Centronics_ComsPort;
      }
   }
       
  add_port_node(list, "serial",   &RS232_ComsPort);
  add_port_node(list, "parallel", &Centronics_ComsPort);
  return(2L);      /* two directory entries added */
}

/**
*** Midi is optional on the ST - Atari will get it when they pay for it
**/
#if Midi_supported

PRIVATE UBYTE *Midi_write_buffer, *Midi_read_buffer;
PRIVATE WORD Midi_written, Midi_towrite, Midi_read, Midi_toread;

#define Midi 3

WORD Midi_initlist()
{ Midi_write_buffer = (UBYTE *) NULL;
  Midi_read_buffer  = (UBYTE *) NULL;
  return(0L);
}

WORD Midi_send(id, amount, buffer)
WORD  id, amount;
UBYTE *buffer;
{ if (Midi_write_buffer ne (UBYTE *) NULL)
   return(FALSE);
  Midi_write_buffer = buffer;
  Midi_written      = 0L;
  Midi_towrite      = amount;
  Midi_errno        = 0L;
  return(TRUE);
  use(id)
}

WORD Midi_pollwrite(id)
WORD id;
{ if (Midi_errno)
   { Midi_write_buffer = (UBYTE *) NULL;
     return(Midi_written);
   }

  for ( ; Midi_written < Midi_towrite ;)
    { if (Bcostat(Midi))                 /* can I send some data ? */
       { Bconout(Midi, *Midi_write_buffer++);             /* yes */
         if (Midi_errno)
          { Midi_write_buffer = (UBYTE *) NULL;
            return(Midi_written);
          }
	 Midi_written++;
       }
      else
       return(-2L);                /* Cannot send so return to loop */
    }
                                   /* All written */
  Midi_write_buffer = (UBYTE *) NULL;
  return(-1L);                      /* Indicate success */  
  use(id)
}

WORD Midi_abortwrite(id)
WORD id;
{ Midi_write_buffer = (UBYTE *) NULL;
  return(Midi_written);
  use(id)
}

WORD Midi_receive(id, amount, buffer)
WORD  id, amount;
UBYTE *buffer;
{ if (Midi_read_buffer ne (UBYTE *) NULL)
   return(FALSE);
  Midi_read_buffer = buffer;
  Midi_read        = 0L;
  Midi_toread      = amount;
  Midi_errno       = 0L;
  return(TRUE);
  use(id)
}

WORD Midi_pollread(id)
WORD id;
{ if (Midi_errno)
   { Midi_read_buffer = (UBYTE *) NULL;
     return(Midi_read);
   }

  for ( ; Midi_read < Midi_toread ;)
    { if (Bconstat(Midi))                    /* any data waiting ? */
       { *Midi_read_buffer++ = Bconin(Midi);               /* yes */
         if (Midi_errno)
          { Midi_read_buffer = (UBYTE *) NULL;
            return(Midi_read);
          }
	 Midi_read++;
       }
      else
       return(-2L);                /* no data so return to loop */
    }
                                   /* All read */
  Midi_read_buffer = (UBYTE *) NULL;
  return(-1L);                      /* Indicate success */  
  use(id)
}

WORD Midi_abortread(id)
WORD id;
{ Midi_read_buffer = (UBYTE *) NULL;
  return(Midi_read);
  use(id)
}

void Midi_done(id)
WORD id;
{ use(id)
}


void Midi_configure(id, attr)
WORD id;
Attributes *attr;
{ use(id)
  use(attr)
}

#endif /* Midi supported */

/*------------------------------------------------------------------------
--
-- The following code implements the local file IO routines
--
------------------------------------------------------------------------*/

/**
*** There is a system call to do the create, but it leaves the resulting file
*** open so I need to close it.
**/
WORD create_file(name)
char *name;
{ int handle = Fcreate(name, FileAttr_File);
  if (handle <= 0)                                  /* did an error occur ? */
    return(FALSE);

  Fclose(handle);      /* Helios create does not leave the file open */
  return(TRUE);
}

/**
*** get_file_info() is called only after a successful object_exists() i.e.
*** a Fsfirst(), so that all the information required is available in
*** searchbuffer.
**/
WORD get_file_info(name, Heliosinfo)
char    *name;
ObjInfo *Heliosinfo;
{ WORD itsadirectory = ((searchbuffer.attr eq FileAttr_Dir) ? TRUE : FALSE);
  WORD time = unixtime(searchbuffer.date, searchbuffer.time);

  { int i, j;
    for (i=0,j=0; i < 14; i++)             /* copy name excluding spaces */
      if (searchbuffer.name[i] eq ' ')
        continue;
      elif (searchbuffer.name[i] eq '\0')
        break;
      else                      /* lower case everything */
        Heliosinfo->DirEntry.Name[j++] = ToLower(searchbuffer.name[i]);
    Heliosinfo->DirEntry.Name[j] = '\0';
  }

                          /* we have got the info, now convert and store it */
  Heliosinfo->DirEntry.Type = swap(itsadirectory ? Type_Directory : Type_File);
  Heliosinfo->DirEntry.Matrix = swap(itsadirectory ?
                                     DefDirMatrix : DefFileMatrix);

  Heliosinfo->DirEntry.Flags  = swap(0L);
  Heliosinfo->Account   = swap(0L);
  Heliosinfo->Size      = swap(searchbuffer.size);
  Heliosinfo->Creation  = swap(time);
  Heliosinfo->Access    = swap(time);
  Heliosinfo->Modified  = swap(time);

  return(TRUE);
use(name)
}

/**
*** The TOS rename is not entirely satisfactory because it cannot overwrite
*** an existing file. Hence I may have to delete the file. Ofcourse I had
*** better check that the two files are not the same before I start deleting.
**/
WORD rename_object(fromname, toname)
char *fromname, *toname;
{ int temp;

  if (!mystrcmp(fromname, toname))                 /* rename on top of itself */
    return(TRUE);

         /* On the ST I cannot rename a file on top of an existing one, so I  */
         /* have to delete the destination if it exists                       */
  if (Fsfirst(toname, 16) eq 0)
    Fdelete(toname);

  temp = Frename(0, fromname, toname);
  return(temp eq 0 ? TRUE : FALSE);
}

/**
*** Setting the date stamp is quite tricky under TOS because it can only be
*** done on an open file. Also, I need to do some time stamp conversions.
**/
WORD set_file_date(name, unixstamp)
char *name;
WORD unixstamp;
{ int MSdate, MStime;
  int handle;
  int temp[2];
                                   /* Get the stamps in the right format */
  MSdate = MSdate_stamp(unixstamp);
  MStime = MStime_stamp(unixstamp);

  handle = Fopen(name, O_ReadOnly);
  if ((handle eq AEFILNF) ||(handle < 0) )
    return(FALSE);

  temp[0] = MStime; temp[1] = MSdate;

  Fdatime(&(temp[0]), handle, 1);

                           /* close the file again now that I am finished */
  Fclose(handle);

  return(TRUE);  
}

/**
*** There is a TOS call Dfree() which does not work as documented but which
*** does provide all the information I want.
**/
WORD get_drive_info(name, reply)
char *name;
servinfo *reply;
{ WORD data[4];
   int  drive;

  if (name[0] eq '\\')
   drive = 0;
  elif ((ToLower(name[0]) < 'a') || (ToLower(name[0]) > 'z'))
   return(FALSE);
  else
   drive = ToLower(name[0]) - 'a' + 1;      /* drive identifier, A=1 etc.*/

   reply->type = swap(Type_Directory);

   Dfree(&(data[0]), drive);

      /* data[0] is used x, data[1] is total x, x is data[2] * data[3]*/
   reply->size  = swap(data[1] * data[2] * data[3]);
   reply->used  = swap(data[0] * data[2] * data[3]);
   reply->alloc = swap(data[2] * data[3]);

   return(TRUE);
}

/**
*** This is fairly straightforward, apart from worrying about whether or not
*** to create/truncate the file.
**/
int open_file(name, Heliosmode)
char *name;
WORD Heliosmode;
{ int  handle;
  word itsmode, mymode;

  itsmode = Heliosmode & 0x0F;  /* ignore create bit */
  if (itsmode eq O_ReadOnly)
    mymode = OpenMode_ReadOnly;
  elif (itsmode eq O_WriteOnly)
    mymode = OpenMode_WriteOnly;
  elif (itsmode eq O_ReadWrite)
    mymode = OpenMode_ReadWrite;
  else
   { Server_errno = EC_Error + SS_IOProc + EG_Parameter + EO_Message;
     return(0);
   }

                                /* Open the file, creating if not found and */
                                /* the create bit is set.                   */
                                /* If WriteOnly, I need to Create because   */
                                /* Fopen does not bother with that detail   */
  handle = (mymode ne OpenMode_WriteOnly) ? Fopen(name, (int) mymode) :
		Fcreate(name, FileAttr_File);
 
  if ((handle eq AEFILNF) && ( O_Create & Heliosmode) )
    handle = Fcreate(name, FileAttr_File);

  if (handle <= 0)
   { if (handle eq AENHNDL)
      Server_errno = EC_Error + SS_IOProc + EG_NoMemory + EO_Stream;
     elif (handle eq AENSMEM)
      Server_errno = EC_Error + SS_IOProc + EG_NoMemory + EO_Server;

     return(0);
   }
  else
    return(handle);
}

/**
*** Getting the size is fairly expensive, as I need to seek to the end of file
*** and then back to my old position. In theory I could avoid the second seek
*** by fiddling with the file position pointer in the File_extra field, but
*** I prefer not to do that.
**/

WORD get_file_size(handle, old_pos)
int  handle;
WORD old_pos;
{ WORD end;

  end = Fseek(0L, handle, 2);   /* position of EOF */

  if ((Fseek(old_pos, handle, 0 ) eq -1L) || (end eq -1L))
    return(-1L); 
  else
    return(end);
}

/**
*** search_directory() is responsible for searching through the entire directory
*** whose name it is given, converting the information obtained to a
*** Helios DirEntryStructure, and storing that in the linked list passed as
*** argument. This conversion is done by routine add_node(). For details of
*** Fsfirst() and Fsnext() please consult an MSdos manual. The routine
*** returns -1 to indicate an error, otherwise the number of entries in the
*** list.
**/

PRIVATE WORD add_node();

WORD search_directory(pathname, header)
char    *pathname;
List    *header;
{ word result;
  int count = 0;

  strcat(pathname, "\\*.*");		/* get a name for search for first */

  result = (word) Fsfirst(pathname, search_FileOrDir);

  for (;;)
    { if (result eq 0L)    /* not reached the end of the search */
        { count++; unless(add_node(header)) return(-1); }
      elif ((result eq AEFILNF) || (result eq AENMFIL))
        return(count);           /* end of search */
      else
        return(-1);

      result = Fsnext();
    }
}

/**
*** When the search through the directory has revealed another entry, all
*** the information about this entry will be stored in searchbuffer. It
*** is necessary to convert this information to the Helios DirEntry
*** structure, amongst other things changing the name to lower case because
*** it looks nicer. This done I can add the entry to the linked list.
**/

PRIVATE word add_node(header)
List *header;
{ DirEntryNode *newnode;
  word type;
  int i, j;

  if (searchbuffer.attr eq search_FileOrDir)
    type = Type_Directory;
  else
    type = Type_File;

  newnode = (DirEntryNode *) malloc(sizeof(DirEntryNode));
  unless(newnode) return(FALSE);
      
  memset(newnode, 0, sizeof(DirEntryNode));
  (newnode->direntry).Type     = swap(type);
  (newnode->direntry).Flags    = swap(0L);
  (newnode->direntry).Matrix   = swap( (type eq Type_Directory) ? 
                                       DefDirMatrix : DefFileMatrix  );
  for (i=0,j=0; i < 14; i++)             /* copy name excluding spaces */
    if (searchbuffer.name[i] eq ' ') continue;
    elif (searchbuffer.name[i] eq '\0')
      break;
    else                               /* lower case everything - prettier */  
      (newnode->direntry).Name[j++] = ToLower(searchbuffer.name[i]);
  (newnode->direntry).Name[j] = '\0';

  AddTail(newnode, header);    /* put the node at the end of the list */

  return(TRUE);
}

/**
*** Code for formatting floppies. Formatting under Helios involves 16 separate
*** requests, with count varying from 0-15. For the first 14 requests I format
*** a suitable number of tracks, for the 15th request I initialise the directory
*** structure by zeroing the first two tracks, and for the 16th request I
*** prototype the boot sector.
**/

WORD format_floppy(name, count, sides, tracks, label)
char *name;
WORD count, sides, tracks;
char *label;
{ int drive = (name[0] eq 'a' || name[0] eq 'A') ? 0 : 1;
  WORD *buffer = (WORD *) malloc(8 * 1024);

  if (buffer eq NULL) return(FALSE);

  if (count eq 15L)
   { int type = (tracks eq 40L) ? ((sides eq 1) ? 0 : 1) : 
                                  ((sides eq 1) ? 2 : 3) ;
     Protobt(buffer, 0x10000000L, type, 0);
     if (Flopwr(buffer, 0L, drive, 1, 0, 0, 1) < 0)
       { free(buffer); return(FALSE); }
     else
       { free(buffer); return(TRUE); }  
   }
  elif (count eq 14L)
   { int i, temp;
     for (i = 0; i < (2 * 1024); i++)                     /* clear the buffer */
       buffer[i] = 0L;
     for (i = 0; i < 2; i++)
       { if ((temp = Flopwr(buffer, 0L, drive, 1, i, 0, 9)) < 0)
          { critical_handler(temp); return(FALSE); }
         if (sides eq 2L)
         if ((temp = Flopwr(buffer, 0L, drive, 1, i, 1, 9)) < 0)
          { critical_handler(temp); return(FALSE); }
       }
     return(TRUE);
   }
  else
   { int first_track = (tracks eq 40L) ? 3 * (int) count : 6 * (int) count;
     int last_track, i, temp;

     last_track = first_track + ((tracks eq 40L) ? 2 : 5 );
 
     if (first_track > (int) tracks) { free(buffer);  return(TRUE); }
     if (last_track >= (int) tracks) last_track = (int) tracks - 1;

     for (i = first_track; i <= last_track; i++)
      { if ((temp = 
         Flopfmt(buffer, 0x0L, drive, 9, i, 0, 1, 0x87654321L, 0xE5E5)) < 0)
          { free(buffer); critical_handler(temp); return(FALSE); }
        if (sides eq 2L)
          if (Flopfmt(buffer, 0x0L, drive, 9, i, 1, 1, 0x87654321L, 0xE5E5) < 0)
          { free(buffer); return(FALSE); }
      }

     free(buffer); return(TRUE);
   }    

  use(label)
}


static WORD trap_addr = -1L;
#define default_trap 3L

WORD call_a_trap(mcb)
MCB *mcb;
{ WordFnPtr fun;

  if (trap_addr eq -1L)
   { WORD temp = get_int_config("trap_number");
     if (temp eq Invalid_config) temp = default_trap;
     trap_addr = 0x0080L + (temp << 2);
   }

  fun = (WordFnPtr) peekl(trap_addr);
  return((*fun)(mcb));
}


/**
*** Rawdisk support
**/

extern WORD *raw_size();
extern WORD raw_io();
PRIVATE WORD *size_table;

int number_rawdisks()
{ char *conf = get_config("BOX");
  int  i;
  if (mystrcmp(conf, "ATW"))  /* KUMA board does not support raw disk */
   return(0);

  size_table = raw_size(0);

  if (size_table eq (WORD *) NULL) return(0);
  for (i = 0; i < 8; i++)
   if (size_table[i] eq 0L)
    break;

  return(i);
}

WORD size_rawdisk(drive)
int drive;
{ 
   return(size_table[drive]);
}

WORD read_rawdisk(drive, offset, size, buff)
int drive;
WORD offset;
WORD size;
BYTE *buff;
{ 
  return (raw_io(1, drive, offset, (int) size, buff));
}

WORD write_rawdisk(drive, offset, size, buff)
int drive;
WORD offset;
WORD size;
BYTE *buff;
{ 
  return(raw_io(2, drive, offset, (int) size, buff));
}


