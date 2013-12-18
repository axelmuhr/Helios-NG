/*
 * C++ interface to OS file system functions
 */

#ifndef os_fs_h
#define os_fs_h

extern int open(const char*, int, int=0644);
extern int creat(const char*, int);
extern int read(int, void*, unsigned);
extern int write(int, const void*, unsigned);
extern int close(int);
extern long lseek(int, long, int);
extern int truncate(char* path, int length);
extern int ftruncate(int fd, int length);

extern int link(const char*, const char*);
extern int unlink(const char*);

extern int dup(int);
extern int dup2(int, int);
extern int pipe(int*);

extern int ioctl(int, int ...);

extern int chmod(const char*, int);
extern int umask(int);
extern int chown(const char*, int, int);
extern int chroot(const char*);

extern int chdir(const char*);
extern char* getwd(char*);
extern char* getcwd(char*, int);
extern int mkdir(const char*, int);
extern int mknod(const char*, int, int);
extern int mount(const char*, const char*, int, int);
extern int umount(const char*);

extern int access(const char*, int);

extern void sync();

#endif
