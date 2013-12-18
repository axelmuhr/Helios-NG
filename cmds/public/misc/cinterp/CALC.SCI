char *Lineptr;
int Stack[10], Stackptr, Stacktop;

calc()
{
   char line[80];
   char c;


   Stacktop = 10;
   while(1)
   {
      puts("-> ");
      Lineptr = line;
      c = gets( line );
      if(c)
      {
         if( *Lineptr=='x' )
            return;
         addition();
         printf( "%d\n", pop() );
      }
   }
}

number()
{
   if( isdigit( *Lineptr ) )
   {
      push( atoi( Lineptr ) );
      while( isdigit( *Lineptr ) )
         ++Lineptr;
   }
}

addition()
{
   int num;

   while(1)
   {
      multiplication();
      if( *Lineptr=='+' )
      {
         ++Lineptr;
         multiplication();
         push( pop() + pop() );
      }
      else if ( *Lineptr=='-' )
      {
         ++Lineptr;
         multiplication();
         num = pop();
         push( pop() - num );
      }
      else
         return;
   }
}

multiplication()
{
   int num;

   while(1)
   {
      number();
      if( *Lineptr=='*' )
      {
         ++Lineptr;
         number();
         push( pop() * pop() );
      }
      else if ( *Lineptr=='/' )
      {
         ++Lineptr;
         number();
         num = pop();
         push( pop() / num );
      }
      else
         return;
   }
}

push( n )
{
   if( Stackptr<Stacktop )
      return Stack[ Stackptr++ ] = n;
   puts( "stack overflow\n" );
}

pop()
{
   if( Stackptr>0 )
      return Stack[ --Stackptr ];
   puts( "stack underflow\n" );
}

isdigit(c){ return '0'<=c && c<='9';}
