#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>

#include "bash.h"


/*----------------------------------------------------------------------*/
/*                                                   Forward references */
/*----------------------------------------------------------------------*/

void CheckEntries(Directory *d,
		  int *numValidFiles, int *numValidDirs,
		  int *numInvalidFiles, int *numInvalidDirs);


/*----------------------------------------------------------------------*/
/*                                                     Global variables */
/*----------------------------------------------------------------------*/

void *safetyBlock;      /* 'Spare' memory pointer: we free the block
			   it points to if we run out of memory, delete
			   some stuff, then re-allocate it & carry on */
int  releasingMem;      /* Normally zero, this flag is set during the
			   process of deleting files when we've run out of
			   memory, in order to release some. */
int cautious, verbose;  /* Command line flags */


/*----------------------------------------------------------------------*/
/*                                                             VerifyFS */
/*----------------------------------------------------------------------*/

/* Check the RRD for consistency, complaining about faulty 
   files / directories, and keeping a running total of the numbers
   of OK and faulty files / directories found.

   Returns 1 if the FS is OK,
           0 if some part of it is corrupt
*/

int VerifyFS(void) 
{ 
  DIR *d; 
  Directory dir;

  int numValidFiles, numValidDirs; 
  int numInvalidFiles,  numInvalidDirs;

  printf("\n**************\nVerifying filesystem ...\n");
  fflush(stdout);

  if ( (d=opendir(ROOTDIR)) == NULL) 
    { 
      fprintf(stderr, 
	      "Can't open root directory - this shouldn't happen!\n"
	      );
      return 0; 
    }

  /* Fill in the 'Directory' structure corresponding to the root directory */ 
  dir.DirLevel=0;
  strcpy(dir.DirPathName, ROOTDIR);

  /* Now validate the filesystem, print out the results if necessary */ 
  CheckEntries(&dir, 	 
	       &numValidFiles, &numValidDirs,
	       &numInvalidFiles, &numInvalidDirs); 

  printf("done\n");
  fflush(stdout);

  printf("Total number of valid files=%d\n",
	 numValidFiles); 
  printf("Total number of valid directories=%d\n",
	 numValidDirs); 
  printf("Total number of invalid files=%d\n",
	 numInvalidFiles); 
  printf("Total number of invalid directories=%d\n",
	 numInvalidDirs); 

  if ( closedir(d)!=0 )
  { printf("Can't close root directory %s - this shouldn't happen!",
	   ROOTDIR);
    exit(2);
  }

  if ( (numInvalidFiles==0) && (numInvalidDirs==0) ) 
    {
      printf("Filesystem OK\n***************\n\n");
      return 1; 
    }
  else
    {
      printf("Filesystem corrupt!\n**************\n\n");
      return 0; 
    }
}


/*----------------------------------------------------------------------*/
/*                                                           TreeWander */
/*----------------------------------------------------------------------*/

/* Travel a random depth down into the FS, starting at directory d,     */
/* returning a pointer to a subdirectory structure. Note that if the    */
/* directory structure pointer returned is different from that passed   */
/* in, then the space occupied by the directory structure passed in     */
/* will have been freed.                                                */

/* Returns NULL in the case of an error.                                */

