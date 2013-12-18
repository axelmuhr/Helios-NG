
/*
 *		PARSE CONFIG FILE FUNCTIONS
 */

#include <stdio.h>

#include "defs.h"
#include "externs.h"

#ifndef NULL
# define NULL	0
#endif

#ifndef TRUE
# define TRUE	1
# define FALSE	0
#endif

/*
 * The basic format of the configuration file is
 *
 *	<keyword> '=' <value>
 */

parse_ptr	ParseHead = NULL;
parse_ptr*	ParseTail = &ParseHead;

FILE *	ConfigFp = NULL;	/* set to config_fp, exported for tidyup () */

struct keyword_read_data
{
	char 	keyword_name[KEYWORD_MAX];

	int	value_type;

	int	opt_arg;	/* TRUE => optional integer argument */
};

struct keyword_read_data KeywordData[] =
{
	{T_PROCESSOR, STRING_VAL, FALSE},

	{T_NUCLEUS_DIR, STRING_VAL, FALSE},

	{T_KERNEL, MODULE_VAL, FALSE},
	{T_SYSLIB, MODULE_VAL, FALSE},
	{T_SERVLIB, MODULE_VAL, FALSE},
	{T_UTILLIB, MODULE_VAL, FALSE},
	{T_BOOT, MODULE_VAL, FALSE},
	{T_PROCMAN, MODULE_VAL, FALSE},
	{T_LOADER, MODULE_VAL, FALSE},
	{T_CLIB, MODULE_VAL, FALSE},
	{T_POSIXLIB, MODULE_VAL, FALSE},

	{T_ROMSERV, MODULE_VAL, TRUE},
	{T_ROMDISK_FILE, STRING_VAL, FALSE},
	{T_ROMDISK_DIR, STRING_VAL, FALSE},
	{T_ROMDISK_ROOT, STRING_VAL, FALSE},
	{T_ROMDISK_WRITE, NO_VAL, FALSE},

	{T_MODULE, MODULE_VAL, TRUE},

	{T_BOOTSTRAP, STRING_VAL, FALSE},
	{T_BOOTSTRAP_SEPARATE, NO_VAL, FALSE},

	{T_IMAGE_SIZE, INT_VAL, FALSE},

	{T_FIRST_PROGRAM, INT_VAL, FALSE},
#ifdef JUST_TESTING
	{T_SWAP_BYTES, NO_VAL, FALSE},
#endif

	{"", -1}
};

int	Config_LineNumber = 1;

#define is_whitespace_(c)	((c) == ' ' || (c) == '\t' || (c) == '\n')

#define is_validchar_(c)	(  ((c) >= 'A' && (c) <= 'Z') \
				|| ((c) >= 'a' && (c) <= 'z') \
				|| ((c) >= '0' && (c) <= '9') \
				|| ((c) == '_'))

#define is_digit_(c)	((c) >= '0' && (c) <= '9')

#define is_hexdigit_(c)	(  ((c) >= '0' && (c) <= '9') \
			|| ((c) >= 'A' && (c) <= 'F') \
			|| ((c) >= 'a' && (c) <= 'f'))

int ignore_whitespace (FILE *	config_fp)
{
	int	c = getc (config_fp);

	while (is_whitespace_(c))
		c = getc (config_fp);

	return c;
}

void nextline (FILE *	config_fp)
{
	int	c = getc (config_fp);

	while (c != '\n' && c != EOF)
		c = getc (config_fp);
}

int read_keyword (FILE *	config_fp,
		  char		keyword[KEYWORD_MAX])
{
	int	c;

	while ((c = ignore_whitespace (config_fp)) != EOF)
	{
		if (c == '#')
		{
			/* found a comment, so ignore it */
			nextline (config_fp);
		}
		else if (is_validchar_(c))
		{
			int	i;

			keyword[0] = c;

			c = getc (config_fp);

			for (i = 1; is_validchar_(c) ; i++)
			{
				keyword[i] = c;

				c = getc (config_fp);
			}
			keyword[i] = '\0';

			ungetc (c, config_fp);

			return TRUE;
		}
		else
		{
			sysbuild_warning ("Unknown character %c", c);
		}
	}

	return FALSE;
}

