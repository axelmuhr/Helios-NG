/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1992, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- netutils : passwd							--
--									--
--	Author:  BLV 2/10/92						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/nupasswd.c,v 1.2 1993/08/11 10:39:39 bart Exp $*/

/**
*** This module provides a very simple password encryption system.
*** Given a clear-text password and an output buffer the routine
*** generates an encoded ascii string. This can then be compared with the
*** contents of the password file or inserted into the password file.
***
*** This approach is significantly worse than the Unix approach with
*** its use of the salt. It means that all passwords in a password file
*** can be attacked using a single dictionary attack, rather than requiring
*** a dictionary attack for every single password.
**/

#include <helios.h>
#include <string.h>
#include <syslib.h>
#include "netutils.h"

static char BinToText[65] = 
	"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ./";

void	EncodePassword(char *ascii_text, char *output)
{ char	key[8];
  char	text[8];
  int	i;

	/* Conceivably the supplied password text might contain eight	*/
	/* bit characters, so performing any sort of filtering on the	*/
	/* password is dangerous. This means that passwords of more	*/
	/* than eight characters serve no purpose.			*/
  for (i = 0; i < 8; i++)
   { if (*ascii_text == '\0')
      ascii_text = text;
     text[i] = *ascii_text++;
   }

	/* The encoded password is encrypted with itself.	*/
  memcpy(key, text, 8);
  DES_ECB(true, key, text);

	/* The encrypted password is decoded into the output buffer.	*/
  output[0]	= (text[0] >> 2) & 0x3F;
  output[1]	= ((text[3] & 0x03) << 4) | ((text[7] >> 4) & 0x0F);
  output[2]	= ((text[4] & 0x0F) << 2) | ((text[3] >> 6) & 0x03);
  output[3]	= text[5] & 0x3F;
  output[4]	= (text[3] >> 2) & 0x3F;
  output[5]	= ((text[6] & 0x03) << 4) | ((text[1] >> 4) & 0x0F);
  output[6]	= ((text[7] & 0x0F) << 2) | ((text[2] >> 6) & 0x03);
  output[7]	= text[2] & 0x3F;
  output[8]	= (text[6] >> 2) & 0x3F;
  output[9]	= ((text[0] & 0x03) << 4) | ((text[4] >> 4) & 0x0F);
  output[10]	= ((text[1] & 0x0F) << 2) | ((text[5] >> 6) & 0x03);

  for (i = 0; i < (Passwd_Max - 1); i++)
   output[i] = BinToText[output[i]];
  output[Passwd_Max - 1] = '\0';
}

