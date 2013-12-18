/*
 * Mandelbrot engine for the TMS320C40
 */

typedef	unsigned long	u_long;


volatile u_long	start_flag = 0;
volatile u_long	line_done = 0;
volatile u_long	width;
volatile u_long	height;
volatile u_long	maxit;
volatile u_long	intpri, intvec;
volatile double	x_start = 3.14159265359;
volatile double	x_step = -3.14159265359;
volatile double	y_start;
volatile double	y_step;
volatile double	max_val;
volatile u_long	y_coord;
u_long	buf1[1200], buf2[1200];
u_long	ycoord1, ycoord2;

unsigned long *VIC_virsr = (unsigned long *) 0xbfff0020;

#ifndef NOCC
#define	HOST_INTERRUPT()			\
	*(VIC_virsr + intpri) = intvec;		\
	*(VIC_virsr) = ((1 << intpri) + 1);	
#else
#define HOST_INTERRUPT()	asm( " trap 7" )
#endif

main()
{
	int	y;

	/*
	 * run-time initialization
	 */
	GIEOn();

	/*
	 * wait for host to start me
	 */
	while (1) {
		while (start_flag == 0) ;

		process_line(0, buf1);	/* start filling line 0 */
		ycoord1 = y_coord--;
		line_done = 1;
		HOST_INTERRUPT();
		height--;
		y = 0;
		while (height > 0) {
			process_line(y++, buf2);
			ycoord2 = y_coord--;
			while (line_done) ;
			line_done = 2;
			HOST_INTERRUPT();
			height--;
			if (height == 0) break;

			process_line(y++, buf1);
			ycoord1 = y_coord--;
			while (line_done) ;
			line_done = 1;
			HOST_INTERRUPT();
			height--;
		}
		start_flag = 0;
	}
}

process_line(int y, u_long *buf)
{
	int	x, it;
	float	a, b, real, imag, r2, i2, mag;

	b = y_start;
	for (x=0; x<width; x++) {
		a = x_start + x*x_step;
		real = a;
		imag = b;
		for (it=maxit; it>0; it--) {
			mag = (r2=real*real) + (i2=imag*imag);
			if (mag > 4.0) break;
			imag = 2*real*imag + b;
			real = r2 - i2 + a;
		}
		*buf++ = 125*it/maxit + 1;
	}
	y_start += y_step;
}
