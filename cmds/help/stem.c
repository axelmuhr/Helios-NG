/*------------------------------------------------------------------------
--                                                                      --
--                 H E L I O S   H E L P   S Y S T E M                  --
--                 -----------------------------------                  --
--                                                                      --
--             Copyright (C) 1992, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- stem.c:          							--
--		find the root stem of the given word                    --
--									--
--	Author:  Martin Porter (1988)					--
--	Translated to C:  Martyn Tovey (1992)				--
--                                                                      --
------------------------------------------------------------------------*/
#ifdef __TRAN
static char *rcsid = "$Id: stem.c,v 1.3 1994/05/12 11:40:10 nickc Exp $";
#endif

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

#define FALSE 0
#define TRUE 1

static u_char *term;			/* copy of term to be stemmed */
static int term_length = 1;		/* length of term */
static int term_ptr = 1;		/* current working position */

#ifdef __HELIOS
static int consonant(int i);
static int count_con_seq(void);
static int vowelinstem(void);
static int doubleconsonant(int j);
static int con_vowel_con(int i);
static int ends(char *s);
static void setto(char *s);
static void replace(char *s);
static void step1(void);
static void step2(void);
static void step3(void);
static void step4(void);
static void step5(void);
static void step6(void);
static void move_buffer(int n, u_char *p, int c, u_char *q, int d);
u_char *stem(u_char *wordptr);
#else
static int consonant();
static int count_con_seq();
static int vowelinstem();
static int doubleconsonant();
static int con_vowel_con();
static int ends();
static void setto();
static void replace();
static void step1();
static void step2();
static void step3();
static void step4();
static void step5();
static void step6();
static void move_buffer();
u_char *stem();
#endif

/* consonant(i) is TRUE if term[i] is a consonant. */

static int consonant(i)
int i;
{
    u_char ch = term[i-1];

    if(ch == 'a' || ch == 'e' || ch == 'i' || ch == 'o' || ch == 'u')
	return FALSE;
    if(ch == 'y')
	return( i == 0 ? TRUE : !consonant(i-1));
    return TRUE;
}

/* count_con_seq() measures the number of consonant sequences in
   term[1..term_ptr] if c is a consonant sequence and v a vowel sequence,
   and <..> indicates arbitrary presence,

      <c><v>       gives 0
      <c>vc<v>     gives 1
      <c>vcvc<v>   gives 2
      <c>vcvcvc<v> gives 3
      ....
*/

static int count_con_seq()
{
    int n = 0;
    int i = 1;

    while(1)
    {
	if(i > term_ptr)
	    return n;
        if(! consonant(i))
	    break;
        i++;
    }

    i++;

    while(1)
    {
	while(1)
        {
	    if(i > term_ptr)
	        return n;
            if(consonant(i))
		break;
            i++;
        }

        i++;
        n++;

	while(1)
        {
	    if(i > term_ptr)
		return n;
            if(!consonant(i))
		break;
            i++;
        }

        i++;
    }
}

/* vowelinstem() is TRUE if term[0..term_ptr] contains a vowel */

static int vowelinstem()
{
   int i;
   for(i = 1; i <= term_ptr; i++)
	if(! consonant(i))
	    return TRUE;

    return FALSE;
}

/* doubleconsonant(term_ptr) is TRUE if term_ptr,term_ptr-1 contain
   a double consonant. */

static int doubleconsonant(j)
int j;
{
    return((j < 2 || term[j-1] != term[j-2]) ? FALSE : consonant(j));
}

/* con_vowel_con(i) is TRUE if i-2,i-1,i has the form
   consonant - vowel - consonant and also if the second c is not w,x or y.
   This is used when trying to resore an e at the end of a short word. e.g.

      cav(e), lov(e), hop(e), crim(e), but
      snow, box, tray.
*/

static int con_vowel_con(i)
int i;
{
    u_char ch = term[i-1];

    if(i < 3 || !consonant(i) || consonant(i-1) || !consonant(i-2))
	return FALSE;

    return ((ch == 'w' || ch == 'x' || ch == 'y') ? FALSE : TRUE);
}

