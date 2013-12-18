
#ifndef __EXTERNS_H__
#define __EXTERNS_H__	1

#include "defs.h"

#include <stdlib.h>
#include <string.h>

/* sysbuild2.c */

extern void	sysbuild_fatal (char *, ...);
extern void	sysbuild_error (char *, ...);
extern void	sysbuild_warning (char *, ...);
extern void	sysbuild_debug (char *, ...);
extern void	sysbuild_info (char *, ...);

/* configdata.c */
extern struct config_str	ConfigData;

extern void	print_configdata (void);
extern void	init_configdata (void);
extern void	update_configdata (void);

/* parsecfg.c */

extern int	Info;

extern FILE *	ConfigFp;
extern void	parse_config_file (char []);

extern parse_ptr	get_data (char []);
extern parse_ptr *	get_multi_data (char []);

extern void	delete_elem (parse_ptr);
extern void	delete_list (void);

/* make_nucleus.c */

extern word 	swap (word);

extern void	add_module (char *);

extern FILE *	ModFp;
extern void	make_nucleus (void);

/* image.c */
extern ubyte *	Image;
extern ubyte *	ImagePtr;
extern word *	VectorPtr;
extern int	ModuleNumber;
extern int	ImageSize;

extern void	init_image (void);
extern void	file_to_image (FILE *, int, char *);
extern void	char_to_image (ubyte);
extern void	chars_to_image (ubyte *, int);
extern void	align_image (void);
extern void	patch_vector (ubyte *);
extern void	patch_space (int);
extern void	do_patch (ubyte *, ubyte *, int);

/* util.c */
extern void	capitalise (char *);
extern int	getfilesize (char *);

/* romdisk.c */

extern FILE *	RomDiskFp;
extern FILE *	RomInFp;

extern void	add_romdisk (void);

/* output_image.c */

extern int	ImageMax;
extern FILE *	NucOutFp;

extern void	output_image (void);

/* bootcheck.c */

extern FILE *	BootStrapFp;

extern void	check_bootstrap (void);

/* byte_order.c */
extern int	check_byte_order (char *);

#endif /* __EXTERNS_H__ */