Directory *TreeWander(Directory *d)
{
  DIR           *dir;
  struct dirent *entry;
  char   dirNames[MAXNUMDIRS][FILENAMESIZE];
  int    numFiles, numDirs, descendValue;

  if (verbose)
    printf("TreeWander called on directory '%s'\n", d->DirPathName);

  /* Examine the current directory, skipping over the special entries */
  /* '.' and '..' and all those that are files, and complaining       */
  /* if other entries are found whose type we don't understand        */

  if ( ((dir=opendir(d->DirPathName)) == NULL) )
    { printf("TreeWander: can't open directory %s\n", d->DirPathName);

      /* Assume that the open failed because we ran out of memory: */
      /* try to free some & carry on */
      free(d);
      free(safetyBlock);
      ReleaseMem();

      return NULL;
    }

  numFiles=0; numDirs=0;
  while (1)
  { char subPathName[PATHNAMESIZE];

    if ( (entry = readdir(dir)) == NULL )
      break;

    strcpy(subPathName, d->DirPathName);
    strcat(subPathName, "/");
    strcat(subPathName, entry->d_name);
      
    if ( entry->d_type == Type_File )
      numFiles++;

    /* We record the names of all the subdirectories */
    else if ( entry->d_type == Type_Directory )
    { if ( strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".." ) )
	strcpy(dirNames[numDirs++], entry->d_name);
      continue;
    }

    else
    { printf("TreeWander: found an object of unknown type at pathname '%s'\n",
	     subPathName);

      free(d);
      free(safetyBlock);
      ReleaseMem();
   
      return NULL;
    }
  }
  
  if ( closedir(dir) != 0 )
  { printf("TreeWander: failed to close directory '%s'\n", d->DirPathName);

    free(d);
    free(safetyBlock);
    ReleaseMem();

    return NULL;
  }

  /* At this point, we choose whether to descend deeper into the   */
  /* filesystem (provided that this directory has subdirectories), */
  /* or to return a pointer to the directory we are in.            */
  /* This choice is made randomly.                                 */

  descendValue=rand()*(DESCEND_RANGE+1)/RAND_MAX;
  if ( descendValue >= DESCEND_THRESH && (numDirs != 0) )
    /* Call TreeWander recursively to get a subdirectory */
  { DIR             *sub;
    Directory       *subDir;
    int             whichDir;
    char            subPathName[PATHNAMESIZE];
#ifdef OPENDIR
    DIR             *sub;
    struct dirent   *entry;
#endif
    
    if (verbose)
      printf("TreeWander: recursing\n");

    /* Must deallocate the parent directory's 'Directory' structure */
    /* before recursing, since no handle on it is then kept.        */
    free(d);

    whichDir=rand()*numDirs/RAND_MAX;
    strcpy(subPathName, d->DirPathName);
    strcat(subPathName, "/");
    strcat(subPathName, dirNames[whichDir]);
     
#ifdef OPENTEST
    /* Check that we can open, read from and close the directory selected */

    if ( (sub=opendir(subPathName)) == NULL) 
    { printf("TreeWander: can't open directory %s\n", subPathName);

      free(safetyBlock);
      ReleaseMem();

      return NULL;
    }
    
    if ( (entry = readdir(sub)) == NULL )
    { if (errno !=0)
      { printf("TreeWander: directory '%s' unreadable\n", subPathName);

	free(safetyBlock);
	ReleaseMem();

	return NULL;
      }
      else
      { printf(
    "TreeWander: directory '%s' completely empty, ie. no '.' or '..'\n",
	       subPathName);
	free(d);
	return NULL;
      }
    }

    if ( closedir(sub) != 0 )
    { printf("TreeWander: failed to close directory '%s'\n",
	     subPathName);
      
      free(safetyBlock);
      ReleaseMem();

      return NULL;
    }

    if (verbose)
      printf("TreeWander: successfully opened, read from & closed dir '%s'\n",
	     subPathName);

#endif /* OPENTEST */

    if ( ( subDir = malloc(sizeof(Directory)) ) == NULL )
    { printf("TreeWander: out of memory!\n");

      free(safetyBlock);
      ReleaseMem();
      
      return NULL;
    }

    subDir->DirLevel    = (d->DirLevel) + 1;
    strcpy(subDir->DirPathName, subPathName);
    subDir->DirNumFiles = -1;
    subDir->DirNumSub   = -1;

    return TreeWander(subDir);
  }

  else
  { /* Return the directory structure passed in, having first */
    /* filled in the DirNumFiles and DirNumSub fields - which */
    /* we didn't know before.                                 */
    d->DirNumFiles = numFiles;
    d->DirNumSub   = numDirs;

    if (verbose)
      printf("TreeWander: returning directory '%s'\n", d->DirPathName);

    return d;
  }
}


