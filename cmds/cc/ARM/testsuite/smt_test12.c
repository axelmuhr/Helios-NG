/*> smt_test12.c <*/
/* Split Module Table test program 12 */

#include <stdio.h>

typedef struct {
		unsigned int   fred ;
		unsigned char  jim ;
		char	       sheila[32] ;
               } example ;

example array[] = {
		   {1,'a', "one"},
		   {2,'b', "two"},
		   {3,'c', "three"},
		   {4,'d', "four"},
		   {5,'e', "five"},
		   {6,'f', "six"},
		   {7,'g', "seven"},
		   {8,'h', "eight"},
		   {9,'i', "nine"},
		   {0,'\0',""}
                  } ;

int main(void)
{
 int loop ;
 printf("Hello Split Module Table world (12)\n") ;
 for (loop = 0; array[loop].jim; loop++)
  printf("\"%s\"\n",array[loop].sheila) ;
 return(0) ;
}

/*> EOF smt_test12.c <*/

