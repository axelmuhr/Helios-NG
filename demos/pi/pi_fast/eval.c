void
eval(
     int 	position,
     int	number_workers,
     int	intervals,
     double *	result )
{
  int		first;
  int		current;
  int		last;
  double	width;
  double	sum;
  double	tmp;

  
  sum		  = 0.0;
  number_workers *= intervals;  
  width		  = 1.0 / (double)number_workers;
  first		  = position * intervals;
  last		  = first + intervals;
  
  for (current = first; current < last; current++)
    {
      tmp = (0.5 + (double) current) * width;
      sum = sum + width * (4.0 / (1.0 + tmp * tmp));
    }

  *result = sum;

  return;
  
} /* eval */    

