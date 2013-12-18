/*... Eight queens problem.
//... Eight queens are to placed on a chess board in such a way
//... that no queen checks against any other queen.
//...
//... Algorithms + Data Structures = Programs
//... Nicklaus Wirth
//... Printice-Hall, N.J.,   1976.
//... Chapter 3
//...
//... Bath Univ Lib code  518.43 WIR
//... ISBN             0-13-022418-9
//...
//... Today is 25th of March 1988.
*/

#include <stdio.h>

#define   true   1
#define   false  0
#define   m      0                  /*... initial posiiton of queen 1 */


int   a[8];                     /*... rows                */
int   b[15];                    /*... / left diagonal     */
int   c[15];                    /*... \ right diagonal    */
int   x[8];                     /*... columns             */

int   n;                        /*... no of possibilities */
                                /*... ignoring angular similarities */

void try(int);
void qprint(void);

main ()

{
   int i;

   n = 1;
   for (i = 0; i < 8;  ++i)  a[i]  = true;  /*... all rows,cols and*/
   for (i = 0; i < 15; ++i)  b[i]  = true;  /*... diagonals initialised*/
   for (i = 0; i < 15; ++i)  c[i]  = true;  /*... by assuming 'empty'*/
   for (i = 0; i < 8;  ++i)  x[i]  = 0;     /*... false stands for 'occupied'*/

   printf("Working...\n");

   printf("                             columns 0 to 7 \n\n");

   try (m);

   printf("\n");

}

void try (int k)
{
   char s[20];
   int  j;

   for (j=0; j<8; ++j)
   {
      if( (a[j] == true) && (b[k+j] == true) && (c[k-j+7] == true) )
      {
         x[k]     = j;
         a[j]     = false;            /*... occupied*/
         b[k+j]   = false;            /*... occupied*/
         c[k-j+7] = false;            /*... occupied*/

         if ( k < 7)
            try (k+1);
         else
         {
            qprint();
            printf("Do you want to see next possibility?(Y/N)...\n");
            gets(s);                /*... reject previous <CR>*/
            if (s[0] == 'N' || s[0] == 'n' )  exit(0);
            printf("\n");
            ++n;                    /*... next possibility*/
         }
         a[j]     = true;             /*... empty*/
         b[k+j]   = true;             /*... empty*/
         c[k-j+7] = true;             /*... empty*/
      }
   }
}

void qprint()                             /*... out results*/
{  int r;
   printf("possibilty.(rows).   %d ...",n);
   for (r = 0; r<8; ++r)  printf ("   %d",x[r]);
   printf("\n\n");
}

