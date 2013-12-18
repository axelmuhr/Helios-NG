extern char ** _environ;

#define NULL (char*)0

static char *getenv1(char *name, char *envst)
{
  while (*name!='\0') {
    if (*envst == '\0') return NULL; /* Ran out of environment */
    if (*name++ != *envst++) return NULL; /* Not equal at start */
  }
  return (*envst++ == '=' ? envst : NULL);
}

extern char *getenv(char *name)
{
  char **env = _environ;
  char *ee;
  char *ans;
  while ((ee = *env++) != 0) {
    if ((ans = getenv1(name, ee)) != 0)
      return ans;
  }
  return NULL;
}
