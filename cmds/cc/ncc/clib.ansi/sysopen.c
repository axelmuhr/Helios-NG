#include <sys/file.h>

FILEHANDLE _sys_open(const char *filename, int openmode)
{   /* nasty magic number interface for openmode */
    static int modtab[6] = { /* r = */  O_RDONLY,
			     /* r+ = */ O_RDWR,
                             /* w = */  O_WRONLY|O_CREAT|O_TRUNC,
			     /* w+ = */ O_RDWR  |O_TRUNC|O_CREAT,
                             /* a = */  O_WRONLY|O_CREAT,
			     /* a+ = */ O_RDWR  |O_CREAT|O_APPEND };
    char *name = (char *)filename;            /* yuk */
    FILEHANDLE fh;
    openmode = modtab[openmode >> 1];         /* forget the 'b'inary bit */
    fh = _syscall3(SYS_open, (int)name, openmode, 0666);
    if (fh<=0) return NONHANDLE;            /* not found             */
    return fh;
    }
}
