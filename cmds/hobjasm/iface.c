/* -> iface/c
 * Title:               Command line decoding
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
/*---------------------------------------------------------------------------*/

#include "constant.h"
#include "formatio.h"
#include "getline.h"
#include "globvars.h"
#include "iface.h"
#include "version.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/*---------------------------------------------------------------------------*/

static char input[MaxLineLength + 1] ;
static char output[MaxLineLength + 1] ;
static char errors[MaxLineLength + 1] ;

static char hdrpath[MaxLineLength + 1] ;

static BOOLEAN gotinput = FALSE ;
static BOOLEAN gotoutput = FALSE ;
static BOOLEAN goterrors = FALSE ;

static BOOLEAN gothdrpath = FALSE ;

/*---------------------------------------------------------------------------*/
/* Case insensitive string compare */
static int cistrcmp(char *s1,char *s2)
{
 char ch1 ;
 char ch2 ;
 do
  {
   ch1 = toupper(*s1++) ;
   ch2 = toupper(*s2++) ;
   if ((ch1 == ch2) && (ch1 == 0))
    return 0 ;
  } while (ch1 == ch2) ;
 return((ch1 < ch2) ? -1 : 1) ;
}

/*---------------------------------------------------------------------------*/

static void Usage(void)
{
 printf("(Upper case indicates allowable abbreviation)\n");
 printf("-From      <file> : Source\n") ;
 printf("-To        <file> : Image\n") ;
 printf("-Object    <file> : Image\n") ;
 printf("-Hdr       <path> : \"GET\" path\n") ;
 printf("-Warning   <file> : Send warnings and errors to the named file\n") ;
 printf("-Print            : Print flag (default off)\n") ;
 printf("-NOTerse          : Terse flag off (default on)\n") ;
 printf("-WIdth     <num>  : Printed width (default 131)\n") ;
 printf("-Length    <num>  : Printed length (default 60)\n") ;
 printf("-Xref             : Collect Xref information (default off)\n") ;
 printf("-NOCache          : Do not cache source files\n") ;
 printf("-help             : Print this information\n") ;
 printf("-MaxCache  <num>  : Set source file cache size (maximum 8MBytes)\n") ;
 printf("-LIbrary          : Generate code suitable for building a Helios library\n") ;
 printf("-Makedef          : Initialise GBLL variable \"make_def\" to {TRUE}\n") ;
 printf("-Smt              : Initialise GBLL variable \"make_SMT\" to {TRUE} (default)\n") ;
 printf("-NOSmt            : Initialise GBLL variable \"make_SMT\" to {FALSE}\n") ;
} /* End Usage */

/*---------------------------------------------------------------------------*/

