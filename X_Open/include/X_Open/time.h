/******************************************************************************
**
**
**	FILE		:	X_Open/time.h
**
**
**	DESCRIPTION	:	X-Open header file : <time.h>
**				already exists under helios
**
******************************************************************************/


extern	char	*tzname[2];
extern	long	timezone;
extern	int	daylight;

extern	struct tm *Gmtime(time_t *);
extern	struct tm *Localtime(time_t *);
extern	void Tzset(void);
extern	time_t Mktime(struct tm *);
extern	char *Asctime(struct tm *);
extern	time_t Time(time_t *);
