
/*
 * configdata.c
 *
 *	Functions to handle initialisation and updating of the ConfigData
 * structure which basically holds all the information concerning the
 * final image file to be created.
 */

#include <stdio.h>
#include <string.h>

#include "defs.h"
#include "externs.h"

#define bit_(n)	(1 << ((n) - 1))

#define set_loaded_bit_(n)	(ConfigData.loaded |= (n))
#define check_loaded_bit_(n)	(ConfigData.loaded & (n))

/*
 *   Dependency tree - some specific modules depend on the presence of other
 * modules in the nucleus.
 */

/* module key values */

#define NO_MOD		0
#define MOD_KERNEL	0x1
#define MOD_SYSLIB	0x2
#define MOD_SERVLIB	0x4
#define MOD_UTILLIB	0x8
#define MOD_BOOTUTIL	0x10
#define MOD_PROCMAN	0x20
#define MOD_LOADER	0x40
#define MOD_CLIB	0x80
#define MOD_POSIXLIB	0x100

#define NUMBER_OF_MODS	9

int NumberOfMods = NUMBER_OF_MODS;

/*
 * Lookup table for names of modules indexed by their key value defined above
 */

struct mod_lookup_str
{
	int	mod_key;

	long	mod_deps;

	char	mod_name[32];
};

struct mod_lookup_str ModData[] =
{
	{MOD_KERNEL,	NO_MOD,					 "kernel"},
	{MOD_SYSLIB,	MOD_KERNEL | MOD_UTILLIB,		 "system lib"},
	{MOD_SERVLIB,	MOD_KERNEL | MOD_SYSLIB | MOD_UTILLIB,	 "server lib"},
	{MOD_UTILLIB, 	MOD_KERNEL | MOD_SYSLIB,		 "utility lib"},
	{MOD_BOOTUTIL,	MOD_KERNEL,				 "proc. booter"},
	{MOD_PROCMAN,	MOD_SYSLIB | MOD_SERVLIB | MOD_UTILLIB,	 "procman"},
	{MOD_LOADER,	MOD_PROCMAN,				 "loader"},
	{MOD_POSIXLIB,	MOD_KERNEL | MOD_SYSLIB | MOD_UTILLIB,	 "POSIX lib"},
	{MOD_CLIB,	MOD_SYSLIB | MOD_UTILLIB | MOD_POSIXLIB, "C lib"}
};

int	IgnoreModule = 0;	/* used in check_dependencies () to ignore a 
				 * module not normally found in a nucleus for a
				 * particular processor (eg boot util for an ARM)
				 */

struct config_str	ConfigData;

/*
 *		CONFIG DATA INFORMATION FUNCTIONS
 */

void pad (int	n)
{
	while (n--)	putchar (' ');
}

