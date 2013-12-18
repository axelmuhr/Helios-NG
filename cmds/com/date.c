/*------------------------------------------------------------------------
--									--
--   date command							--
--									--
--   All-singing, all-dancing date command, author BLV 29-4-88		--
--									--
------------------------------------------------------------------------*/

static char *rcsid = "$Header: /hsrc/cmds/com/RCS/date.c,v 1.3 1993/07/12 10:50:30 nickc Exp $";

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include <syslib.h>
#include <root.h>
                  
void usage(void), read_date(char *);
void handle_string(char *, struct tm *), handle_date(char *, struct tm *),
     handle_time(char *, struct tm *);
time_t get_date(char *, time_t);
int mystrcmp(char *, char *);

int main(int argc, char *argv[])
{ time_t Now;
  char buffer[300];
  char *temp;
  int i;
  DateSet dates;
  RootStruct *Root = GetRoot();
    
  Now = time((time_t *) NULL);    /* get hold of current time */
  	
                    /* first case - no argument so print out the current date */
  if (argc == 1)
    { printf("Date : %s", ctime(&Now));
      return(0);
    }
                 /* second case : DATE -q gives prompt and user types in data */
  if (argc == 2)
    { temp = argv[1];
      if (*temp++ == '-')
       { if (*temp == 'q' || *temp == 'Q')
           { read_date(buffer); goto analyse_buffer; }
         else
           usage();
       }
    } 
             /* default : concatenate all the arguments, */	
  for( argv++, i=1, buffer[0] = 0; i < argc; i++, argv++)
    { strcat(buffer, *argv); strcat(buffer, " "); }

analyse_buffer:
             /* work out what the user meant */
  Now = get_date(buffer, Now);
  
             /* and change the date */
  dates.Creation = 0;
  dates.Access = 0;
  dates.Modified = Now;

  Root->Time = Now;
  (void)SetDate(CurrentDir, "/clock", &dates);
  printf("Date : %s", ctime(&Now));

  return 0;
} 

void usage(void)
{ printf("Usage : date [-q] [date string]\n");
  exit(1);
}

void read_date(char *buffer)
{ printf("Please type new system date and time : DD-MMM-YY HH:MM:SS\n? ");
  fflush(stdout);
  if (gets(buffer) == NULL)
    { printf("Invalid input.\n"); exit(1); }
}

      /* character comparison ignoring case differences... yet again */
int mystrcmp(char *s1, char *s2)
{ for (; *s1 != '\0' && *s2 != '\0'; s1++, s2++)
    if ( (islower(*s1) ? toupper(*s1) : *s1 ) !=
         (islower(*s2) ? toupper(*s2) : *s2))
      break;

   if (*s1 == '\0' && *s2 == '\0') return(0);
   if (*s1 < *s2) return(-1);
   return(1);	
}

time_t get_date(char *buffer, time_t Now)
{ struct tm mytm;
  time_t temp;
  char   *current_pos, *start_of_string;
  current_pos = buffer;
    
  (void) memcpy(&mytm, localtime(&Now), sizeof(struct tm));
  
  forever
   { while (isspace(*current_pos)) current_pos++;
     if (*current_pos == '\0') break;
     start_of_string = current_pos;

            /* check for today, tomorrow, etc. */
     if (isalpha(*current_pos))
       { while (isalpha(*(++current_pos)));
         if (*current_pos == '\0')
           { handle_string(start_of_string, &mytm); break; }
         if (isspace(*current_pos))
           { *current_pos++ = '\0';
             handle_string(start_of_string, &mytm); continue;
           }
         printf("Invalid argument near %s\n", start_of_string);
         exit(1);
       }      
     
          /* must be either date stamp, i.e. DD-MMM-YY or time HH:MM:SS */
     if (!isdigit(*current_pos))
       { printf("Invalid argument near %s\n", current_pos); exit(1); }
     while (isdigit(*(++current_pos)));
                               /* Is it a time stamp ? */
     if (*current_pos == ':')
       { while (isdigit(*(++current_pos)));
         if (*current_pos == ':') while (isdigit(*(++current_pos)));
         if (*current_pos == '\0')
           { handle_time(start_of_string, &mytm); break; }
         if (isspace(*current_pos))
           { *current_pos++ = '\0';
             handle_time(start_of_string, &mytm);
             continue;
           }
         printf("Invalid argument near %s\n", current_pos); exit(1);
       }
                           /* is it a date stamp */
     if (*current_pos == '-')
       { while (isalnum(*(++current_pos)));
         if (*current_pos == '-')
           while (isdigit(*(++current_pos)));
         if (*current_pos == '\0')
           { handle_date(start_of_string, &mytm); break; }
         if (isspace(*current_pos))
           { *current_pos++ = '\0';
             handle_date(start_of_string, &mytm);
             continue;
           }
         printf("Invalid argument near %s\n", current_pos); exit(1);
       }
   }   

  temp = mktime(&mytm);
  if (temp != -1)
    return(temp);
  printf("Invalid date.\n");
  exit(1);
  return temp;	/* to pacify C compiler */
}

        /* for the following code, mktime will handle carries/borrows for */
        /* field mday but not for wday.                                   */
void handle_string(char *str, struct tm *mytm)
{ static char *days[7] = { "SUNDAY", "MONDAY", "TUESDAY", "WEDNESDAY",
        "THURSDAY", "FRIDAY", "SATURDAY" };
  int i;
        
  if (!mystrcmp(str, "TODAY")) return;
  if (!mystrcmp(str, "TOMORROW"))  { mytm->tm_mday += 1; return; }
  if (!mystrcmp(str, "YESTERDAY")) { mytm->tm_mday -=1; return;  }

  for (i = 0; i < 7; i++)
   if (!mystrcmp(str, days[i]))
     { mytm->tm_mday += ((7 + i - mytm->tm_wday) % 7); return; }

  printf("Unknown string %s.\n", str);
}

     /* the time is of the form HH:MM and optionally :SS at the end */
void handle_time(char *str, struct tm *mytm)
{ int hours = 0, minutes = 0, seconds = 0;
  while (isdigit(*str)) hours = (10 * hours) + *str++ - '0';
  str++;   /* skip past first colon */
  while (isdigit(*str)) minutes = (10 * minutes) + *str++ - '0';
  if (*str == ':')
    { str++;
      while (isdigit(*str)) seconds = (10 * seconds) + *str++ - '0';
    }
  mytm->tm_hour = hours;
  mytm->tm_min  = minutes;
  mytm->tm_sec  = seconds;
}

         /* the date is of the form DD-MMM-YY */
void handle_date(char *str, struct tm *mytm)
{ static char *months_names[12] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
                     "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" } ;
  int days =0, month, year = mytm->tm_year, i;
  char buff[4];
  
  while (isdigit(*str)) days = (10 * days) + *str++ - '0';
  if (days == 0) days = 1;

  if (*str != '-') { printf("Invalid date near %s\n", str); return; }
  str++;
  for (i=0; i < 3; i++) buff[i] = *str++;
  buff[3] = '\0';
  for (month = 0; month < 12; month++)
   if (!mystrcmp(buff, months_names[month])) goto success;
  printf("Error - unknown month near %s.\n", buff); return;

success:
  while (*str != '-' && *str != '\0') str++;
  if (*str == '-')
    { str++; year = 0; while(isdigit(*str)) year = (10 * year) + *str++ - '0'; }

  if (year > 1900) year -= 1900;

  mytm->tm_mday = days; mytm->tm_mon = month; mytm->tm_year = year;

}
