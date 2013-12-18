/*> buildtime.c <*/
/*---------------------------------------------------------------------------*/
/* Copyright (c) 1990, Active Book Company, Cambridge, United Kingdom.       */
/*---------------------------------------------------------------------------*/
/* The "buildtime" utility constructs a "hobjasm" assembler file containing  */
/* variables that hold the current time and date.                            */
/*---------------------------------------------------------------------------*/

#include <stdio.h>
#include <time.h>

/*---------------------------------------------------------------------------*/

#define whoami		"buildtime"	/* name of this program */
#define cfile		"timedate.s"	/* destination header file */
#define maxstring	256		/* maximum string length */

int main(int argc,char **argv)
{
 unsigned int  tval = time(NULL) ;   /* seconds since "January 1st 1970 GMT" */
 struct tm    *timeptr ;	     /* full time structure */
 char          destpath[maxstring] ; /* destination path */
 char	       filename[maxstring] ; /* full pathname */
 int           index ;		     /* general counter */
 char         *tptr ;		     /* temporary index */
 FILE	      *outfile = stdout ;    /* output the file */

 destpath[0] = NULL ;		/* NULL path by default */

 /* parse the command line options */
 for (index=1; (index < argc); index++)
  {
   if (argv[index][0] == '-')
    {
     switch (argv[index][1])
      {
       case 'h' :
       case 'H' : /* help */
                  printf("%s (%s at %s)\n",whoami,__DATE__,__TIME__) ;
		  printf("-h suitable help message\n") ;
 	          printf("-p path where \"%s\" files to be placed\n",cfile) ;
		  exit(0) ;

       case 'p' :
       case 'P' : /* destination path */
                  {
		   char *cptr ;
	           if (argv[index][2] == '\0')
		    cptr = argv[++index] ;
		   else
		    cptr = &argv[index][2] ;
		   if (strlen(cptr) >= maxstring)
		    {
                     fprintf(stderr,"%s: path too long\n",whoami) ;
                     exit(2) ;
		    }
		   else
		    strcpy(destpath,cptr) ;
		  }
                  break ;

       default  : /* unrecognised option */
                  fprintf(stderr,"%s: unrecognised option -%c\n",whoami,argv[index][1]) ;
                  exit(1) ;
                  break ;
      }
    }
   else
    {
     fprintf(stderr,"%s: unrecognised parameter \"%s\"\n",whoami,argv[index]) ;
     exit(1) ;
    }
  }

 if (destpath[0] == '\0')
  sprintf(filename,"%s",cfile) ;
 else
  sprintf(filename,"%s/%s",destpath,cfile) ;

 /* create the file */
 if ((outfile = fopen(filename,"wb")) == NULL)
  {
   fprintf(stderr,"%s: cannot open outfile file \"%s\"\n",whoami,filename) ;
   exit(3) ;
  }

 timeptr = gmtime(&tval) ;

 /* output the file header */
 fprintf(outfile,"\tTTL\tCreated by \"%s\"\t> %s\n\n",whoami,cfile) ;
 fprintf(outfile,"old_opt\tSETA\t{OPT}\n") ;
 fprintf(outfile,"\tOPT\t(opt_off)\n\n") ;

 fprintf(outfile,"\t\tGBLL\ttimedate_s\n") ;
 fprintf(outfile,"timedate_s\tSETL\t{TRUE}\n\n") ;

 fprintf(outfile,"\t; Time and Date variables\n") ;
 fprintf(outfile,"\tGBLA\tMakeTime\t; secs since 1st Jan 1970\n") ;
 fprintf(outfile,"\tGBLS\tMakeDate\t; full ANSI time and date\n") ;
 fprintf(outfile,"\tGBLS\tMakeDay\t\t; full weekday name\n") ;
 fprintf(outfile,"\tGBLS\tMakeMDay\t; month day\n") ;
 fprintf(outfile,"\tGBLS\tMakeMonth\t; full month name\n") ;
 fprintf(outfile,"\tGBLS\tMakeYear\t; full year number\n") ;
 fprintf(outfile,"\tGBLS\tMakeClock\t; 24hour clock\n\n") ;

 fprintf(outfile,"MakeTime\tSETA\t&%08X\n",tval) ;

 sprintf(destpath,"%s",ctime(&tval)) ;
 for (tptr = destpath; (*tptr && (*tptr != '\n')); tptr++) ;
 *tptr = '\0' ;	/* replace newline with NULL */
 fprintf(outfile,"MakeDate\tSETS\t\"%s\"\n",destpath) ;

 index = strftime(destpath,maxstring,"%A",timeptr) ;
 destpath[index] = '\0' ;
 fprintf(outfile,"MakeDay\t\tSETS\t\"%s\"\n",destpath) ;

 index = strftime(destpath,maxstring,"%d",timeptr) ;
 destpath[index] = '\0' ;
 fprintf(outfile,"MakeMDay\tSETS\t\"%s\"\n",destpath) ;

 index = strftime(destpath,maxstring,"%B",timeptr) ;
 destpath[index] = '\0' ;
 fprintf(outfile,"MakeMonth\tSETS\t\"%s\"\n",destpath) ;

 index = strftime(destpath,maxstring,"%Y",timeptr) ;
 destpath[index] = '\0' ;
 fprintf(outfile,"MakeYear\tSETS\t\"%s\"\n",destpath) ;

 index = strftime(destpath,maxstring,"%H:%M",timeptr) ;
 destpath[index] = '\0' ;
 fprintf(outfile,"MakeClock\tSETS\t\"%s\"\n",destpath) ;

 /* terminate the file cleanly */
 fprintf(outfile,"\n\tOPT\t(old_opt)\n") ;
 fprintf(outfile,"\tEND\n") ;

 fclose(outfile) ;
 return(0) ;
}

/*---------------------------------------------------------------------------*/
/*> EOF buildtime.c <*/

