/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- domain.c								--
--                                                                      --
--	The main user domain administration command.			--
--                                                                      --
--	Author:  BLV 4/9/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/domain.c,v 1.11 1993/08/11 10:28:59 bart Exp $*/

#include <helios.h>
#include <stdarg.h>
#include <stdio.h>
#include <syslib.h>
#include <gsp.h>
#include <codes.h>
#include <nonansi.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <posix.h>
#include <root.h>
#include <ctype.h>
#include "session.h"
#include "private.h"
#include "rmlib.h"
#include "exports.h"
#include "netutils.h"

#ifndef	eq
#define	eq ==
#define ne !=
#endif

static	void		do_get(    int, char **);
static	void		do_free(   int, char **);
static	void		do_book(   int, char **);
static	void		do_cancel( int, char **);
static	void		do_info(   int, char **);
static	void		do_monitor(int, char **);
static	void		do_help(   int, char **);
static	void		do_show(   int, char **);
static	void		do_list(   int, char **);
static	void		do_lock(   int, char **);
static	void		do_unlock( int, char **);
static	void		do_preload(int, char **);
static	void		usage(void);
static	int		init_domain(RmProcessor Processor, ...);
static	int		set_domain(RmProcessor Processor, ...);
static	int		delete_unwanted(RmProcessor Processor, ...);
static	int		show_missing(RmProcessor Processor, ...);

int main(int argc, char **argv)
{ char	**real_args;

  if (argc <= 1) usage();

  if (RmGetTfm() eq Null(Object))
   { fputs("domain: cannot access a Taskforce Manager for this session.\n", stderr);
     exit(EXIT_FAILURE);
   }
   
  argc	-= 2;
  real_args = &(argv[2]);

  if (!strcmp(argv[1], "get"))
   do_get(argc, real_args);
  elif (!strcmp(argv[1], "free"))
   do_free(argc, real_args);
  elif (!strcmp(argv[1], "book"))
   do_book(argc, real_args);
  elif (!strcmp(argv[1], "cancel"))
   do_cancel(argc, real_args);
  elif (!strcmp(argv[1], "info"))
   do_info(argc, real_args);
  elif (!strcmp(argv[1], "monitor"))
   do_monitor(argc, real_args);
  elif (!strcmp(argv[1], "show"))
   do_show(argc, real_args);
  elif (!strcmp(argv[1], "list"))
   do_list(argc, real_args);
  elif (!strcmp(argv[1], "help"))
   do_help(argc, real_args);
  elif (!strcmp(argv[1], "lock"))
   do_lock(argc, real_args);
  elif (!strcmp(argv[1], "unlock"))
   do_unlock(argc, real_args);
  elif (!strcmp(argv[1], "preload"))
   do_preload(argc, real_args);
  else
   { fprintf(stderr, "domain: unknown option %s\n", argv[1]);
     usage();
   }

  return(EXIT_SUCCESS);
}

static	void	usage(void)
{ fputs("domain: usage should be one of the following\n", stderr);
  fputs("      : domain list\n", stderr);
  fputs("      : domain show\n", stderr);
  fputs("      : domain get     <processor descriptions>\n", stderr);
  fputs("      : domain free    <processors>\n", stderr);
  fputs("      : domain book    <processors>\n", stderr);
  fputs("      : domain cancel  <processors>\n", stderr);
  fputs("      : domain info    [processors]\n", stderr);
  fputs("      : domain monitor [interval] [processors]\n", stderr);
  fputs("      : domain lock\n", stderr);
  fputs("      : domain unlock\n", stderr);
  fputs("      : domain preload [C | Fortran]\n", stderr);
  fputs("      : domain help\n", stderr);
  exit(EXIT_FAILURE);
}

/**
*** do_get(), the nasty one.
***
*** 1) check the arguments. This may include templates and processor names.
*** 2) if processor names are given, get details of the whole network and
***    filter out the unwanted ones.
*** 3) if no processor names are given, create a network to hold the
***    template
*** 4) if templates are given, parse the templates and add them to the
***    network
*** 5) obtain whatever is left, and print out the results
**/

static	void	ParseTemplate(RmNetwork template, char *str);
static	int	do_get_aux1(RmProcessor, ...);
static	int	do_get_aux2(RmProcessor, ...);