/* ends(s) is TRUE if term[1..term_length] ends with the string s. */

static int ends(s)
char *s;
{
    int length = strlen(s);

    term_ptr = term_length;

    if(length > term_length)
	return FALSE;

    if(strncmp((char *)s, (char *)&term[term_length-length], length))
	return FALSE;

    term_ptr = term_length-length;
    return TRUE;
}

/* setto(s) sets term[term_ptr+1..term_length] to the characters in the
   string s, readjusting term_length. */

static void setto(s)
char *s;
{
    int length = strlen(s);

    move_buffer(length,(u_char *)s,1,term,term_ptr+1);

    term_length  = term_ptr+length;
}

/* replace(s) is used below. */

static void replace(s)
char *s;
{
    if(count_con_seq() > 0)
	setto(s);
}

/* step1() gets rid of plurals and -ed or -ing. e.g.

       caresses  ->  caress
       ponies    ->  poni
       ties      ->  ti
       caress    ->  caress
       cats      ->  cat

       feed      ->  feed
       agreed    ->  agree
       disabled  ->  disable

       matting   ->  mat
       mating    ->  mate
       meeting   ->  meet
       milling   ->  mill
       messing   ->  mess

       meetings  ->  meet

*/

static void step1()
{
    if(term[term_length-1] == 's')
	{
    	if(ends("sses"))
	    term_length = term_length-2;
    	else if(ends("ies"))
	    setto("i");
    	else if(term[term_length-1-1] != 's')
	    term_length--;
	}

    if(ends("eed"))
	{
	if(count_con_seq() > 0)
	    term_length--;
	}
    else if((ends("ed") || ends("ing")) && vowelinstem()) 
	{
    	term_length = term_ptr;
       	if(ends("at"))
	    setto("ate");
       	else if( ends("bl"))
	    setto("ble");
       	else if( ends("iz"))
	    setto("ize");
       	else if(doubleconsonant(term_length))
       	    {
	    u_char ch;

	    term_length--;
       	    ch = term[term_length-1];
            if(ch == 'l' || ch == 's' || ch == 'z')
		term_length++;
	    else if(!consonant(1))
		term_length++;
       	    }
       	else if(count_con_seq() == 1 && con_vowel_con(term_length))
	    setto("e");
	}
}

/* step2() turns terminal y to i when there is another vowel in the stem */

static void step2()
{
    if(ends("y") && vowelinstem())
	term[term_length-1] = 'i';
}


/* step3() maps double suffices to single ones. so -ization = -ize plus -ation
   maps to -ize etc. note that the string before the suffix must give
   count_con_seq() > 0. */

static void step3()
{
    switch(term[term_length-2])
	{
    	case 'a':
	    if(ends("ational"))
		{
		replace("ate");
		break;
		}
            if(ends("tional"))
		{
		replace("tion");
		break;
		}
            break;

    	case 'c':
	    if(ends("enci"))
		{
		replace("ence");
		break;
		}
            if(ends("anci"))
		{
		replace("ance");
		break;
		}
            break;

    	case 'e':
	    if(ends("izer"))
		{
		replace("ize");
		break;
		}
            break;

    	case 'l':
	    if(ends("bli"))
		{
		replace("ble");
		break;
		}
            if(ends("alli"))
		{
		replace("al");
		break;
		}
            if(ends("entli"))
		{
		replace("ent");
		break;
		}
            if(ends("eli"))
		{
		replace("e");
		break;
		}
            if(ends("ousli"))
		{
		replace("ous");
		break;
		}
            break;

    	case 'o':
	    if(ends("ization"))
		{
		replace("ize");
		break;
		}
            if(ends("ation"))
		{
		replace("ate");
		break;
		}
            if(ends("ator"))
		{
		replace("ate");
		break;
		}
            break;

    	case 's':
	    if(ends("alism"))
		{
		replace("al");
		break;
		}
            if(ends("iveness"))
		{
		replace("ive");
		break;
		}
            if(ends("fulness"))
		{
		replace("ful");
		break;
		}
            if(ends("ousness"))
		{
		replace("ous");
		break;
		}
            break;

    	case 't':
	    if(ends("aliti"))
		{
		replace("al");
		break;
		}
            if(ends("iviti"))
		{
		replace("ive");
		break;
		}
            if(ends("biliti"))
		{
		replace("ble");
		break;
		}
            break;

    	case 'g':
	    if(ends("logi"))
		{
		replace("log");
		break;
		}
	}
}

