#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>


#include "bash.h"


/*----------------------------------------------------------------------*/
/*                                                   Forward references */
/*----------------------------------------------------------------------*/

static int       CreateFile(Directory *d);
static int       AppendFile(Directory *d);
static int       RenRRD(char *oldName, char *newName);
static int       RemoveRootPath(char *pathName);
static int       CreateDir(Directory *d);
static int       DeleteDir(Directory *d);
static int       FindUnusedFileName(Directory *d, char *pathName);
static int       FindUnusedDirName(Directory *d, char *pathName);


/*----------------------------------------------------------------------*/
/*                                                                 main */
/*----------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
  DIR *d;
  int c;
#ifdef SLEAKTEST
  int slt=0;
#endif

  cautious=0;  /* If set, we do a filesystem check after every operation */
  verbose=0;   /* If set, print out lots of info as we go along */
  releasingMem=0;  /* Memory situation is OK at the moment */


  if ( (safetyBlock=malloc(SAFETYBLOCKSIZE)) == NULL )
    {
      printf("No memory to allocate safety block!\n");
      exit(2);
    }

  for (c=1; c<argc; c++)
    if (!strcmp(argv[c], "-s"))
      {
	int seed=atoi(argv[++c]);
	srand(seed);
	printf("Random number generator seed set to %d\n", seed);
      }
    else if ( !strcmp(argv[c], "-cautious") || !strcmp(argv[c], "-c") )
      cautious=1;
    else if (!strcmp(argv[c], "-verbose") || !strcmp(argv[c], "-v") )
      verbose=1;
#ifdef SLEAKTEST
    else if (!strcmp(argv[c], "-slt"))
      slt=1;
#endif
    else
      {
#ifdef SLEAKTEST
	printf("Usage: %s [-s <seed>] -cautious -verbose -slt\n", argv[0]);
#else
	printf("Usage: %s [-s <seed>] -cautious -verbose\n", argv[0]);
#endif
	exit(2);
      }

  /* If the root directory doesn't yet exist, create it */
  if ( (d=opendir(ROOTDIR)) == NULL) 
  { if ( mkdir(ROOTDIR, S_IRWXU) )
    { printf("Can't create root directory %s\n", ROOTDIR);
      exit(2);
    }
  }
  else 
  { if ( closedir(d)!=0 )
    { printf("Can't close root directory %s - this shouldn't happen!",
	     ROOTDIR);
      exit(2);
    }
  }

  while (1)
  { int operation, veriFlag, am;
    Directory *dir, *subDir;

    fflush(stdout);

    veriFlag=1;

    /* First get a handle on some directory within the FS */

#if 0
    if ( ((d=opendir(ROOTDIR)) == NULL) )
    { printf("Can't open root directory %s\n", ROOTDIR);

      /* Assume that the open failed because we ran out of memory: */
      /* try to free some & carry on */
      free(safetyBlock);
      ReleaseMem();
      continue;
    }
#endif

    if ( (dir=malloc(sizeof(Directory))) == NULL)
    { printf("Can't allocate memory for root directory struct\n");
      
      /* Try to free some memory in case we've run out */
      free(safetyBlock);
      ReleaseMem();
      continue;
    }

    dir->DirLevel=0;
    dir->DirNumFiles=-1;  /* -1 means that we don't know how many files */
    dir->DirNumSub=-1;    /* and subdirectories this directory contains */
    strcpy(dir->DirPathName, ROOTDIR);
    if ( (subDir=TreeWander(dir)) == NULL )
      continue;

    /* If simply looking for storage leaks, loop executing TreeWander */
    /* but not calling the functions which modify the filesystem.     */

       
#ifdef SLEAKTEST
    if (!slt)
    {
#endif
    /* Now randomly choose an operation to perform on this directory or */
    /* its contents                                                     */
    operation=rand()*(LAST_OP+1)/RAND_MAX;

    /* Perform the operation - returns 0 if unsuccessful */

    if ( (operation>=OPS_DELETE_DIR_LOW) && 
	      (operation<=OPS_DELETE_DIR_HIGH) )
      veriFlag=DeleteDir(subDir);

    else if ( (operation>=OPS_DELETE_FILE_LOW) && 
	      (operation<=OPS_DELETE_FILE_HIGH) )
      veriFlag=DeleteFile(subDir);

    else if ((am=(word)Malloc(-5)) > MINMEMFREE) {
    	if ( (operation>=OPS_CREATE_FILE_LOW) && 
		 (operation<=OPS_CREATE_FILE_HIGH) )
	      veriFlag=CreateFile(subDir);

	    else if ( (operation>=OPS_APPEND_FILE_LOW) && 
		      (operation<=OPS_APPEND_FILE_HIGH) )
	      veriFlag=AppendFile(subDir);

	    else if ( (operation>=OPS_CREATE_DIR_LOW) && 
		      (operation<=OPS_CREATE_DIR_HIGH) )
	      veriFlag=CreateDir(subDir);
	}
	else if (verbose) printf("No create operations as available memory (%d) < %d'\n",am, MINMEMFREE);

    /* Even if none of the filesystem operations above have failed,  */
    /* we do filesystem checks at random intervals anyway.           */

    /* If we're running in 'cautious' mode we do a check after every */
    /* operation.                                                    */

    if (veriFlag != 0)
      veriFlag=rand()*(VERI_FREQ+1)/RAND_MAX;
    if (!veriFlag || cautious) 
      VerifyFS();

#ifdef SLEAKTEST
    }
#endif

    /* Tidy up before next iteration */
    free(subDir);
  }
}


