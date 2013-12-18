#include <stdio.h>
#include <time.h>

int main(argc, argv)
int argc;
char *argv[];
{
  char buf[32];
  time_t now;
  char *ct;

  time(&now);
  ct = ctime(&now);
  sprintf(buf, "%.3s %.2s %.4s", ct+4, ct+8, ct+20);

  if (argc ==2)
  {   char *arg = argv[1];
      if (arg[0] == '-')
      {   if (arg[1] == 'h' || arg[1] == 'H')
          {   printf("#define  __DATE__ \"%s\"\n", buf);
              printf("#define  __TIME__ \"%.8s\"\n", ct+11);
          }
          else if (arg[1] == 0)
              fputs(buf, stdout);
          else
              goto defolt;
          return 0;
      }
  }
defolt:
  printf("CurrentDate     SETS \"%s\"\n", buf);
  printf("                END\n");

  return 0;
}
