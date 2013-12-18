/*------------------------------------------------------------------------
--                                                                      --
--          H E L I O S   I N P U T / O U T P U T   S E R V E R         --
--          ---------------------------------------------------         --
--                                                                      --
--               Copyright (C) 1988, Perihelion Software Ltd.           --
--                             All Rights Reserved.                     --
--                                                                      --
--  Gem.c                                                               --
--                                                                      --
--           This module contains code to interact with a GEM VDI       --
--                                                                      --
--           driver.                                                    --
--                                                                      --
--  Author:  BLV 13/7/87                                                --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: gem.c,v 1.1 1992/06/02 08:37:11 craig Exp $ */
/* Copyright (C) 1988, Perihelion Software Ltd.        */
/**
*** The usual header file and some function declarations.
**/

#define Gem_Module
#include "helios.h"

/**
*** Supporting GEM as part of the server is fairly straightforward. I need
*** two main routines,
*** int gem_loaded(void), returns non-0 if a GEM VDI is accessible. If not
*** then the Server does not create a coroutine for the GEM server, so none
*** of the code in here ever gets called.
*** void vdi(int **vdi_tab), do a vdi call with the usual arrays. vdi_tab
*** contains contrl, intin, ptsin, intout, ptsout.
***
*** There is some additional stuff for handling device interrupts. That still
*** needs to be sorted out.
**/

#if gem_supported

/**
*** vdi_tab is the table passed to the vdi call. It contains pointers
*** to the contrl, intin etc. vectors, and it is set up by Gem_InitServer.
***
*** input_modes is used to determine whether the four input devices
*** (locator, valuator, choice, string) are currently in request or sample
*** mode as far as the client is concerned. If in request mode, then I have
*** to explicitly turn them into sample mode.
*** NOTE : I may bypass the GEM input calls in the very near future and
*** implement them myself, using the information obtained by the interrupt
*** vectors.
***
*** As soon as the first work station is opened I take over the interrupt
*** vectors for mouse_move, mouse_button, and cursor_change. This allows me
*** to keep careful control over the devices. When the same work station is
*** closed I assume that the vectors are gone as well. Hmmm, this is somewhat
*** dodgy - the client might open metafile, then screen, then close metafile,
*** leaving me without events. 
*** NOTE = problem area
**/

PRIVATE unsigned int *vdi_tab[5];
PRIVATE unsigned int contrl[32], intin[255], ptsin[510], intout[255], ptsout[510];
PRIVATE int input_modes[4];
PRIVATE int vectors_installed = 0, vectors_handle;
PRIVATE void fn( install_vectors, (int));
PRIVATE void fn( pack,            (void));
PRIVATE void fn( unpack,          (void));
PRIVATE void fn( handle_input,    (Conode *));

/**
*** routine VDI() requires an array of 5 pointers which I set up here.
**/
void Gem_InitServer(myco)
Conode *myco;
{ vdi_tab[0] = &(contrl[0]);
  vdi_tab[1] = &(intin[0]);
  vdi_tab[2] = &(ptsin[0]);
  vdi_tab[3] = &(intout[0]);
  vdi_tab[4] = &(ptsout[0]);
  input_modes[0] = 1;       /* set all the devices to input request mode */
  input_modes[1] = 1;       /* a safe default */
  input_modes[2] = 1;
  input_modes[3] = 1;
  use(myco)
}

/**
*** Not sure yet what tidying is required, but I am sure there will be
*** something.
**/
void Gem_TidyServer(myco)
Conode *myco;
{ 
  use(myco)
}

/**
*** Couldn't be easier - the only purpose of opening a GEM stream is to provide
*** a communications channel. All the open_workstation stuff etc. has to be done
*** by messages.
**/
void Gem_Open(myco)
Conode *myco;
{ 
  if (strcmp(IOname, "gem") )
    { Request_Return(EC_Error + SS_IOProc + EG_Name + EO_Message, 0L, 0L);
      return;
    }

  NewStream(Type_File, Flags_Closeable + Flags_Interactive, NULL, 
            Gem_Handlers);
  use(myco)
}

/**
*** On the PC, I worry about multiple windows.
**/

#if (PC && multiple_windows)
extern void fn( switch_to_gem, (int));
extern void fn( switch_window, (int));
#endif