/*----------------------------------------------------------------------*/
/*                                                           CreateFile */
/*----------------------------------------------------------------------*/

/* Create a file with a random name and corresponding size in the       */
/* directory supplied.                                                  */

/* The format of each file is that the 1st word is an integer giving    */
/* depth of the file in the directory structure, ie. files in the root  */
/* directory have depth 0, files in subdirectories of the root have     */
/* depth 1, subdirectories of these subdirectories level 2, and so on.  */

/* If a file is called 'fn' where n is an integer, the 2nd word of the  */
/* file should also be n.                                               */

/* The rest of the file consists of n blocks of size BUFSIZE bytes,     */
/* where the 1st block (block 0) consists of all 0 bytes, block 1 of    */
/* all 1 bytes and so on. Block 256 would consist of 0 bytes again,     */
/* were a file containing such a block to be created.                   */

/* Having created the file, verify that it is valid.                    */
   
/* Return 0 if the operation was unsuccessful, 1 otherwise.             */

/* If the file is partially-created and then there is an error, it is   */
/* deleted from the filesystem if possible, so that filesystem          */
/* correctness is maintained.                                           */

static int CreateFile(Directory *d)
{
  int         fileNum, i, j;
  char        pathName[PATHNAMESIZE], buf[BUFSIZE];
  FILE        *f;

  if (verbose)
    printf("CreateFile called on directory '%s'\n", d->DirPathName);

  if ( (fileNum=FindUnusedFileName(d, pathName)) == -1 )
  { if (verbose)
      printf("CreateFile: FindUnusedFileName failed\n");
    return 0;
  }

  printf("Creating file %s\n", pathName);

  if ( (f=fopen(pathName, "wb")) == NULL)
  { printf("CreateFile: failed to open file %s\n", pathName);

    free(safetyBlock);
    ReleaseMem();

    return 0;
  }
  
  if ( !fwrite(&(d->DirLevel), sizeof(int), 1, f) ||
       !fwrite(&fileNum, sizeof(int), 1, f) )
  { printf("CreateFile: error writing header of file %s\n", pathName);

    free(safetyBlock);
    ReleaseMem();

    if ( fclose(f) == EOF)
      printf("CreateFile: couldn't close file %s\n", pathName);

    if ( unlink(pathName) == -1)
      printf("CreateFile: couldn't delete file %s\n", pathName);

    return 0;
  }

  for (i=0; i<fileNum; i++)
  { for (j=0; j<BUFSIZE; j++)
      buf[j]=(unsigned char)i;

    if ( fwrite(buf, 1, BUFSIZE, f) == 0 )
    { printf("CreateFile: error in writing contents of file %s\n", 
	     pathName);

      free(safetyBlock);
      ReleaseMem();
      
      if ( fclose(f) == EOF )
	printf("CreateFile: couldn't close file %s\n", pathName);
      
      if ( unlink(pathName) == -1)
	printf("CreateFile: couldn't delete file %s\n", pathName);
      
      return 0;
    }
  }

#ifndef DEBUG
  if (verbose)
#endif
    printf("CreateFile: created file %s\n", pathName);

  if ( fclose(f) ==EOF )
  { printf("CreateFile: couldn't close file %s\n", pathName);

    free(safetyBlock);
    ReleaseMem();

    if ( unlink(pathName) == -1)
      printf("CreateFile: couldn't delete file %s\n", pathName);

    return 0;
  }
 
  if ( TestFile(pathName, d->DirLevel) )
    return 1;
  else
  { printf("CreateFile: new file '%s' is corrupt!\n", pathName);
    return 0;
  }
}


