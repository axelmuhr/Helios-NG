#include <helios.h>
#include <stdio.h>
#include <posix.h>
#include <stdlib.h>

typedef struct pi_data {
	int	position;
	int	number_workers;
	int	intervals;
} pi_data;

#define to_worker(i)   (5 + i + i)
#define from_worker(i) (4 + i + i)

int main(int argc, char **argv)
{ pi_data 	data;
  int	  	i;
  double	result, temp;
  int		number_workers = atoi(argv[1]);
  int		intervals      = atoi(argv[2]);
  
  data.number_workers = number_workers;
  data.intervals      = intervals;
  
  for (i = 0; i < data.number_workers; i++)
   { data.position = i;
     write(to_worker(i), (BYTE *) &data, sizeof(pi_data));
   }
 
  printf("Pi controller : evaluating a total of %d intervals on %d workers.\n",
         number_workers * intervals, number_workers);
         
  result = 0.0;
  for (i = 0; i < number_workers; i++)
   { read(from_worker(i), (BYTE *) &temp, sizeof(double));
     result = result + temp;
   }
   
  printf("\nCalculated value of pi is %.18f.\n", result);

  return(0);
}