/*----------------------------------------------------------------------*/
/*                                                 FindExistingFileName */
/*----------------------------------------------------------------------*/

/* Find the full pathname of a file in the supplied directory if one    */
/* exists, the choice of which file being made randomly.                */
/*                                                                      */
/* Inputs: 'd'         the directory in question                        */
/*         'pathName'  pointer to the space into which the full         */
/*                     pathname of the file is to be copied, if we find */
/*                     it.                                              */
/*                                                                      */
/* Outputs: return value = x    if we're returning pathName of file fx  */
/*                         -1   if not returning a pathname but believe */
/*                              that filesystem is ok.                  */
/*                         -2   if we've spotted an inconsistency &     */
/*                              want a filesystem check.                */

int FindExistingFileName(Directory *d, char *pathName)
{
  char           fileNames[MAXNUMFILES][FILENAMESIZE];
  struct dirent  *entry;
  DIR            *dir;
  int            whichFile, fileNum;
  int            numFiles=0;
  
  if ( ((dir=opendir(d->DirPathName)) == NULL) )
    { printf("FindExistingFileName: can't open directory %s\n", 
	     d->DirPathName);

      free(safetyBlock);
      ReleaseMem();

      return -2;
    }

  /* Find all the files in this directory */
  while (1)
  { if ( (entry = readdir(dir)) == NULL )
    { if (errno !=0)
      { printf(
	    "FindExistingFileName: error whilst reading from directory '%s'\n",
	       d->DirPathName);

	free(safetyBlock);
	ReleaseMem();

	return -2;
      }
      else 
	break;
    }

    strcpy(pathName, d->DirPathName);
    strcat(pathName, "/");
    strcat(pathName, entry->d_name);

    /* If it was a file that was just found, record its name, but       */
    /* don't validate it other than checking that its name is of the    */
    /* correct form - we could do so, but the program would run too     */
    /* slowly.                                                          */

    if ( (entry->d_type == Type_File) )
    { int index;

      if ( sscanf(entry->d_name, "f%d", &index) != 1)
      { printf("FindExistingFileName: file '%s' has an invalid name\n", 
	       entry->d_name);
	return -2;
      }
      else
      {
	strcpy(fileNames[numFiles++], entry->d_name);
      }
    }

    else if ( entry->d_type == Type_Directory )
    { int index;

      /* Ignore . & .. entries */
      if ( strcmp(entry->d_name, ".")  && 
	   strcmp(entry->d_name, "..") &&
	   (sscanf(entry->d_name, "d%d", &index) != 1) )
      { printf("FindExistingFileName: dir '%s' has an invalid name\n", 
	       pathName);
	return -2;
      }
    }

    else
    { printf("FindExistingFileName: found object '%s' of an unknown type\n",
	     pathName);
      return -2;
    }
  }

  if ( closedir(dir) != 0 )
  { printf("FindExistingFileName: failed to close directory '%s'\n", 
	   d->DirPathName);

    free(safetyBlock);
    ReleaseMem();

    return -2;
  }

  /* If there are no files in this directory, give up now */
  if ( numFiles == 0 )
  { if (verbose)
      printf("FindExistingFileName: directory '%s' contains no files\n",
	     d->DirPathName);
    return -1;
  }

  whichFile=rand()*numFiles/RAND_MAX;
  strcpy(pathName, d->DirPathName);
  strcat(pathName, "/");
  strcat(pathName, fileNames[whichFile]);

  if (verbose)
    printf("FindExistingFileName: found file '%s'\n", pathName);

  sscanf(fileNames[whichFile], "f%d", &fileNum);
  return fileNum;
}



/* Not presently used .. */

#if 0 
/*----------------------------------------------------------------------*/
/*                                                  FindExistingDirName */
/*----------------------------------------------------------------------*/

