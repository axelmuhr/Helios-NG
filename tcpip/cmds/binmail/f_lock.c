#ifdef __TRAN
static char *rcsid = "$Header: /users/nickc/RTNucleus/tcpip/cmds/binmail/RCS/f_lock.c,v 1.3 1994/03/17 16:40:02 nickc Exp $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <syslib.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include "f_lock.h"

#define MAX_LOCKS    20
#define MAX_FAILS    20
#define EMPTY        -1
#define FILE_EXT     ".__@"

static bool initialized = FALSE ;
static Stream *lock_file [MAX_LOCKS] ;
static int lock_table [MAX_LOCKS] ;

#define F_LOCK_ERR(lock_index) \
  if ((lock_index >= 0) && (lock_index < MAX_LOCKS)) \
    lock_table [lock_index] = EMPTY ;\
  return -1

extern void debugf (char *, ...) ;

int f_lock (int fd, int mode)
{
  void initialize (void) ;
  void f_lock_exit (void) ;
  int f_unlock (int) ;
  int set_index (int) ;
  int f_lock_drastic (char *) ;
  int lock_index ;
  char path_name [512] ;
  Stream *fd_str ;
  char *p, *file_name ;
  Object *cur_dir = CurrentDir ;
  int attempts = 0 ;

  if (mode == LOCK_UN)
    return (f_unlock (fd)) ;
  else
  {              
    if ((fd_str = fdstream(fd)) == NULL)
    {
      debugf ("f_lock: invalid fd") ;
      return -1 ;
    }

    if (!initialized)
    {
      atexit (f_lock_exit) ;
      initialize () ;
      initialized = TRUE ;
    }
  
    strcpy (path_name, fd_str->Name) ;

    if ((lock_index = set_index (fd)) < 0)
    {
      debugf ("Lock limit exceeded - cannot lock file: %s", path_name) ;
      return -1 ;
    } ; 
/*
-- crf: to make life easy, munging the file name into the lock name is 
-- highly simplified ... 
-- "filename" --> "filename.<FILE_EXT>" (this also applies to "filename.xyz")
-- Have to be a little bit careful about what part of the name is going
-- to get changed - it is not sufficient to simply do a reverse search for the
-- first "." in the name.
*/
    if ((file_name = strrchr (path_name, '/')) == NULL)
    {
      debugf ("flock: error in path_name") ;
      return -1 ;
    }
    if ((p = strchr (file_name, '.')) != NULL)
      strcpy (p, FILE_EXT) ;
    else    
      strcat (path_name, FILE_EXT) ;
    debugf ("f_lock: path_name = %s", path_name) ;
    lock_file [lock_index] = NULL ;
    while (lock_file [lock_index] == NULL)
    {
      lock_file [lock_index] = Open (cur_dir, path_name, 
                                    O_Create + O_Exclusive + O_ReadWrite) ;
      if (lock_file [lock_index] != NULL)
      {
        debugf ("f_lock: OK") ;
        return 0 ;
      }
      else
      {
        debugf ("f_lock: Open fail - Result2 = %x", cur_dir->Result2) ;

        debugf ("f_lock: Delay()") ;
        Delay (OneSec) ;
        attempts ++ ;
        if (attempts > MAX_FAILS)
        {
          /* 
          -- crf: something's gone wrong ... an unclaimed lock file is
          -- floating around. Solution: delete it !!! (probably a bit 
          -- drastic, but in these circumstances, I'm **fairly** certain 
          -- that no-one owns it ...)
          */
          if (f_lock_drastic(path_name) != 0)
          {
            debugf ("f_lock: fail") ;
            F_LOCK_ERR (lock_index) ;
          } 
        }
      }
    }
  }
}

int f_unlock (int fd) 
{
  void delete_index (int) ;
  int get_index (int) ;
  int lock_index = get_index (fd) ;
  Object *lf_obj ;
  debugf ("f_unlock: lock_index = %d", lock_index) ;
  if (lock_index < 0)
  {
    debugf ("invalid lock_table entry - can't unlock") ;
    return -1 ;
  }
  else
  {
    debugf ("f_unlock: Name = %s", lock_file [lock_index]->Name) ;
    lf_obj = Locate (NULL, lock_file [lock_index]->Name) ;
    lf_obj->Access = lock_file [lock_index]->Access ;
    if (Close (lock_file [lock_index]) < 0)
    {
      debugf ("can't Close lock_file: %s", lock_file [lock_index]->Name) ;
      return -1 ;
    };
    if (Delete (lf_obj, lock_file [lock_index]->Name) < 0)
    {
      debugf ("can't Delete lock_file: %s", lock_file [lock_index]->Name) ;
      return -1 ;
    };
    delete_index (lock_index) ;
  }
  debugf ("f_unlock: exiting ...") ;
  return 0 ;
}

void f_lock_exit ()
{
  int f_unlock (int) ; 
  int i ;
  debugf ("f_lock_exit()") ;
  for (i = 0 ; i < MAX_LOCKS ; i++)
    if (lock_table [i] != EMPTY)
      if (f_unlock (lock_table [i]) < 0)
        debugf ("f_lock_exit: can't unlock") ;
}

void initialize ()
{
  int i ;
  for (i = 0 ; i < MAX_LOCKS ; i ++)
  {
    lock_file [i] = NULL ;
    lock_table [i] = EMPTY ;
  }
}

int set_index (int fd)
{
  int i = 0 ;
  while ((lock_table [i] != EMPTY) && (i < MAX_LOCKS))
    i ++ ;
  if (i >= MAX_LOCKS)
  {
      debugf ("Too many locks") ;
      return -1 ;
  }
  lock_table [i] = fd ;
  return i ;
}

int get_index (int fd)
{
  int i = 0 ;
  while ((lock_table [i] != fd) && (i < MAX_LOCKS))
    i ++ ;
  if (i >= MAX_LOCKS)
  {
      debugf ("fd not in lock_table") ;
      return -1 ;
  }
  return i ;
}

void delete_index (int lock_index)
{
  lock_file [lock_index] = NULL ;
  lock_table [lock_index] = EMPTY ;
}

int f_lock_drastic(char *path_name)
{
  extern int syslog (int, char *, ... ) ;

  Object *lf_obj ;
  syslog (LOG_WARNING, ": Timeout on lock operation") ;
  syslog (LOG_WARNING, ": Deleting lock file: %s", path_name) ;
  lf_obj = Locate (NULL, path_name) ;
  return ((int) Delete (lf_obj, path_name)) ;
}
