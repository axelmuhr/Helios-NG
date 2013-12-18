/*------------------------------------------------------------------------
--									--
--		   Real Time Power System Simulator			--
--		   ================================			--
--									--
--		Copyright (C) 1989, University Of Bath.			--
--			All Rights Reserved.				--
--									--
--	msgplot.c							--
--									--
--	MicroSoft Graph Plot Library					--
--									--
--	Author:  K.W. Chan						--
--									--
--	Started: 13 Jul 89						--
--									--
------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#define MSDOS

#ifdef MSDOS
# include <graph.h>
#else
# include "graph.h"
#endif

#include "msgplot.h"

PlotScale Psca;

#define AxisColour	6
#define TextColour	3
#define TitleColour	14

#define small_no	1.0e-6
#define famod(x,y)	((x)-(int)((x)/(y))*(y))

static int CharHigh, CharWide;

typedef struct PlotVar {
	float	Min;
	float	Max;
	float	Scale;
	float	Step;
	int	Nstep;
	int	LogMin;
	int	LogMax;
} PlotVar;

static PlotVar Xvar, Yvar;

static void DoPlot( int npts, float *x, float *y, int col )
{
	float Xmin = Xvar.Min, Xsca = Xvar.Scale;
	float Ymin = Yvar.Min, Ysca = Yvar.Scale;
	float Xref = Xvar.LogMin, Yref = Yvar.LogMin;

	_setcolor( col );
	_setcliprgn( Xvar.LogMin, Yvar.LogMax, Xvar.LogMax, Yvar.LogMin);
	_moveto( (*x++ - Xmin)*Xsca + Xref, (*y++ - Ymin)*Ysca + Yref);

	while (--npts > 0)
	{
		_lineto( (*x++ - Xmin)*Xsca + Xref, (*y++ - Ymin)*Ysca + Yref);
	}
#ifndef MSDOS
	FlushMSG( WAIT );
#endif
}

static float chop( float x, int *index )
{
	int i;
	static float stptyp[] = {
		0.0f,	0.1f,	0.2f,	0.25f,	0.3f,
		0.4f,	0.5f,	0.6f,	0.75f,	0.8f,	1.0f
	};
	for (i = 0; i < sizeof(stptyp); i++) {
		if (x >= stptyp[i] && x <= stptyp[i+1]) {
			*index = ++i;
			break;
		}
	}
	return (stptyp[i]);
}

static void Round( float min, float max, PlotVar *r )
{
	static float weight[] = {
		1.0f,	0.2f,	0.3f,	0.6f,	0.8f,
		0.5f,	0.3f,	0.8f,	0.8f,	0.6f,	0.1f
	};
	float wbest, range = max - min;
	int i, iamin = 5, iamax = r->Nstep;

	for (i = iamin; i <= iamax; i++ )
	{
		int nstyp, nstep = i;
		float rd, aexp, min2, max2, w;

		float step = range / nstep;
		float logstep = log10(step);
		int nex = (int)logstep + 1;

		if (logstep < -1.0f) nex -= 1;
		aexp = pow(10.0, nex);
		rd = step = chop( step/aexp, &nstyp) * aexp;

		min2 = fabs( famod( min, rd));
		if (min < 0.0) min2 = rd - min2;
		if (fabs(min2 - rd) < (0.001 * step)) min2 = 0.0f;
		min2 = min - min2;

		max2 = min2 + nstep * step;
		if (max2 < (max - 0.001 * step)) {
			max2 += step;
			nstep++;
		}
		if (nstep > iamax) continue;

		w = ((max2 - max) + (min - min2)) * weight[nstyp];
		if (i == iamin || w < wbest) {
			r->Min = min2;
			r->Max = max2;
			r->Step = step;
			r->Nstep = nstep;
			wbest = w;
		}
	}
}

static void PreScl( float xmin, float ymin, float xmax, float ymax )
{
	float xrange, yrange;

	Xvar.Nstep = Yvar.Nstep = 10;

	Round( xmin, xmax, &Xvar);
	Round( ymin, ymax, &Yvar);

	xrange = Xvar.Max - Xvar.Min;
	yrange = Yvar.Max - Yvar.Min;

	Xvar.Step = xrange / Xvar.Nstep;
	Yvar.Step = yrange / Yvar.Nstep;

	Xvar.Scale = (Xvar.LogMax - Xvar.LogMin) / xrange;
	Yvar.Scale = (Yvar.LogMax - Yvar.LogMin) / yrange;
}

static float ArrLim( float *a, int dim, float *max )
{
	float *end = &a[dim];
	float amax = *a;
	float amin = amax;

	while (++a < end)
	{
		if (*a < amin) amin = *a;
		else if (*a > amax) amax = *a;
	}
	*max = amax;
	return amin;
}

static int SetViewPort( int col, int row, int ncol, int nrow )
{	int x;

	struct videoconfig vc;

	_setvideomode(_HRESBW);
        
	_getvideoconfig( &vc );
                
	CharWide = vc.numxpixels / vc.numtextcols;
	CharHigh = vc.numypixels / vc.numtextrows;

	if (CharWide == 0 || CharHigh == 0)
	{
		fprintf( stderr, "No Graphics Support ?????\n");
		fflush( stderr );
		return 0;
	}

	Xvar.LogMin = col * CharWide + CharWide/2;
	Xvar.LogMax = Xvar.LogMin + ncol * CharWide;

	Yvar.LogMin = row * CharHigh - CharHigh/2;
	Yvar.LogMax = Yvar.LogMin - nrow * CharHigh;

	return 1;
}

static void SetScales( int npts, float *x, float *y, int flags )
{
	float smx, smy;

	if (!(flags & P_XranOn)) Psca.Xmin = ArrLim( x, npts, &Psca.Xmax); 
	if (!(flags & P_YranOn)) Psca.Ymin = ArrLim( y, npts, &Psca.Ymax);

	smx = (Psca.Xmin + Psca.Xmax) * 0.5 * small_no;
	smy = (Psca.Ymin + Psca.Ymax) * 0.5 * small_no;

	if (flags & P_XorgOn) {
		if (Psca.Xmax < 0.0f) Psca.Xmax = 0.0f;
		else if (Psca.Xmin > 0.0f) Psca.Xmin = 0.0f;
	}
	if (flags & P_YorgOn) {
		if (Psca.Ymax < 0.0f) Psca.Ymax = 0.0f;
		else if (Psca.Ymin > 0.0f) Psca.Ymin = 0.0f;
	}
	if (fabs( Psca.Xmin - Psca.Xmax) <= smx) {
		Psca.Xmin = Psca.Xmin - smx;
		Psca.Xmax = Psca.Xmax + smx;
	}
	if (fabs( Psca.Ymin - Psca.Ymax) <= smy) {
		Psca.Ymin = Psca.Ymin - smy;
		Psca.Ymax = Psca.Ymax + smy;
	}
	PreScl( Psca.Xmin, Psca.Ymin, Psca.Xmax, Psca.Ymax);
}

static void DrawGrid( void )
{
	int i;
	float Xstep = Xvar.Step * Xvar.Scale;
	float Ystep = Yvar.Step * Yvar.Scale;

	_setcolor( AxisColour );

	for (i = 0; i <= Xvar.Nstep; i++)
	{
		int x = Xvar.LogMin + i * Xstep;
		_moveto( x, Yvar.LogMin );
		_lineto( x, Yvar.LogMax );
	}
	for (i = 0; i <= Yvar.Nstep; i++)
	{
		int y = Yvar.LogMin + i * Ystep;
		_moveto( Xvar.LogMin, y );
		_lineto( Xvar.LogMax, y );
	}
}

static void DrawAxis( void )
{
	int i, x, y;
	float Xstep = Xvar.Step * Xvar.Scale;
	float Ystep = Yvar.Step * Yvar.Scale;

	_setcolor( AxisColour );

	x = Xvar.LogMin;
	y = Yvar.LogMin;
	_moveto( x, y );
	_lineto( x, y + CharHigh/3 );

	for (i = 0; i <= Xvar.Nstep; i++)
	{
		_lineto( x, y );
		x = Xvar.LogMin + i * Xstep;
		_lineto( x, y );
		_lineto( x, y + CharHigh/3 );
	}

	x = Xvar.LogMin;
	y = Yvar.LogMin;
	_moveto( x, y );
	_lineto( x - CharWide/2, y );

	for (i = 0; i <= Yvar.Nstep; i++)
	{
		_lineto( x, y );
		y = Yvar.LogMin + i * Ystep;
		_lineto( x, y );
		_lineto( x - CharWide/2, y );
	}
}

static void PrintText( char *mt, char *xt, char *yt )
{
	char nb[20];
	int i, inc, xpos, ypos;

	float Xstep = Xvar.Step * Xvar.Scale;
	float Ystep = Yvar.Step * Yvar.Scale;

	_settextcolor( TextColour );

	inc = (Xvar.Nstep > 7) ? 2 : 1;

	for (i = 0; i <= Xvar.Nstep; i += inc)
	{
		xpos = Xvar.LogMin + i * Xstep;
		sprintf( nb, "%8g", Xvar.Min + i * Xvar.Step);
		_settextposition( Yvar.LogMin/CharHigh + 2, xpos/CharWide - 6);
		_outtext(nb);
	}

	inc = (Yvar.Nstep > 6) ? 2 : 1;

	for (i = 0; i <= Yvar.Nstep; i += inc)
	{
		ypos = Yvar.LogMin + i * Ystep;
		sprintf( nb, "%8g", Yvar.Min + i * Yvar.Step);
		_settextposition( ypos/CharHigh + 1, Xvar.LogMin/CharWide - 8);
		_outtext(nb);
	}

	_settextcolor( TitleColour );

	ypos = Yvar.LogMax/CharHigh - 1;
	xpos = (Xvar.LogMin + (Xvar.LogMax - Xvar.LogMin) / 2) / CharWide;

	_settextposition( ypos, xpos - strlen(mt)/2 );
	_outtext( mt );

	_settextposition( Yvar.LogMin/CharHigh + 4, xpos - strlen(xt)/2 );
	_outtext( xt );

	_settextposition( ypos, Xvar.LogMin/CharWide - strlen(yt)/2 - 1);
	_outtext( yt );
}

void Plot(
int npts, float *x, float *y, char *mt, char *xt, char *yt, int col, int flags )
{
/*	FlushMSG( WAIT );*/

	if ((flags & P_Redraw) || (flags & P_AutoScale))
	{
#if 1
		if (SetViewPort( 9, 22, 65, 19 ) == 0) return;
#else
		if (SetViewPort( 20, 17, 45, 10 ) == 0) return;
#endif
		if (flags & P_AutoScale) flags &= ~(P_XranOn | P_YranOn);

		SetScales( npts, x, y, flags );

		if (flags & P_Grid) DrawGrid();

		else DrawAxis();

		PrintText( mt, xt, yt );
	}
	DoPlot( npts, x, y, col );
}
#define TEST
#ifdef TEST

#define NPTS	30

int main()
{
	int i;
	float XXX[NPTS], YYY[NPTS];
	extern int rand( void );

	for ( i = 0; i < NPTS; i++) {
		YYY[i] = rand() * 0.12345678f - 1500.0f;
		XXX[i] = i;
	}
	_setvideomode(_HRESBW);

	Plot( NPTS, XXX, YYY, "Kevin", "X Axis", "Y Axis", 12, 
#ifdef GRID
	P_AutoScale | P_Redraw | P_Grid );
#else
	P_AutoScale | P_Redraw );
#endif
	for ( i = 0; i < NPTS; i++)
	{
		YYY[i] = YYY[i] * 2.45678f - 123.45643f;
		XXX[i] = XXX[i] * 1.12345678f - 0.765432f;
	}
	Plot( NPTS, XXX, YYY, "Kevin", "X Axis", "Y Axis", 14, 0); 
	while(getchar()!='\n');
	_setvideomode(_TEXTC80);
}

#endif


/***  end of msgplot.c  ***/