WORD Gem_InitStream(myco)
Conode *myco;
{
#if (PC && multiple_windows)
   switch_to_gem(1);
#endif
  return(ReplyOK);
  use(myco)
}

/**
*** Any more tidying up to be done ?
**/
void Gem_Close(myco)
Conode *myco;
{ if (mcb->MsgHdr.Reply ne 0L)
    Request_Return(ReplyOK, 0L, 0L);

#if (PC && multiple_windows)
  switch_to_gem(0);
#endif
  Seppuku();
  use(myco)
}


/**
*** GEM VDI calls are converted to private requests to a stream to /gem.
*** These requests have to be unpacked, and then I check the function code.
*** Most functions are perfectly safe and I can just call vdi().
*** The special cases are :
***
*** 1) open workstations : I do not allow access to some of the devices, only
***    to ones I know are safe. Also, if the workstation is the first to be
***    opened successfully I have to give it the interrupt vectors.
***
*** 2) close workstation : this may leave the interrupt vectors unset.
***
*** 3) rasterops : not yet implemented
***
*** 4) exchange timer vector : this can be implemented entirely on the
***    client side, so why bother me with it.
***
*** 5) mouse_move, mouse_button and cursor_change vectors : these are done
***    via Gem_EnableEvents, not by packing VDI messages. Using
***    EnableEvents makes it easy for me to get a reply port to which I can
***    send messages.
***
*** 6) set input mode : I need to remember this too.
***
*** 7) the various inputs : tricky because of the danger of being suspended
***    inside an input in request mode. I would like to handle them entirely
***    myself.
***
*** 8) some of the escape sequences : there is no way I can let myself be
***    suspended whilst the screen is dumped to a printer, for example.
***
*** Once a function has been processed, either by myself or by the VDI,
*** I can pack up the resulting data and send it back to the transputer.
**/

