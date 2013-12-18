#include <helios.h>
#include <stdio.h>
#include <posix.h>
#include <stdlib.h>

typedef struct pi_data {
	int	position;
	int	number_workers;
	int	intervals;
} pi_data;

double eval(int position, int number_workers, int intervals);

int main(void)
{ pi_data 	data;
  double	result;

  read(0, (BYTE *) &data, sizeof(pi_data));

  result = eval(data.position, data.number_workers, data.intervals);

  write(1, (BYTE *) &result, sizeof(double));

  return(0);
}

double eval(int position, int number_workers, int intervals)
{ int		first, current, last;
  double	width, sum, tmp;
  
  sum		= 0.0;
  width		= 1.0 / (double) (number_workers * intervals);
  first		= position * intervals;
  last		= first + intervals;
  
  for (current = first; current < last; current++)
   { tmp = (0.5 + (double) current) * width;
     sum = sum + width * (4.0 / (1.0 + tmp * tmp));
   }
   
  return(sum);
}     

