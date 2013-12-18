/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--            Copyright (C) 1987,1990, Perihelion Software Ltd.         --
--                        All Rights Reserved.                          --
--                                                                      --
-- syslib/device.c							--
--                                                                      --
--	System Library, the common program interface to the operating   --
--	system.								--
--                                                                      --
--	Author:  NHG 16/8/87						--
--		 NHG 03/8/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId:	 %W%	%G%	Copyright (C) 1987, Perihelion Software Ltd.*/
/* $Id: device.c,v 1.7 1992/11/23 16:42:42 nick Exp $ */

#include "sys.h"

/* System library device support */

#include <device.h>

extern DCB *
OpenDevice(
	   string	name,
	   void *	info )
{
  Object *	o    = NULL;
  Object *	code = NULL;
  Stream *	s    = NULL;
  DCB *		dcb  = NULL;
  MPtr		dev;
  fncast 	openfn;
	
  /* if the device is opened with an absolute path name	*/
  /* always load the code.				*/

  if ( *name == c_dirchar ) o = Locate(NULL,name);
  else
    {
      Object *	lib;


      lib = Locate( NULL, "/loader" );
		
      o = Locate( lib, name );

      Close( (Stream *)lib );
      
      if ( o == NULL )
	{
	  lib = Locate( NULL, "/helios/lib" );
	  
	  o = Locate( lib, name );
	  
	  Close( (Stream *)lib );
	}
    }

  if ( o == NULL ) goto done;

  /* we now have a handle on the device code, load it */
  
  code = Load( NULL, o );
  
  if ( code == NULL ) goto done;
  
  /* open for execute, so we can find the code */
  
  s = Open( code, NULL, O_Execute );
  
  if ( s == NULL ) goto done;
  
  /* map code into a Device and check it */
  
  dev = (MPtr)(s->Server);
  
  if ( DeviceWord_(dev,Type) != T_Device ) goto done;

  /* call the open function in the device */

  openfn.mp = DeviceOpen_(dev);

  dcb = (DCB *)(*openfn.wfn)( dev, info );
    
  if ( dcb == NULL ) goto done;
  
  /* install the code stream into the DCB so it stay */
  
  dcb->Code = s;
  s = NULL;
  
done:
  if ( o    != NULL ) Close((Stream *)o);
  if ( code != NULL ) Close((Stream *)code);
  if ( s    != NULL ) Close(s);

  return dcb;
}

extern word CloseDevice(DCB *dcb)
{
	Stream *code = dcb->Code;
	word res = (dcb->Close)(dcb);
	Close(code);
	return res;
}

