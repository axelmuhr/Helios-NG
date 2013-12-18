#ifdef __HELIOS
#ifdef __TRAN
static char *rcsid = "$Header: /users/nickc/RTNucleus/tcpip/cmds/lpr/RCS/utils.c,v 1.3 1994/03/17 16:44:42 nickc Exp $";
#endif

/* debugf */
#include <stdio.h>
#include <stdarg.h>
#include <nonansi.h> 

void debugf (char *format, ...)
{
#ifdef DEBUG
  extern char* name ;
  char buf [256] ;
  va_list args;
  va_start (args, format);
  vsprintf (buf, format, args);
  IOdebug ("%s: %s", name, buf) ;
  va_end (args);
#endif  
}

/* scandir */
#include <sys/dir.h>
#include <dirent.h>
#include <string.h>
#include <syslib.h>
#include <stdlib.h>

int scan_dir (char *dir_name, struct direct *(*addr[]), 
#ifdef OLDCODE
              WordFnPtr select_fn, WordFnPtr compar_fn)
#else
              int (*select_fn)(struct direct *),
              int (*compar_fn)(struct direct **, struct direct **))
#endif              
{
  DIR *dir_ptr = opendir (dir_name) ;
  struct dirent *dir_entry ;
  struct direct **files ;
  int num_files = 0 ;
  int file_cnt = 0 ;
  
  if (dir_ptr == NULL)
    return -1 ;

/*
-- crf : 1st pass - get no. of  files
*/
  dir_entry = readdir (dir_ptr) ;
  while (dir_entry)
  {
    if ((*select_fn)(dir_entry))
      num_files ++ ;
    dir_entry = readdir (dir_ptr) ;
  }
  debugf ("scan_dir: found %d  files", num_files) ;
/*
-- crf : 2nd pass - get  file names
*/
  if (num_files > 0)
  {
    files = (struct dirent **) 
            Malloc (((long)num_files + 1) * sizeof (struct dirent *)) ;
/*
-- crf : extra one for last (null) entry
*/            
    if (!files)
      return -1 ; /* out of memory */

    rewinddir (dir_ptr) ;

    dir_entry = readdir (dir_ptr) ;
    while (dir_entry)
    {
      if ((*select_fn)(dir_entry)) /* double check */
      {
        files [file_cnt] = (struct dirent *) Malloc (sizeof (struct dirent)) ;
        if (!files [file_cnt])
          return -1 ; /* out of memory */
/*
-- crf : I'm only interested in the file names ...
*/          
        strcpy (files [file_cnt]->d_name, dir_entry->d_name) ;
        debugf ("scandir: file : %s", files [file_cnt]->d_name) ;
        file_cnt ++ ;
      }
      dir_entry = readdir (dir_ptr) ;
    }
    
    files [file_cnt] = (struct dirent *) NULL ;
      
    if (file_cnt != num_files)
      return -1 ; /* inconsistent  directory */

    if (compar_fn != NULL)
      qsort(files, file_cnt, sizeof(struct files *), (int (*)())compar_fn);

    *addr = files ;    
  } /* if num_files > 0 */    
  if (closedir (dir_ptr) != 0)
    return -1 ; /* cannot close directory */

  return (file_cnt) ;
}

int scan_free (struct direct **files)
{
  int i = 0 ;
  while (files [i] != (struct dirent *) NULL)
  {
    debugf ("scan_free: freeing file : %s", files[i]->d_name) ;
    if (Free (files[i]) != 0)
      return -1 ;      
    i ++ ;
  }

  debugf ("scan_free: freeing file-ptr") ;
  if (Free (files) != 0)
    return -1 ;

  return 1 ;  
}

/* lockmode */
/*
-- crf: need to be carefull with f_lock() - make sure that any programs
-- that call get_mode() and set_mode() are set up to handle signals
-- appropriately (i.e. must call f_lock_exit()).
*/
#include <stdlib.h>
#include <stdio.h>
#include "f_lock.h"

int get_mode (char *file_name)
{
  FILE *stream ;
  int result = -1 ;
  int fd = -1 ;
  int f_lock (int, int) ;
          
  if ((stream = fopen (file_name, "r")) != NULL)
  {
    fd = fileno(stream) ;
    if ((f_lock(fd, LOCK_EX) < 0) || (fscanf (stream, "%d", &result) == EOF))
      debugf ("get_mode: failed") ;
    fclose (stream) ;
  }
  else
  {
    debugf ("get_mode: failed to fopen") ;
  }
  if (fd >= 0)
    (void) f_lock(fd, LOCK_UN) ;
  return result ;
}

int set_mode (char *file_name, int mode)
{
  FILE *stream ;
  int result = -1 ;
  int fd = -1 ;
  int f_lock (int, int) ;
          
  if ((stream = fopen (file_name, "w")) != NULL)
  {
    fd = fileno(stream) ;
    if ((f_lock(fd, LOCK_EX) < 0) || 
       ((result = fprintf (stream, "%d\n", mode)) <= 0))
    {
      result = -1 ;
      debugf ("set_mode: failed") ;
    }
    fclose (stream) ;
  }
  else
  {
    debugf ("set_mode: failed to fopen") ;
  }
  if (fd >= 0)
    (void) f_lock(fd, LOCK_UN) ;
  return result ;
}

/* killtask */
#include <syslib.h>
#ifdef OLDCODE
#include <signal.h>
#endif

#ifdef OLDCODE
int kill_task (char *task_name, int signal)
#else
int kill_task (char *task_name)
#endif
{
#ifdef OLDCODE
  Stream *strm ;
#endif  
  Object *obj ;

  if ((obj = Locate (NULL, task_name)) == NULL)
  {
    debugf ("kill_task : Locate failed") ;
    return -1 ;
  }

#ifdef OLDCODE
  if ((strm = Open (obj, task_name, O_ReadWrite)) == NULL)
    debugf ("kill_task : Open failed") ;

  if (SendSignal (strm, signal) >= 0)
#else
  if (Delete (obj, task_name) >= 0)
#endif  
    return 1 ;
  else
    return -1 ;
}

#endif