int read_equal (FILE *	config_fp)
{
	int	c;

	if ((c = ignore_whitespace (config_fp)) != '=')
	{
		sysbuild_warning ("Failed to find expected '='");

		return FALSE;
	}

	return TRUE;
}

int read_string (FILE *	config_fp,
		 char	str[NAME_LEN_MAX])
{
	int	c;
	int	i;

	if ((c = ignore_whitespace (config_fp)) != '"')
	{
		sysbuild_warning ("Failed to find starting quote (\")");

		return FALSE;
	}

	c = getc (config_fp);

	for (i = 0; c != '"' && c != EOF; i++)
	{
		str[i] = c;

		c = getc (config_fp);
	}
	str[i] = '\0';

	if (c == EOF)
	{
		sysbuild_warning ("Unexpected end of file");

		return FALSE;
	}
	
	return TRUE;
}

int read_hex (FILE *	config_fp,
	      int *	value)
{
	int	c = getc (config_fp);

	if (!is_hexdigit_(c))
	{
		sysbuild_warning ("Missing hex number");

		return FALSE;
	}

	while (is_hexdigit_(c))
	{
		if (c >= '0' && c <= '9')
		{
			*value = (*value * 16) + (c - '0');
		}
		else if (c >= 'A' && c <= 'F')
		{
			*value = (*value * 16) + (c - 'A' + 10);
		}
		else
		{
			*value = (*value * 16) + (c - 'a' + 10);
		}

		c = getc (config_fp);
	}

	/* return non hex digit char */
	ungetc (c, config_fp);

	return TRUE;
}

int read_dec (FILE *	config_fp,
	      int *	value)
{
	int	c = getc (config_fp);

	while (is_digit_(c))
	{
		*value = (*value * 10) + (c - '0');
	
		c = getc (config_fp);
	}

	/* return non digit char */
	ungetc (c, config_fp);

	return TRUE;
}

int read_int (FILE *	config_fp,
	      int *	value)
{
	int	c1, c2, c3;

	/*
	 * Problems, problems ...
	 *
	 * There are several possibilites to consider -
	 *
	 * 1	What follows is not a digit => error
	 * 2	What follows is a decimal number
	 * 3 	What follows is a hex number (0x...)
	 * 4	What follows is a decimal number followed by an 'x'
	 *
	 */

	*value = 0;

	c1 = ignore_whitespace (config_fp);

	if (!is_digit_(c1))
	{
		sysbuild_warning ("Found %c instead of a digit", c1);

		return FALSE;
	}

	c2 = getc (config_fp);
	c3 = getc (config_fp);

	if (c1 == '0' && c2 == 'x')
	{
		if (is_hexdigit_(c3))
		{
			/* found hex digit */

			/* put c3 back and let read_hex () handle it */

			ungetc (c3, config_fp);

			return (read_hex (config_fp, value));
		}
		else
		{
			/* found a 0 ended by an 'x' */
			*value = 0;

			/* put back unused characters */
			ungetc (c3, config_fp);
			ungetc (c2, config_fp);

			return TRUE;
		}
	}
	else
	{
		/* found a 'long' decimal number */
		*value = (c1 - '0');

		/* put back c2 and c3 and let read_dec () handle it */

		ungetc (c3, config_fp);
		ungetc (c2, config_fp);

		return (read_dec (config_fp, value));
	}
}

int read_char (FILE *	config_fp,
	       int *	value)
{
	int	c;

	c = ignore_whitespace (config_fp);

	/* we allow any character (other than whitespace) to be valid */

	*value = c;

	return TRUE;
}

int read_opt_val (FILE *	config_fp,
		  int *		value)
{
	int	c;

	if ((c = ignore_whitespace (config_fp)) != '[')
	{
		/* No optional argument to read */
		ungetc (c, config_fp);

		return TRUE;
	}

	if (read_int (config_fp, value) != TRUE)
	{
		return FALSE;
	}

	if ((c = ignore_whitespace (config_fp)) != ']')
	{
		sysbuild_warning ("No closing bracket for optional argument");

		return FALSE;
	}

	return TRUE;
}

int is_validkeyword (char	keyword[])
{
	int	i;

	for (i = 0; KeywordData[i].value_type != -1; i++)
	{
		if (strequ_(keyword, KeywordData[i].keyword_name))
		{
			return i;
		}
	}

	return -1;
}

