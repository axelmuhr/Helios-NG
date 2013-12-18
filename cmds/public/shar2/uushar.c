#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

/* ENC is the basic 1 character encoding function to make a char printing */
#define ENC(c) ((((c) & 077) + ' ') | ((c & 077) == 0 ? 0100 : 0))

encode (in, out)
    FILE *in;
    FILE *out;
{
    char  buf[80];
    int  i, n;

    for (;;)
    {
    /* 1 (up to) 45 character line */
	n = fr (in, buf, 45);
	putc (ENC (n), out);

	for (i = 0; i < n; i += 3)
	    outdec (&buf[i], out);

	putc ('\n', out);
	if (n <= 0)
	    break;
    }
}

/*
 * output one group of 3 bytes, pointed at by p, on file f.
 */
outdec (p, f)
    char *p;
    FILE *f;
{
    int  c1, c2, c3, c4;

    c1 = *p >> 2;
    c2 = (*p << 4) & 060 | (p[1] >> 4) & 017;
    c3 = (p[1] << 2) & 074 | (p[2] >> 6) & 03;
    c4 = p[2] & 077;
    putc (ENC (c1), f);
    putc (ENC (c2), f);
    putc (ENC (c3), f);
    putc (ENC (c4), f);
}

/* fr: like read but stdio */
int
     fr (fp, buf, cnt)
    FILE *fp;
    char *buf;
    int  cnt;
{
    int  c, i;

    for (i = 0; i < cnt; i++)
    {
	c = getc (fp);
	if (c == EOF)
	    return (i);
	buf[i] = c;
    }
    return (cnt);
}
