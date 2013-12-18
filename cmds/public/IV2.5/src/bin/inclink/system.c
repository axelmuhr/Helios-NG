/* 
 * Keep track of what files have changed.
 */

#include "errhandler.h"
#include "system.h"
#include <errno.h>
#include <osfcn.h>
#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>

const char* arMagicString = "!<arch>\n";

static boolean IsLibFile(const char*);

static boolean IsLibFile (const char* b) {
    return strncmp(b, arMagicString, arMagicLen) == 0;
}

time_t UpdateTime (const char* filename) {
    struct stat st;
    stat(filename, &st);
    return st.st_mtime;
}

FileType GetFileType (FILE* f) {
    const int SAMPLELEN = 2;
    int sample[SAMPLELEN];
    int status = fread((char*)sample, SAMPLELEN * sizeof(int), 1, f);
    if (status != 1) {
        Panic("GetFileType: read error");
    }
    fseek(f, 0L, 0);
    if (IsObjectFile(sample)) {
        return OBJECT_FILE;
    } else if (IsLibFile((char*)sample)) {
        return LIBRARY_FILE;    
    } else {
        return OTHER_FILE;
    }
}

unsigned short GetMode (FILE* f) {
    struct stat s;

    fstat(fileno(f), &s);
    return s.st_mode;
}

void ReadBuff (
    FILE* f, Address buff, int len, const char* errMsg, int exitStatus
) {
    if (len != 0) {
	int status = fread((char*)buff, len, 1, f);
	if (status != 1) {
	    if (errMsg == nil) {
		Warning("ReadBuff screwed up");
	    } else {
		Warning("ReadBuff : %s", errMsg);
	    }
	    exit(exitStatus);
	}
    }
}

void WriteBuff (FILE* f, Address buff, int len) {
    if (len != 0) {
	int status = fwrite((char*)buff, len, 1, f);
	if (status != 1) {
	    Fatal("WriteBuff screwed up");
	}
    }
}

const char* SegTypeToStr (SegmentType st) {
    const char *s;
    switch (st) {
	case TEXT_SEG:  s = "TEXT_SEG"; break;
	case DATA_SEG:  s = "DATA_SEG"; break;
	case BSS_SEG:   s = "BSS_SEG"; break;
	case ANON_SEG:  s = "ANONSEG"; break;
	case UNDEF_SEG: s = "UNDEF_SEG"; break;
	case MISC_SEG:  s = "MISC_SEG"; break;
	case ABS_SEG:   s = "ABS_SEG"; break;
        case SYM_SEG:   s = "SYM_SEG"; break;
        case STR_SEG:   s = "STR_SEG"; break;
	case ERROR_SEG: s = "ERROR_SEG"; break;
    }
    return s;
}

// search colon separated directory path for name, return nil or path/name. 
const char* SearchPath (const char* name, const char* path) {
    static const int BuffSize = 1024;
    static char pathBuff[1024];
    const char *start = path;
    const char *p = start;
    boolean found = false;
    boolean done = false;
    while (!done) {
        done = (*p == '\0' || *p == '\n' || p-&path[0] >= BuffSize);
        if (*p == ':' || *p == '\n' || *p == nil) {
            if (start != p) {
                int dirlen = p-start;
                strncpy(pathBuff, start, dirlen);
                if (*p != '/') {
                    pathBuff[dirlen] = '/';
                    dirlen++;
                }
                strcpy(&pathBuff[dirlen], name);
                FILE* f = fopen(pathBuff, "r");
                if (f != nil) {
                    found = true;
                    fclose(f);
                    break;
                } else {
                    start = p + 1;
                }
            }
        }
        p++;
    }
    if (!found) {
        return nil;
    } else {
        return &pathBuff[0];
    }
}

const char* StripPath (const char* fullPath) {
    for (register int length = strlen(fullPath) - 1; length > 0; length--) {
        if (fullPath[length] == '/') {
            return &fullPath[length + 1];
        }
    }
    return fullPath;
}

/*
 *  Strip tail of leading relative directories.
 *  Return number of directories to back up.
 *
 *  Assume no path has imbedded relative directory names (i.e. d1/../d2).
 */

static int StripRelative (char* &tail) {
    int upcount = 0;
    int strip = 0;
    do {
        if (strncmp(tail, "../", 3) == 0) {
            strip = 3;
            upcount++;
        } else if (strncmp(tail, "./", 2) == 0) {
            strip = 2;
        } else if (strncmp(tail, "/", 1) == 0) {       // multiple /'s
            strip = 1;
        } else if (strcmp(tail, "..") == 0) {          // trailing .. s
            strip = 2;
            upcount++;
        } else if (strcmp(tail, ".") == 0) {           // trailing .
            strip = 1;
        } else {
            strip = 0;
        }
        tail += strip;
    } while (strip > 0);
    return upcount;
}

/*
 *  Get and remove 'upcount' number of trailing directories from 'cwd'.
 *
 *  Assume all directories are of the form "/d1/d2/d3..."
 *  with no double slashes "//".
 */

static void GetPrefix (char* const cwd, int upcount) {
    if (getcwd(cwd, MAXPATHLEN) == 0) {
        Panic("failed in getcwd, errno = %d", errno);
    }

    while (upcount > 0) {
        char* lastSlash = strrchr(cwd, '/');
        if (lastSlash == NULL) {
            Fatal("current directory not that deep");
        }
        lastSlash[0] = '\0';
        upcount--;
    }

    /*
     *  Restore root directory if missing.
     */
    if (cwd[0] == '\0') {
        cwd[0] = '/';
        cwd[1] = '\0';
    }
}

/*
 *  Concatenate and return a copy of head and tail in a dynamic string.
 */

static char* MergePath(const char* head, const char* tail) {
    char* fullpath;
    int taillen = strlen(tail);
    if (taillen == 0) {
        fullpath = new char[strlen(head) + 1];
        strcpy(fullpath, head);
    } else {
        fullpath = new char[strlen(head) + 1 + strlen(tail) + 1];
        sprintf(fullpath, "%s/%s", head, tail);
    }
    return fullpath;
}

/*
 *  Return full path (relative to current directory) for path tail.
 *
 *  The user is responsible for deleting the returned pathname.
 */

char* FullPath (char* const tail) {
    char* fullpath;

    /*
     *  Done if is absolute pathname already.
     */
    if (tail[0] == '/') {
        fullpath = new char[strlen(tail) + 1];
        strcpy(fullpath, tail);
        return fullpath;
    }

    char* tailp = tail;
    int upcount = StripRelative(tailp);
    char head[MAXPATHLEN];
    GetPrefix(head, upcount);
    char* fullpath =  MergePath(head, tailp);
    return fullpath;
}