/*----------------------------------------------------------------------*/
/*                                                           AppendFile */
/*----------------------------------------------------------------------*/


/* Append to a file chosen at random from those in the current          */
/* directory, if any exist.                                             */

/* Returns:  -1 if no file was appended to because of memory shortage,  */
/*               this should have been corrected if AppendFile          */
/*               terminates - or if the directory supplied contains     */
/*               no files,                                              */
/*            0 if an attempt was made to append to a file which was    */
/*               unsuccessful for some other reason,                    */
/*            1 otherwise.                                              */

/* This operation involves more operations that simply appending to a   */
/* file because we encode the number of blocks a file contains in its   */
/* name, thus the append process is actually:                           */

/*  1. if the filename is fx with 0 <= x < (MAXNUMFILES -1),            */
/*     choose a y with y > x such that file fy does not exist in        */
/*     the given directory (and give up if such a y cannot be found),   */

/*  2. open the file in "r+" mode and modify the 2nd header word to     */
/*     y since the file is to be called 'fy'.                           */

/*  3. Seek to the end of the file and add the extra blocks on to       */
/*     the end,                                                         */

/*  3. rename the file to 'fy',                                         */

/*  4. verify that file fy is OK.                                       */


static int AppendFile(Directory *d)
{ FILE        *f;
  char        pathName[PATHNAMESIZE], newPathName[PATHNAMESIZE], buf[BUFSIZE];
  int         i, j;
  int         oldIndex, newIndex, iFlag;


  if (verbose)
    printf("AppendFile called with directory '%s'\n", d->DirPathName);
  

  if ( (oldIndex=FindExistingFileName(d, pathName)) == -1 )
    {
	printf("AppendFile: failed to get a file to append to\n");
	return -1;
    }
  else if (oldIndex == -2 )
    {
      printf("AppendFile: found a flaw in the filing system\n");
      return 0;
    }

  printf("AppendFile: appending to file '%s'\n", pathName);
 
  /* Having selected a file to append to, we now have FINDUNUSEDFILETRIES */
  /* goes at finding a name that we can rename it to, having appended     */
  /* to it, such that the new name and the file size match (the number    */
  /* of blocks is encoded in the name), and the new size is greater than  */
  /* the old - as it must be, of course, since we're appending.           */
  
  newIndex=-1; iFlag=0;

  for (i=0; i<FINDUNUSEDFILETRIES; i++)
    if ( (newIndex=FindUnusedFileName(d, newPathName)) > oldIndex )
    { iFlag=1; break; }

  if (!iFlag)
  { printf("AppendFile: failed to find a suitable new name for file %s\n",
	   pathName);
    return 1;  /* Don't request a filesystem check, this is tedious. */
  }


  /* Open file in "r+" mode, so we can overwrite the 2nd header word without */
  /* losing the rest of the file's contents.                                 */
  if ( (f=fopen(pathName, "r+")) == NULL )
  { printf("AppendFile: failed to open file '%s'\n", pathName);
    return 0;
  }

  /* Move file pointer to the 2nd word, & change it to correspond to the */
  /* file's new name.                                                    */
  fseek(f, 4, SEEK_SET);

  if ( !fwrite(&newIndex, sizeof(int), 1, f) )
  { printf("AppendFile: failed to modify header byte of file '%s'\n", 
	    pathName);

    free(safetyBlock);
    ReleaseMem();

    if ( fclose(f) == EOF )
      printf("AppendFile: failed to close file '%s'\n", pathName);
    
    if ( unlink(pathName) == -1 )
      printf("AppendFile: couldn't delete file '%s'\n", pathName);

    return 0;
  }

  /* Now move the file pointer to its end and append the new blocks on to */
  /* the end.                                                             */
  fseek(f, 0, SEEK_END);

  for (i=oldIndex; i<newIndex; i++)
  { for (j=0; j<BUFSIZE; j++)
      buf[j]=(unsigned char)i;

    if ( !fwrite(buf, 1, BUFSIZE, f) )
    { printf("AppendFile: failed to append a block to file '%s'\n", 
	     pathName);

      free(safetyBlock);
      ReleaseMem();

      if ( fclose(f) == EOF )
	printf("AppendFile: failed to close file '%s'\n", pathName);

      if ( unlink(pathName) == -1 )
	printf("AppendFile: couldn't delete file '%s'\n", pathName);
    }
  }

  if ( fclose(f) ==EOF )
    {
      printf("AppendFile: couldn't close file %s\n", pathName);

      free(safetyBlock);
      ReleaseMem();

      if ( unlink(pathName) == -1 )
	printf("CreateFile: couldn't delete file %s\n", pathName);

      return 0;
    }

  /* Having appended to the file, give it its new name & check its validity */

#if 0
  if (verbose)
#endif
    printf("AppendFile: renaming file '%s' to '%s'\n", 
	   pathName, newPathName);

  /* There's a bug in the posix 'rename' function, so use one of my own */
  /* devising instead.                                                  */
  if ( !RenRRD(pathName, newPathName) )
  { printf("CreateFile: failed to rename file '%s' as '%s'\n", 
	   pathName, newPathName);
    printf("CreateFile: errno is %d\n", errno);
    return 0;
  }
 
  if ( TestFile(newPathName, d->DirLevel) )
    return 1;
  else
    {
      printf("AppendFile: new file '%s' is corrupt!\n", newPathName);
      return 0;
    }
}


