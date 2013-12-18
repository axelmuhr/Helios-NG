#include <helios.h>
#include <stdio.h>
#include <posix.h>
#include <nonansi.h>

int main(void)
{ int		number_workers, intervals;
  double	total;
  int		comm_start, comm_end, comp_start, comp_end;
    
  number_workers = 0;
  write(5, (BYTE *) &number_workers, sizeof(int));
  read( 4, (BYTE *) &number_workers, sizeof(int));
 
  printf("Pi controller : the number of workers is %d.\n", number_workers); 
 
  write(5, (BYTE *) &number_workers, sizeof(int));
  read( 4, (BYTE *) &number_workers, sizeof(int));
  
  printf("Please enter the number of intervals per worker : ");
  fflush(stdout);
  scanf("%d", &intervals);
  printf("Evaluating a total of %d intervals.\n", number_workers * intervals);
  
  comm_start = _cputime();
  write(5, (BYTE *) &intervals, sizeof(int));
  read( 4, (BYTE *) &intervals, sizeof(int));
  comm_end   = _cputime();
    
  total = 0.0;
  comp_start = _cputime();
  write(5, (BYTE *) &total, sizeof(double));
  read( 4, (BYTE *) &total, sizeof(double));
  comp_end = _cputime();
  
  printf("\nCalculated value of pi is %.17f.\n", total);
  printf("Computation time is %.3f seconds.\n",
         ((double)(comp_end - comp_start)) / 100.0);
  printf("Communication time around ring is %.3f seconds.\n",
         ((double) comm_end - comm_start) / 100.0);
  printf("Rating is approximately %d flops.\n", (int)
         ( 100.0 * 8.0 * (double)(number_workers * intervals) /
           (double)(comp_end - comp_start) ) );
         
  return(0);
}
  
