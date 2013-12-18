/*
 * IOCTL call values for the Inmos S502 device driver
 * 
 *
 * COPYRIGHT 1988 INMOS Limited.
 *
 */

struct B014_SETF {
    unsigned int op:16;   
    unsigned int val:16;
};

struct B014_READF {
    unsigned int reserved:28;
    unsigned int read_f:1;
    unsigned int write_f:1;
    unsigned int timeout_f:1;
    unsigned int error_f:1;
};

union B014_IO {
    struct B014_SETF set;
    struct B014_READF status;
};

#define RESET			(1)
#define ANALYSE			(2)
#define SETTIMEOUT		(3)
#define SETERRORSIGNAL		(4)
#define RESETERRORSIGNAL	(5)

/*
 * _IOR and _IOW encode the read/write instructions to the kernel within the
 * ioctl command code.
 */

#if (__GNUC__ > 0)
#define READFLAGS	_IOR('k', 0, union B014_IO)
#define SETFLAGS	_IOW('k', 1, union B014_IO)
#else
#define READFLAGS	_IOR(k, 0, union B014_IO)
#define SETFLAGS	_IOW(k, 1, union B014_IO)
#endif