/* Find the full pathname of a subdirectory in the supplied directory   */
/* if one exists, the choice of which dir being made randomly.          */
/*                                                                      */
/*   Inputs: 'd'         the directory in question                      */
/*           'pathName'  pointer to the space into which the full       */
/*                       pathname of the file is to be copied, if we    */
/*                       find it.                                       */
/*                                                                      */
/*   Outputs: return value = x  if we're returning pathName of dir dx   */
/*                           -1 if not returning a pathname but believe */
/*                              that filesystem is ok.                  */
/*			     -2 if we've spotted an inconsistency &     */
/*                              want a filesystem check.                */

int FindExistingDirName(Directory *d, char *pathName)
{
  char           dirNames[MAXNUMDIRS][FILENAMESIZE];
  struct dirent  *entry;
  DIR            *dir;
  int            whichDir, dirNum;
  int            numDirs=0;
  
  if ( ((dir=opendir(d->DirPathName)) == NULL) )
    { printf("FindExistingDirName: can't open directory %s\n", d->DirPathName);

      free(safetyBlock);
      ReleaseMem();

      return -2;
    }

  /* Find all the subdirectories of this directory. */
  while (1)
  { if ( (entry = readdir(dir)) == NULL )
    { if (errno !=0)
      { printf(
	    "FindExistingDirName: error whilst reading from directory '%s'\n",
	       d->DirPathName);

	free(safetyBlock);
	ReleaseMem();

	return -2;
      }
      else 
	break;
    }

    strcpy(pathName, d->DirPathName);
    strcat(pathName, "/");
    strcat(pathName, entry->d_name);


    /* If it was a dir that was just found, record its name, but        */
    /* don't validate it other than checking that its name is of the    */
    /* correct form - we could do so, but the program would run too     */
    /* slowly.                                                          */

    if ( entry->d_type == Type_Directory )
    { /* Ignore . & .. entries */
      if ( strcmp(entry->d_name, ".")  && strcmp(entry->d_name, "..") )
      { int index;

	if ( sscanf(entry->d_name, "d%d", &index) != 1 )
	{ printf("FindExistingDirName: dir '%s' has an invalid name\n", 
		 pathName);
	  return -2;
	}
        else
	  strcpy(dirNames[numDirs++], entry->d_name);
      }
    }

    else if ( entry->d_type == Type_File )
    { int index;
int rcode;
printf("FindExistingDirName: sscanf(%s, f.perc.d, &index)\n", 
       entry->d_name);
rcode=sscanf(entry->d_name, "f%d", &index);
printf("rcode is %d, index is %d\n", rcode, index);
      if ( sscanf(entry->d_name, "f%d", &index) != 1)
      { printf("FindExistingDirName: file '%s' has an invalid name\n", 
	       pathName);
	return -2;
      }
    }

    else
    { printf("FindExistingDirName: found object '%s' of an unknown type\n",
	     pathName);
      return -2;
    }
  }

  if ( closedir(dir) != 0 )
  { printf("FindExistingDirName: failed to close directory '%s'\n", 
	   d->DirPathName);

    free(safetyBlock);
    ReleaseMem();

    return -2;
  }

  /* If there are no subdirectories of this directory, give up now */
  if ( numDirs == 0 )
  { if (verbose)
      printf("FindExistingDirName: directory '%s' contains no subdirs\n",
	     d->DirPathName);
    return -1;
  }

  whichDir=rand()*numDirs/RAND_MAX;
  strcpy(pathName, d->DirPathName);
  strcat(pathName, "/");
  strcat(pathName, dirNames[whichDir]);

  if (verbose)
    printf("FindExistingDirName: found subdir '%s'\n", pathName);

  sscanf(dirNames[whichDir], "d%d", &dirNum);
  return dirNum;
}
#endif


/*----------------------------------------------------------------------*/
/*                                                         CheckEntries */
/*----------------------------------------------------------------------*/

