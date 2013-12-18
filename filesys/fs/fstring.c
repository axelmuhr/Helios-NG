
                                                                                /*
  []-------------------------------------------------------------------------[]
   |                                                                         |
   |                    (c) 1991 by parsytec GmbH, Aachen                    |
   |                          All rights reserved.                           |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |                               Utitlities                                |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  fstring.c                                                              |
   |                                                                         |
   |    String handling                                                      |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  History:                                                               |
   |    1 - O.Imbusch - 19 March 1991 - Basic version                        |
   |                                                                         |
  []-------------------------------------------------------------------------[]
                                                                                */

#ifndef TEST
#define TEST 0
#endif

#include <helios.h>
#include <string.h>

#define DEBUG    0
#define GEPDEBUG 0
#define FLDEBUG  1

#if 0
#include "error.h"
#endif
#include "fstring.h"

char *CutLast (char *String,
               int   NoOfChars)
{
  int   SL;
  char *NewEnd;

  if (NoOfChars <= 0)
    return (String);
  elif (NoOfChars > (SL = strlen (String)))
    return (NULL);
  else
  {
    *(NewEnd = String + SL - NoOfChars) = '\0';
    return (String);
  }
}	

char *CutLeading (char *String,
                  char *What)
{
  char *NewStart = String;

  while ((*NewStart != '\0') && (strchr (What, *NewStart) != NULL))
    ++NewStart;

  return (strcpy (String, NewStart));
}

char *CutAppending (char *String,
                    char *What)
{
  int LastCharPos = strlen (String) - 1;
  
  if (LastCharPos >= 0)
    while (strchr (What, String [LastCharPos]) != NULL)
      String [LastCharPos--] = '\0';
    
  return (String);
}

char *PolyStr (char *Dest,
               char *Base,
               int   NOfCopies)
{
  *Dest = '\0';

  while (NOfCopies-- > 0)
    strcat (Dest, Base);

  return (Dest);
}

char *Replace (char *Source,
               char  Old,
               char  New)
{
  char *CP = Source;

  while (*CP != '\0')
  {
    if (*CP == Old)
      *CP = New;
    ++CP;
  }

  return (Source);
}

char *UpCase (char *Source)
{
  char *CP = Source;

  while (*CP != '\0')
  {
    if ((*CP >= 'a') && (*CP <= 'z'))
      *CP -= ('a' - 'A');
#if 0
    elif (*CP == 0x04)
      *CP = 0x0e;
    elif (*CP == 0x14)
      *CP = 0x19;
    elif (*CP == 0x01)
      *CP = 0x1a;
#endif
    ++CP;
  }

  return (Source);
}

char *LowCase (char *Source)
{
  char *CP = Source;

  while (*CP != '\0')
  {
    if ((*CP >= 'A') && (*CP <= 'Z'))
      *CP += ('a' - 'A');
#if 0
    elif (*CP == 0x0e)
      *CP = 0x04;
    elif (*CP == 0x19)
      *CP = 0x14;
    elif (*CP == 0x1a)
      *CP = 0x01;
#endif
    ++CP;
  }

  return (Source);
}

char *strrstr (char *S1,
               char *S2)
{
  char *OldOccur = NULL,
       *Occur    = S1;
  int   S2Len    = strlen (S2);       

  if ((strlen (S1) > 0) && (S2Len > 0))
  {
    while ((Occur = strstr (Occur, S2)) != NULL)
    {  
      OldOccur = Occur;
      Occur += S2Len;
    }    
  }

  return (OldOccur);
}

#if TEST

#include <stdio.h>

