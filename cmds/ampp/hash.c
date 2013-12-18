/* RcsId : $Id: hash.c,v 1.1 1990/09/26 19:02:38 paul Exp $ Copyright (C) Perihelion Software Ltd.	*/

INT hash(s)
char *s;
{
        char *p;
        UWORD h = 0, g;

        for( p = s ; *p != 0 ; p++ )
        {
                h = (h << 4) + *p;
                if( g = h & 0xf0000000 )
                {
                        h = h ^ (g >> 24);
                        h = h ^ g;
                }
        }
        return (UWORD)(h % HASHSIZE);
}
