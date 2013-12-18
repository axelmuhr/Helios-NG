/* stat.h: Posix file stat						*/
/* SccsId: %W% %G% (C) Copyright 1990, Perihelion Software Ltd.		*/
/* RcsId: $Id: stat.h,v 1.3 91/02/27 15:32:07 nick Exp $ */

#ifndef __stat_h
#define __stat_h

struct stat
{
	/* the following fields correspond exactly to a Helios	*/
	/* ObjInfo structure, DO NOT ALTER.			*/
	long	st_type;
	long	st_flags;
	long	st_matrix;
	char	st_name[32];
	uid_t	st_uid;		/* server's account id in fact */
	off_t	st_size;
	time_t	st_ctime;
	time_t	st_atime;
	time_t	st_mtime;

	/* the value of st_mode is made up from the values of	*/
	/* st_type and st_matrix.				*/
	mode_t	st_mode;

	/* the following fields must be present, but are all zeroed */
	ino_t	st_ino;
	dev_t	st_dev;
	dev_t	st_blksize;	/* st_rdev removed, this is BSD compatible */
	nlink_t	st_nlink;
	uid_t	st_gid;

};

#ifdef _BSD

/* POSIX have removed st_rdev from the stat structure, for BSD	*/
/* compatibility we define it on top of st_dev.			*/

#define st_rdev	st_dev

#endif

#define S_IRWXU		00000007
#define S_IRUSR 	00000001
#define S_IWUSR 	00000002
#define S_IXUSR 	00000004

#define S_IRWXG		00000070
#define S_IRGRP 	00000010
#define S_IWGRP 	00000020
#define S_IXGRP 	00000040

#ifndef _POSIX_SOURCE
/* Helios has 4 access groups */
#define S_IRWXY		00000700
#define S_IRYYY 	00000100
#define S_IWYYY 	00000200
#define S_IXYYY		00000400
#endif

#define S_IRWXO		00007000
#define S_IROTH 	00001000
#define S_IWOTH 	00002000
#define S_IXOTH 	00004000

#ifndef _POSIX_SOURCE
#ifdef _BSD
#define	S_IREAD		S_IRUSR
#define	S_IWRITE	S_IWUSR
#define	S_IEXEC		S_IXUSR
#endif
#define S_IFMT  	07770000
#define S_IFSOCK	00100000
#define S_IFCHR		00200000
#define S_IFBLK		00400000
#define S_IFLNK		01000000
#endif

#define S_IFDIR 	00010000
#define S_IFREG 	00020000
#define S_IFIFO 	00040000

/* setuid and setgid do not exist in Helios		*/
/* however we allocate some bits for compatibility	*/
#define S_ISUID		010000000
#define S_ISGID		020000000

#define S_ISDIR(m) (m&S_IFDIR)
#define S_ISCHR(m) (m&S_IFCHR)
#define S_ISBLK(m) (m&S_IFBLK)
#define S_ISREG(m) (m&S_IFREG)
#define S_ISFIFO(m) (m&S_IFIFO)

#ifndef _POSIX_SOURCE
#define S_ISLNK(m) (m&S_IFLNK)
#define S_ISSOCK(m) (m&S_IFSOCK)
#endif

extern int 	umask(mode_t cmask);
extern int 	mkdir(char *path, mode_t mode);
extern int 	mkfifo(char *path, mode_t mode);
extern int	stat(char *path, struct stat *buf);
extern int	fstat(int fd, struct stat *buf);
extern int 	chmod(char *path, mode_t mode);

#ifndef _POSIX_SOURCE
extern int	lstat(char *path, struct stat *buf);
extern int	readlink(char *path, char *buf, int bufsiz);
#endif

#endif

/* end of sys/stat.h */
