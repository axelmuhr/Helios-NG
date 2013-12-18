/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- syslib/des.c								--
--                                                                      --
--	System Library, the common program interface to the operating   --
--	system.								--
--                                                                      --
--	Author:  NHG 16/8/87						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId:	 %W%	%G%	Copyright (C) 1987, Perihelion Software Ltd.*/
/* $Id: des.c,v 1.4 1992/04/21 17:11:03 paul Exp $ */
 
#include "sys.h"

#ifdef __TRAN
typedef unsigned long	bits_t;
#else
typedef uword bits_t;
#endif

#if 0
/* example Sfn */
static unsigned char S[][] = {
	{
	14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7,
	0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8,
	4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0,
	15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13,
	},
	{
	15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10,
	3,13,4,7,15,2,8,14,12,0,1,10,6,9,11,5,
	0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15,
	13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9,
	},
	{
	10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8,
	13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1,
	13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7,
	1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12,
	},
	{
	7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15,
	13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9,
	10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4,
	3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14,
	},
	{
	2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9,
	14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6,
	4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14,
	11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3,
	},
	{
	12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11,
	10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8,
	9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6,
	4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13,
	},
	{
	4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1,
	13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6,
	1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2,
	6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12,
	},
	{
	13,2,8,4,6,15,11,1,10,9,3,14,5,0,12,7,
	1,15,13,8,10,3,7,4,12,5,6,11,0,14,9,2,
	7,11,4,1,9,12,14,2,0,6,10,13,15,3,5,8,
	2,1,14,7,4,10,8,13,15,12,9,0,3,5,6,11
	}
};

bits_t Sfn(int n, bits_t B)
{
	int ix = (B & 0x1e) >> 1;
	if( B & 1 ) ix |= 0x10;
	ix |= ( B & 0x20 );
	return S[n][ix];
}
#else
extern bits_t Sfn(int n, bits_t B);
#endif

#include "perm.c"

extern void DES_KeySchedule(bits_t *key, bits_t *ks)
{
	bits_t C;
	bits_t D;
	bits_t CD[2];
	int i;

	permute(key, &C, PERM_PC1C );
	permute(key, &D, PERM_PC1D );

	for( i = 0; i < 16; i++ )
	{
		int shift;
		if( i < 2 || i == 8 || i == 15 ) shift = 1;
		else shift = 2;

		while( shift-- )
		{
			bits_t mask = (1<<28);
			if( C&1 ) C |= mask;
			if( D&1 ) D |= mask;
			C >>= 1;
			D >>= 1;
		}
		
		CD[0] = C, CD[1] = D;

		permute(CD,ks+2*i,PERM_PC2);
	}
}

static bits_t fun(bits_t R, bits_t *K)
{
	bits_t RE[2];
	bits_t L = 0;

	permute(&R,RE,PERM_E);
	
	RE[0] ^= K[0];
	RE[1] ^= K[1];
	
	L |= Sfn(0,RE[0]);
	L |= Sfn(1,RE[0]>>6)<<4;
	L |= Sfn(2,RE[0]>>12)<<8;	
	L |= Sfn(3,RE[0]>>18)<<12;
	L |= Sfn(4,RE[0]>>24)<<16;
	L |= Sfn(5,(RE[0]>>30)|(RE[1]<<2))<<20;
	L |= Sfn(6,RE[1]>>4)<<24;
	L |= Sfn(7,RE[1]>>10)<<28;

	permute(&L,&R,PERM_P);	
	
	return R;
}

extern void DES_Inner(bool encrypt, bits_t *source, bits_t *dest, bits_t *ks)
{
	bits_t T;
	bits_t LR[2];
		
	permute(source,LR,PERM_IP);

	if( encrypt )
	{
		LR[0] ^= fun(LR[1],ks); ks += 2;
		LR[1] ^= fun(LR[0],ks); ks += 2;
		LR[0] ^= fun(LR[1],ks); ks += 2;
		LR[1] ^= fun(LR[0],ks); ks += 2;
		LR[0] ^= fun(LR[1],ks); ks += 2;
		LR[1] ^= fun(LR[0],ks); ks += 2;
		LR[0] ^= fun(LR[1],ks); ks += 2;
		LR[1] ^= fun(LR[0],ks); ks += 2;
		LR[0] ^= fun(LR[1],ks); ks += 2;
		LR[1] ^= fun(LR[0],ks); ks += 2;
		LR[0] ^= fun(LR[1],ks); ks += 2;
		LR[1] ^= fun(LR[0],ks); ks += 2;
		LR[0] ^= fun(LR[1],ks); ks += 2;
		LR[1] ^= fun(LR[0],ks); ks += 2;
		LR[0] ^= fun(LR[1],ks); ks += 2;
		LR[1] ^= fun(LR[0],ks); ks += 2;	
	}
	else
	{
		ks += 30;
		LR[0] ^= fun(LR[1],ks); ks -= 2;
		LR[1] ^= fun(LR[0],ks); ks -= 2;
		LR[0] ^= fun(LR[1],ks); ks -= 2;
		LR[1] ^= fun(LR[0],ks); ks -= 2;
		LR[0] ^= fun(LR[1],ks); ks -= 2;
		LR[1] ^= fun(LR[0],ks); ks -= 2;
		LR[0] ^= fun(LR[1],ks); ks -= 2;
		LR[1] ^= fun(LR[0],ks); ks -= 2;
		LR[0] ^= fun(LR[1],ks); ks -= 2;
		LR[1] ^= fun(LR[0],ks); ks -= 2;
		LR[0] ^= fun(LR[1],ks); ks -= 2;
		LR[1] ^= fun(LR[0],ks); ks -= 2;
		LR[0] ^= fun(LR[1],ks); ks -= 2;
		LR[1] ^= fun(LR[0],ks); ks -= 2;
		LR[0] ^= fun(LR[1],ks); ks -= 2;
		LR[1] ^= fun(LR[0],ks); ks -= 2;
	}

	T = LR[0];
	LR[0] = LR[1];
	LR[1] = T;
	
	permute(LR,dest,PERM_IPinv);
}

extern void DES_ECB(bool encrypt, char *key, char *text)
{
	bits_t ks[32];
	
	/* switch bytes depending on endianness */
#ifdef __BIG_ENDIAN
#  error no BIG_ENDIAN code yet	
#endif
	DES_KeySchedule((bits_t *)key, ks);
	
	DES_Inner(encrypt, (bits_t *)text, (bits_t *)text, ks);

}

extern void DES_CFB(bool encrypt, char *key, char *text, int size)
{
	bits_t ks[32];
	bits_t clear[2];
	bits_t crypt[2];
	int i;
	
	DES_KeySchedule((bits_t *)key, ks);
	
	clear[0] = 0;
	clear[1] = 0;
	
	for( i = 0; i < size; i++ )
	{
		char bits;
		
		DES_Inner(TRUE, clear, crypt, ks);
		
		if( encrypt )
		{
			text[i] ^= (char)crypt[0];
			bits = text[i];
		}
		else
		{
			bits = text[i];
			text[i] ^= (char)crypt[0];
		}
		
		memcpy(clear,((char *)clear)+1,7);
		((char *)clear)[7] = bits;
	}
	
}


/* end of des.c */