void print_configdata ()
{
	int	i;

	int	output = FALSE;

	int	mod_key = 1;

	if (!Info)	return;

	printf ("\t\tConfiguration Data:\n\n");

	printf ("Nucleus File: %s\n\n", ConfigData.nucleus);

	printf ("Processor: %s\n\n", ConfigData.processor);

	printf ("Nucleus Directory: %s\n\n", ConfigData.nucleus_dir);

	for (i = 0; i < NumberOfMods && ConfigData.modules[i] != NULL; i++, mod_key  <<= 1)
	{
		if (i == 0)
		{
			/* 8 - 12 - ... */
			printf ("Slot:"); pad (3);
			printf ("Name:"); pad (15);
			printf ("Module:");
			putchar ('\n');
		}

		printf (" %d", i + 1);
		pad ((i > 9) ? 6 : 7);

		if (check_loaded_bit_(mod_key))
		{
			printf ("%s", ModData[i].mod_name);
			pad (20 - strlen (ModData[i].mod_name));
		}
		else
		{
			pad (20);
		}

		printf ("%s\n", ConfigData.modules[i]);

		output = TRUE;
	}

	for ( ; i < MODULE_SLOTS && ConfigData.modules[i] != NULL; i++)
	{
		printf (" %d", i + 1);
		pad ((i + 1 > 9) ? 6 : 7);

		pad (20);

		printf ("%s\n", ConfigData.modules[i]);

		output = TRUE;
	}

	if (output)
	{
		putchar ('\n');
		output = FALSE;
	}

	if (ConfigData.romdisk_dir[0] != '\0')
	{
		printf ("Romdisk Directory : %s\n", ConfigData.romdisk_dir);

		output = TRUE;
	}
       
	if (ConfigData.romdisk_file[0] != '\0')
	{
		printf ("Romdisk File      : %s\n", ConfigData.romdisk_file);

		output = TRUE;
	}

	if (   ConfigData.romdisk_file[0] != '\0'
	    || ConfigData.romdisk_dir[0] != '\0')
	{
		printf ("Romdisk Root      : /%s\n", ConfigData.romdisk_root);
		printf ("Romdisk Write File: %s\n", (ConfigData.romdisk_write == 'Y') ? "yes" : "no");

		output = TRUE;
	}

	if (output)
	{
		putchar ('\n');
		output = FALSE;
	}

	printf ("Bootstrap         : %s\n", ConfigData.bootstrap);
	printf ("Bootstrap Separate: %s\n\n", (ConfigData.bootstrap_separate) ? "yes" : "no");

	printf ("Image  Size       : %ld (0x%lx)\n", ConfigData.memory_size, ConfigData.memory_size);

	printf ("Vector Table Size : %d (plus NULL)\n", ConfigData.vector_table_size - 1);
	printf ("First Program     : %d\n", ConfigData.first_program);
	printf ("Swap Bytes        : %s\n\n", ((ConfigData.swap_bytes) ? "yes" : "no"));
}

/*
 *		CONFIG UTILITY FUNCTIONS
 */

/*
 * File names may be given with a "~", which must be replaced by the string in
 * ConfigData.nucleus_dir.
*/
void replace_tilda (char old_name[NAME_LEN_MAX])
{
	char	new_name[NAME_LEN_MAX];

	int	old_pos = 0;
	int	new_pos = 0;

	int	nucleus_dir_len = strlen (ConfigData.nucleus_dir);

	new_name[0] = '\0';

	while (old_pos < NAME_LEN_MAX)
	{
		if (old_name[old_pos] == '\0')
		{
			break;
		}
		else if (old_name[old_pos] == '~')
		{
			/* replace with nucleus_dir */
			strcat (new_name, ConfigData.nucleus_dir);

			new_pos += nucleus_dir_len;
			old_pos++;
		}
		else
		{
			new_name[new_pos++] = old_name[old_pos++];

			new_name[new_pos] = '\0';
		}
	}
	if (old_pos == NAME_LEN_MAX)
	{
		sysbuild_fatal ("Name %s too long after nucleus dir replacement", old_name);
	}

 	strcpy (old_name, new_name);
}

void replace_module_tildas (parse_ptr *	module_ptrs)
{
	int	i;

	for (i = 0; module_ptrs[i] != NULL; i++)
	{
		replace_tilda (string_val_(module_ptrs[i]));
	}
}

/*
 *		SETUP CONFIGURATION DATA
 */

void init_configdata ()
{
	int	i;

	strcpy (ConfigData.config_file, DEFAULT_CONFIG_FILE);

	strcpy (ConfigData.nucleus, DEFAULT_NUCLEUS_FILE);

	strcpy (ConfigData.nucleus_dir, DEFAULT_NUCLEUS_DIR);

	ConfigData.loaded = 0;

	for (i = 0; i < MODULE_SLOTS; i++)
	{
		ConfigData.modules[i] = NULL;
	}

	ConfigData.romdisk_file[0] = '\0';
	ConfigData.romdisk_dir[0] = '\0';
	ConfigData.romdisk_root[0] = '\0';

	ConfigData.memory_size = MEMSIZE;

	ConfigData.vector_table_size = 0;	/* Start at 1 to allow for
						   terminating NULL	   */

	ConfigData.first_program = -1;
	ConfigData.swap_bytes = 0;
}

/*
 *		UPDATE CONFIGURATION DATA
 */

/*
 * check_dependencies ():
 * 	Checks that required modules are in the kernel for a given module.
 */
