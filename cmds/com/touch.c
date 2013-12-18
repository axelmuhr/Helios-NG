#include <stdio.h>
#include <syslib.h>
#include <time.h>
#include <codes.h>
#include <gsp.h>

int
main(
     int 	argc,
     char *	argv[] )
{
  DateSet 	Now;
  int		i;
  Object *	obj;
  WORD		error;
  int		suppress_create = 0;
  int		error_ret = 0;

  
  for (i = 1; i < argc; i++)
   {
     char *	current = argv[ i ];
     
     
     if (*current == '-')
       {
	 current++;

	 if (*current == 'c')
	   suppress_create = 1;
         else
	   if (*current != 'f')
	     {
	       fprintf( stderr, "%s: Warning, unknown option %s.\n",
		       argv[ 0 ], argv[ i ] );
	       
	       error_ret = 1;
	     }
	 
         argv[ i ] = Null(char);
       }
   }

  for (i = 1; i < argc; i++)
   {
     if (argv[ i ] == Null(char))
       continue;
     
     Now.Creation = 0;
     Now.Access   = 0;
     Now.Modified = (Date) time((time_t *) NULL);
     
     if ((obj = Locate( CurrentDir, argv[ i ] )) == NULL)
       {
	 if (!suppress_create)
	   {
	     obj = Create( CurrentDir, argv[ i ], Type_File, 0, Null(char) );

	     if (obj != Null(Object))
	       {
		 (void) Close(obj);
		 continue;
               }
           }

	 error = Result2(CurrentDir);

	 fprintf( stderr, "%s: Failed to create %s - error code %8lx\n",
		 argv[ 0 ], argv[ i ], error );

	 error_ret = 3;
       }
     else
       {
	 if ((error = SetDate( obj, Null(char), &Now )) < 0)
	   {
	     fprintf( stderr, "%s: Failed to SetDate %s - error code %8lx\n",
		     argv[ 0 ], argv[ i ], error );
	     
	     error_ret = 2;
           }
       }
   }

  return(error_ret);

} /* main */    
