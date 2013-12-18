/*------------------------------------------------------------------------
--                                                                      --
--         H E L I O S   I N P U T / O U T P U T   S E R V E R          --
--         ---------------------------------------------------          --
--                                                                      --
--         Copyright (C) 1989, Perihelion Software Ltd.                 --
--                       All Rights Reserved.                           --
--                                                                      --
--  srv.c                                                               --
--                                                                      --
--  Author:  BLV 8/6/89                                                 --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: srv.c,v 1.2 1990/10/17 10:06:17 alan Exp $ */
/* Copyright (C) 1989, Perihelion Software Ltd.       			*/

#include <suntool/sunview.h>
#include <suntool/tty.h>
#include <sys/wait.h>
#include <stdio.h>

Icon icon;
static short icon_image[] = {
#include "/home/sp2sun1/user1/demo.icon"
};

mpr_static(helios,64,64,1,icon_image);
char *my_argv[] = {
   "/home/sp2sun1/user1/helios/server",
   "-C/home/sp2sun1/user1/helios/host.3",
0};

Frame frame;
static Notify_value my_frame_interposer();

int main()
{
  icon  = icon_create(ICON_IMAGE, &helios, 0);
  frame = window_create(NULL,FRAME,
                        FRAME_LABEL, "Tim's Demo",
			FRAME_CLOSED, TRUE,
			WIN_ROWS,    5,
                        FRAME_ICON, icon,
                        0);

  (void) notify_interpose_event_func(frame,
       my_frame_interposer, NOTIFY_SAFE);
 
  window_main_loop(frame);
}

static Notify_value my_frame_interposer(frame, event, arg, type)
Frame frame;
Event *event;
Notify_arg arg;
Notify_event_type type;
{ 
  int closed_initial, closed_current;
  Notify_value value;
  Tty tty;

  closed_initial = (int) window_get(frame, FRAME_CLOSED);

  value = notify_next_event_func(frame, event, arg, type);

  closed_current = (int) window_get(frame, FRAME_CLOSED);

  if (closed_initial != closed_current)
   if (!closed_current)
    { window_set(frame, FRAME_CLOSED, TRUE, 0); 
      run_server(); 
    }

  return value;
}

extern char **environ;

run_server()
{ int child_id;
   if ((child_id = vfork()) == 0)
     execve(my_argv[0], my_argv, environ);
   else
    wait(NULL);
}