void Gem_PrivateStream(myco)
Conode *myco;
{ unpack();

  switch(contrl[0])
   { case   1 :         /* v_openwk         */
     case 100 :         /* v_openvwk        */
                        /* I do not allow access to printer, plotter, or  */
                        /* camera via this vdi server, because they might */
                        /* cause me to hang for a long time.              */
                        /* Also, tablets can cause problems.              */
                if (((11 <= intin[0]) && (intin[0] < 31)) ||
                    (intin[0] > 40) )
                 { contrl[2] =0; contrl[4] = 0; contrl[6] = 0; break; }
                else
                  vdi(&(vdi_tab[0]));
                pack();       /* Set up the reply, and send it off */
                Request_Return(ReplyOK, 0L, (WORD) mcb->MsgHdr.DataSize);  
                if (!vectors_installed && (contrl[6] ne 0))
                 { vectors_handle = contrl[6];
                   install_vectors(contrl[6]);
                   vectors_installed = 1;
                 }
                return;
                
     case   2 :         /* v_clswk          */
                if (contrl[6] eq vectors_handle)
                  vectors_installed = 0;
     case   3 :         /* v_clrwk          */
     case   4 :         /* v_updwk          */
     case   6 :         /* v_pline          */
     case   7 :         /* v_pmarker        */
     case   8 :         /* v_gtext          */
     case   9 :         /* v_fillarea       */
     case  10 :         /* v_cellarray      */
     case  11 :         /* generalised draw */
     case  12 :         /* vst_height       */
     case  13 :         /* vst_rotation     */
     case  14 :         /* vs_color         */
     case  15 :         /* vsl_type         */
     case  16 :         /* vsl_width        */
     case  17 :         /* vsl_color        */
     case  18 :         /* vsm_type         */
     case  19 :         /* vsm_height       */
     case  20 :         /* vsm_color        */
     case  21 :         /* vst_font         */
     case  22 :         /* vst_color        */
     case  23 :         /* vsf_interior     */
     case  24 :         /* vsf_style        */
     case  25 :         /* vsf_color        */
     case  26 :         /* vq_color         */
     case  27 :         /* vq_cellarray     */
     case  32 :         /* vswr_mode        */
     case  35 :         /* vql_attributes   */
     case  36 :         /* vqm_attributes   */
     case  37 :         /* vqf_attributes   */
     case  38 :         /* vqt_attributes   */
     case  39 :         /* vst_alignment    */
     case 101 :         /* c_clsvwk         */
     case 102 :         /* vq_extnd         */
     case 103 :         /* v_contourfill    */
     case 104 :         /* vsf_perimeter    */
     case 105 :         /* vst_background   */
     case 106 :         /* vst_effects      */
     case 107 :         /* vst_point        */
     case 108 :         /* vsl_end_style    */
     case 111 :         /* vsc_form         */
     case 112 :         /* vsf_udpat        */
     case 113 :         /* vsl_udsty        */
     case 114 :         /* vr_recfl         */
     case 115 :         /* vqi_mode         */
     case 116 :         /* vqt_extent       */
     case 117 :         /* vqt_width        */
     case 119 :         /* vst_load_fonts   */
     case 120 :         /* vst_unload_fonts */
     case 122 :         /* v_show_c         */
     case 123 :         /* v_hide_c         */
     case 124 :         /* vq_mouse         */
     case 128 :         /* vq_key_s         */
     case 129 :         /* vs_clip          */
     case 130 :         /* vqt_name         */
     case 131 :         /* vqt_fontinfo     */
                 vdi(&(vdi_tab[0]));
                 break;

     case 109 :         /* vro_cpyfm        */
     case 110 :         /* vr_trnfm         */
     case 121 :         /* vrt_cpyfm        */

     case 118 :         /* vex_timv         */
     case 125 :         /* vex_butv         */
     case 126 :         /* vex_motv         */
     case 127 :         /* vec_curv         */
     default  :
                contrl[0] = 0; contrl[2] = 0; contrl[4] = 0;
                break;

     case  33 :         /* vsin_mode        */
                if (((intin[0] < 1) || (intin[0] > 4)) ||
                    ((intin[1] < 1) || (intin[1] > 2)))
                  { contrl[0] = 0;
                    contrl[2] = 0;
                    contrl[4] = 1;
                    intout[0] = 1;
                    break;
                  }
                vdi(&(vdi_tab[0]));
                input_modes[intin[0] - 1] = intout[0];
                break;
                
     case  28 :         /* vrq_locator, vsq_locator   */
     case  29 :         /* vrq_valuator, vsq_valuator */
     case  30 :         /* vrq_choice, vsq_choice     */
     case  31 :         /* vrq_string, vsq_string     */
                        /* If the device is guaranteed in sample mode, fine */
                if (input_modes[contrl[0] - 28] eq 2)
                  vdi(&(vdi_tab[0]));
                else
                  handle_input(myco);     /* Else I have to do some polling */
                break;
                      
     case   5 :         /* escape sequences */
      switch(contrl[5])
       { case   1 :     /* vq_chcells       */
         case   2 :     /* v_exit_cur       */
         case   3 :     /* v_enter_cur      */
         case   4 :     /* v_curup          */
         case   5 :     /* v_curdown        */
         case   6 :     /* v_curright       */
         case   7 :     /* v_curleft        */
         case   8 :     /* v_curhome        */
         case   9 :     /* v_eeos           */
         case  10 :     /* v_eeol           */
         case  11 :     /* vs_curaddress    */
         case  12 :     /* v_curtext        */
         case  13 :     /* v_rvon           */
         case  14 :     /* v_rvoff          */
         case  15 :     /* vq_curaddress    */
         case  16 :     /* vq_tabstatus     */
         case  18 :     /* v_dspcur         */
         case  19 :     /* v_rmcur          */
         case  24 :     /* vq_scan          */
         case  60 :     /* vs_palette       */
         case  61 :     /* v_sound          */
         case  98 :     /* v_meta_extents   */
         case  99 :     /* v_write_meta     */
         case 100 :     /* vm_filename      */

                    vdi(&(vdi_tab[0]));
                    break;
         case  17 :     /* v_hardcopy        */
         case  20 :     /* v_form_adv        */
         case  21 :     /* v_output_window   */
         case  22 :     /* v_clear_disp_list */
         case  23 :     /* v_bit_image       */
         case  25 :     /* v_alpha_text      */
         case  91 :     /* vqp_films         */
         case  92 :     /* vqp_state         */
         case  93 :     /* vsp_state         */
         case  94 :     /* vsp_save          */
         case  95 :     /* vsp_message       */
         case  96 :     /* vqp_error         */
                   
                    contrl[0] =0; contrl[2] = 0; contrl[4] = 0;
                    break;
       }
      break;
   }

   pack();       /* Set up the reply, and send it off */
   Request_Return(ReplyOK, 0L, (WORD) mcb->MsgHdr.DataSize);  
}

