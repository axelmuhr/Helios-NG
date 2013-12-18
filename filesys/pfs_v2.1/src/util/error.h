                                                                                /*
  []-------------------------------------------------------------------------[]
   |                                                                         |
   |                    (c) 1991 by parsytec GmbH, Aachen                    |
   |                          All rights reserved.                           |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |                               Utitlities                                |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  error.h							             |
   |                                                                         |
   |    -Prototypes and definitions for error.c                              |
   |    -Handling of debug messages switches                                 |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  History:                                                               |
   |    1 - O.Imbusch - 27 March 1991 - Basic version                        |
   |                                                                         |
  []-------------------------------------------------------------------------[]
                                                                                */

#ifndef __ERROR_H
#define __ERROR_H

#ifndef DEBUG
#define DEBUG 0
#endif

#ifndef GEPDEBUG
#define GEPDEBUG 0
#endif

#ifndef FLDEBUG
#define FLDEBUG 0
#endif

#ifndef HDEBUG
#define HDEBUG 0
#endif

#include <limits.h>
#include <sem.h>
#include <string.h>

#include "procname.h"

#define DoNothing(Format,Params)           /* nothing */

extern int       ContextLine;
extern char      ContextFileName [PATH_MAX];
extern char      ContextFuncName [64];
extern char      ContextCallName [64];
extern Semaphore PESem;
extern bool      SemSet;
extern word      NOfPorts;

#define SetContext ContextLine = _LINE_,            \
                   strcpy (ContextFileName, _FILE_) 

extern int _Debug   (char *Format, ...);
extern int _Report  (char *Format, ...);
extern int _Error   (char *Format, ...);
extern int _Serious (char *Format, ...);
extern int _Fatal   (int   ExitStatus,
                     char *Format, ...);
 
#define Error   SetContext, _Error
#define Serious SetContext, _Serious
#define Fatal   SetContext, _Fatal
#define Report  SetContext, _Report

#if RELEASE

#define GEPdebug     DoNothing 
#define DEBdebug     DoNothing
#define FLdebug      DoNothing
#define Hdebug	     DoNothing
#define DebNewPort   NewPort
#define DebFreePort  FreePort
#define DebWait      Wait
#define DebSignal    Signal

#elif CHCKFRMT

#include <stdio.h>

#define GEPdebug printf
#define DEBdebug printf
#define FLdebug  printf
#define Hdebug   printf

#else

#if GEPDEBUG
#define GEPdebug SetContext, _Debug
#else
#define GEPdebug DoNothing
#endif

#if DEBUG
#define DEBdebug SetContext, _Debug
#else
#define DEBdebug DoNothing
#endif

#if FLDEBUG
#define FLdebug SetContext, _Debug
#else
#define FLdebug 
#endif

#if HDEBUG
#define Hdebug SetContext, _Debug
#else
#define Hdebug 
#endif

/*
#if FLDEBUG
#define DebNewPort()    (FLdebug ("NewPort (%d)", ++NOfPorts), NewPort ())
#define DebFreePort(P)  FLdebug ("FreePort (%d)", --NOfPorts), FreePort (P)
#else
*/
#define DebNewPort   NewPort
#define DebFreePort  FreePort
/*
#endif
*/

#if HDEBUG
#define DebWait(Sem)    Hdebug ("<W"), Wait(Sem), Hdebug (">W")
#define DebSignal(Sem)  Hdebug ("<S"), Signal(Sem), Hdebug (">S")
#else
#define DebWait(Sem)    Wait(Sem)
#define DebSignal(Sem)  Signal(Sem)
#endif

#endif

#endif

/*******************************************************************************
**
**  error.h
**
*******************************************************************************/