/* Check the contents of the directory supplied, counting the total     */
/* numbers of valid and invalid files and subdirectories reachable from */
/* here by recursing deeper into the filesystem.                        */
/*                                                                      */
/* Note that a directory is included in the invalid tally if an error   */
/* is generated whilst we are trying to read an entry from it, or if    */
/* there is a problem with the '.' or '..' entries that it should       */
/* contain, but not if one of its files or subdirectories is in some    */
/* way faulty.  Otherwise each faulty directory would get counted twice.*/

void CheckEntries(Directory *d,
		  int *numValidFiles, int *numValidDirs,
		  int *numInvalidFiles, int *numInvalidDirs) 
{ 
  char          dirNames[MAXNUMDIRS][PATHNAMESIZE],
                fileNames[MAXNUMFILES][PATHNAMESIZE],
                pathName[PATHNAMESIZE]; 
  DIR           *dir;
  struct dirent *entry; 
  int           i, numSubDirs, numFiles, dotCount, dotdotCount, badDir;


  /* Zero counts ... */ 

  *numValidFiles=0; *numValidDirs=0;
  *numInvalidFiles=0; *numInvalidDirs=0;

  /* Zero the count of how many '.' and '..' entries we have found in */
  /* this directory so far - we should find exactly one of each.      */

  dotCount=0; dotdotCount=0;

  /* Reset the flag which indicates that there is something wrong with */
  /* the contents of this directory.                                   */

  badDir=0;

  if ( ((dir=opendir(d->DirPathName)) == NULL) )
  { printf("CheckEntries: can't open directory %s\n", d->DirPathName);
    (*numInvalidDirs)++;
    return;
  }

  /* Find the pathnames of all files and subdirectories contained in */
  /* this directory.                                                 */

  numFiles=numSubDirs=0;
  while (1)
  { if ( (entry = readdir(dir)) == NULL )
    { if (errno !=0)
      { printf(
	    "CheckEntries: error whilst reading from directory '%s'\n",
	       d->DirPathName);
	badDir=1;
      }
      break;
    }

    strcpy(pathName, d->DirPathName);
    strcat(pathName, "/");
    strcat(pathName, entry->d_name);

    if ( entry->d_type == Type_Directory )
    { 
      if ( !strcmp(entry->d_name, ".") )
      { DIR *dir;

	dotCount++;
	if (dotCount==2)
	{ printf(
          "CheckEntries: directory '%s' contains more than one '.' entry!\n",
		 d->DirPathName);
	  continue;
	}

	if ( (dir=opendir(pathName)) == NULL)
	{ printf(
		 "CheckEntries: cannot open '.' entry of directory '%s'\n",
		 d->DirPathName);
	  badDir=1;
	  continue;
	}

	if ( closedir(dir) == -1)
	{ printf("CheckEntries: can't close '.' entry of directory '%s'\n",
		 d->DirPathName);
	  badDir=1;
	  continue;
	}
      }

      else if ( !strcmp(entry->d_name, "..") )
      { DIR *dir;

	dotdotCount++;
	if (dotdotCount==2)
	{ printf(
	 "CheckEntries: directory '%s' contains more than one '..' entry!\n",
		 d->DirPathName);
	  continue;
	}

	if ( (dir=opendir(pathName)) == NULL)
	{ printf("CheckEntries: can't open '..' entry of directory '%s'\n",
		 d->DirPathName);
	  badDir=1;
	  continue;
	}

	if ( closedir(dir) == -1)
	{ printf("CheckEntries: can't close '..' entry of directory '%s'\n",
		 d->DirPathName);
	  badDir=1;
	  continue;
	}
      }

      /* If the directory entry isn't '.' or '..', check that its name is */
      /* of the form 'dx' and save its pathname away for later.           */
      else
      { int index;

	if ( sscanf(entry->d_name, "d%d", &index) != 1)
 	{ printf("CheckEntries: dir '%s' has an invalid name\n", 
		 pathName);
	  (*numInvalidDirs)++;
	  continue;
	}
      else
	strcpy(dirNames[numSubDirs++], pathName);
      }
    }

    else if ( entry->d_type == Type_File )
    { int index;

      /* Check that any file found has a name of the form 'fx', and save */
      /* its pathname away for later.                                    */

      if ( sscanf(entry->d_name, "f%d", &index) != 1)
      { printf("CheckEntries: file '%s' has an invalid name\n", 
	       pathName);
	(*numInvalidFiles)++;
      }
      else
	strcpy(fileNames[numFiles++], pathName);
    }

    else
    { printf("CheckEntries: found object '%s' of an unknown type\n",
	     pathName);
      badDir=1;
    }
  }

  /* Having obtained the pathnames of all files in the specified directory, */
  /* close the directory stream pointer before validating the files or      */
  /* recursing on the subdirectories. Things are done this way rather than  */
  /* inspecting and recursing as pathnames are obtained because otherwise   */
  /* recursion might lead to there being many directory streams             */
  /* simultaneously open, and the Helios limit to the number of open file   */
  /* descriptors could be exceeded.                                         */

  if ( closedir(dir) != 0 )
  { printf("CheckEntries: failed to close directory '%s'\n", d->DirPathName);
    (*numInvalidDirs)++; 
    return;
  }

  /* If the given directory is flawed, it gets added to the count of   */
  /* invalid directories, but we still check out the files and subdirs */
  /* it contains. Otherwise 1 invalid filename in a directory would    */
  /* cause all entries lower in the filesystem to be ignored.          */

  if ( badDir || (dotCount!=1) || (dotdotCount!=1) )
    (*numInvalidDirs)++;

  /* Validate the contents of all files within the given directory, and  */
  /* call CheckEntries recursively on the subdirectories to validate the */
  /* files and subdirectories that they themselves contain.              */

  /* First the files .. */
  for (i=0; i<numFiles; i++)
  {    
    if ( TestFile(fileNames[i], d->DirLevel) )
      (*numValidFiles)++;
    else
      (*numInvalidFiles)++;
  }

  /* Now the subdirectories .. */
  for (i=0; i<numSubDirs; i++)
  { int        subNumValidFiles,   subNumValidDirs;
    int        subNumInvalidFiles, subNumInvalidDirs;
    Directory  subDir;

    subDir.DirLevel  = d->DirLevel + 1;
    strcpy(subDir.DirPathName, dirNames[i]);
	      
    CheckEntries(&subDir,
		 &subNumValidFiles, &subNumValidDirs,
		 &subNumInvalidFiles, &subNumInvalidDirs);
    
    *numValidFiles   += subNumValidFiles;
    *numValidDirs    += subNumValidDirs;
    *numInvalidFiles += subNumInvalidFiles;
    *numInvalidDirs  += subNumInvalidDirs;
  }
}