/*----------------------------------------------------------------------*/
/*                                                               RenRRD */
/*----------------------------------------------------------------------*/

/* RenRRD:                                                              */
/*                                                                      */
/* Takes 2 pathnames in the directory of the RRD given by the macro     */
/* ROOTDIR, the first being the name of a file that presently exists.   */
/* This file is renamed to have the pathname which is the 2nd argument. */

/* Returns 1 if rename was performed successfully, 0 otherwise.         */

static int RenRRD(char *oldName, char *newName)
{ char     first[PATHNAMESIZE], second[PATHNAMESIZE];
  Object   *root;

  strcpy(first, oldName);
  strcpy(second, newName);

  if ( !RemoveRootPath(first) || !RemoveRootPath(second) )
  { fprintf(stderr, 
	    "RenRRD: trying to rename '%s' as '%s' - this shouldn't happen!\n",
	    oldName, second);
    exit(2);
  }

  if ( (root=Locate(NULL, ROOTDIR)) == NULL)
  { fprintf(stderr, "RenRRD: failed to locate root directory '%s'\n", ROOTDIR);
    exit(2);
  }

  if ( Rename(root, first, second) != 0 )
    return 0;
  else
    return 1;
}


/*----------------------------------------------------------------------*/
/*                                                       RemoveRootPath */
/*----------------------------------------------------------------------*/

/* RemoveRootPath:                                                      */
/*                                                                      */
/* Modify the supplied pathname, which is assumed to begin with the     */
/* pathname specified by the macro ROOTDIR, so that it doesn't begin in */
/* this way, but is instead relative to ROOTDIR.                        */

/* Returns 1 if this operation is performed OK, 0 otherwise - which     */
/* will happen if the pathname doesn't being with ROOTDIR.              */

static int RemoveRootPath(char *pathName)
{ char rootSlash[PATHNAMESIZE], rootLess[PATHNAMESIZE];

  strcpy(rootSlash, ROOTDIR);/* The ROOTDIR macro doesn't end with a slash. */
  strcat(rootSlash, "/");

  if ( strncmp(pathName, rootSlash, strlen(rootSlash)) )
    return 0;

  /* Copy 'pathName' sans 'rootSlash ' prefix into 'rootLess' and then */
  /* back into pathName.                                               */
  strcpy(rootLess, &pathName[strlen(rootSlash)]);
  strcpy(pathName, rootLess);

  return 1;
}

  
/*----------------------------------------------------------------------*/
/*                                                            CreateDir */
/*----------------------------------------------------------------------*/

/* CreateDir:                                                           */
/*                                                                      */
/* Create a randomly-named subdirectory of the directory supplied.      */
/*                                                                      */
/* Return 0 if the operation was unsuccessful, 1 otherwise.             */

