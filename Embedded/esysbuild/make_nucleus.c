
/*
 *		NUCLEUS CREATION FUNCTIONS
 */

#include <stdio.h>
#include <module.h>

#include "defs.h"
#include "externs.h"

FILE *	ModFp = NULL;

#ifdef IBMPC
ubyte *	lmalloc ();
#endif

/* if not odd, swap () defined as a macro in defs.h */
word swap (word	x)
{
	if (ConfigData.swap_bytes)
	{
		word	r = 0;

		r |= ((x >> 24) & 0xff);
		r |= ((x >> 16) & 0xff) << 8;
		r |= ((x >> 8) & 0xff) << 16;
		r |= (x & 0xff) << 24;

		return r;
	}
	else
		return x;
}

void add_module (char *	module)
{
	word	mod_hdr[3];	/* ImageHdr really */

	int	cnt;

	word	mod_size;	/* size of module from header */

	ubyte *	mod_start = ImagePtr;	/* where the module starts */

	int	mod_offset = ImageSize;	/* used for info only */

	sysbuild_debug ("adding module %s", module);

	if (*module == '\0')
	{
		/*
		 * If we made it this far with an empty module,
		 * place a dummy pointer in the vector table.
		 */
		VectorPtr[ModuleNumber++] = -1;

		return;
	}

#if defined (SUN4) || defined (R140)
	if ((ModFp = fopen (module, "r")) == NULL)
#else
	if ((ModFp = fopen (module, "rb")) == NULL)
#endif
	{
		sysbuild_error ("Failed to open module %s\n", module);

		return;
	}

	if ((cnt = fread (mod_hdr, 1, 12, ModFp)) != 12)
	{
		sysbuild_error ("Failed to read module file (%s) header", module);

		fclose (ModFp);

		return;
	}

	mod_size = swap (mod_hdr[2]);	

	sysbuild_debug ("mod_size = %d (0x%lx), ImageSize = %d (0x%lx)",
				mod_size, mod_size, ImageSize, ImageSize);
	
	file_to_image (ModFp, (int) mod_size, module);

	sysbuild_info ("%s - ", module);
	sysbuild_info ("\t\toffset: 0x%08lx;\tsize: 0x%08lx", 
					mod_offset, mod_size);

	/* Align to next word boundary */
	align_image ();

	/* Patch vector table pointers */
	patch_vector (mod_start);

	fclose (ModFp);
	ModFp = NULL;
}

void make_nucleus ()
{
#ifdef OLD
	module_files_ptr	p;
#endif
	int	i;

	/* Initialise the image buffer, etc */
	init_image ();

	/* Add the files into the nucleus ... */

#ifdef OLD
	add_module (ConfigData.kernel);
	add_module (ConfigData.syslib);
	add_module (ConfigData.servlib);
	add_module (ConfigData.utillib);
	add_module (ConfigData.boot);
	add_module (ConfigData.procman);
	add_module (ConfigData.loader);

	/* add extra files */
	for (p = ConfigData.files_head; p != NULL; p = p -> next)
	{
		add_module (p -> modfile_name);
	}

	add_module (ConfigData.romserv);
#endif
	for (i = 0; i < MODULE_SLOTS && ConfigData.modules[i] != NULL; i++)
	{
		add_module (ConfigData.modules[i]);
	}

	add_romdisk ();

	/* Patch size and terminating NULL */
	VectorPtr[0] 		= swap (ImageSize);
	VectorPtr[ModuleNumber]	= 0L;
}