static	void	do_get(int argc, char **argv)
{ RmNetwork	template;
  RmNetwork	obtained;
  bool		contains_names		= FALSE;
  bool		contains_templates	= FALSE;
  int		i;
  int		current_owner;
  int		purpose;
  int		state;
  int		rc;
  int		obtained_count;
          
  for (i = 0; i < argc; i++)
   if (*(argv[i]) eq '/')
    contains_names = TRUE;
   else
    contains_templates = TRUE;
    
  if (contains_names)
   { template	= RmGetNetwork();
     if (template eq (RmNetwork) NULL)
      { fprintf(stderr, "domain: failed to get details of network, RmLib error %s\n",
      		RmMapErrorToString(RmErrno));
      	exit(EXIT_FAILURE);
      }
     (void) RmApplyProcessors(template, &init_domain);
     for (i = 0; i < argc; i++)
      if (*(argv[i]) eq '/')
       { RmProcessor temp = RmLookupProcessor(template, argv[i]);
         if (temp eq (RmProcessor) NULL)
          { fprintf(stderr, "domain: processor %s is not in the network\n",
          	argv[i]);
            continue;
          }
	 current_owner = RmGetProcessorOwner(temp);
	 if (current_owner ne RmO_FreePool)
	  { fprintf(stderr, "domain: processor %s is not available\n",
          		argv[i]);
            continue;
          }
	 purpose = RmGetProcessorPurpose(temp);
	 if ((purpose ne RmP_Helios) && (purpose ne RmP_IO))
	  { fprintf(stderr, "domain: processor %s is not suitable\n",
	  	argv[i]);
	    continue;
	  }
	 state = RmGetProcessorState(temp);
	 unless(state & RmS_Running)
	  { fprintf(stderr, "domain: processor %s is not currently running\n",
	  		argv[i]);
	    continue;
	  }
	 RmSetProcessorPrivate(temp, 1);
       }
     (void) RmApplyProcessors(template, &delete_unwanted);
   }
  else
   { template = RmNewNetwork();
     if (template eq (RmNetwork) NULL)
      { fputs("domain: not enough memory to build template\n", stderr);
        exit(EXIT_FAILURE);
      }
   }
        
  if (contains_templates)
   for (i = 0; i < argc; i++)
    if (*(argv[i]) ne '/')
     ParseTemplate(template, *argv);

  if (RmCountProcessors(template) eq 0)
   exit(EXIT_FAILURE);
   
  obtained = RmObtainNetwork(template, FALSE, &rc);
  if (obtained eq (RmNetwork) NULL)
   { fputs("domain: failed to get any of the requested processors\n", stderr);
     exit(EXIT_FAILURE);
   }
  obtained_count = RmCountProcessors(obtained);
  RmApplyProcessors(obtained, &do_get_aux1);
  
  if (rc eq RmE_PartialSuccess)
   fprintf(stdout, "domain: only obtained %d of the requested processors.\n",
   	obtained_count);
  if ((rc eq RmE_PartialSuccess) || contains_templates)
   { fprintf(stdout, "Obtained processor%s : ", 
   	(obtained_count > 1) ? "s" : " ");
     (void) RmApplyProcessors(obtained, &do_get_aux2);
     putchar('\n');
   }
  RmReleaseNetwork(obtained);
}

static int do_get_aux1(RmProcessor Processor, ...)
{ RmSetProcessorPermanent(Processor);
  return(0);
}

static int do_get_aux2(RmProcessor Processor, ...)
{ 
  putchar('/');
  fputs(RmGetProcessorId(Processor), stdout);
  putchar(' ');
  return(0);
}