BOOLEAN GetInterface(int argc,char *argv[],char *fileName)
{
 int   i = 1 ;
 char *arg ;

 if (argc <= 1)
  return 0 ;
 while (i < argc)
  {
   arg = argv[i] ;
   if (*arg == '-')
    {
     int ok = 1 ;
     switch (toupper(arg[1]))
      {
       case 'C' : if ((arg[2] == 0) || (cistrcmp(arg+2, "loseexec") == 0))
                   closeExec = 1 ;
                  else
                   ok = 0 ;
                  break ;

       case 'F' : if ((arg[2] == 0) || (cistrcmp(arg+2, "rom") == 0))
                   {
                    if (i+1 < argc)
                     {
                      if (!gotinput)
                       {
                        strcpy(input,argv[++i]) ;
                        gotinput = 1 ;
                       }
                      else
                       {
                        printf("Input file already specified\n") ;
                        return 0 ;
                       }
                     }
                    else
                     {
                      printf("Input file missing\n") ;
                      return 0 ;
                     }
                   }
                  else
                   ok = 0 ;
                  break ;

       case 'H' : if ((arg[2] == 0) || (cistrcmp(arg+2,"dr") == 0))
                   {
                    /* "-hdr" path specified for "GET" files */
                    if ((i + 1) < argc)
                     {
                      if (!gothdrpath)
                       {
                        strcpy(hdrpath,argv[++i]) ;
                        gothdrpath = 1 ;
                       }
                      else
                       {
                        printf("Hdr path already specified\n") ;
                        return(0) ;
                       }
                     }
                    else
                     {
                      printf("Pathname missing\n") ;
                      return(0) ;
                     }
                   }
                  else
                   {
                    if (cistrcmp(arg+2,"elp") == 0)
                     {
                      Usage() ;
                      exit(0) ;
                     }
                    else
                     ok = 0 ;
                   }
                  break ;


       case 'L' : if ((arg[2] == 0) || (cistrcmp(arg+2, "ength") == 0))
                   {
                    if (i+1 < argc)
                     {
                      int j = atoi(arg) ;
                      if ((j < 0) || (j > MaxVal))
                       printf("Length out of range, ignored\n") ;
                      else
                       maxRows = j ;
                     }
                    else
                     {
                      printf("Length specifier missing\n") ;
                      return 0 ;
                     }
                   }
                  else
                   if ((cistrcmp(arg+2,"i") == 0) || (cistrcmp(arg+2,"ibrary") == 0))
                    librarycode = TRUE ;
                   else
                    ok = 0 ;
                  break ;

       case 'M' : if ((arg[2] == 0) || (cistrcmp(arg+2, "odule") == 0))
                   module = 1 ;
                  else
                   if ((arg[2] == 0) || (cistrcmp(arg+2,"akedef") == 0))
                    clmake_def = TRUE ;
                   else
                    if ((cistrcmp(arg+2, "c") == 0) || (cistrcmp(arg+2, "axcache") == 0))
                     {
                      if (i+1 < argc)
                       {
                        int j ;
                        arg = argv[++i] ;
                        if (isdigit(*arg))
                         j = ((*arg == '0') && (arg[1] == 'x') && (isxdigit(arg[2]))) ? (int)strtoul(arg,NULL,16) : atoi(arg) ;
                        else
                         if ((*arg == '&') && isxdigit(arg[1]) && (arg[2] != 'x'))
                          j = (int)strtoul(arg+1,NULL,16) ;
                         else
                          {
                           printf("Bad value for maxCache\n") ;
                           return 0 ;
                          }
                        if (j < 0)
                         printf("MaxCache negative, ignored\n") ;
                        else
                         {
                          if (j <= MaxCache)
                           {
                            caching = -1 ;      /* TRUE */
                            maxCache = j ;
                           }
                          else
                           {
                            printf("MaxCache value too large\n") ;
                            return(0) ;
                           }
                         }
                       }
                      else
                       {
                        printf("Length specifier missing\n") ;
                        return 0 ;
                       }
                     }
                    else
                     ok = 0 ;
                  break ;

       case 'N' : if ((cistrcmp(arg+2,"ot") == 0) || (cistrcmp(arg+2,"oterse") == 0))
                   terseState = 0 ;
                  else
                   if ((cistrcmp(arg+2,"oc") == 0) || (cistrcmp(arg+2,"ocache") == 0))
                    caching = 0 ;
                   else
                    if ((cistrcmp(arg+2,"os") == 0) || (cistrcmp(arg+2,"osmt") == 0))
                     clmake_SMT = FALSE ;
                    else

                     ok = 0 ;
                  break ;

       case 'O' : if ((arg[2] == 0) || (cistrcmp(arg+2,"bject") == 0))
                   {
                    if (i+1 < argc)
                     {
                      if (!gotoutput)
                       {
                        strcpy(output,argv[++i]) ;
                        gotoutput = 1 ;
                       }
                      else
                       {
                        printf("Output file already specified\n") ;
                        return 0 ;
                       }
                     }
                    else
                     {
                      printf("Output file missing\n") ;
                      return 0 ;
                     }
                   }
                  else
                   ok = 0 ;
                  break ;

       case 'P' : if ((arg[2] == 0) || (cistrcmp(arg+2,"rint") == 0))
                   printState = 1 ;
                  else
                   ok = 0 ;
                  break ;

       case 'S' : if ((arg[2] == 0) || (cistrcmp(arg+2,"mt") == 0))
                   clmake_SMT = TRUE ;
                  else
                   ok = 0 ;
                  break ;

       case 'T' : if ((arg[2] == 0) || (cistrcmp(arg+2,"o") == 0))
                   {
                    if (i+1 < argc)
                     {
                      if (!gotoutput)
                       {
                        strcpy(output,argv[++i]) ;
                        gotoutput = 1 ;
                       }
                      else
                       {
                        printf("Output file already specified\n") ;
                        return 0 ;
                       }
                     }
                    else
                     {
                      printf("Output file missing\n") ;
                      return 0 ;
                     }
                   }
                  else
                   if ((cistrcmp(arg+2,"r") == 0) || (cistrcmp(arg+2,"race") == 0))
                    traceon = TRUE ;
                   else
                    ok = 0 ;
                  break ;

       case 'W' : if ((arg[2] == 0) || (cistrcmp(arg+2, "arning") == 0))
                   {
                    if (i+1 < argc)
                     {
                      if (!goterrors)
                       {
                        strcpy(errors,argv[++i]) ;
                        goterrors = 1 ;
                       }
                      else
                       {
                        printf("Warning file already specified\n") ;
                        return 0 ;
                       }
                     }
                    else
                     {
                      printf("Warning file missing\n") ;
                      return 0 ;
                     }
                   }
                  else
                   if ((cistrcmp(arg+2,"i") == 0) || (cistrcmp(arg+2,"idth") == 0))
                    {
                     if (i+1 < argc)
                      {
                       int j = atoi(argv[++i]) ;
                       if ((j < 0) || (j > MaxVal))
                        printf("Width out of range, ignored\\n") ;
                       else
                        maxCols = j ;
                      }
                     else
                      {
                       printf("Width specifier missing\n") ;
                       return 0 ;
                      }
                    }
                   else
                    ok = 0 ;
                  break ;

       case 'X' : if ((arg[2] == 0) || (cistrcmp(arg+2,"ref") == 0))
                   xrefOn = 1 ;
                  else
                   ok = 0 ;
                  break ;

       default  : ok = 0 ;
                  break ;
      }
     if (!ok)
      {
       printf("Unrecognised parameter '%s'\n",arg) ;
       return 0 ;
      }
    }
   else
    {
     /* Either source or code file coming up */
     if (!gotinput)
      {
       strcpy(input,arg) ;
       gotinput = 1 ;
      }
     else
      if (!gotoutput)
       {
        strcpy(output,arg) ;
        gotoutput = 1 ;
       }
      else
       {
        printf("Bad command line parameter '%s'\n",arg) ;
        return 0 ;
       }
    }
   i++ ;
  }
 if (gotoutput)
  {
   strcpy(fileName,output) ;
   return 1 ;
  }
 return 0 ;
} /* End GetInterface */

/*---------------------------------------------------------------------------*/

BOOLEAN InputFile(char *fileName)
{
  if (gotinput) strcpy(fileName, input);
  return gotinput;
} /* End InputFile */

/*---------------------------------------------------------------------------*/

BOOLEAN HdrPathname(char *pathname)
{
 if (gothdrpath)
  strcpy(pathname,hdrpath) ;
 return(gothdrpath) ;
}

/*---------------------------------------------------------------------------*/

BOOLEAN Interface_ErrorFile(char *fileName)
{
  if (goterrors) {
    strcpy(fileName, errors);
    return 1;
  };
  return 0;
} /* End ErrorFile */

/*---------------------------------------------------------------------------*/
/* EOF iface/c */