static int CreateDir(Directory *d)
{ char        pathName[PATHNAMESIZE];

  if (verbose)
    printf("CreateDir called on directory '%s'\n", d->DirPathName);

  if ( !FindUnusedDirName(d, pathName) )
  { if (verbose)
      printf("CreateDir: FindUnusedDirName failed\n");
    return 0;
  }

  printf("Creating directory %s\n", pathName);

  if ( mkdir(pathName, S_IRWXU) == -1 )
  { printf("CreateDir: failed to create directory %s\n", pathName);

    free(safetyBlock);
    ReleaseMem();

    return 0;
  }
  else
    return 1;
}


/*----------------------------------------------------------------------*/
/*                                                            DeleteDir */
/*----------------------------------------------------------------------*/

/* DeleteDir:                                                           */
/*                                                                      */
/* Delete the directory supplied, along with any files or               */
/* subdirectories that it contains.                                     */
/*                                                                      */
/* Returns 1 if the operation was successful, 0 otherwise.              */

static int DeleteDir(Directory *d)
{ char           dirNames[MAXNUMDIRS][PATHNAMESIZE],
                 fileNames[MAXNUMFILES][PATHNAMESIZE];
  int            numDirs, numFiles, i;
  DIR            *dir;


  if (verbose)
    printf("DeleteDir called with directory '%s'\n", d->DirPathName);

  if ( !strcmp(d->DirPathName, ROOTDIR) )
  { printf("DeleteDir: can't delete root directory '%s'\n", ROOTDIR);
    return 1;
  }

  if ( (dir=opendir(d->DirPathName)) == NULL )
  { printf("DeleteDir: failed to open directory '%s'\n", d->DirPathName);
    
    free(safetyBlock);
    ReleaseMem();

    return 0;
  }

  /* Find all the files and subdirs of the selected directory */

  numDirs=numFiles=0;
  while (1)
  { struct dirent  *entry;
    char           subPathName[PATHNAMESIZE];

    if ( (entry = readdir(dir)) == NULL )
    { 
      if (errno !=0)
      { printf("DeleteDir: error whilst reading from directory '%s'\n", 
	       d->DirPathName);

	free(safetyBlock);
	ReleaseMem();

	return 0;
      }
      else
	break;
    }

    strcpy(subPathName, d->DirPathName);
    strcat(subPathName, "/");
    strcat(subPathName, entry->d_name);


    /* Record the name of the file or dir we just found, but don't     */
    /* validate it other than checking that its name is of the correct */
    /* form - we could do so, but the program would run more slowly    */
    if ( entry->d_type == Type_Directory )
    { /* Ignore . & .. entries */
      if ( strcmp(entry->d_name, ".")  && strcmp(entry->d_name, "..") )
      { int index;

	if ( sscanf(entry->d_name, "d%d", &index) != 1 )
	{ printf("DeleteDir: dir '%s' has an invalid name\n", subPathName);
	  return 0;
	}
	else
	  strcpy(dirNames[numDirs++], subPathName);
      }
    }

    else if ( entry->d_type == Type_File )
    { int index;

      if ( sscanf(entry->d_name, "f%d", &index) != 1 )
      { printf("DeleteDir: file '%s' has an invalid name\n", subPathName);
	return 0;
      }
      else
	strcpy(fileNames[numFiles++], subPathName);
    }

    else
    { printf("DeleteDir: found object '%s' of an unknown type\n", subPathName);
      return 0;
    }
  }
  
  if ( closedir(dir) != 0 )
  { printf("DeleteFile: failed to close directory '%s'\n", d->DirPathName);

    free(safetyBlock);
    ReleaseMem();

    return NULL;
  }


  /* Delete all files found in the subdirectory */

  for (i=0; i<numFiles; i++)
    if ( unlink(fileNames[i]) == -1 )
    { printf("DeleteDir: failed to unlink file %s\n", fileNames[i]);

      free(safetyBlock);
      ReleaseMem();

      return 0;
    }


  /* Now delete all subdirectories found in the subdirectory by calling */
  /* DeleteDir recursively.                                             */

  for (i=0; i<numDirs; i++)
  { Directory subD;

    strcpy(subD.DirPathName, dirNames[i]);
     /* Subdir is 1 level down from 'd' in directory hierarchy */
    subD.DirLevel = (d->DirLevel) + 1;
    subD.DirNumFiles=subD.DirNumSub=-1;

    if ( !DeleteDir(&subD) )
      return 0;
  }


  /* Finally unlink the selected directory itself, which should */
  /* now be empty.                                              */

  if ( unlink(d->DirPathName) == -1 )
    { printf("DeleteDir: failed to unlink dir '%s'\n", d->DirPathName);

      free(safetyBlock);
      ReleaseMem();

      return 0;
     }

  return 1;
}


