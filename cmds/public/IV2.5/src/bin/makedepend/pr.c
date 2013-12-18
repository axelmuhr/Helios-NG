/*
 * $XConsortium: pr.c,v 1.6 88/09/30 08:55:13 jim Exp $
 */
#include "def.h"

extern struct	inclist	inclist[ MAXFILES ],
			*inclistp;
extern char	*objfile;
extern int	width;
extern boolean	printed;
extern boolean	verbose;
extern boolean	show_where_not;

add_include(file, file_red, include, dot)
	struct inclist	*file, *file_red;
	char	*include;
	boolean	dot;
{
	register struct inclist	*newfile;
	register struct filepointer	*content;

	/*
	 * First decide what the pathname of this include file really is.
	 */
	newfile = inc_path(file->i_file, include, dot);
	if (newfile == NULL) {
		if (file != file_red)
			log("%s (reading %s): ",
				file_red->i_file, file->i_file);
		else
			log("%s: ", file->i_file);
		log("cannot find include file \"%s\"\n", include);
		show_where_not = TRUE;
		newfile = inc_path(file->i_file, include, dot);
		show_where_not = FALSE;
	}

	included_by(file, newfile);
	if (!newfile->i_searched) {
		newfile->i_searched = TRUE;
		content = getfile(newfile->i_file);
		find_includes(content, newfile, file_red, 0);
		freefile(content);
	}
}

recursive_pr_include(head, file, base)
	register struct inclist	*head;
	register char	*file, *base;
{
	register int	i;

	if (head->i_marked)
		return;
	head->i_marked = TRUE;
	if (head->i_file != file)
		pr(head, file, base);
	for (i=0; i<head->i_listlen; i++)
		recursive_pr_include(head->i_list[ i ], file, base);
}

pr(ip, file, base)
	register struct inclist  *ip;
	char	*file, *base;
{
	static char	*lastfile;
	static int	current_len;
	register int	len, i;
	char	buf[ BUFSIZ ];

	printed = TRUE;
	len = strlen(ip->i_file)+1;
	if (current_len + len > width || file != lastfile) {
		lastfile = file;
		sprintf(buf, "\n%s%s: %s", base, objfile, ip->i_file);
		len = current_len = strlen(buf);
	}
	else {
		buf[0] = ' ';
		strcpy(buf+1, ip->i_file);
		current_len += len;
	}
	fwrite(buf, len, 1, stdout);

	/*
	 * If verbose is set, then print out what this file includes.
	 */
	if (! verbose || ip->i_list == NULL || ip->i_notified)
		return;
	ip->i_notified = TRUE;
	lastfile = NULL;
	printf("\n# %s includes:", ip->i_file);
	for (i=0; i<ip->i_listlen; i++)
		printf("\n#\t%s", ip->i_list[ i ]->i_incstring);
}

#if defined (mips) && defined (SYSTYPE_SYSV)
void catch()
{
	fflush(stdout);
	log_fatal("got signal\n");
}
#else /* not (mips && SYSTYPE_SYSV) */
#ifdef ultrix
void
#endif
catch(n)
{
	fflush(stdout);
	log_fatal("got signal %d\n", n);
}
#endif /* mips && SYSTYPE_SYSV */

