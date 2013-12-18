
static char *rcsid = "$Header: /hsrc/cmds/com/RCS/ps.c,v 1.2 1990/08/23 10:28:02 james Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <syslib.h>
#include <string.h>
#include <gsp.h>
#include <codes.h>

#define eq ==
#define ne !=
#define name_len 256

void display_subnet(char *name);
void display_processor(char *name);
int  is_subnet(char *name);
int  is_processor(char *name);
WORD WalkDir(char *name, WordFnPtr fn, WORD a);
char *lastbit(char *text);
void pathcat(char *dest, char *new);

int main(int argc, char **argv)
{ 

  if (argc eq 1)   /* ps by itself */
   { char name[name_len], *temp;
     MachineName(name);
     
     temp = lastbit(name) - 1;
     if (temp eq name)  /* processor that is not part of a network */
      { display_processor(name); return(0); }

                    /* part of a subnet, display that current subnet */      
      *temp = '\0';
      display_subnet(name);
      return(0);
   }
   
  for (argv++, argc--; argc > 0; argc--, argv++)
   { if (!strcmp(*argv, "/"))
      { if (argc eq 1)
         { printf("Usage : ps <subnet>\n"); return(0); }
        else
         continue;
      }
     elif (is_subnet(*argv))
      display_subnet(*argv);
     elif (is_processor(*argv))
      display_processor(*argv);
     else
      printf("Unknown object %s.\n", *argv);
   }
   
  return(0);
}

         /* a subnet contains a network server, /subnet/ns */
int is_subnet(char *name)
{ char   fullname[name_len];
  Object *o;
  
  strcpy(fullname, name);
  strcat(fullname, "/ns");
  if ((o = Locate(Null(Object), fullname)) ne Null(Object))
   { Close(o); return(1); }
   
  return(0);
}

         /* a processor contains a processor manager, /tasks */
int is_processor(char *name)
{ char   fullname[name_len];
  Object *o;
  
  strcpy(fullname, name);
  strcat(fullname, "/tasks");
  if ((o = Locate(Null(Object), fullname)) ne Null(Object))
   { Close(o); return(1); }
   
  return(0);
}

/**
*** This code deals with displaying the tasks within a processor.
*** It involves reading the /tasks directory of that processor,
*** i.e. a simple WalkDir with a display function. The only problem
*** is keeping the display tidy, which can be done using a static.
**/
int task_count;

WORD display_task(char *parent, DirEntry *entry, WORD a)
{ task_count++;
  if (task_count eq 4)
   { printf("\n           "); task_count = 1; }
   
  printf("%-20s", entry->Name);
  parent = parent; a = a; return(0);
}

void display_processor(char *name)
{ char fullname[name_len];

  strcpy(fullname, name);
  strcat(fullname, "/tasks");
  printf("%-8s : ", lastbit(name));
  task_count = 0;
  (void) WalkDir(fullname, &display_task, 0);
  putchar('\n');
}

/**
*** This code deals with displaying all the tasks within a subnet.
*** The subnet contains a /ns directory which must be read to
*** determine its subnets or processors, and whether those processors
*** are Helios/System, IO, or native ones.
**/
WORD display_subnet_aux(char *nsname, DirEntry *entry, char *subnetname);

void display_subnet(char *name)
{ char fullname[name_len];

  printf("Subnet %s :\n", name);
  strcpy(fullname, name);
  strcat(fullname, "/ns");
  (void) WalkDir(fullname, &display_subnet_aux, (WORD) name);
}

/**
*** This code is not very efficient...
***
*** When walking through the /ns directory, the first step is to check
*** whether the entry is another subnet, a Helios/system processor, a
*** native processor, or an IO processors. Subnets involve recursive
*** calls, Helios and system processors require calls to display_processor,
*** and native and I/O processors must be ignored. To determine the type
*** it is necessary to do an ObjectInfo and check the size.
**/
#define Native 1
#define Helios 2
#define IO     4
#define System 8

WORD display_subnet_aux(char *nsname, DirEntry *entry, char *subnetname)
{ char    fullname[name_len];
  ObjInfo info;

  strcpy(fullname, nsname);
  pathcat(fullname, entry->Name);
  
  if (ObjectInfo(CurrentDir, fullname, (BYTE *) &info) < Err_Null)
{ IOdebug("ObjectInfo failed");
   return(0);
}
  
  switch(info.Size)
   { case Native :
     case IO     : return(0);
     case Helios :
     case System : strcpy(fullname, subnetname);
                   pathcat(fullname, entry->Name);
                   if (is_processor(fullname))
                    display_processor(fullname);
                   return(0);
                   
   }
   
  if ((info.Size % sizeof(DirEntry)) ne 0)
   return(0);
   
  strcpy(fullname, subnetname);
  pathcat(fullname, entry->Name);
  if (is_subnet(fullname))
   display_subnet(fullname);
  return(0);
}


char *lastbit(char *name)
{ while (*name ne '\0') name++;
  while (*name ne '/') name--;
  return(++name);
}

void pathcat(char *dest, char *source)
{ if (*dest eq '\0') { *dest = '/'; dest[1] = '\0'; }

  dest = dest + strlen(dest) - 1;
  if (*dest ne '/')
   *(++dest) = '/'; 

  if (*source eq '/') source++;
  strcpy(++dest, source);
}
  
WORD WalkDir(char *name, WordFnPtr fn, WORD a)
{ Object *o = Locate(CurrentDir, name);
  WORD   sum = 0;
  
  if (o == Null(Object))
   { fprintf(stderr, "Error : unknown directory %s\n", name);
     return(0);
   }
   
  if ((o->Type & Type_Flags) eq Type_Stream)
   return(0);
   
  { Stream *s = Open(o, Null(char), O_ReadOnly);
    WORD size, i;
    DirEntry *entry, *cur;
        
    if (s eq Null(Stream))
     { fprintf(stderr, "Error : unable to open directory %s\n", name);
       Close(o);
       return(0);
     }

    size = GetFileSize(s);
    if (size eq 0) return(0);
    entry = (DirEntry *) Malloc(size);
    if (entry == Null(DirEntry))
     { Close(s); Close(o); return(0); }
     
    if (Read(s, (BYTE *) entry, size, -1) ne size)
     { fprintf(stderr, "Error reading directory %s\n", name);
       Close(s); Close(o); Free(entry);
       return(0);
     }
    
    cur = entry;
    for (i = 0; i < size; cur++, i += sizeof(DirEntry) )
     { if ( (!strcmp(cur->Name, ".")) || (!strcmp(cur->Name, "..")) )
        continue;
       sum += (*fn)(s->Name, cur, a);
     }

    Free(entry);
    Close(s);
  }  
    
  Close(o);
  return(sum);
}