parse_ptr make_int_parse_str (char	keyword_name[KEYWORD_MAX],
			      int	int_val)
{
	parse_ptr	p;

	p = (parse_ptr)(malloc (sizeof (struct parse_str)));

	if (p == NULL)
	{
		sysbuild_error ("Out of memory");
	}

	p -> next = NULL;

	strcpy (p -> keyword_name, keyword_name);

	int_val_(p) = int_val;

	p -> value_type = INT_VAL;

	return p;
}

parse_ptr make_str_parse_str (char	keyword_name[KEYWORD_MAX],
			      char	str_val[NAME_LEN_MAX])
{
	parse_ptr	p;

	p = (parse_ptr)(malloc (sizeof (struct parse_str)));

	if (p == NULL)
	{
		sysbuild_error ("Out of memory");
	}

	p -> next = NULL;

	strcpy (p -> keyword_name, keyword_name);

	strcpy (string_val_(p), str_val);

	p -> value_type = STRING_VAL;

	return p;
}

parse_ptr make_mod_parse_str (char	keyword_name[KEYWORD_MAX],
			      char	mod_name[NAME_LEN_MAX],
			      int	mod_slot)
{
	parse_ptr	p;

	p = (parse_ptr)(malloc (sizeof (struct parse_str)));

	if (p == NULL)
	{
		sysbuild_error ("Out of memory");
	}

	p -> next = NULL;

	strcpy (p -> keyword_name, keyword_name);

	strcpy (module_name_(p), mod_name);
	module_slot_(p) = 	 mod_slot;

	p -> value_type = MODULE_VAL;

	return p;
}

parse_ptr make_char_parse_str (char	keyword_name[KEYWORD_MAX],
			      char	char_val)
{
	parse_ptr	p;

	p = (parse_ptr)(malloc (sizeof (struct parse_str)));

	if (p == NULL)
	{
		sysbuild_error ("Out of memory");
	}

	p -> next = NULL;

	strcpy (p -> keyword_name, keyword_name);

	char_val_(p) = char_val;

	p -> value_type = CHAR_VAL;

	return p;
}


parse_ptr make_null_parse_str (char	keyword_name[KEYWORD_MAX])
{
	parse_ptr	p;

	p = (parse_ptr)(malloc (sizeof (struct parse_str)));

	if (p == NULL)
	{
		sysbuild_error ("Out of memory");
	}

	p -> next = NULL;

	strcpy (p -> keyword_name, keyword_name);

	p -> value_type = NO_VAL;

	return p;
}

parse_ptr read_line (FILE *	config_fp)
{
	char	keyword[KEYWORD_MAX];

	int	key_index;

	int	val_type;
	char		str_val[NAME_LEN_MAX];
	int		int_val;

	int		opt_val;

	while (read_keyword (config_fp, keyword))
	{
		opt_val = -1;

		if ((key_index = is_validkeyword (keyword)) == -1)
		{
			/* bad keyword */
			sysbuild_warning ("Unknown keyword %s", keyword);

			nextline (config_fp);

			continue;
		}

		val_type = KeywordData[key_index].value_type;

		if (KeywordData[key_index].opt_arg == TRUE)
		{
			if (read_opt_val (config_fp, &opt_val) == FALSE)
			{
				nextline (config_fp);

				continue;
			}
		}

		if (val_type != NO_VAL)
		{
			if (read_equal (config_fp) == FALSE)
			{
				nextline (config_fp);

				continue;
			}
		}

		switch (val_type)
		{
		case MODULE_VAL:
			if (read_string (config_fp, str_val) == FALSE)
			{
				nextline (config_fp);

				continue;
			}

			return (make_mod_parse_str (keyword, str_val, opt_val));

		case STRING_VAL:
			if (read_string (config_fp, str_val) == FALSE)
			{
				nextline (config_fp);

				continue;
			}
			/* actually found valid line */
			return (make_str_parse_str (keyword, str_val));

		case INT_VAL:
			if (read_int (config_fp, &int_val) == FALSE)
			{
				nextline (config_fp);

				continue;
			}
			return (make_int_parse_str (keyword, int_val));

		case CHAR_VAL:
			if (read_char (config_fp, &int_val) == FALSE)
			{
				nextline (config_fp);

				continue;
			}

			return make_char_parse_str (keyword, int_val);

		case NO_VAL:

			return make_null_parse_str (keyword);

		default:
			sysbuild_error ("Internal inconsistency in argument type %d", val_type);

			break;
		}
	}

	return NULL;
}