int main (void)
{
  char Buff [30];	

#define TestStr "abcABC..abcABC"
  
  printf ("strlen (%s) = %d\n\n", TestStr, strlen (TestStr));
  
#define TestCutLast(S, N) printf ("CutLast (\"%s\", %3d) = \"%s\"\n", S, N, CutLast (S, N));

  TestCutLast ((char*) NULL, 1);
  TestCutLast ("",           1);
  TestCutLast (TestStr,     -1);
  TestCutLast (TestStr,      0);
  TestCutLast (TestStr,      1);
  TestCutLast (TestStr,      strlen (TestStr) / 2);
  TestCutLast (TestStr,      strlen (TestStr) - 1);
  TestCutLast (TestStr,      strlen (TestStr));
  TestCutLast (TestStr,      strlen (TestStr) + 1);
  TestCutLast (TestStr,      strlen (TestStr) + 2);
  printf ("\n");

#define TestCutLeading(S, W) printf ("CutLeading (\"%s\", \"%s\") = \"%s\"\n", S, W, CutLeading (S, W));

  TestCutLeading (TestStr, (char *) NULL);
  TestCutLeading (TestStr, "");
  TestCutLeading (TestStr, "a");
  TestCutLeading (TestStr, "b");
  TestCutLeading (TestStr, "abcABC");
  TestCutLeading (TestStr, TestStr);
  printf ("\n");
  
#define TestCutAppending(S, W) printf ("CutAppending (\"%s\", \"%s\") = \"%s\"\n", S, W, CutAppending (S, W));

  TestCutAppending (TestStr, (char *) NULL);
  TestCutAppending (TestStr, "");
  TestCutAppending (TestStr, "C");
  TestCutAppending (TestStr, "B");
  TestCutAppending (TestStr, "abcABC");
  TestCutAppending (TestStr, TestStr);
  printf ("\n");
  
#define TestPolyStr(D,B,N) printf ("PolyStr (Buff, \"%s\", %3d) = \"%s\"\n", B, N, PolyStr (D, B, N));

  TestPolyStr (Buff, (char *) NULL, 3);
  TestPolyStr (Buff, "",            3);
  TestPolyStr (Buff, "a",           3);
  TestPolyStr (Buff, "abc",         3);
  printf ("\n");

#define TestReplace(S,O,N) printf ("Replace (\"%s\", \'%c\', \'%c\') = \"%s\"\n", S, O, N, Replace (S, O, N));

  TestReplace ((char *) NULL, 'a',  'b'); 
  TestReplace ("",            'a',  'b'); 
  TestReplace (TestStr,       'a',  'b'); 
  printf ("Replace (\"%s\", %c, \'\\0\') = \"%s\"\n", TestStr, 'b', Replace (TestStr, 'b', '\0'));
  printf ("Replace (\"%s\", \'\\0\', %c) = \"%s\"\n", TestStr, 'b', Replace (TestStr, '\0', 'b'));
  printf ("\n");

#define TestUpCase(S) printf ("UpCase (\"%s\") = \"%s\"\n", S, UpCase (S));

  TestUpCase ((char *) NULL);	
  TestUpCase ("");	
  TestUpCase (TestStr);	
  printf ("\n");

#define TestLowCase(S) printf ("LowCase (\"%s\") = \"%s\"\n", S, LowCase (S));

  TestLowCase ((char *) NULL);	
  TestLowCase ("");	
  TestLowCase (TestStr);	
  printf ("\n");

#define Teststrstr(S1, S2) printf ("strstr (\"%s\", \"%s\") = \"%s\"\n", S1, S2, strstr (S1, S2));

  Teststrstr ((char *) NULL, (char *) NULL);
  Teststrstr ("",            (char *) NULL);
  Teststrstr ((char *) NULL, "");
  Teststrstr ("",            "");
  Teststrstr ((char *) NULL, "abc");
  Teststrstr ("",            "abc");
  Teststrstr ("abc",         (char *) NULL);
  Teststrstr ("abc",         "");
  Teststrstr (TestStr,       "abc");
  Teststrstr (TestStr,       "z");
  printf ("\n");

#define Teststrrstr(S1, S2) printf ("strrstr (\"%s\", \"%s\") = \"%s\"\n", S1, S2, strrstr (S1, S2));

  Teststrrstr ((char *) NULL, (char *) NULL);
  Teststrrstr ("",            (char *) NULL);
  Teststrrstr ((char *) NULL, "");
  Teststrrstr ("",            "");
  Teststrrstr ("abc",         (char *) NULL);
  Teststrrstr ("abc",         "");
  Teststrrstr (TestStr,       "abc");
  Teststrrstr (TestStr,       "z");
  printf ("\n");

}

#endif

/*******************************************************************************
**
**  fstring.c
**
*******************************************************************************/