void check_dependencies (long	mod_key)
{
	long	reqd_modules = 0;
	long	found_modules;
	long	missing_modules;

	char *	mod_name = NULL;

	int	i;

	if (mod_key & IgnoreModule)
	{
		return;
	}

	/* find reqd_modules from depend_str array */
	for (i = 0; i < NumberOfMods; i++)
	{
		if (ModData[i].mod_key == mod_key)
		{
			reqd_modules = ModData[i].mod_deps;

			break;
		}
	}
	
	if (i == NumberOfMods)
	{
		sysbuild_fatal ("Unknown module key 0x%lx", mod_key);

		return;
	}

	/* now check against what has been loaded */
	found_modules = ConfigData.loaded & reqd_modules;

	if (found_modules == reqd_modules)
	{
		/* everything is okay */
		return;
	}

	/* find the missing modules */
	missing_modules = reqd_modules - found_modules;

	/* find the name of this module */
	for (i = 0; i < NumberOfMods; i++)
	{
		if (ModData[i].mod_key == mod_key)
		{
			mod_name = ModData[i].mod_name;

			break;
		}
	}

	/* now warn about missing modules */
	for (i = 0; i < NumberOfMods; i++)
	{
		if (ModData[i].mod_key & missing_modules)
		{
			sysbuild_warning ("%s specified without %s\n", mod_name, ModData[i].mod_name);
		}
	}
}

/*
 * sort_by_slots ():
 *	Move all elements with a slot number not -1 to the top,
 * and then sort them.
 */
void sort_by_slots (parse_ptr *	module_ptrs)
{
	parse_ptr	tmp;

	int	not_sorted;
	int	i = 0;

	if (module_ptrs[0] == NULL)
	{
		/* Nothing to sort */
		return;
	}

	not_sorted = TRUE;
	while (not_sorted)
	{
		if (module_ptrs[i + 1] == NULL)
		{
			/* the end */
			not_sorted = FALSE;
		}
		else if (  module_slot_(module_ptrs[i]) == -1
			&& module_slot_(module_ptrs[i + 1]) != -1)
		{
			/* swap */
			tmp = module_ptrs[i];
			module_ptrs[i] = module_ptrs[i + 1];
			module_ptrs[i + 1] = tmp;

			/* start again */
			i = 0;

			not_sorted = TRUE;
		}
		else
		{
			i++;
		}
	}

	/* sort the positive pointers */
	not_sorted = TRUE;
	i = 0;
	while (not_sorted)
	{
		if (module_ptrs[i + 1] == NULL)
		{
			not_sorted = FALSE;
		}
		else if (module_slot_(module_ptrs[i + 1]) == -1)
		{
			not_sorted = FALSE;
		}
		else if (module_slot_(module_ptrs[i]) > module_slot_(module_ptrs[i + 1]))
		{
			/* swap */
			tmp = module_ptrs[i];
			module_ptrs[i] = module_ptrs[i + 1];
			module_ptrs[i + 1] = tmp;

			/* start again */
			i = 0;
		}
		else if (module_slot_(module_ptrs[i]) == module_slot_(module_ptrs[i + 1]))
		{
			/* error */
			sysbuild_error ("Found identical slot number %d for %s and %s",
					module_slot_(module_ptrs[i]),
					module_name_(module_ptrs[i]),
					module_name_(module_ptrs[i + 1]));

			sysbuild_warning ("%s will be ignored", module_name_(module_ptrs[i]));

			i++;	/* continue sorting if errors ignored */
		}
		else
		{
			i++;
		}
	}
}

/*
 * update_modules_data ():
 *	First place all modules with a specified slot, then fill
 * in the gaps with modules with no specified slot.  Finally
 * check for any remaining gaps, raising a fatal error if there
 * are any.
 *
 * At this point the module_ptrs array should be sorted, with all
 * modules with non-specified slots at the end.
 */