#ifndef DEBUG
#define print_parse_lines(p)
#else
int print_parse_lines (parse_ptr	p)
{
	if (p == NULL)	return;

	if (p -> value_type == INT_VAL)
	{
		sysbuild_debug ("%lx: [%lx, %s, %d]",
				(long)p, (long)(p -> next),
				p -> keyword_name,
				(p -> value).int_val);
	}
	else if (p -> value_type == STRING_VAL)
	{
		sysbuild_debug ("%lx: [%lx, %s, %s]",
				(long)p, (long)(p -> next),
				p -> keyword_name,
				(p -> value).string_val);
	}
	else
	{
		sysbuild_debug ("%lx: [%lx, %s, ???]",
				(long)p, (long)(p -> next),
				p -> keyword_name);
	}

	print_parse_lines (p -> next);
}
#endif


void parse_config_file (char	config_file[])
{
	FILE *	config_fp;

	parse_ptr	parse_line;

	sysbuild_debug ("Config File: %s", config_file);

	/* lets go ... */
	if ((config_fp = fopen (config_file, "r")) == NULL)
	{
		/* whoops */
		sysbuild_error ("Cannot open config file %s", config_file);
	}

	ConfigFp = config_fp;

	/*
	 * Basic algorithm -
	 *
	 * 	while (keywords available)
	 *		find keyword
	 *		find value
	 *		add to list
	 * (simple uh?)
	 */

	while ((parse_line = read_line (config_fp)) != NULL)
	{
		*ParseTail = parse_line;
		ParseTail = &(parse_line -> next);
	}

	fclose (config_fp);
	ConfigFp = NULL;

	print_parse_lines (ParseHead);
}

/*
 *		EXPORTED FUNCTIONS TO MANIUPLATE THE LIST
 */

parse_ptr get_data (char 	keyword[KEYWORD_MAX])
{
	parse_ptr	p;

	for (p = ParseHead; p != NULL; p = p -> next)
	{
		if (strequ_(p -> keyword_name, keyword))
		{
			return p;
		}
	}

	return NULL;
}

/*
 * get_multi_data ():
 * 	Some keywords may appear in the list several times.
 *	Returns an array of pointers to the parse information.
 */

parse_ptr * get_multi_data (char	keyword[KEYWORD_MAX])
{
	parse_ptr	p;
	parse_ptr *	arr_p;

	int	n = 0;	/* number of matching elements in the list */

	/* find out how many matching elements there are */

	for (p = ParseHead; p != NULL; p = p -> next)
	{
		if (strequ_(p -> keyword_name, keyword))
		{
			n++;
		}
	}

	if (n == 0)	return NULL;

	n++;	/* add one for terminating NULL */

	/* malloc the required amount of memory */
	arr_p = (parse_ptr *)(malloc (n * sizeof (parse_ptr)));

	if (arr_p == NULL)	return NULL;

	/* fill in the elements of the array */
	for (n = 0, p = ParseHead; p != NULL; p = p -> next)
	{
		if (strequ_(p -> keyword_name, keyword))
		{
			arr_p[n++] = p;
		}
	}
	arr_p[n] = NULL;

	return arr_p;
}

void delete_elem (parse_ptr	del_p)
{
	parse_ptr	p, prev_p = NULL;

	/* find where del_p is in the list */

	for (p = ParseHead; p != NULL; p = p -> next)
	{
		if (p == del_p)
		{
			break;
		}

		prev_p = p;
	}

	if (p == NULL)
	{
		/* del_p not in list */
		return;
	}

	if (prev_p == NULL)
	{
		/* del_p is the head of the list */

		/* patch link */
		ParseHead = p -> next;

		free (p);

		return;
	}

	/* patch link */
	prev_p -> next = p -> next;

	free (p);
}

void delete_list ()
{
	parse_ptr	p, next_p;

	if (ParseHead == NULL)
	{
		return;
	}

	p = ParseHead;
	next_p = ParseHead -> next;

	while (p != NULL)
	{
		free (p);

		p = next_p;
		next_p = next_p -> next;
	}
}