/*----------------------------------------------------------------------*/
/*                                                   FindUnusedFileName */
/*----------------------------------------------------------------------*/

/* Try to come up with a filename that hasn't been used in the supplied */
/* directory.                                                           */
   
/* Inputs: 'd'         the directory in question                        */
/*         'pathName'  pointer to the space into which the full pathname*/
/*                     of the file is to be copied, if we find it.      */ 

/* Outputs: return value = x    if the filename we're returning is 'fx' */
/*                              (x>=0)                                  */
/*                         -1   otherwise                               */

static int FindUnusedFileName(Directory *d, char *pathName)
{
  int    foundFlag=0;     /* Set if we're successful in finding a filename */
  int    i, fileNum;
  char   fileName[FILENAMESIZE];
  struct stat s;


  /* Try FINDUNUSEDFILETRIES times to find a filename */

  for (i=0; i<=FINDUNUSEDFILETRIES; i++)
    {
      fileNum=MAXNUMFILES*rand()/RAND_MAX;
      sprintf(fileName, "/f%d", fileNum);
      strcpy(pathName, d->DirPathName);
      strcat(pathName, fileName);
      if ( stat(pathName, &s) == -1)
	  {
	    if (errno==ENOENT)  /* Here stat has failed because the file
				   doesn't exist - so it's ok to create it */
	    { foundFlag=1;
	      break;
	    }
	    else
	    {                 /* Here stat has failed for some unexpected */
			      /* reason                                   */
	      printf("Couldn't stat file '%s' for some reason\n", pathName);

	      free(safetyBlock); /* Could be because we ran out of memory */
	      ReleaseMem();
	      return -1;
	    }
	  }
    }

  /* Give up now if we failed to find a suitable filename */
  if (!foundFlag)
  { if (verbose)
      printf(
	"FindUnusedFileName: couldn't find a suitable filename: giving up!\n"
	     );
    return -1;
  }
  else
    return fileNum;
}


/*----------------------------------------------------------------------*/
/*                                                    FindUnusedDirName */
/*----------------------------------------------------------------------*/

/* Try to come up with a directory name that hasn't been used in the     */
/* supplied directory.                                                   */
   
/* Inputs: 'd'         the directory in question                         */
/*         'pathName'  pointer to the space into which the full pathname */
/*                     of the directory is to be copied, if we find it.  */ 

/* Outputs: return value = 1 if a directory name could be found,         */
/*                         0 otherwise                                   */

static int FindUnusedDirName(Directory *d, char *pathName)
{
  int    foundFlag=0;     /* Set if we're successful in finding a filename */
  int    i, fileNum;
  char   fileName[FILENAMESIZE];
  struct stat s;


  /* Try FINDUNUSEDFILETRIES times to find a dirname */

  for (i=0; i<=FINDUNUSEDFILETRIES; i++)
    {
      fileNum=MAXNUMDIRS*rand()/RAND_MAX;
      sprintf(fileName, "/d%d", fileNum);
      strcpy(pathName, d->DirPathName);
      strcat(pathName, fileName);
      if ( stat(pathName, &s) == -1)
	  {
	    if (errno==ENOENT)  /* Here stat has failed because the dir.   */
				/* doesn't exist - so it's ok to create it */
	    { foundFlag=1;
	      break;
	    }
	    else
	    {                 /* Here stat has failed for some unexpected */
			      /* reason                                   */
	      printf("Couldn't stat file '%s' for some reason\n", pathName);

	      free(safetyBlock); /* Could be because we ran out of memory */
	      ReleaseMem();
	      return 0;
	    }
	  }
    }

  /* Give up now if we failed to find a suitable filename */
  if (!foundFlag)
  { if (verbose)
      printf(
	"FindUnusedDirName: couldn't find a suitable name: giving up!\n"
	     );
    return 0;
  }
  else
    return 1;
}
