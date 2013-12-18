
int acker(m,n)
int m;
int n;
{
   if( m == 0 ) return n+1;
   if( n == 0 ) return acker(m-1,1);
   return( acker(m-1,acker(m,n-1)) );
}

__main()
{ int i;
   for( i = 1; i <= 10; i++ )
      acker(3,6);
}
