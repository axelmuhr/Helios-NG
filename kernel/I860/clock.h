#ifndef clock_h
#define clock_h

#define CLOCK_40 

typedef struct clock {
	volatile long	clock;
} clock;

#define clk_active 1

#ifdef CLOCK_33
#define MICROS_PER_CLK 960
#endif

#ifdef CLOCK_40
#define MICROS_PER_CLK 800
#endif

#define CLOCK ((clock *)(0xfa000000))
#endif
