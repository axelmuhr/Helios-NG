
/*
 *		BOOTSTRAP FILE CHECKING FUNCTIONS
 */

#include <stdio.h>

#include "defs.h"
#include "externs.h"

#define CONFIG_STRING	"Bootstrap Config Structure"

FILE *	BootStrapFp;

int move_to_configstruct (FILE *	fp,
			  char *	search_string)
{
	int	c;
	int	s = 0;

	while ((c = getc (fp)) != EOF)
	{
		if (c == search_string[s])
		{
			s++;
		}
		else
		{
			s = 0;
		}

		if (search_string[s] == '\0')
		{
			/* align file pointer to longword boundary */
			while (ftell (fp) % 4)
				c = getc (fp);

			return TRUE;
		}
	}

	return FALSE;
}

int move_to_first_program (FILE *	fp)
{
	/*
	 * Skip over 5 words (20 bytes), and the next word should
	 * be the value for the first program.
	 */
	int	i;

	for (i = 0; i < 20; i++)
	{
		if (getc (fp) == EOF)	return FALSE;
	}

	return TRUE;
}

void check_bootstrap ()
{
	word	first_program;

	if (!strcmp( ConfigData.processor, "ARM" ))	/* ARM has no bootstrap */
	{
		return;
	}
	
	sysbuild_info ("Checking bootstrap file");

	if (ConfigData.bootstrap[0] == '\0' || ConfigData.first_program == -1)
	{
		/* No Bootstrap file to check */
	  
		sysbuild_warning ("Unable to check bootstrap file");

		return;
	}

	if ((BootStrapFp = fopen (ConfigData.bootstrap, "rb")) == NULL)
	{
		sysbuild_warning ("Failed to open bootstrap file %s for checking",
					ConfigData.bootstrap);

		return;
	}

	if (move_to_configstruct (BootStrapFp, CONFIG_STRING) == FALSE)
	{
		sysbuild_warning ("Failed to find config structure in %s",
					ConfigData.bootstrap);

		fclose (BootStrapFp);
		BootStrapFp = NULL;

		return;
	}

	if (move_to_first_program (BootStrapFp) == FALSE)
	{
		sysbuild_warning ("Premature EOF in bootstrap file %s",
					ConfigData.bootstrap);

		fclose (BootStrapFp);
		BootStrapFp = NULL;

		return;
	}

	if (fread (&first_program, 1, sizeof (word), BootStrapFp) != sizeof (word))
	{
		sysbuild_warning ("Failed to read data for first program");

		fclose (BootStrapFp);
		BootStrapFp = NULL;

		return;
	}

	sysbuild_debug ("\tFirst program in bootstrap = 0x%08lx", first_program);

	if (ConfigData.swap_bytes)
	{
		first_program = swap (first_program);
	}

	if (first_program != ConfigData.first_program)
	{
		sysbuild_warning ("Invalid first program in bootstrap file");
	}
	else
	{
		sysbuild_info ("\tValid first program in bootstrap file");
	}

	fclose (BootStrapFp);
	BootStrapFp = NULL;
}