/**
*** Parsing a template, fun fun fun
**/
static void ParseTemplate(RmNetwork template, char *str)
{ int	number		= 0;
  int	ptype		= RmT_Default;
  int	memory		= 0;
  char	*attribs[10];
  int	next_attrib	= 0;
  char	*temp;
  bool  found_close	= FALSE;
  bool	native		= FALSE;
      
  while (isspace(*str)) str++;  

  while(isdigit(*str))
   { number = (10 * number) + (*str - '0');
     str++;
   }

  while(isspace(*str)) str++;
  
  if (*str eq '\0') goto done;
  if (*str ne '{')
   { fprintf(stderr, "domain: invalid template, expected {, not %s\n", str);
     exit(EXIT_FAILURE);
   }
  str++;
  while(isspace(*str)) str++;

  until((*str eq '}') || (found_close))
   { if (*str eq '\0')
      { fprintf(stderr, "domain: invalid template, missing }\n");
        exit(EXIT_FAILURE);
      }
      
     for (temp = str;
          (!isspace(*temp)) && (*temp ne '\0') && (*temp ne '=') &&
          (*temp ne ';') && (*temp ne '}');
          temp++);
     if (*temp eq '\0')
      { fprintf(stderr, "domain: invalid template, expecting processor, memory, native or attrib\n");
        exit(EXIT_FAILURE);
      }
     if (*temp eq '}')
      found_close = TRUE;
     *temp++ = '\0';

     if (!mystrcmp(str, "native"))
      native = TRUE;
     elif ((!mystrcmp(str, "processor")) || (!mystrcmp(str, "ptype")))
      { if (ptype ne RmT_Default)
         { fprintf(stderr, "domain: invalid template, processor type doubly defined.\n");
           exit(EXIT_FAILURE);
         }
        str = temp;
	while(isspace(*str)) str++;
	if (*str eq '\0')
	 { fprintf(stderr, "domain: invalid template, missing processor type.\n");
	   exit(EXIT_FAILURE);
	 }
        for (temp = str; 
             (!isspace(*temp)) && (*temp ne '\0') && (*temp ne ';') &&
             	(*temp ne '}');
             temp++);
        if (*temp eq '\0')
         { fprintf(stderr, "domain: invalid template, missing ; or } after %s\n",
         		str);
           exit(EXIT_FAILURE);
         }
        if (isspace(*temp))
         { *temp++ = '\0'; 
           while(isspace(*temp)) temp++;
           if ((*temp ne ';') && (*temp ne '}'))
            { fprintf(stderr, "domain: invalid template, expecting ; or } after %s\n",
            		str);
              exit(EXIT_FAILURE);
            }
         }
         if (*temp eq '}')
          { *temp++ = '\0'; found_close = TRUE; }
         elif (*temp eq ';')
          *temp++ = '\0';
        if   (!mystrcmp(str, "T800"))  ptype = RmT_T800;
        elif (!mystrcmp(str, "T414"))  ptype = RmT_T414;
        elif (!mystrcmp(str, "T400"))  ptype = RmT_T400;
        elif (!mystrcmp(str, "T425"))  ptype = RmT_T425;
        elif (!mystrcmp(str, "H1"))    ptype = RmT_T9000;
        elif (!mystrcmp(str, "T9000")) ptype = RmT_T9000;
        elif (!mystrcmp(str, "T212"))  ptype = RmT_T212;
        elif (!mystrcmp(str, "i860"))  ptype = RmT_i860;
        elif (!mystrcmp(str, "Arm"))   ptype = RmT_Arm;
        elif (!mystrcmp(str, "680x0")) ptype = RmT_680x0;
        elif (!mystrcmp(str, "C40"))   ptype = RmT_C40;
        else
         { fprintf(stderr, "domain: invalid template, unknown processor type %s\n",
         		str);
           fprintf(stderr, "      : recognised types are T800, T414, T425, T400, i860 and C40\n");
           exit(EXIT_FAILURE);
         }
      }
     elif(!mystrcmp(str, "memory"))
      { if (memory ne 0)
         { fprintf(stderr, "domain: invalid template, duplicate memory specification\n");
           exit(EXIT_FAILURE);
         }
        str = temp;
        while (isspace(*str)) str++;
        if (*str eq '\0')
         { fprintf(stderr, "domain: invalid template, missing memory size.\n");
           exit(EXIT_FAILURE);
         }
        for (temp = str;
             (!isspace(*temp)) && (*temp ne '\0') && (*temp ne ';') &&
             	(*temp ne '}');
             temp++);
        if (*temp eq '\0')
         { fprintf(stderr, "domain: invalid template, missing ; or } after %s\n",
                   str);
           exit(EXIT_FAILURE);
         }
        if (isspace(*temp))
         { *temp++ = '\0';
           while (isspace(*temp)) temp++;
           if ((*temp ne ';') && (*temp ne '}'))
            { fprintf(stderr, "domain: invalid template, expecting ; or } after %s\n",
            		str);
              exit(EXIT_FAILURE);
            }
         }
        if (*temp eq '}')
         { *temp++ = '\0'; found_close = TRUE; }
        elif (*temp eq ';')
         *temp++ = '\0';

        unless(isdigit(*str))
         { fprintf(stderr, "domain: invalid template, expected digit, not %s\n",
         	str);
           exit(EXIT_FAILURE);
         }
         
        if ((*str eq '0') && ((str[1] eq 'x') || (str[1] eq 'X')))
         { str = &(str[2]);
           while (isxdigit(*str))
            { memory = 16 * memory;
              if (*str <= '9')
               memory += (*str - '0');
              elif (('a' <= *str) && (*str <= 'f'))
               memory += (10 + *str - 'a');
              else
               memory += (10 + *str - 'A');
              str++;
            }
           if (*str ne '\0')
            { fprintf(stderr, "domain: invalid memory size, expecting hex digit, not %s\n",
            	str);
              exit(EXIT_FAILURE);
            }
         }
        else
         { while (isdigit(*str))
            { memory = (10 * memory) + (*str - '0'); str++; }
           if (*str ne '\0')
            { fprintf(stderr, "domain: invalid memory size, expecting digit, not %s\n",
            		str);
              exit(EXIT_FAILURE);
            }
         }
      }
     elif (!mystrcmp(str, "attrib"))
      { if (next_attrib eq 10)
         { fprintf(stderr, "domain: too many attributes specified.\n");
           exit(EXIT_FAILURE);
         }
        str = temp;
        while (isspace(*str)) str++;
        if (*str eq '\0')
         { fprintf(stderr, "domain: invalid template, missing attribute string.\n");
           exit(EXIT_FAILURE);
         }
        for (temp = str;
             (!isspace(*temp)) && (*temp ne '\0') && (*temp ne ';') &&
             	(*temp ne '}');
             temp++);
        if (*temp eq '\0')
         { fprintf(stderr, "domain: invalid template, missing ; or } after %s\n",
                   str);
           exit(EXIT_FAILURE);
         }
        if (isspace(*temp))
         { *temp++ = '\0';
           while (isspace(*temp)) temp++;
           if ((*temp ne ';') && (*temp ne '}'))
            { fprintf(stderr, "domain: invalid template, expecting ; or } after %s\n",
            		str);
              exit(EXIT_FAILURE);
            }
         }  
        if (*temp eq '}')
         { *temp++ = '\0'; found_close = TRUE; }
        elif (*temp eq ';')
         *temp++ = '\0';
	if ((*str eq ';') || (*str eq '}'))
	 { fprintf(stderr, "domain: invalid attribute string %s\n", str);
	   exit(EXIT_FAILURE);
	 }
        attribs[next_attrib++] = str;
      }
     else
      { fprintf(stderr,
         "domain: invalid template, expecting processor, memory, or attrib, not %s\n",
         	str);
        exit(EXIT_FAILURE);
      }
    str = temp;
    while (isspace(*str)) str++;
  }

done:

  if (number eq 0) number = 1;
  for ( ; number > 0; number--)
   { RmProcessor	new = RmNewProcessor();
     int		i;
     
     if (new eq (RmProcessor) NULL)
      { fprintf(stderr, "domain: out of memory building template\n");
        exit(EXIT_FAILURE);
      }
     if (ptype ne RmT_Default)
      RmSetProcessorType(new, ptype);
     if (memory ne 0)
      RmSetProcessorMemory(new, memory);
     if (native)
      RmSetProcessorPurpose(new, RmP_Native);
     for (i = 0; i < next_attrib; i++)
      if (RmAddProcessorAttribute(new, attribs[i]) ne RmE_Success)
       { fprintf(stderr, "domain: out of memory building template\n");
         exit(EXIT_FAILURE);
       }

     if (RmAddObjectAttribute((RmObject) new, "NEW=1", TRUE) ne RmE_Success)
      { fprintf(stderr, "domain: failed to build template\n");
        exit(EXIT_FAILURE);
      }

     if (RmAddtailProcessor(template, new) eq (RmProcessor) NULL)
      { fprintf(stderr, "domain: failed to build template\n");
        exit(EXIT_FAILURE);
      }
   }
}

