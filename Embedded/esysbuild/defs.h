#ifndef __DEFS_H__
#define __DEFS_H__	1

/*
 * Definitions for the Helios (ROM) system builder
 *
 * Copyright (c) 1994 Perihelion Distributed Software Ltd.
 *
 * RCS Id: $Id: defs.h,v 1.8 1994/05/16 10:00:36 nickc Exp $
 */

/*{{{  Constants */

#ifndef TRUE
# define TRUE	1
# define FALSE	0
#endif

/* How to recognize a ROM disk - defined in module.h in releases later than 1.3.1 */
#ifndef T_RomDisk
# define	T_RomDisk	0x60fb60fbl
#endif

#if defined(IBMPC) || defined(__HELIOS)
# define MEMSIZE 50000L		/* 50k max nucleus size */
#else
# define MEMSIZE 300000L	/* 300k max nucleus */
#endif

#define DEFAULT_NUCLEUS_FILE	"nucleus"
#define	DEFAULT_CONFIG_FILE	"nucleus.con"
#define DEFAULT_ROMDISK_ROOT	"helios"
#define DEFAULT_ROMDISK_FILE 	"romdisk.fil"
#define DEFAULT_PROCESSOR	"ARM"
#define DEFAULT_NUCLEUS_DIR	"."

#define NAME_LEN_MAX		256
#define KEYWORD_MAX		32

#define SYSBUILD_OLD		2
#define SYSBUILD_OK		1
#define SYSBUILD_FAIL		0

#define SYSBUILDERR_BADOPT	1
#define SYSBUILDERR_FAIL	2


/*
 *		KEYWORDS
 */

#define T_PROCESSOR	"processor"

#define T_NUCLEUS_DIR	"nucleus_dir"

#define T_KERNEL	"kernel"
#define T_SYSLIB	"syslib"
#define T_SERVLIB	"servlib"
#define T_UTILLIB	"utillib"
#define T_BOOT		"boot"
#define T_PROCMAN	"procman"
#define T_LOADER	"loader"
#define T_CLIB		"Clib"
#define T_POSIXLIB	"POSIXlib"

#define T_ROMSERV	"romdisk_server"
#define T_ROMDISK_FILE	"romdisk_file"
#define T_ROMDISK_DIR	"romdisk_directory"
#define T_ROMDISK_ROOT	"romdisk_root"
#define T_ROMDISK_WRITE "romdisk_writefile" /* write a separate romdisk file */

#define T_BOOTSTRAP		"rombootstrap"
#define T_BOOTSTRAP_SEPARATE	"bootstrap_separate"

#define T_MODULE	"module"

#define T_IMAGE_SIZE	"image_size"

#define T_FIRST_PROGRAM	"first_program"

#ifdef JUST_TESTING
#define T_SWAP_BYTES	"swap_bytes"
#endif

/*}}}*/
/*{{{  Macros */

#define strequ_(s1,s2)		(strcmp (s1, s2) == 0)
#define strnequ_(s1,s2,n)	(strncmp (s1, s2, n) == 0)

/*}}}*/
/*{{{  Types */

#ifndef WORD
typedef long		word;
typedef char *		string;
typedef unsigned char	ubyte;
#endif

/*
 *		STRUCTURE DEFINITIONS
 */

typedef struct
  {
    char	module_name[ NAME_LEN_MAX ];
    int		module_slot;
  }
module_data;

/*
 * struct parse_str:
 *	Used to hold information retrieved from the config file.
 */
struct parse_str
  {
    struct parse_str *	next;

    char		keyword_name[KEYWORD_MAX];
    
    int			keyword_def;
    
    union
      {
	char		string_val[NAME_LEN_MAX];
	int		int_val;
	char		char_val;
	module_data	module_val;
      }
    value;

    int			value_type;	/* codes defined below */
};

#define NO_VAL		0
#define STRING_VAL	1
#define INT_VAL		2
#define CHAR_VAL	3
#define MODULE_VAL	4

#define string_val_(p)	((p -> value).string_val)
#define int_val_(p)	((p -> value).int_val)
#define char_val_(p)	((p -> value).char_val)
#define module_val_(p)	((p -> value).module_val)
#define module_name_(p)	(module_val_(p).module_name)
#define module_slot_(p)	(module_val_(p).module_slot)

typedef struct parse_str	parse_elem;
typedef struct parse_str *	parse_ptr;


/*
 * struct module_files_str:
 *	Used to set up a linked list of files to be included in the nucleus.
 */

struct module_files_str
  {
    struct module_files_str *	next;

    char	modfile_name[NAME_LEN_MAX];
  };

typedef struct module_files_str *	module_files_ptr;


/*
 * config_str:
 *	Holds nucleus configuration information.
 */

#define MODULE_SLOTS	64
struct config_str
  {	
    char	config_file[NAME_LEN_MAX];

    char	processor[8];	/* TRAN, C40, ARM */

	/* nucleus file name */
    char	nucleus[NAME_LEN_MAX];

    char	nucleus_dir[NAME_LEN_MAX];	/* used to replace "~" in file names */

	/*
	 * Array of modules in the nucleus -
	 *
	 *	1	kernel
	 *	2	syslib
	 *	3	servlib
	 *	4	utillib
	 *	5	remote booter
	 *	6	procman
	 *	7	loader
	 *	8	C library
	 *	9	POSIX library
	 */
	
	/*
	 * For information only, we set the corresponding bit in
	 * loaded to 1 if the above modules are explicitly defined
	 */
    long	loaded;

    char *	modules[MODULE_SLOTS];

	/* rom server and rom disk information */

    char	romdisk_file[NAME_LEN_MAX];
    char	romdisk_dir[NAME_LEN_MAX];
    char	romdisk_root[NAME_LEN_MAX];
    char	romdisk_write;

	/* bootstrap binary file */
    char	bootstrap[NAME_LEN_MAX];
    int		bootstrap_separate;	/* don't concatenate bootstrap and nucleus */

	/* other changeable bits and pieces */

    long	memory_size;

	/* calculated bits */
    int		vector_table_size;

    int		first_program;
    int		swap_bytes;	/* Set if the boot strap file is a different
				   sex to the host machine */
};

/*}}}*/

#endif /* __DEFS_H__ */
