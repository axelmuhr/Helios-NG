/**
*** pi2.h
***	This header file defines the job and reply packets
***	used within the second pi example, and declares the
***	worker routine.
**/

typedef struct Pi2_Job {
	int	Intervals;
} Pi2_Job;

typedef struct Pi2_Reply {
	double	PartialArea;
} Pi2_Reply;

extern	void Pi2_Worker(void);