void update_modules_data (parse_ptr *	module_ptrs)
{
	int	i;

	int	mod_index;

	int	last_spec_index = 0;	/* keep track of where the last
					   specified module is 		*/
	int	next_index;

	for (i = 0; module_ptrs[i] != NULL; i++)
	{
		if (module_slot_(module_ptrs[i]) == -1)
		{
			/* finished with specified slots */
			break;
		}

		/* slot count starts from 1, so subtract 1 for array index */
		mod_index = module_slot_(module_ptrs[i]) - 1;

		ConfigData.modules[mod_index] = (char *)(malloc (strlen (module_name_(module_ptrs[i])) + 1));

		strcpy (ConfigData.modules[mod_index], module_name_(module_ptrs[i]));

		last_spec_index = mod_index;
	}

	/* After running out of gaps, place remaining modules at next_index */
	next_index = last_spec_index + 1;

	/* Now fill in any gaps */
	for ( ; module_ptrs[i] != NULL; i++)
	{
		int	j;

		for (j = 0; j < last_spec_index; j++)
		{
			if (ConfigData.modules[j] == NULL)
			{
				/* found a gap */

				sysbuild_debug ("found a gap at slot %d, placing %s",
						j + 1,
						module_name_(module_ptrs[i]));

				ConfigData.modules[j] =
					(char *)(malloc (strlen (module_name_(module_ptrs[i])) + 1));

				strcpy (ConfigData.modules[j], module_name_(module_ptrs[i]));

				break;
			}
		}

		if (j == last_spec_index)
		{
			/* failed to find a gap - hence no gaps to find */

			last_spec_index = 0;	/* no point checking anymore */

			ConfigData.modules[next_index] =
				(char *)(malloc (strlen (module_name_(module_ptrs[i])) + 1));

			strcpy (ConfigData.modules[next_index], module_name_(module_ptrs[i]));
			next_index++;
		}
	}

	/* check for gaps, going backwards from last_spec_index */
	while (last_spec_index--)
	{
		if (ConfigData.modules[last_spec_index] == NULL)
		{
			sysbuild_error ("Found a gap at slot %d", last_spec_index + 1);

			/*
			 * If gap errors are ignored, pad the table with -1.
			 * The actual padding is done later in make_nucleus ()
			 */

			ConfigData.modules[last_spec_index] = (char *)(malloc (1));

			ConfigData.modules[last_spec_index][0] = '\0';
		}
	}

	/* set the vector table size */
	ConfigData.vector_table_size = i + 1;
}

/*
 * IVecXXX - the values for the various types of modules are taken from config.h
 */

#define IVecKernel	1
#define IVecSysLib	2
#define IVecServLib	3
#define IVecUtil	4
int	IVecBootStrap = 5;	/* Different if processor is ARM */
int	IVecProcMan   =	6;	/*	-------	" -------	 */
int	IVecServers   = 7;	/*	-------	" -------	 */
int	IVecPosix     = 8;	/*	-------	" -------	 */
int	IVecCLib      = 9;	/*	-------	" -------	 */

void set_ivecs (char *	processor)
{
	int	i;

	if (strequ_(processor, "ARM"))
	{
		IVecBootStrap 	= -1;
		IVecProcMan 	= 5;
		IVecServers	= 6;
		IVecPosix	= 7;
		IVecCLib	= 8;

		NumberOfMods--;

		IgnoreModule |= MOD_BOOTUTIL;

		/* move the module data up one step from entry 4 */
		for (i = 5; i < NumberOfMods; i++)
		{
			ModData[i - 1].mod_key	= ModData[i].mod_key;
			ModData[i - 1].mod_deps	= ModData[i].mod_deps;
			strcpy (ModData[i - 1].mod_name, ModData[i].mod_name);
		}
	}
}

/*
 * After parsing the file, we update the nucleus configuration data.
 * First check the slot numbers for the kernel, syslib, servlib, etc,
 * and set them to the required values.  At the same time, change their
 * types to T_MODULE.
 * Then call for all T_MODULEs and sort them in slot order, with default
 * slot numbers (-1) appearing at the end.
 * Check for gaps in the vector table, and then add to the modules array
 * in ConfigData.
 */
