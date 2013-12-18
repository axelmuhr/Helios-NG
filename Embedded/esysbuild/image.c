
/*
 *		IMAGE BUFFER MANIPULATION FUNCTIONS
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __HELIOS
#include <nonansi.h>
#include <syslib.h>
#endif

#include <module.h>

#include "defs.h"
#include "externs.h"

#define ImageNull	(ubyte)('\0')

ubyte *	Image = NULL;	/* internal image of the nucleus */
ubyte *	ImagePtr;	/* position to add next module   */
word *	VectorPtr;	/* pointer to the vector table	 */

int	ImageSize = 0;
int	ImageMax;

int	ModuleNumber = 1;	/* starts at 1 to get over size word */

#ifdef IBMPC
ubyte *	lmalloc ();
#define malloc lmalloc
#endif

void init_image ()
{
	Image = (ubyte *)(malloc ((int) ConfigData.memory_size));

	if (Image == NULL)
	{
		sysbuild_error ("Failed to initialise image buffer");
	}

	/*
	 *   Set iptr to the start of the modules.  Additional 4 is
	 * for the size of the nucleus at the beginning of the file.
	 */
	ImagePtr = Image + (4 + ConfigData.vector_table_size * 4);

	/* Set up word pointer to size and vector table */
	VectorPtr = (word *)(Image);

	/* Start image size count */
	ImageSize = ImagePtr - Image;
	ImageMax  = (int) ConfigData.memory_size;
}

void file_to_image (FILE *	in_fp,
		    int		n_bytes,
		    char *	module)
{
	int	cnt;

	if (ImageSize + n_bytes > ImageMax)
	{
		sysbuild_error ("Overrun Image Buffer (Size = 0x%08lx, Max = 0x%08lx)", ImageSize + n_bytes, ImageMax);
	}

	ImageSize += n_bytes;
		
#if defined (IBMPC) || defined (MWC)
	{
		word	s = n_bytes;
		ubyte *	b = ImagePtr;
		int	rd;

		while (s > 0)
		{
			int	tfr = (s > 30000L) ? 30000 : s;

			if ((rd = fread (b, 1, tfr, in_fp)) != tfr)
			{
				sysbuild_error ("Failed to read %s (expecting %d bytes, read %d bytes)",
							module, tfr, rd);
			}

			s = s - tfr;
			b = (ubyte *)((word)b + tfr);
		}
	}
#else
	if ((cnt = fread (ImagePtr, 1, n_bytes, in_fp)) != n_bytes)
	{
		sysbuild_error ("Failed to read %s (expecting %d bytes, read %d bytes)",
					module, n_bytes, cnt);
	}
#endif
	ImagePtr += n_bytes;
}

void char_to_image (ubyte	b)
{
	if (ImageSize == ImageMax)
	{
		sysbuild_error ("Overrun Image Buffer");
	}

	*ImagePtr++ = b;
	ImageSize++;
}

void chars_to_image (ubyte *	bytes,
		     int	n_bytes)
{
	if (ImageSize + n_bytes > ImageMax)
	{
		sysbuild_error ("Overrun Image Buffer");
	}

	ImageSize += n_bytes;

	while (n_bytes--)
	{
		*ImagePtr++ = *bytes++;
	}
}

void align_image ()
{
	while (ImageSize & 3)
	{
		char_to_image (ImageNull);
	}
}

void patch_vector (ubyte *	mod_start)
{
	word ip;
#ifdef IBMPC
	ip = (word)mod_start - ((word)Image + (4 * ModuleNumber));
#else
	ip = mod_start - Image - (4 * (word)ModuleNumber);
#endif

	VectorPtr[ModuleNumber++] = swap (ip);
}

void patch_space (int	n_bytes)
{
	if (ImageSize + n_bytes > ImageMax)
	{
		sysbuild_error ("Overrun Image Buffer");
	}

	while (n_bytes--)
	{
		*ImagePtr++ = ImageNull;
		ImageSize++;
	}
}

void do_patch (ubyte *	ptr,
	       ubyte *	bytes,
	       int	n_bytes)
{
	while (n_bytes--)
	{
		*ptr = *bytes;
		ptr++;
		bytes++;
	}
}
