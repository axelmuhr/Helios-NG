/* SccsId : @(#)hash.c   1.3   1.3 Copyright (C) Perihelion Software Ltd.   */
INT hash(s)
char *s;
{
        char *p;
        UWORD h = 0, g;

        for( p = s ; *p != 0 ; p++ )
        {
                h = (h << 4) + *p;
                if( (g = (h & 0xf0000000)) != 0 )
                {
                        h = h ^ (g >> 24);
                        h = h ^ g;
                }
        }
        return (UWORD)(h % HASHSIZE);
}