/**
*** Here are the functions to pack and unpack the messages used to hold VDI
*** requests. It is important that they are kept exactly in step with the
*** related functions on the transputer side, in vdibind.c
**/
PRIVATE void unpack()
{ register UBYTE *a_ptr = mcb->Data;
  register int j;

  contrl[0] = (int) (mcb->MsgHdr.FnRc & 0x00ffL);
  contrl[1] = *a_ptr++;
  contrl[3] = *a_ptr++;
  contrl[6] = *a_ptr++;
  if ((contrl[0] eq 11) || (contrl[0] eq 5))
     contrl[5] = *a_ptr++;
  if (contrl[0] eq 10)
    { contrl[7] = *a_ptr++ << 8; contrl[7] |= *a_ptr++;
      contrl[8] = *a_ptr++ << 8; contrl[8] |= *a_ptr++;
      contrl[9] = *a_ptr++ << 8; contrl[9] |= *a_ptr++;
      contrl[10] = *a_ptr++ << 8; contrl[10] |= *a_ptr++;
    }
  if (contrl[0] eq 27)
    { contrl[7] = *a_ptr++ << 8; contrl[7] |= *a_ptr++;
      contrl[8] = *a_ptr++ << 8; contrl[8] |= *a_ptr++;
    }
    
  for (j = 0; j < contrl[3]; j++)
    { intin[j] = *a_ptr++ << 8;
      intin[j] |= *a_ptr++;
    }

  for (j = 0; j < (2 * contrl[1]); j++)
    { ptsin[j] = *a_ptr++ << 8;
      ptsin[j] |= *a_ptr++;
    }
}

PRIVATE void pack()
{ register UBYTE *a_ptr = &((mcb->Data)[0]);
  register int j;
  
   *a_ptr++ = (UBYTE) contrl[0];
   *a_ptr++ = (UBYTE) contrl[2];
   *a_ptr++ = (UBYTE) contrl[4];
   if ((contrl[0] eq 1) || (contrl[0] eq 100))
     *a_ptr++ = (UBYTE) contrl[6];
   if (contrl[0] eq 27)
     { *a_ptr++ = (UBYTE) (contrl[9] >> 8 & 0x00ff);
       *a_ptr++ = (UBYTE) (contrl[9] & 0x00ff);
       *a_ptr++ = (UBYTE) (contrl[10] >> 8 & 0x00ff);
       *a_ptr++ = (UBYTE) (contrl[10] & 0x00ff);
       *a_ptr++ = (UBYTE) (contrl[11] >> 8 & 0x00ff);
       *a_ptr++ = (UBYTE) (contrl[11] & 0x00ff);
     }
     
   for (j = 0; j < contrl[4]; j++)
     { *a_ptr++ = (UBYTE) ((intout[j] >> 8) & 0x00ff);
       *a_ptr++ = (UBYTE) ((intout[j] & 0x00ff));
     }
   for (j = 0; j < (2 * contrl[2]); j++)
     { *a_ptr++ = (UBYTE) ((ptsout[j] >> 8) & 0x00ff);
       *a_ptr++ = (UBYTE) ((ptsout[j] & 0x00ff));
     }

   mcb->MsgHdr.DataSize = (USHORT) ((WORD) a_ptr - (WORD) &((mcb->Data)[0]));
}


/**
*** The input devices. These are nasty, surprise surprise. The first thing
*** I do is take over the event interrupts, so that I know exactly what is
*** going on. I need some external routines, almost certainly in assembler
*** which get called on an interrupt basis. They have to set the variables
*** described below, to tell my polling loop when something has changed.
*** The polling loop may send the changed data on to the transputer, if
*** the transputer has enabled an event. For that I need to remember the
*** transputer ports.
**/

extern  void fn( mousemove_vector,    (void));
extern  void fn( mousebutton_vector,  (void));
extern  void fn( cursorchange_vector, (void));

extern  WORD syscursor;
extern  int button_state, button_changed;
extern  int mouse_x, mouse_y, mouse_changed;
extern  int cursor_changed;
PRIVATE WORD button_port, mouse_port, cursor_port;

