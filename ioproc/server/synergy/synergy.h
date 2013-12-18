/*------------------------------------------------------------------------
--                                                                      --
--         H E L I O S   I N P U T / O U T P U T   S E R V E R          --
--         ---------------------------------------------------          --
--                                                                      --
--              Copyright (C) 1989, Perihelion Software Ltd.            --
--                         All Rights Reserved.                         --
--                                                                      --
--  synergy.h                                                           --
--                                                                      --
--           A device for the synergy image processing subsystem        --
--                                                                      --
--  Author:  BLV 10/1/89                                                --
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: 3.8 28/3/90  Copyright (C) 1989, Perihelion Software Ltd.       */


/**
*** The section below is taken from technical note no. 11, adding devices
*** to the Helios I/O Server.
**/

{ bool temp;

  #define Synergy_InitServer       IgnoreVoid
  #define Synergy_TidyServer       IgnoreVoid
  #define Synergy_Private          Invalidfn_handler
  extern  void Synergy_Testfun(bool *);
  extern  void Synergy_Open(Conode *);
  #define Synergy_Locate           Create_handler
  #define Synergy_Create           Create_handler
  #define Synergy_Delete           Invalidfn_handler
  #define Synergy_ObjectInfo       Device_ObjectInfo_handler
  #define Synergy_ServerInfo       Invalidfn_handler
  #define Synergy_Rename           Invalidfn_handler
  #define Synergy_Link             Invalidfn_handler
  #define Synergy_Protect          Invalidfn_handler
  #define Synergy_SetDate          Invalidfn_handler
  #define Synergy_Refine           Invalidfn_handler
  #define Synergy_CloseObj         Invalidfn_handler

  PRIVATE VoidFnPtr Synergy_Handlers[handler_max] =
   { Synergy_InitServer,  Synergy_TidyServer,  Synergy_Private,
     Synergy_Testfun,
     Synergy_Open,        Synergy_Create,      Synergy_Locate,
     Synergy_ObjectInfo,  Synergy_ServerInfo,  Synergy_Delete,
     Synergy_Rename,      Synergy_Link,        Synergy_Protect,
     Synergy_SetDate,     Synergy_Refine,      Synergy_CloseObj };


  Synergy_Testfun(&temp);
  if (temp)
   { tempco            = NewCo(General_Server);
     unless(tempco) return(FALSE);
     Device_count     += 1;
     AddTail(&(tempco->node), WaitingCo);
     tempco->id        = CoCount++;
     tempco->timelimit = MAXINT;
     strcpy(tempco->name, "synergy");
     tempco->handlers  = Synergy_Handlers;
     tempco->extra     = (ptr) Type_File;
   }
}