/**
*** do_free(). The requested processors are obtained and changed from permanent
*** to temporary.
**/
static	int	do_free_aux(RmProcessor, ...);
static	int	do_free_aux2(RmProcessor, ...);

static	void	do_free(int argc, char **argv)
{ RmNetwork	domain;
  RmNetwork	obtained;
  int		rc;
  int		tempargc;
  char		**tempargv;

  if (argc eq 0) usage();
  domain = RmGetDomain();
  if (domain eq (RmNetwork) NULL)
   { fprintf(stderr, "domain: failed to access user's domain, RmLib error %s\n",
   	     RmMapErrorToString(RmErrno));
     exit(EXIT_FAILURE);
   }
  (void) RmApplyProcessors(domain, &init_domain);

  if ((argc eq 1) && (!strcmp(*argv, "all")))
   RmApplyProcessors(domain, &set_domain);
  else
   for ( tempargc = argc, tempargv = argv; tempargc > 0; tempargc--, tempargv++)
    { RmProcessor temp = RmLookupProcessor(domain, *tempargv);
      if (temp eq (RmProcessor) NULL)
       { fprintf(stderr, "domain: processor %s is not in the user's domain\n",
       		*tempargv);
         *tempargv = NULL;
       	 continue;
       }
      RmSetProcessorPrivate(temp, 1);
    }

  (void) RmApplyProcessors(domain, &delete_unwanted);
  obtained = RmObtainNetwork(domain, FALSE, &rc);
  if (obtained eq (RmNetwork) NULL)
   { fprintf(stderr,
   	 "domain: error, failed to get access to the requested processors\n");
     exit(EXIT_FAILURE);
   }
  RmFreeNetwork(domain);
  (void) RmApplyProcessors(obtained, &do_free_aux);
  RmReleaseNetwork(obtained);
  RmFreeNetwork(obtained);

  Delay(OneSec);
  domain = RmGetDomain();
  if (domain eq (RmNetwork) NULL)
   { fprintf(stderr, "domain: failed to re-examine user's domain, RmLib error %s\n",
		RmMapErrorToString(RmErrno));
     exit(EXIT_FAILURE);
   }

  if ((argc eq 1) && (*argv ne NULL) && (!strcmp(*argv, "all")))
   RmApplyProcessors(domain, &do_free_aux2);
  else
   for ( tempargc = argc, tempargv = argv; tempargc > 0; tempargc--, tempargv++)
    { RmProcessor temp;
      if (*tempargv eq NULL) continue;
      temp = RmLookupProcessor(domain, *tempargv);
      if (temp ne (RmProcessor) NULL)
       (void) do_free_aux2(temp);
    }
  exit(EXIT_SUCCESS);
}

static	int	do_free_aux(RmProcessor Processor, ...)
{ int	rc;

  rc = RmSetProcessorTemporary(Processor);
  if (rc ne RmE_Success)
   fprintf(stderr, "domain: warning, failed to release processor %s, RmLib error %s\n",
   	RmGetProcessorId(Processor), RmMapErrorToString(rc));  
  return(0);
}

static int do_free_aux2(RmProcessor processor, ...)
{ fprintf(stderr, "domain: warning, failed to release processor %s\n",
		RmGetProcessorId(processor));
  return(0);
}

/**
*** Book : much the same as free
**/
static	int	do_book_aux(RmProcessor, ...);

