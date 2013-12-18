char * ptr;

int
func( int y )
{
  y >= 0 ? *ptr++ = (y % 10) + '0' : func( (y % 10) + '0' ) ;
  return 0;
}
