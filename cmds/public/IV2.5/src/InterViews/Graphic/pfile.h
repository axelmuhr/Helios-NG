/*
 * PFile encapsulates standard file operations.
 */

#ifndef pfile_h
#define pfile_h

#include <stdio.h>
#include <InterViews/defs.h>

class PFile {
public:
    PFile(char* filename);
    ~PFile();
    
    char* GetName();
    boolean Exists();
    boolean Exists(char* filename);

    boolean Read(short& i);
    boolean Read(int& i);
    boolean Read(float& f);
    boolean Read(char& c);
    boolean Read(short* i, int count);
    boolean Read(int* i, int count);
    boolean Read(long* i, int count);
    boolean Read(float* f, int count);
    boolean Read(char* string);
    boolean Read(char* string, int count);

    boolean Write(int i);
    boolean Write(float f);
    boolean Write(short* i, int count);
    boolean Write(int* i, int count);
    boolean Write(long* i, int count);
    boolean Write(float* f, int count);
    boolean Write(char* string);
    boolean Write(char* string, int count);
    
    boolean SeekTo(long offset); /* offset bytes from beginning of file */
    boolean SeekToBegin();
    boolean SeekToEnd();

    long CurOffset();
    boolean IsEmpty();
    boolean Erase();
protected:
    char* name;
    FILE* fd;
};

#endif