static	void	do_book(int argc, char **argv)
{ RmNetwork	domain;
  RmNetwork	obtained;
  int		rc;
  
  if (argc eq 0) usage();
  domain = RmGetDomain();
  if (domain eq (RmNetwork) NULL)
   { fprintf(stderr, "domain: failed to access user's domain, RmLib error %s\n",
   		RmMapErrorToString(RmErrno));
     exit(EXIT_FAILURE);
   }
  (void) RmApplyProcessors(domain, &init_domain);

  if ((argc eq 1) && (!strcmp(*argv, "all")))
   RmApplyProcessors(domain, &set_domain);
  else
   for ( ; argc > 0; argc--, argv++)
    { RmProcessor temp = RmLookupProcessor(domain, *argv);
      if (temp eq (RmProcessor) NULL)
       { fprintf(stderr, "domain: processor %s is not in the user's domain\n",
       		*argv);
       	continue;
       } 
      RmSetProcessorPrivate(temp, 1);
    }
  (void) RmApplyProcessors(domain, &delete_unwanted);

  obtained = RmObtainNetwork(domain, FALSE, &rc);
  if (obtained eq (RmNetwork) NULL)
   { fprintf(stderr,
   	 "domain: error, failed to get access to requested processors\n");
     exit(EXIT_FAILURE);
   }
  if (rc ne RmE_Success)
   RmApplyProcessors(domain, &show_missing, obtained);

  RmFreeNetwork(domain);
  (void) RmApplyProcessors(obtained, &do_book_aux);
  RmReleaseNetwork(obtained);
  RmFreeNetwork(obtained);
  exit(EXIT_SUCCESS);
}

static	int	do_book_aux(RmProcessor Processor, ...)
{ int	rc;

  rc = RmSetProcessorBooked(Processor);
  if (rc ne RmE_Success)
   fprintf(stderr, "domain: warning, failed on processor %s, RmLib error %s\n",
   	RmGetProcessorId(Processor), RmMapErrorToString(rc));  
  return(0);
}

/**
*** Cancel : much the same as free, again
**/
static	int	do_cancel_aux(RmProcessor, ...);

static	void	do_cancel(int argc, char **argv)
{ RmNetwork	domain;
  RmNetwork	obtained;
  int		rc;
  
  if (argc eq 0) usage();
  domain = RmGetDomain();
  if (domain eq (RmNetwork) NULL)
   { fprintf(stderr, "domain: failed to access user's domain, RmLib error %s\n",
   		RmMapErrorToString(RmErrno));
     exit(EXIT_FAILURE);
   }
  (void) RmApplyProcessors(domain, &init_domain);

  if ((argc eq 1) && (!strcmp(*argv, "all")))
   RmApplyProcessors(domain, &set_domain);
  else
   for ( ; argc > 0; argc--, argv++)
    { RmProcessor temp = RmLookupProcessor(domain, *argv);
      if (temp eq (RmProcessor) NULL)
       { fprintf(stderr, "domain: processor %s is not in the user's domain\n",
       		*argv);
         continue;
       }
      RmSetProcessorPrivate(temp, 1);
    }
  (void) RmApplyProcessors(domain, &delete_unwanted);

  obtained = RmObtainNetwork(domain, FALSE, &rc);
  if (obtained eq (RmNetwork) NULL)
   { fprintf(stderr,
   	 "domain: error, failed to get access to requested processors\n");
     exit(EXIT_FAILURE);
   }
  if (rc ne RmE_Success)
   RmApplyProcessors(domain, &show_missing, obtained);

  RmFreeNetwork(domain);
  (void) RmApplyProcessors(obtained, &do_cancel_aux);
  RmReleaseNetwork(obtained);
  RmFreeNetwork(obtained);
  exit(EXIT_SUCCESS);
}

static	int	do_cancel_aux(RmProcessor Processor, ...)
{ int	rc;

  rc = RmSetProcessorCancelled(Processor);
  if (rc ne RmE_Success)
   fprintf(stderr, "domain: warning, failed on processor %s, RmLib error %s\n",
   	RmGetProcessorId(Processor), RmMapErrorToString(rc));  
  return(0);
}

/**
*** list: a less verbose version of show
**/
static int do_list_aux(RmProcessor, ...);

static	void	do_list(int argc, char **argv)
{ RmNetwork	domain;

  if (argc ne 0) usage();
  domain = RmGetDomain();
  if (domain eq (RmNetwork) NULL)
   fprintf(stderr,
    "domain: failed to get information on current domain, RmLib error %s\n",
        	RmMapErrorToString(RmErrno));
  else
   { fputs("Current domain : ", stdout);
     (void) RmApplyProcessors(domain, &do_list_aux);
     putchar('\n');
   }

  argv = argv;
}

static int do_list_aux(RmProcessor Processor, ...)
{ 
  printf("/%s ", RmGetProcessorId(Processor));
  return(0);
}

/**
*** Show : simply print out the current domain, in all its gory detail
**/
static	void	do_show(int argc, char **argv)
{ RmNetwork	domain;

  domain = RmGetDomain();
  if (domain eq (RmNetwork) NULL)
   fprintf(stderr,
    "domain: failed to get information on current domain, RmLib error %s\n",
        	RmMapErrorToString(RmErrno));
  elif (argc eq 0)
   PrintNetwork(domain);
  else
   { init_PrintNetwork(TRUE);
     for ( ; argc > 0; argc--, argv++)
      { RmProcessor	current = RmLookupProcessor(domain, *argv);
        if (current eq (RmProcessor) NULL)
         fprintf(stderr, "domain: processor %s is not in the current domain\n",
		*argv);
        else
         PrintProcessor(current, 0);
      }
     tidy_PrintNetwork();
   }

}