/*----------------------------------------------------------------------*/
/*                                                             TestFile */
/*----------------------------------------------------------------------*/

/* Examine the file with pathname 'pathName', which is 'level'          */
/* directories down in our test RRD filesystem, and has index number    */
/* x if the last component of the pathname is fx . The index should     */
/* correspond to the index part of the file header word and to the      */
/* number of blocks it contains.                                        */
/*                                                                      */
/* Returns 1  if the file is ok,                                        */
/*         0  otherwise.                                                */

int TestFile(char *pathName, int level)
{
  FILE *f;
  unsigned char buf[BUFSIZE];
  int index, i, j, rcode, fileLevel, fileIndex;

  /* Get the last component of the file pathname given, which should */
  /* be of the form 'fx', and set the integer 'index' to x.          */

  for (i=strlen(pathName)-1; (i>=0) && isalnum(pathName[i]); i--)
    ;
  if ( (i<0) || (pathName[i]!='/') )
  { printf(
	"TestFile called with invalid pathname '%s'  - should never happen!\n",
	   pathName);
    return 0;
  }

  if ( sscanf(&pathName[i+1], "f%d", &index) != 1 )
  { printf(
	"TestFile called with invalid pathname '%s'  - should never happen!\n",
	   pathName);
    return 0;
  }

  if ( (f=fopen(pathName, "rb")) == NULL)
    {
      printf("TestFile: can't open file '%s'\n", pathName);
      rcode=0;
      goto TestFileEnd;
    }

  if ( ! ( fread(&fileLevel, sizeof(int), 1, f) &&
	   fread(&fileIndex, sizeof(int), 1, f) &&
           ( fileLevel == level) &&
	   ( fileIndex == index) ) )
    {
      printf("TestFile: file '%s' has a corrupt header\n", pathName);
      printf("TestFile: level=%d, index=%d\n", level, index);
      printf("TestFile: read-in values: level=%d, index=%d\n", 
	     fileLevel, fileIndex);
      rcode=0;
      goto TestFileEnd;
    }

  for (i=0; i<index; i++)
    {
      if ( fread(buf, 1, BUFSIZE, f) != BUFSIZE  )
	{
	  printf("TestFile: can't read block %d of file '%s'\n", i, pathName);
	  rcode=0;
	  goto TestFileEnd;
	}
      for (j=0; j<BUFSIZE; j++)
	if ( buf[j]!=i )
	  {
	    printf("TestFile: byte %d of block %d of file '%s' is corrupt\n",
		   j, i, pathName);
	    rcode=0;
	    goto TestFileEnd;
	  }
    }

  rcode=1;

 TestFileEnd:

  if (f!=NULL)
    if (fclose(f) == EOF)
      {
	printf("TestFile: can't close file %s - this shouldn't happen!", 
	       pathName);
	rcode=0;
      }

  return rcode;
}


