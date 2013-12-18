/**
*
* A noddy version of `cat'.
* It first creates a list structure containing the names of the files to be
* concatenated and then `walks' this list copying each file to the standard
* output.
*
**/

#include <helios.h>
#include <queue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
  Node node;
  char name[1];
} FileNode;

List filelist;

/**
*
* Add a filename to a list.
*
**/
void addfile(char *name)
{
  FileNode *filenode = (FileNode *)malloc(sizeof(FileNode) + strlen(name));

  strcpy(filenode->name, name);
  AddTail(&filelist, &filenode->node);
}

/**
*
* Copy a file to the standard output
*
**/
void copyfile(FILE *file)
{
  int c;

  until ((c = fgetc(file)) == EOF)
    putchar(c);
}

/**
*
* Open a file and copy it to the standard output stream.
*
**/
void putfile(FileNode *filenode)
{
  FILE *file;

  if ((file = fopen(filenode->name, "r")) == NULL)
  {
    fprintf(stderr, "Unable to open file `%s'\n", filenode->name);
    return;
  }
  copyfile(file);
  fclose(file); 
}

int main(int argc, char *argv[])
{
  int i;

  InitList(&filelist);
  if (argc == 1)
    copyfile(stdin);
  else
  {
    for (i = 1; i < argc; i++)
      addfile(argv[i]);
    WalkList(&filelist, (WordFnPtr)putfile, 0);
  }
  return 0;
}