/**
*** Set up my own interrupt vectors. Not quite sure how much of the
*** ptr->int conversion is machine-dependant.
**/

PRIVATE void install_vectors(handle)
int handle;
{ unsigned long a_ptr;

  contrl[0] = 125; contrl[1] = 0; contrl[3] = 0;
  contrl[6] = handle;
  a_ptr = (unsigned long) mousebutton_vector;
  contrl[7] = (int) (a_ptr & 0x0000FFFFL);
  contrl[8] = (int) ((a_ptr >> 16) & 0x0000FFFFL);
  vdi(&(vdi_tab[0]));
 
  contrl[0] = 126; contrl[1] = 0; contrl[3] = 0;
  contrl[6] = handle;
  a_ptr = (unsigned long) mousemove_vector;
  contrl[7] = (int) (a_ptr & 0x0000FFFFL);
  contrl[8] = (int) ((a_ptr >> 16) & 0x0000FFFFL);
  vdi(&(vdi_tab[0]));

#ifdef never

Fiddling with the cursor change call does not work yet. The problem is that
the interrupt routine, cursorchange_vector, needs to call the system routine
which actually draws the mouse. All this has to be done in assembler, because
I cannot rely on any of my segment registers.

  contrl[0] = 127; contrl[1] = 0; contrl[3] = 0;
  contrl[6] = handle;
  a_ptr = (unsigned long) cursorchange_vector;
  contrl[7] = (int) (a_ptr & 0x0000FFFFL);
  contrl[8] = (int) ((a_ptr >> 16) & 0x0000FFFFL);
  vdi(&(vdi_tab[0]));
  a_ptr = (((WORD) contrl[10]) << 16) & 0xffff0000L;
  a_ptr |= (((WORD) contrl[0]) & 0x0000ffffL);
  syscursor = a_ptr;

#endif
}

/**
*** When the transputer gets a vex_butv(), vex_motv(), or vex_curv() call
*** this is converted to an EnableEvents message. If the new function to
*** be used happens to be the original system one then the client program
*** wants to do a disable - this is converted to an EnableEvents with a
*** negative mask. Otherwise the transputer does an EnableEvents with a
*** positive mask corresponding to the VDI call made. If valid, I store the
*** reply port and send back success. This reply port is used by poll_gem()
*** to send events.
**/

void Gem_EnableEvents(myco)
Conode *myco;
{ word mask = mcb->Control[EnableEventsMask_off];

  if (mask < 0)     /* supposed to disable */
   { if (mask eq -125) button_port = 0L;
     if (mask eq -126) mouse_port  = 0L;
     if (mask eq -127) cursor_port = 0L;
     if ((mask >=  -127) && (mask <= -125))
      { Request_Return(ReplyOK, 0L, 0L);
#if multi_tasking
        ClearMultiwait(myco, Multi_GemInput);
#endif
      }
     else
      Request_Return(EC_Error + SS_IOProc + EG_WrongFn + EO_Stream, 0L, 0L);
     return;
   }   

  if (125L <= mask <= 127)
   { if (mask eq 125) button_port = mcb->MsgHdr.Reply;
     if (mask eq 126) mouse_port  = mcb->MsgHdr.Reply;
     if (mask eq 127) cursor_port = mcb->MsgHdr.Reply;
     mcb->Control[Reply1_off] = mask;
     mcb->MsgHdr.Flags = MsgHdr_Flags_preserve;
#if multi_tasking
     AddMultiwait(myco, Multi_GemInput);
#else
     myco->type = CoReady;
#endif
     Request_Return(ReplyOK, 1L, 0L);
   }
  else
    Request_Return(EC_Error + SS_IOProc + EG_WrongFn + EO_Stream, 0L, 0L);
}

/**
*** This routine initialises some variables used by poll_gem. It is called
*** from restart_devices.
**/
void restart_gem()
{ button_port = 0L; mouse_port = 0L; cursor_port = 0L;
}
 
