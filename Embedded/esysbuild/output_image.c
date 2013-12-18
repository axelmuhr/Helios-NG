
/*
 * output_image.c
 *
 *	The final image consists of the ROM bootstrap binary concatenated
 * onto the Helios nucleus, unless the bootstrap_separate option is
 * specified, in which case only the nucleus is output.
 */

#include <stdio.h>

#include "defs.h"
#include "externs.h"

FILE *	NucOutFp;

void output_nucleus ()
{
	ubyte *	p;

	sysbuild_debug ("output_nucleus () - ImageSize = %d,  Image = 0x%lx, ImagePtr = 0x%lx",
				ImageSize, (long)Image, (long)ImagePtr);

	for (p = Image; p < ImagePtr; p++)
	{
		putc (*p, NucOutFp);
	}
}

int output_bootstrap ()
{
	int	c;
	int	bootstrap_size;
	FILE *	boot_fp;

	if (ConfigData.bootstrap[0] == '\0')
	{
		if (!ConfigData.bootstrap_separate)
		{
			sysbuild_error ("No bootstrap file given, unable to create image");

			return SYSBUILD_FAIL;
		}
		else
		{
			return SYSBUILD_OK;
		}
	}

	bootstrap_size = getfilesize (ConfigData.bootstrap);

	if (bootstrap_size + ImageSize > ImageMax)
	{
		sysbuild_error ("Bootstrap and Nuclues exceed image size");

		return SYSBUILD_FAIL;
	}

	if ((boot_fp = fopen (ConfigData.bootstrap, "rb")) != NULL)
	{
		sysbuild_error ("Unable to open bootstrap file %s", ConfigData.bootstrap);

		return SYSBUILD_FAIL;
	}

	while ((c = getc (boot_fp)) != EOF)
	{
		putc (c, NucOutFp);
	}

	fclose (boot_fp);

	return SYSBUILD_OK;
}
	
void output_image ()
{
	if ((NucOutFp = fopen (ConfigData.nucleus, "wb")) == NULL)
	{
		sysbuild_error ("Unable to open nucleus file %s", ConfigData.nucleus);
	}

	if (output_bootstrap () == SYSBUILD_FAIL)
	{
		return;
	}

	output_nucleus ();

	fclose (NucOutFp);
	NucOutFp = NULL;
}