/*----------------------------------------------------------------------*/
/*                                                           DeleteFile */
/*----------------------------------------------------------------------*/


/* DeleteFile:                                                          */
/*                                                                      */
/* Delete a file at random from the current directory, if one exists.   */
/*                                                                      */
/* Returns:  -1 if no file was deleted because of memory shortage -     */
/*              this should have been corrected if DeleteFile           */
/*              terminates - or if the directory supplied contains no   */
/*              files.                                                  */
/*                                                                      */
/*            0 if an unsuccessful attempt was made to delete a file,   */
/*                                                                      */
/*            size of file deleted otherwise.                           */

int DeleteFile(Directory *d)
{
  FILE        *f;
  char        pathName[PATHNAMESIZE];
  int         fileSize, findCode;

  if (verbose)
    printf("DeleteFile called with directory '%s'\n", d->DirPathName);

  if ( (findCode=FindExistingFileName(d, pathName)) == -1 )
  { if (verbose)
      printf("DeleteFile: failed to get a file to delete\n");
    return -1;
  }
  else if (findCode == -2)
  { if (verbose)
      printf("DeleteFile: spotted a filesystem flaw\n");
    return 0;                         /* Force a filesystem check */
  }
     
  printf("Deleting file %s\n", pathName);

  if ( (f=fopen(pathName, "r")) == NULL )
  { free(safetyBlock);  /* Assume fopen failed because of memory shortage. */
    ReleaseMem();

    printf("DeleteFile: failed to open file\n");

    return -1;
  }

  if ( fseek(f, 0, SEEK_END) == -1 )
  { fclose(f);
    free(safetyBlock);
    ReleaseMem();

    printf("DeleteFile: failed to seek to end of file\n");

    return -1;
  }

  if ( (fileSize=(int)ftell(f)) == -1)
  { fclose(f);
    free(safetyBlock);
    ReleaseMem();

    printf("DeleteFile: failed to get size of file\n");

    return -1;
  }

  if ( fclose(f) == EOF )
  { free(safetyBlock);
    ReleaseMem();

    printf("DeleteFile: failed to close file\n");

    return -1;
  }

  if (verbose)
    printf("DeleteFile: deleting file '%s'\n", pathName);

  if ( unlink(pathName) == -1 )
  { printf("DeleteFile: failed to unlink file %s\n", pathName);

    free(safetyBlock);
    ReleaseMem();

    return 0;
  }

  return fileSize;
}