/**
*** This routine is called everytime around my main loop, via
*** poll_the_devices. 
**/
void poll_gem()
{
  if (button_changed & (button_port ne 0L))
   { BYTE *a_ptr = mcb->Data;
     IOEvent event;
     mcb->Data         = (BYTE *) &event;
     mcb->MsgHdr.Reply = button_port;
     mcb->MsgHdr.Dest  = (word) NULL;
     mcb->MsgHdr.Flags = MsgHdr_Flags_preserve;
#if swapping_needed
     event.Device.Mouse.Y = button_state;
#else
     event.Device.Mouse.X = button_state;
#endif

     Request_Return(EventRc_IgnoreLost, 0L, (word) sizeof(IOEvent));
     mcb->Data = a_ptr;
     button_changed = 0;
   }

  if (mouse_changed & (mouse_port ne 0L))
   { BYTE *a_ptr = mcb->Data;
     IOEvent event;
     mcb->Data         = (BYTE *) &event;
     mcb->MsgHdr.Reply = mouse_port;
     mcb->MsgHdr.Dest  = (word) NULL;
     mcb->MsgHdr.Flags = MsgHdr_Flags_preserve;
#if swapping_needed
     event.Device.Mouse.X = mouse_y;
     event.Device.Mouse.Y = mouse_x;
#else
     event.Device.Mouse.X = mouse_x;
     event.Device.Mouse.Y = mouse_y;
#endif

     Request_Return(EventRc_IgnoreLost, 0L, (word) sizeof(IOEvent));
     mcb->Data = a_ptr;
     mouse_changed = 0;
   }
  if (cursor_changed & (cursor_port ne 0L))
   { BYTE *a_ptr = mcb->Data;
     IOEvent event;
     mcb->Data         = (BYTE *) &event;
     mcb->MsgHdr.Reply = cursor_port;
     mcb->MsgHdr.Dest  = (word) NULL;
     mcb->MsgHdr.Flags = MsgHdr_Flags_preserve;

#if swapping_needed
     event.Device.Mouse.X = mouse_y;
     event.Device.Mouse.Y = mouse_x;
#else
     event.Device.Mouse.X = mouse_x;
     event.Device.Mouse.Y = mouse_y;
#endif

     Request_Return(EventRc_IgnoreLost, 0L, (word) sizeof(IOEvent));
     mcb->Data = a_ptr;
     button_changed = 0;
   }
}

/**
*** This is the code used to handle the various inputs in request mode, which I
*** have to implement by repeated inputs in sample mode. The first step is to
*** put the device into sample mode. Then I can enter a polling loop, which
*** varies subtly from device. During this polling loop other messages can
*** arrive fouling up the arrays, so I need to save state. When the polling loop
*** finishes I must put the device back into request mode.
***
*** NOTE : I probably want to do all this input stuff myself, rather than
*** via GEM VDI calls, since the interrupt routines provide most of the data
*** I need to have. Particularly if/when I get around to taking over the
*** keyboard interrupt.
**/

PRIVATE void do_string(), sample_mode(), request_mode(), wait();
PRIVATE void show_mouse(), hide_mouse();
PRIVATE int  get_current_chwidth();

PRIVATE void handle_input(myco)
Conode *myco;
{ int device = contrl[0] - 27, handle = contrl[6];
  int intin0 = intin[0], intin1 = intin[1];
  int ptsin0 = ptsin[0], ptsin1 = ptsin[1]; 

  sample_mode(device);

  switch(device)
   { case 1 :       /* the locator */
              show_mouse(handle);
              forever
               { contrl[0] = 28; contrl[1] = 1; contrl[3] = 0;
                 contrl[6] = handle; ptsin[0] = ptsin0; ptsin[1] = ptsin1;
                 vdi(&(vdi_tab[0]));
                 if (contrl[4] eq 1) break;    /* the terminator */
                 if (contrl[2] eq 1) 
                  { ptsin0 = ptsout[0]; ptsin1 = ptsout[1]; }
                 wait(myco);
               }
              hide_mouse(handle);
              break;

     case 2 :      /* the valuator */
              forever
               { contrl[0] = 29; contrl[1] = 0; contrl[3] = 1;
                 contrl[6] = handle; intin[0] = intin0;
                 vdi(&(vdi_tab[0]));
                 if (contrl[4] eq 2) break;
                 if (contrl[4] eq 1) intin0 = intout[0];
                 wait(myco);
               }
              break;
              
     case 3 :     /* choice */
              forever
               { contrl[0] = 30; contrl[1] = 0; contrl[3] = 1;
                 contrl[6] = handle; intin[0] = intin0;
                 vdi(&(vdi_tab[0]));
                 if (contrl[4] eq 1) break;
                 wait(myco);
               }
              break;

     case 4 :    /* string */
              do_string(myco, intin0, intin1, ptsin0, ptsin1, handle);
              break;       
   }
 
  request_mode(handle, device);
}