void update_configdata ()
{
	parse_ptr	cfg_data;
	parse_ptr *	cfg_arr_data;

	/*
	 * ConfigData.default_index will contain the last index number,
	 * and hence the vector table size.  However, this doesn't
	 * take into account the romdisk file if one is required.
	 */
	int	romdisk_reqd = FALSE;

	int	first_program = -1;

	int	no_romdisk_server = TRUE;

	long	mod_key;
	int	i;

	if ((cfg_data = get_data (T_PROCESSOR)) != NULL)
	{
		capitalise (string_val_(cfg_data));
		strcpy (ConfigData.processor, string_val_(cfg_data));
	}
	else
	{
		sysbuild_warning ("No processor given, assuming %s", DEFAULT_PROCESSOR);

		strcpy (ConfigData.processor, DEFAULT_PROCESSOR);
	}

	if ((cfg_data = get_data (T_NUCLEUS_DIR)) != NULL)
	{
		strcpy (ConfigData.nucleus_dir, string_val_(cfg_data));
	}

	/*
	 * Now we know the processor type, we can set up the various 
	 * IVec values for the slots.
	 */
	set_ivecs (ConfigData.processor);

	/* Next find the kernel.  */
	if ((cfg_data = get_data (T_KERNEL)) != NULL)
	{
		if (module_slot_(cfg_data) != -1 && module_slot_(cfg_data) != 1)
		{
			/* Invalid slot number for the kernel */
			sysbuild_fatal ("Invalid slot number for kernel");
		}
		module_slot_(cfg_data) = IVecKernel;
		strcpy (cfg_data -> keyword_name, T_MODULE);

		set_loaded_bit_(bit_(IVecKernel));
	}

	/* Now set up the other "normal" modules  */
	if ((cfg_data = get_data (T_SYSLIB)) != NULL)
	{
		module_slot_(cfg_data) = IVecSysLib;
		strcpy (cfg_data -> keyword_name, T_MODULE);

		set_loaded_bit_(bit_(IVecSysLib));
	}

	if ((cfg_data = get_data (T_SERVLIB)) != NULL)
	{
		module_slot_(cfg_data) = IVecServLib;
		strcpy (cfg_data -> keyword_name, T_MODULE);

		set_loaded_bit_(bit_(IVecServLib));
	}

	if ((cfg_data = get_data (T_UTILLIB)) != NULL)
	{
		module_slot_(cfg_data) = IVecUtil;
		strcpy (cfg_data -> keyword_name, T_MODULE);

		set_loaded_bit_(bit_(IVecUtil));
	}

	if (IVecBootStrap != -1)
	{
		if ((cfg_data = get_data (T_BOOT)) != NULL)
		{
			module_slot_(cfg_data) = IVecBootStrap;
			strcpy (cfg_data -> keyword_name, T_MODULE);

			set_loaded_bit_(bit_(IVecBootStrap));
		}
	}

	if ((cfg_data = get_data (T_PROCMAN)) != NULL)
	{
		module_slot_(cfg_data) = IVecProcMan;
		strcpy (cfg_data -> keyword_name, T_MODULE);

		set_loaded_bit_(bit_(IVecProcMan));

		first_program = IVecProcMan;

		sysbuild_debug ("first_program = %d", first_program);
	}

	if ((cfg_data = get_data (T_LOADER)) != NULL)
	{
		module_slot_(cfg_data) = IVecServers;
		strcpy (cfg_data -> keyword_name, T_MODULE);

		set_loaded_bit_(bit_(IVecServers));
	}

	if ((cfg_data = get_data (T_POSIXLIB)) != NULL)
	{
		module_slot_(cfg_data) = IVecPosix;
		strcpy (cfg_data -> keyword_name, T_MODULE);

		set_loaded_bit_(bit_(IVecPosix));
	}

	if ((cfg_data = get_data (T_CLIB)) != NULL)
	{
		module_slot_(cfg_data) = IVecCLib;
		strcpy (cfg_data -> keyword_name, T_MODULE);

		set_loaded_bit_(bit_(IVecCLib));
	}

	/* Add in rom disk server */
	if ((cfg_data = get_data (T_ROMSERV)) != NULL)
	{
		strcpy (cfg_data -> keyword_name, T_MODULE);

		no_romdisk_server = FALSE;
	}

	/* A rom disk file may be required with or without an obvious server for it */

	if ((cfg_data = get_data (T_ROMDISK_DIR)) != NULL)
	{
		if (no_romdisk_server)
		{
			sysbuild_warning ("ROM disk directory specified without ROM disk server");
		}
		replace_tilda (string_val_(cfg_data));

		strcpy (ConfigData.romdisk_dir, string_val_(cfg_data));

		if (get_data (T_ROMDISK_WRITE) != NULL)
		{
			ConfigData.romdisk_write = 'Y';

			/* find the name of the file to be written to */
			if ((cfg_data = get_data (T_ROMDISK_FILE)) != NULL)
			{
				replace_tilda (string_val_(cfg_data));

				strcpy (ConfigData.romdisk_file, string_val_(cfg_data));
			}
			else
			{
				strcpy (ConfigData.romdisk_file, DEFAULT_ROMDISK_FILE);
			}
		}
		else
		{
				ConfigData.romdisk_write = 'N';
		}
			
		if (( cfg_data = get_data (T_ROMDISK_ROOT)) != NULL)
		{
			char *	romdsk_rt = string_val_(cfg_data);

			/*
			 * romdisk_root may be specified with a leading "/" which
			 * will need to be stripped off.
			 */
			if (romdsk_rt[0] == '/')
			{
				romdsk_rt++;
			}
			strcpy (ConfigData.romdisk_root, romdsk_rt);
		}
		else
		{
			strcpy (ConfigData.romdisk_root, DEFAULT_ROMDISK_ROOT);
		}

		romdisk_reqd = TRUE;
	}
	else if ((cfg_data = get_data (T_ROMDISK_FILE)) != NULL)
	{
		if (no_romdisk_server)
		{
			sysbuild_warning ("ROM disk file specified without ROM disk server");
		}
		replace_tilda (string_val_(cfg_data));

		strcpy (ConfigData.romdisk_file, string_val_(cfg_data));

		romdisk_reqd = TRUE;
	}
	else if (!no_romdisk_server)
	{
		sysbuild_warning ("ROM disk server specified without associated ROM disk directory or file");
	}

	/* Check the dependencies of all loaded files */
	if (!check_loaded_bit_(MOD_KERNEL))
	{
		sysbuild_error ("No kernel specified");
	}
	for (mod_key = 1, i = 0; i < NumberOfMods; i++)
	{
		if (check_loaded_bit_(mod_key))
		{
			check_dependencies (mod_key);
		}

		mod_key = mod_key << 1;
	}

	/* Find out any extra files */
	if ((cfg_arr_data = get_multi_data (T_MODULE)) != NULL)
	{
		/*
		 * By now all specific modules have been converted to
		 * T_MODULEs and their slot numbers patched.
		 */

		/*
		 * replace all '~''s in the module names with nucleus_dir.
		 */
		replace_module_tildas (cfg_arr_data);

		/*
		 * sort_by_slots () also checks that no slot
		 * numbers are identical
		 */
		sort_by_slots (cfg_arr_data);

		/*
		 * update_modules_data () also checks that
		 * there are no gaps in the vector table.
		 */
		update_modules_data (cfg_arr_data);
	}

	/* Find bootstrap file */
	if ((cfg_data = get_data (T_BOOTSTRAP)) != NULL)
	{
		replace_tilda (string_val_(cfg_data));

		strcpy (ConfigData.bootstrap, string_val_(cfg_data));
	}

	if (strequ_(ConfigData.processor, "ARM") || get_data (T_BOOTSTRAP_SEPARATE) != NULL)
	{
		ConfigData.bootstrap_separate = 1;
	}
	else
	{
		ConfigData.bootstrap_separate = 0;
	}

	/* Finally set up miscellaneous config data */

	if ((cfg_data = get_data (T_IMAGE_SIZE)) != NULL)
	{
		ConfigData.memory_size = int_val_(cfg_data);
	}

	if ((cfg_data = get_data (T_FIRST_PROGRAM)) != NULL)
	{
		if (first_program != -1 && first_program != int_val_(cfg_data))
		{
			sysbuild_warning ("Overriding process manager as first program");
		}

		first_program = int_val_(cfg_data);
	}

	ConfigData.swap_bytes = check_byte_order (ConfigData.processor);

	if (romdisk_reqd)
	{
		ConfigData.vector_table_size++;
	}

	ConfigData.first_program = first_program;
}
