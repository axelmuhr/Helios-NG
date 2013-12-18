/* Changes (C)1992 Perihelion Software Limited                        */
/* Author: Alex Schuilenburg                                          */
/* Date: 5 August 1992                                                */
/* File: devinfo.c                                                    */
/*                                                                    */
/* This file contains the devinfo routines.                           */
/*                                                                    */
/*
 * $Id: devinfo.c,v 1.1 1992/09/16 09:29:06 al Exp $
 * $Log: devinfo.c,v $
 * Revision 1.1  1992/09/16  09:29:06  al
 * Initial revision
 *
 */

#include <helios.h>
#include <module.h>
#include <gsp.h>
#include <device.h>
#include <sem.h>
#include <codes.h>

/* Routines to get the device driver etc info from devinfo */
void *load_devinfo(void)
{
  Stream *s = NULL;
  Object *o;
  void *devinfo = NULL;
  int size;
  ImageHdr hdr;

  /* Locate the devinfo information */
  o = Locate(NULL,"/rom/devinfo");
  if (o == NULL) o = Locate(NULL,"/loader/DevInfo");
  if (o == NULL) o = Locate(NULL,"/helios/etc/devinfo");
  if (o == NULL) return NULL;

  /* Open it and read it */
  s = Open(o,NULL,O_ReadOnly);
  if (s == NULL) { 
    Close(o);
    return NULL;
  }
  if (Read(s,(byte *)&hdr,sizeof(hdr),-1) == sizeof(hdr)) {
    /* The header was read o.k., just check it */
    if (hdr.Magic == Image_Magic ) {
      /* Header was fine, read in from the file */
      size = hdr.Size;
      devinfo = Malloc(size);
      if (devinfo != NULL) {
      	/* Malloc OK, actual read */
        if (Read(s,devinfo,size,-1) != size) { 
          /* Read Failed */
          Free(devinfo);
          devinfo = NULL;
        }
      }
    }
  }
  Close(s);
  Close(o);

  return(devinfo);
}

InfoNode *find_info(void *devinfo, word type, char *name)
{ InfoNode *info = (InfoNode *)((Module *)devinfo + 1);

  forever {
    if ((strcmp(name,RTOA(info->Name)) == 0) && (info->Type == type))
      return(info);
		    
    if (info->Next == 0) break;
    info = (InfoNode *)RTOA(info->Next);
  }
  return(NULL);
}


