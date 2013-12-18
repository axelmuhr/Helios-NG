/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- id.c									--
--	Posix compliant command for examing user ids etc.		--
--                                                                      --
--	Author:  BLV 2/9/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/id.c,v 1.3 1993/08/11 10:30:12 bart Exp $*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <posix.h>
#include <pwd.h>
#include <helios.h>

#ifndef eq
#define eq ==
#define ne !=
#endif

static void usage(void);

int main(int argc, char **argv)
{ bool	group_option		= FALSE;
  bool	name_option		= FALSE;
  bool	real_option		= FALSE;
  bool	effective_option	= FALSE;
  char	*username		= Null(char);
  int	i;
  
  for (i = 1; i < argc; i++)
   { if (*(argv[i]) eq '-')
      { char	*temp = argv[i];
        for (++temp; *temp ne '\0'; temp++)
         switch(*temp)
          { case	'g' :	group_option	= TRUE;
				break;
            case	'u' :	effective_option= TRUE;
            			break;
            case	'n' :	name_option	= TRUE;
            			break;
            case	'r' :	real_option	= TRUE;
            			break;
            default	    : usage();
          }
      }
     else
      { if (username ne Null(char)) usage();
        username = argv[i];
      }
   }

	/* Check for clashing options */
  if (group_option && effective_option)
   { fputs("id: cannot use both -g and -u option.\n", stderr);
     usage();
   }

  if ((!group_option) && (!effective_option))
   { if (name_option)
      { fputs("id: cannot use -n option without -g or -u option\n", stderr);
        usage();
      }
     if (real_option)
      { fputs("id: cannot use -r option without -g or -u option\n", stderr);
        usage();
      }
   }
   
  if (username eq Null(char))
   {	/* The command refers to the current user */
     int	uid	= getuid();
     int	gid	= getgid();
     int	euid	= geteuid();
     int	egid	= getegid();

     if (uid < 0) uid   = 0;
     if (gid < 0) gid   = 0;
     if (euid < 0) euid = 0;
     if (egid < 0) egid = 0;     
     
     if (real_option)
      { if (uid ne euid) euid = uid;
        if (gid ne egid) egid = gid;
      }
      
     if (group_option)
      { if (name_option)
         fputs("\n", stdout);	/* Empty name, as per spec. */
        else
         printf(" %u\n", egid);
        return(EXIT_SUCCESS);
      } 
     elif (effective_option)
      { if (name_option)
         { struct passwd *result = getpwuid(euid);
           if (result eq Null(struct passwd))
            fputs("\n", stdout);	/* empty string */
           else
            printf("%s\n", result->pw_name);
         }   
        else
         printf(" %u\n", euid);
        return(EXIT_SUCCESS);
      }
     else
      { struct passwd 	*result = getpwuid(uid);
	char		*name;
        if (result ne Null(struct passwd))
         name = result->pw_name;
        else
         name = "";
        printf("uid=%u(%s) gid=%u()", uid, name, gid);
        if (euid ne uid)
         { result = getpwuid(euid);
           if (result ne Null(struct passwd))
            name = result->pw_name;
           else
            name = "";
           printf(" euid=%u(%s)", euid, name);
         }
        if (egid ne gid)
         printf(" egid=%u()", egid);/* BLV, standard does not have the space */
        putchar('\n');		    /* the standard is probably wrong */
        return(EXIT_SUCCESS);
      }
   }
  else
   { struct passwd *passwd = getpwnam(username);
     int	uid;
     int	gid;
     
     if (passwd eq Null(struct passwd))
      { fprintf(stderr, "id: failed to get information about user %s",
       		username);
        return(EXIT_FAILURE);
      }
     uid = passwd->pw_uid;
     gid = passwd->pw_gid;
     
     if (group_option)
      { if (name_option)
         fputs("\n", stdout);	/* empty name, as per spec */
        else
         printf(" %u\n", gid);
      }
     elif (effective_option)
      { if (name_option)
         printf("%s\n", passwd->pw_name);
        else
         printf(" %u\n", uid); 
      }
     else
      printf("uid=%u(%s) gid=%u()\n", uid, passwd->pw_name, gid);
     return(EXIT_SUCCESS);
   }      
}

static void usage(void)
{ fputs("id: usage, id [-g|-u] [-nr] [user]\n", stderr);
  exit(EXIT_FAILURE);
}

