/*
 * strchr --- search a string for a character
 *
 * We supply this routine for those systems that aren't standard yet.
 */

char *
strchr (str, c)
register char *str, c;
{
	for (; *str; str++)
		if (*str == c)
			return str;

	return NULL;
}

/*
 * strrchr --- find the last occurrence of a character in a string
 *
 * We supply this routine for those systems that aren't standard yet.
 */

char *
strrchr (str, c)
register char *str, c;
{
	register char *save = NULL;

	for (; *str; str++)
		if (*str == c)
			save = str;

	return save;
}
