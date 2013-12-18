/**
*** pi3.h
***		This header file defines the job and reply packets
***		used within the Monte Carlo version of the pi program.
**/

typedef struct Pi3_Job {
	int	Seconds;
} Pi3_Job;

typedef struct Pi3_Reply {
	int	Experiments;
	int	Hits;
} Pi3_Reply;

extern	void	Pi3_Worker(void);