/**
*** This is by far the most unpleasant. First, as I accumulate characters during
*** my sampling I have to save them somewhere. This involves some hacking around
*** with the vdi_tab table.  Next I need to determine the end condition myself.
*** This can be either a carriage return or having read the requested amount.
*** It is a good idea to limit this requested amount. Finally I need to worry
*** about updating the screen position passed to the VDI as characters are
*** typed in. I only worry about the x-coordinate, and increase this by the
*** widths of the characters typed in each time. I assume a constant width font,
*** and obtain the character width from the vdi.
**/
PRIVATE void do_string(myco, intin0, intin1, ptsin0, ptsin1, handle)
Conode *myco;
int intin0, intin1, ptsin0, ptsin1, handle;
{ int ch_width = get_current_chwidth(handle), read = 0;
  int save_tab[80];
  int *a_ptr = &(save_tab[0]);
  if (intin0 > 80) intin0 = 80;   /* restrict maximum length */
  forever
   { contrl[0] = 31; contrl[1] = 1; contrl[3] = 2;
     contrl[6] = handle;
     intin[0] = intin0; intin[1] = intin1;
     ptsin[0] = ptsin0; ptsin[1] = ptsin1;
     vdi_tab[3] = a_ptr;
     vdi(&(vdi_tab[0]));
     vdi_tab[3] = &(intout[0]);
     if (contrl[4] ne 0)
      { read += contrl[4];
        if ((save_tab[read-1] eq '\n') ||
            (save_tab[read-1] eq '\r'))
          break;
        intin0 -= contrl[4];    /* reduce maximum length */
        if (intin0 eq 0)
          break;
        ptsin[0] += contrl[4] * ch_width; /* update scrn pos */
      }
     wait(myco);
   }      
  for (ch_width = 0; ch_width < read; ch_width++)
   intout[ch_width] = save_tab[ch_width];
  contrl[4] = read;
}           

/**
*** This is fairly standard code for suspending a coroutine for a bit.
**/
PRIVATE void wait(myco)
Conode *myco;
{ WORD tempport = mcb->MsgHdr.Reply;
 
  AddTail(Remove(&(myco->node)), PollingCo);

  myco->timelimit = MAXINT;
  Suspend();                       /* suspend coroutine for a bit. */

  if (myco->type eq CoSuicide) /* Check for some special cases.    */
      Seppuku();

  mcb->MsgHdr.Reply = 0L;

  AddTail(Remove(&(myco->node)), WaitingCo);
}

/**
*** Here are various vdi calls that I need internally. I do not want to link in
*** a complete set of bindings.
**/
PRIVATE void sample_mode(device)
int device;
{ contrl[0] = 33; contrl[1] = 0; contrl[3] = 2;
  intin[0]  = device; intin[1] = 2;
  vdi(&(vdi_tab[0]));
}

PRIVATE void request_mode(handle, device)
int handle, device;
{ contrl[0] = 33; contrl[1] = 0; contrl[3] = 2; contrl[6] = handle;
  intin[0] = device; intin[1] = 1;
  vdi(&(vdi_tab[0]));
}

PRIVATE void show_mouse(handle)
int handle;
{ contrl[0] = 122; contrl[1] = 0; contrl[3] = 1; contrl[6] = handle;
  intin[0] = 1;
  vdi(&(vdi_tab[0]));
}

PRIVATE void hide_mouse(handle)
int handle;
{ contrl[0] = 123; contrl[1] = 0; contrl[3] = 0; contrl[6] = handle;
  vdi(&(vdi_tab[0]));
}

PRIVATE int  get_current_chwidth(handle)
int handle;
{ contrl[0] = 117; contrl[1] = 0; contrl[3] = 1; contrl[6] = handle;
  intin[0] = 'w';
  vdi(&(vdi_tab[0]));
  return(ptsout[0]);
}


#endif