/* step4() deals with -ic-, -full, -ness etc. similar strategy to step3. */

static void step4()
{
    switch ( term[term_length-1] )
	{
    	case 'e':
	    if(ends("icate"))
		{
		replace("ic");
		break;
		}
            if(ends("ative"))
		{
		replace("");
		break;
		}
            if(ends("alize"))
		{
		replace("al");
		break;
		}
            break;

    	case 'i':
	    if(ends("iciti"))
		{
		replace("ic");
		break;
		}
            break;

    	case 'l':
	    if(ends("ical"))
		{
		replace("ic");
		break;
		}
            if(ends("ful"))
		{
		replace("");
		break;
		}
            break;

    	case 's':
	    if(ends("ness"))
		{
		replace("");
		break;
		}
            break;
	}
}

/* step5() takes off -ant, -ence etc., in context <c>vcvc<v>. */

static void step5()
{
    switch(term[term_length-1-1])
	{
	case 'a':
	    if(ends("al"))
		break;
	    return;

        case 'c':
	    if(ends("ance"))
		break;
	    if(ends("ence"))
		break;
	    return;

        case 'e':
	    if(ends("er"))
		break;
	    return;

        case 'i':
	    if(ends("ic"))
		break;
	    return;

        case 'l':
	    if(ends("able"))
		break;
	    if(ends("ible"))
		break;
	    return;
                 
        case 'n':
	    if(ends("ant"))
		break;
	    if(ends("ement"))
		break;
	    if(ends("ment"))
		break;
	    if(ends("ent"))
		break;
	    return;
                 
        case 'o':
	    if(ends("ion")&&(term[term_ptr-1]=='s' || term[term_ptr-1]=='t'))
		break;
            if(ends("ou"))
		break;
	    return;
                 
        case 's':
	    if(ends("ism"))
		break;
	    return;
                 
        case 't':
	    if(ends("ate"))
		break;
	    if(ends("iti"))
		break;
	    return;
                 
        case 'u':
	    if(ends("ous"))
		break;
	    return;
                 
        case 'v':
	    if(ends("ive"))
		break;
	    return;
                 
        case 'z':
	    if(ends("ize"))
		break;
	    return;

        default:
	    return;
	}

    if(count_con_seq() > 1)
	term_length = term_ptr;
}

/* step6() removes a final -e if count_con_seq() > 1. */

static void step6()
{
    term_ptr = term_length;
    if(term[term_length-1] == 'e')
	{
	int a = count_con_seq();
	
        if(a > 1 || a == 1 && !con_vowel_con(term_length-1))
	    term_length--;
	}
    if(term[term_length-1] == 'l' && doubleconsonant(term_length)
		&& count_con_seq() > 1)
	term_length--;
}

u_char *stem(wordptr)
u_char *wordptr;
{
    static u_char buff[512];

    strcpy((char *)buff, (char *)wordptr);
    term = buff;
    term_length = strlen((char *)term);
    if(term_length == 1)
	return(buff);
    step1();
    step2();
    step3();
    step4();
    step5();
    step6();
    buff[term_length] = 0;
    return buff;
}

static void move_buffer(n,p,c,q,d)
int n,c,d;
u_char *p, *q;
{
	u_char *buff = (u_char *) malloc(n);

	bcopy((char *) &p[c-1], (char *) buff, n);
	bcopy((char *) buff, (char *) &q[d-1], n);
	free(buff);
}
