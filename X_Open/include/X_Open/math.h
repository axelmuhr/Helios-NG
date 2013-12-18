/******************************************************************************
**
**
**	FILE		:	X_Open/math.h
**
**
**	DESCRIPTION	:	X-Open header file : <math.h>
**				already exists under Helios
**
*****************************************************************************/

extern	int isnan(double x);

extern	double hypot(double x, double y);
extern	double j0(double x);
extern	double j1(double x);
extern	double jn(int n, double x);
extern	double y0(double x);
extern	double y1(double x);
extern	double yn(int n, double x);
extern	void lcong48(unsigned short *param); 
extern	unsigned short *seed48(unsigned short *seed16v);
extern	void srand48(long seedval);
extern	double drand48(void);
extern	double erand48(unsigned short *xsubi);
long 	lrand48(void);
extern	long nrand48(unsigned short *xsubi);
extern	long mrand48(void);
extern	long jrand48(unsigned short *xsubi);
extern	double erf(double x);
extern	double erfc(double x);
extern	int 	signgam;
extern	double gamma(double x);
extern	double lgamma(double x);

