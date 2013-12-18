/*------------------------------------------------------------------------
--									--
--		   Real Time Power System Simulator			--
--		   ================================			--
--									--
--		Copyright (C) 1989, University Of Bath.			--
--			All Rights Reserved.				--
--									--
--	msgplot.h							--
--									--
--	MicroSoft Graph Plot Library Header				--
--									--
--	Author:  K.W. Chan						--
--									--
--	Started: 12 May 89						--
--									--
------------------------------------------------------------------------*/

#define P_AutoScale	0x00000001
#define P_Redraw	0x00000002
#define P_Grid		0x00000004

#define P_XorgOn	0x00000010
#define P_YorgOn	0x00000020
#define P_XranOn	0x00000040
#define P_YranOn	0x00000080

typedef struct PlotScale {
	float		Xmin;
	float		Xmax;
	float		Ymin;
	float		Ymax;
} PlotScale;

extern PlotScale Psca;

extern void Plot(
  int npts, float *x, float *y, char *mt, char *xt, char *yt, int col, int flags
);


/***  end of msgplot.h  ***/
