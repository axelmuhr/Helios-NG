

struct fileops
{
	int	(*fo_rw)();
	int	(*fo_ioctl)();
	int	(*fo_select)();
	int	(*fo_close)();
};

struct file
{
	int		f_type;
	int 		f_flag;
	int		f_count;
	void		*f_data;
	struct fileops	*f_ops;	
};

#define	DTYPE_SOCKET	1

#define	FREAD		1
#define	FWRITE		2