/*----------------------------------------------------------------------*/
/*                                                           ReleaseMem */
/*----------------------------------------------------------------------*/

/* ReleaseMem:                                                          */
/*                                                                      */
/* Try to free some memory by deleting files, in the case when the      */
/* system has run out of it.                                            */

void ReleaseMem(void)
{
  Directory *dir, *subDir;
  int       i, totalReleased;

  /* Functions called from ReleaseMem (TreeWander, DeleteFile etc.) may  */
  /* run out of memory, even though we've tried to prevent this by       */
  /* freeing the block of size SAFETYBLOCKSIZE that safetyBlock normally */
  /* points to.                                                          */
  /*                                                                     */
  /* In this unfortunate case, said functions will execute the sequence  */
  /*          free(safetyMem);                                           */
  /*          ReleaseMem();                                              */
  /*                                                                     */
  /* ReleaseMem will spot that it has been called recursively (this is   */
  /* the purpose of the flag releasingMem) and bomb out.                 */

  printf("Out of memory, releasing some...\n");fflush(stdout);
  totalReleased=0;

  if (releasingMem)
  { printf("ReleaseMem has been called recursively:\n");
    printf("  maybe the process of freeing memory has itself run out\n");
    printf("  of memory, hence bombing out.\n");
    exit(127);
  }
  else
  { /* safetyBlock must point to a valid block, thought it be only of size */
    /* 1, to protect against further memory failure & the accompanying     */
    /* execution of 'free(safetyBlock)' crashing the system because        */
    /* safetyBlock has already been freed.                                 */

    if ( (safetyBlock=malloc(1))==NULL )
    { printf("ReleaseMem has itself run out of memory: goodbye!\n");
      exit(127);
    }
  }

  releasingMem=1;   /* Flag that we are in ReleaseMem, so that */
                    /* we can spot a recursive call to it.     */


  /* We have at most RELEASEMEMTRIES goes at releasing a total amount */
  /* of memory whose size is at lease RELEASEMULTIPLE times the size  */
  /* of the safety block we allocate, viz. SAFETYBLOCKSIZE.           */

  for (i=0; 
       i<RELEASEMEMTRIES && totalReleased < RELEASEMULTIPLE*SAFETYBLOCKSIZE; 
       i++)
  { int fileSize;

    if ( (dir=malloc(sizeof(Directory))) == NULL)
    { printf("ReleaseMem: can't allocate memory for root directory struct\n" );
      exit(127);
    }
      
    dir->DirLevel=0;
    strcpy(dir->DirPathName, ROOTDIR);
      
    /* Get a directory to delete some files from */
    if ( (subDir=TreeWander(dir)) == NULL )
    { printf("ReleaseMem: TreeWander failed\n");
      exit(127);
    }

    fileSize=DeleteFile(subDir);

    /* Add size of file deleted onto 'totalReleased' sum, if the file was */
    /* deleted OK. NB. the total amount of memory freed is going to be    */
    /* larger than totalReleased, since this figure doesn't include any   */
    /* of the memory overheads associated with the files deleted          */
    /* (checksums, half-empty blocks etc).                                */

    if (fileSize<1)
      continue;
    else
      totalReleased += fileSize;  
  }

  printf("ReleaseMem: tried to release at least %d bytes: got %d\n",
	 RELEASEMULTIPLE*SAFETYBLOCKSIZE, totalReleased);

/* If we didn't get as much memory back as wanted - carry on regardless. */
#if 0
  if ( totalReleased < RELEASEMULTIPLE*SAFETYBLOCKSIZE )
    exit(127);
#endif

  releasingMem=0; 
  /* Finished freeing memory - can now get on with the bashing! */ 

  /* Finally save away space in safety block again for the next time we */
  /* run out of memory.                                                 */
  if ( (safetyBlock=malloc(SAFETYBLOCKSIZE)) == NULL )
  { printf("ReleaseMem: no memory to reallocate safety block!\n");
    exit(2);
  }
}