static	void	do_info(int argc, char **argv)
{ RmNetwork	domain;
  
  domain = RmGetDomain();
  if (domain eq (RmNetwork) NULL)
   { fprintf(stderr,
      "domain: failed to get information on current domain, RmLib error %s\n",
    		RmMapErrorToString(RmErrno));
     exit(EXIT_FAILURE);
   }
  if (argc ne 0)
   { (void) RmApplyProcessors(domain, &init_domain);
      for ( ; argc > 0; argc--, argv++)
       { RmProcessor	current = RmLookupProcessor(domain, *argv);
         if (current eq (RmProcessor) NULL)
           fprintf(stderr, "domain: processor %s is not in the domain\n",
       		*argv);
         else
          RmSetProcessorPrivate(current, 1);
       }
      (void) RmApplyProcessors(domain, &delete_unwanted);
   }

  DisplayInfo(domain);

  RmFreeNetwork(domain);
}

static	void	do_monitor(int argc, char **argv)
{ int		delay = 2;
  RmNetwork	network;

  if ((argc ne 0) && (**argv ne '/'))
   { delay = atoi(*argv);
     argc--; argv++;
     if (delay <= 0) delay = 2;
   }

  network = RmGetDomain();
  if (network eq (RmNetwork) NULL)
   { fprintf(stderr,
      "domain: failed to get information on current domain, RmLib error %s\n",
    		RmMapErrorToString(RmErrno));
     exit(EXIT_FAILURE);
   }

  if (argc ne 0)
   { (void) RmApplyProcessors(network, &init_domain);
      for ( ; argc > 0; argc--, argv++)
       { RmProcessor	current = RmLookupProcessor(network, *argv);
         if (current eq (RmProcessor) NULL)
           fprintf(stderr, "network: processor %s is not in the network\n",
       		*argv);
         else
          RmSetProcessorPrivate(current, 1);
       }
      (void) RmApplyProcessors(network, &delete_unwanted);
   }

  MonitorNetwork(network, delay);   
}

/**
*** do_help(). Print out lots and lots and lots.
**/

static char *text1 = "\
Domain is a command to allow users to administer their own domain of\n\
processors, and examine what is happening. There are various options.\n\n\
";
static char *text2 = "\
domain list : this simply lists the processors in the current domain.\n\
Typical output might be something like:\n\
  Current domain : /01 /02 /03 /06\n\n\
";
static char *text3 = "\
domain show : this is used to give more information about the processors,\n\
including processor type, the amount of memory, and details of connections\n\
to other processors in the domain. The \'network\' program may be used to\n\
obtain details of the whole network, including processors outside the\n\
user's domain.\n\n\
";
static char *text4 = "\
domain info : this is used to get information about the current loading\n\
of the processors in the domain. The output produced is something like:\n\
   Processor 02 : load 1045, memory free 42%\n\
By default the command gives details about all the processors in the\n\
user's domain, but it is possible to restrict the processors by listing\n\
the ones desired, for example:\n\
    domain info /02 /03\n\n\
";
static char *text5 = "\
domain monitor : this is like the info option, but runs continuously\n\
until the user aborts it by pressing control-C. By default information\n\
on every processor is displayed every ten seconds. It is possible to\n\
specify a different interval if desired:\n\
  domain monitor 20\n\
Also, the processors affected may be restricted as per the info command.\n\
  domain monitor 20 /02 /03\n\n\
";
static char *text6 = "\
domain get : this is used to add more processors to the user's domain.\n\
The command takes two forms. First it is possible to get one or more\n\
processors by giving their names, for example :\n\
  domain get /02 /03 /04\n\
Alternatively it is possible to describe the processors required by using\n\
a template. A typical template might be:\n\
  domain get \"8{ processor T800 }\" \n\
This means that the user wants to obtain eight T800's, if possible. \n\
Templates allow processor types, memory sizes, and attributes to be\n\
specified. Attributes should match the attributes defined in the network\n\
resource map. Examples are:\n\
  domain get \"{ memory 0x100000}\"\n\
  domain get \"4{ processor T425; attrib 30MHz}\"\n\
Recognised processor types are: T800, T414, T425, T400, i860 and C40.\n\n\
";
static char *text7 = "\
domain free : this is used to release processors back to the system pool.\n\
The processors should be listed:\n\
  domain free /02 /03 /04\n\n\
";
static char *text8 = "\
domain book : by default the Taskforce Manager may use any processors in\n\
the user's domain when attempting to run programs on the user's behalf.\n\
This is not always desirable, and it is possible to deny access to one\n\
or more processors by booking them. Once a processor has been booked\n\
only requests explicitly involving that processor will be accepted,\n\
for example the \'remote\' program may still be used to run a program\n\
specifically on that processor. To use this option, list the processors\n\
affected.\n\
   domain book /02 /03 /04\n\n\
";
static char *text9 = "\
domain cancel : this performs the inverse operation to \'domain book\'.\n\
It allow the Taskforce Manager to map programs onto the specified processors.\n\
   domain cancel /02 /03 /04\n\n\
";
static char *text10 = "\
domain lock : this stops the Taskforce Manager from obtaining more\n\
processors from the system pool. In addition it prevents the\n\
Taskforce Manager from releasing processors back to the system pool.\n\n\
";
static char *text11 = "\
domain unlock : this releases the lock on the Taskforce Manager, permitting\n\
it to obtain processors from the system pool and releasing them.\n\n\
";
static char *text12 = "\
domain preload : this pre-loads all processors in the current domain with\n\
the libraries required for either C or Fortran taskforces.\n\
   domain preload fortran\n\n\
";
static char *text13 = "\
domain help : the help option gives this information.\n\n\
";

