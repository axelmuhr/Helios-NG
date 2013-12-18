#include "hostsys.h"

char **_environ;

extern int main(int, char **, char**);
extern void ____need_main(void) { main(0,NULL,NULL); }

extern int __init(int, char**);

int __main (int dummy1, int dummy2, int argc, char **argv, char **envp)
{
  _environ = envp;
  return __init(argc, argv);
}

