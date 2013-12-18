#include <helios.h>
#include <stdio.h>
#include <posix.h>

double eval(int position, int number_workers, int intervals);

int main(void)
{ int 		position, number_workers, temp, intervals;
  double	sum, total;
  
 		/* get the worker's position in the pipeline */
  read( 0, (BYTE *) &position, sizeof(int));
  temp = position + 1;
  write(1, (BYTE *) &temp, sizeof(int));

		/* get the length of the pipeline */
  read( 0, (BYTE *) &number_workers, sizeof(int));
  write(1, (BYTE *) &number_workers, sizeof(int));
   
		/* get the number of intervals per worker */
  read( 0, (BYTE *) &intervals, sizeof(int));
  write(1, (BYTE *) &intervals, sizeof(int));
  
  sum = eval(position, number_workers, intervals);
  
  read( 0, (BYTE *) &total, sizeof(double));
  total = total + sum;
  write(1, (BYTE *) &total, sizeof(double));
  
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