static	void	do_help(int argc, char **argv)
{ FILE	*output = popen("/helios/bin/more", "w");

  argc = argc; argv = argv;
  
  if (output eq (FILE *) NULL)
   { fputs("domain: failed to access /helios/bin/more\n", stderr);
     exit(EXIT_FAILURE);
   }
  fputs(text1, output);
  fputs(text2, output);
  fputs(text3, output);
  fputs(text4, output);
  fputs(text5, output);
  fputs(text6, output);
  fputs(text7, output);
  fputs(text8, output);
  fputs(text9, output);
  fputs(text10, output);
  fputs(text11, output);
  fputs(text12, output);
  fputs(text13, output);
  pclose(output);  
}

/**
*** do_lock. This does not go through the resource management library,
*** it is a private operation.
**/
static void do_privateop(int);

static void do_lock(int argc, char **argv)
{ if (argc ne 0) usage();
  argv = argv;
  do_privateop(RmC_Lock);
}

static void do_unlock(int argc, char **argv)
{ if (argc ne 0) usage();
  argv = argv;
  do_privateop(RmC_Unlock);
}

static void do_privateop(int op)
{ int		rc;
  RmRequest	request;
  RmReply	reply;

  Clear(request); Clear(reply);
  request.FnRc	= op;
  rc = RmXch(&RmParent, &request, &reply);

  if (rc ne RmE_Success)
   { fputs("domain: failed to access Taskforce Manager\n", stderr),
     fprintf(stderr, "domain: fault was %s\n", RmMapErrorToString(RmErrno));
     exit(EXIT_FAILURE);
   }
}

/**
*** Preloading of libraries. This is rather complex.
**/

static void	preload_usage(void);
static int	preload(RmProcessor, ...);
static Object	*loadlib(Object *loader, Object **source, char *name, char *tname);

static Object	*FpLib_t8 = Null(Object);
static Object	*FpLib_t4 = Null(Object);
static Object	*FpLib	  = Null(Object);
static Object	*Posix    = Null(Object);
static Object	*Clib     = Null(Object);
static Object	*FpClib   = Null(Object);
static Object	*Flib_t8  = Null(Object);
static Object	*Flib_t4  = Null(Object);
static Object	*Pipe     = Null(Object);

static Semaphore	RmLib_lock;
static Semaphore	Counter_lock;
static Semaphore	Thread_finished;
static int		thread_count = 0;

/**
***
**/
static void do_preload(int argc, char **argv)
{ bool		C	= FALSE;
  bool		Fortran	= FALSE;
  RmNetwork	domain;

  InitSemaphore(&RmLib_lock, 1);
  InitSemaphore(&Counter_lock, 1);
  InitSemaphore(&Thread_finished, 0);
    
  if (argc != 1) preload_usage();
  if (!mystrcmp(*argv, "C"))
   C = TRUE;
  elif (!mystrcmp(*argv, "Fortran"))
   Fortran = TRUE;
  else
   preload_usage();

  domain = RmGetDomain();
  if (domain == (RmNetwork) NULL)
   { fputs("domain: failed to get information about the current domain.\n",
           stderr);
     exit(EXIT_FAILURE);
   }

  (void) RmApplyNetwork(domain, &preload, C, Fortran);
  Wait(&Counter_lock);
  while(thread_count-- > 0)
   Wait(&Thread_finished);
}

/**
*** argument error handling.
**/
static void preload_usage(void)
{ fputs("domain: usage, domain preload [C | Fortran]\n", stderr);
  exit(EXIT_FAILURE);
}

/**
*** The pre-load function. This is called for every processor in the
*** current domain. It should load the appropriate libraries plus the
*** pipe server into that processor, from another processor if possible.
*** The pipe server must be accessed to load it.
**/

static bool first_time = TRUE;
static void preload_aux(RmProcessor processor, bool C, bool Fortran);

static int preload(RmProcessor processor, ...)
{ va_list	args;
  bool		C;
  bool		Fortran;
  
  va_start(args, processor);
  C		= va_arg(args, bool);
  Fortran	= va_arg(args, bool);
  va_end(args);
  
  if (RmIsNetwork(processor))
   return(RmApplyNetwork((RmNetwork) processor, &preload, C, Fortran));

  if (RmGetProcessorPurpose(processor) != RmP_Helios)
   return(0);

  if (first_time)
   { thread_count++;
     preload_aux(processor, C, Fortran);
     first_time = FALSE;
   }
  else
   { (void) Fork(1000, preload_aux, 12, processor, C, Fortran);
     Wait(&Counter_lock);
     thread_count++;
     Signal(&Counter_lock);
   }

  return(0);
}

