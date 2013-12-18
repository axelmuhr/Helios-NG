/**************************************************************************/
/**            Helios I/O Server Under Microsoft Windows                 **/
/**                                                                      **/
/**        Copyright (C) 1993, Perihelion Software Limited               **/
/**                      All Rights Reserved                             **/
/**                                                                      **/
/**   Graph.h                                                            **/
/**                                                                      **/
/**   This function contains the functions shared between winsrvr.c,     **/
/**   graph.c and hel_dde.c                                              **/
/**                                                                      **/
/**************************************************************************/



#ifndef __graph_h
#define __graph_h

PUBLIC LRESULT DeferredResult;
PUBLIC BOOL    bExitMessageLoop;

/* The functions required in the message loop */
void    InitTable(void);         /* initialise the window table */
void    NotifyMenu(UINT);

void    my_Request_Return(word, word, word);
void    send_dde_to_port(Port, UINT, UINT, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD);
void    push_mcb(void);

#endif  /* __graph_h */

