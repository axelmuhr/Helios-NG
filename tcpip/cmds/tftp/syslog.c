
#include <syslog.h>
#include <stdio.h>

int main()
{
  if (openlog("test", 0) == -1)
  {
    fprintf(stderr, "Failed to open log\n");
    exit(1);
  }
  syslog(LOG_ERR, "Hello");
  closelog();
}
