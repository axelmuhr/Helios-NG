unsigned long	intpri;	/* interrupt priority level, sent by host */
unsigned long	intvec;	/* interrupt vector, sent by host */
volatile unsigned long	flag = 0;	/* go flag, sent by host */

unsigned long *VIC_virsr = (unsigned long *)0xbfff0020;

main()
{
	int	iwrd;
	
	GIEOn();
	
	while (1) {
		while (!flag) ;
			
		*(VIC_virsr + intpri) = intvec;
		iwrd = (1 << intpri) + 1;
		*(VIC_virsr) = iwrd;
		
		flag = 0;
	}
}