static void preload_aux(RmProcessor processor, bool C, bool Fortran)
{ Object	*proc		= Null(Object);
  Object	*loader		= Null(Object);
  RmProcessor	obtained;
  int		processor_type;
    
  Wait(&RmLib_lock);
  obtained = RmObtainProcessor(processor);
  Signal(&RmLib_lock);
  
  if (obtained == (RmProcessor) NULL) goto done;

  processor_type = RmGetProcessorType(processor);
  proc = RmMapProcessorToObject(obtained);
  if (proc == Null(Object)) goto done;

  loader = Locate(proc, "loader");
  if (loader == Null(Object)) goto done;
   
  Close(loadlib(loader, &Posix, "Posix", "Posix"));
  if ((processor_type == RmT_C40) || (processor_type == RmT_i860))
   Close(loadlib(loader, &FpLib, "FpLib", "FpLib"));
  elif (processor_type == RmT_T800)
   Close(loadlib(loader, &FpLib_t8, "FpLib", "FpLib.t8"));
  else
   Close(loadlib(loader, &FpLib_t4, "FpLib", "FpLib.t4"));
  if (C)
   { Close(loadlib(loader, &Clib, "Clib", "Clib"));
     Close(loadlib(loader, &FpClib, "fpclib", "fpclib"));
   }
  elif (Fortran && (processor_type != RmT_C40) && (processor_type != RmT_i860))
   { if (processor_type == RmT_T800)
      Close(loadlib(loader, &Flib_t8, "flib", "flib.t8"));
     else
      Close(loadlib(loader, &Flib_t4, "flib", "flib.t4"));
   }

  { Object *pipe_code   = loadlib(loader, &Pipe, "pipe", "pipe");
    Object *pipe_server = Locate(proc, "pipe/..");
    Close(pipe_code);
    Close(pipe_server);
  }

done:
  if (proc != Null(Object)) Close(proc);
  if (loader	!= Null(Object)) Close(loader);
  if (obtained	!= (RmProcessor) NULL)
   { Wait(&RmLib_lock);
     RmReleaseProcessor(obtained);
     Signal(&RmLib_lock);
   }
  Signal(&Thread_finished);
  return; 
}

/**
*** Load a piece of code into a processor, preferably from another
*** processor.
**/
static Object *loadlib(Object *loader, Object **source, char *name, char *hname)
{ Object *helios_lib	= Null(Object);
  Object *disk_copy	= Null(Object);
  Object *result	= Null(Object);

	/* 1) Check whether the object is already in memory	*/
  result = Locate(loader, name);
  if (result != Null(Object))
   { 	/* If so, use it as the source from now on if necessary	*/
     if (*source == Null(Object))
      *source = Locate(result, Null(char));
     return(result);
   }

	/* 2) If the code is already in memory somewhere in the	*/
	/*    network, load it from there.			*/   
  if (*source != Null(Object))
   return(Load(loader, *source));

	/* 3) Locate the binary for this library. This may have	*/
	/*    a .t8 or .t4 extension.				*/
  helios_lib = Locate(Null(Object), "/helios/lib");
  if (helios_lib == Null(Object)) goto done;

  disk_copy = Locate(helios_lib, hname);
  if (disk_copy == Null(Object)) goto done;

	/* 4) Load the code off disk. The /loader entry may	*/
	/*    now have the wrong name.				*/
  result = Load(loader, disk_copy);
  if (result == Null(Object)) goto done;


	/* 5) If the object has been loaded as /loader/FpLib.t8	*/
	/*    it must be renamed to FpLib.			*/
  if (strcmp(hname, name))
   { 	/* Rename /loader/FpLib.t8 to /loader/FpLib */
     Close(result); result = Null(Object);
     if (Rename(loader, hname, name) < Err_Null) goto done;
     result = Locate(loader, name);
   }

	/* 6) Now remember this code for future loading.	*/
  *source = Locate(result, Null(char));

done:
  if (helios_lib != Null(Object)) Close(helios_lib);
  if (disk_copy  != Null(Object)) Close(disk_copy);
  return(result);
}

/**
*** init_domain(), this is used to set a processor's private field to 0,
*** to mark it as unused.
**/
static int init_domain(RmProcessor Processor, ...)
{ 
  RmSetProcessorPrivate(Processor, 0);
  return(0);
}

/**
*** set_domain() sets the private field to 1
**/
static int set_domain(RmProcessor Processor, ...)
{ 
  RmSetProcessorPrivate(Processor, 1);
  return(0);
}

/**
*** complement the above, get rid of any unwanted processors
**/
static int delete_unwanted(RmProcessor Processor, ...)
{ int	rc;

  if (RmGetProcessorPrivate(Processor) ne 0) return(0);
  
  if (RmRemoveProcessor(Processor) eq (RmProcessor) NULL)
   { fputs("domain: internal error manipulating domain (remove)\n", stderr);
     exit(EXIT_FAILURE);
   }
  if ((rc = RmFreeProcessor(Processor)) ne RmE_Success)
   { fprintf(stderr, "domain: internal error manipulating domain (delete %x)\n",
   		rc);
     exit(EXIT_FAILURE);
   }

  return(0);
}

/**
*** This routine is used to warn the user if not all the requested processors
*** could be accessed.
**/
static	int		show_missing(RmProcessor Processor, ...)
{ va_list	args;
  RmNetwork	obtained;
  
  va_start(args, Processor);
  obtained = va_arg(args, RmNetwork);
  va_end(args);
  
  if (RmFindMatchingProcessor(Processor, obtained) eq (RmProcessor) NULL)
   fprintf(stderr, "domain: warning, failed to access processor %s\n",
   		RmGetProcessorId(Processor));
  return(0);
}
