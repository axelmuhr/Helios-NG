/**
*
* Title:  Helios Debugger - Program loading.
*
* Author: Andy England
*
* Date:   September 1988
*
*         (c) Copyright 1988, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
**/

#ifdef __TRAN
static char *rcsid = "$Header: /hsrc/cmds/debugger/RCS/load.c,v 1.3 1992/10/27 13:50:59 nickc Exp $";
#endif

#include "tla.h"

/**
*
* readcode(debug);
*
* Read through loaded object to discover module names.
*
**/
PUBLIC BOOL readcode(DEBUG *debug)
{
  Stream *stream;
  ImageHdr imagehdr;
  Module module;
  BOOL   mod_exists = FALSE;

#ifdef OLDCODE
  debugf("readcode(%s)", debug->env.Objv[1]->Name);
  if ((stream = Open(debug->env.Objv[1], "", O_ReadOnly)) == NULL)return(FALSE);
#endif
/*
-- crf : 12/08/91 - clean up use of Environment Objv
*/
  debugf("readcode(%s)", debug->env.Objv[OV_Code]->Name);
  if ((stream = Open(debug->env.Objv[OV_Code], "", O_ReadOnly)) == NULL)
    return(FALSE);

  Read(stream, (byte *)&imagehdr, sizeof(ImageHdr), -1);
  do
  {
    Read(stream, (byte *)&module, sizeof(ResRef), -1);
    if (module.Type == T_Module) 
    {
    	mod_exists = addmodule(debug, module.Name, (int)module.Id);
    }
    stream->Pos += module.Size - sizeof(ResRef);
  } until (module.Type == 0);
  Close(stream);
  debugf("code has been read");
  return(mod_exists);
}
