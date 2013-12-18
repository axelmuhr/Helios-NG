/**
*** pi.h
***	This header file defines the job and reply packets
***	used within the first pi example.
**/

typedef struct Pi_Job {
	int	WorkerNumber;
	int	NumberWorkers;
	int	Intervals;
} Pi_Job;

typedef struct Pi_Reply {
	double	PartialArea;
} Pi_Reply;

