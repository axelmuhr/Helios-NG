/**
*
* Title:  login
*
* Author: Andy England
*
* Date:   1988
*
*         (C) Copyright 1988, Perihelion Software Limited.
*
*         All Rights Reserved.
*
**/

static char *rcsid = "$Header: /hsrc/cmds/com/RCS/login.c,v 1.2 1990/08/23 10:18:37 james Exp $";

#include <limits.h>
#include <posix.h>
#include <attrib.h>
#include <nonansi.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <pwd.h>

#define AND &&
#define OR  ||
#define strequ(s,t) (strcmp(s,t) == 0)

#define DEFAULT_SHELL "/helios/bin/shell"

#define VERSION "\n\t\t           Helios Operating System\n\t\t                Version 1.2\n\t\t(c) Copyright 1987-90, Perihelion Software Ltd\n"

#define PASS_MAX 256

typedef struct passwd PASSWORD;

PRIVATE char *getname(void);
PRIVATE char *getpass(void);
PRIVATE void putmotd(void);
PRIVATE void initio(void);
PRIVATE void noecho(void);
PRIVATE void echo(void);
PRIVATE void setenv(char *, char *);
PRIVATE void initenv(void);
PRIVATE char *crypt(char *, char *);


int main(int argc, char *argv[]) 
{
  char *name = NULL;
  char *password = NULL;
  PASSWORD *pwd = NULL;
  Object *pwf = NULL;

  if (argc > 1) name = argv[1];

  chdir("/helios");

  initio();

  if (*argv[0] == '-') write(1, VERSION, sizeof(VERSION));

  pwf = Locate(NULL, "/helios/etc/passwd");

  forever
  {
    unless (pwf == NULL)
    {
      int badname = FALSE;

      if (name == NULL AND (name = getname()) == NULL) _exit(0);

      if ((pwd = getpwnam(name)) == NULL) badname = TRUE;

      if (badname OR strlen(pwd->pw_passwd) > 0)
      {
        if ((password = getpass()) == NULL) _exit(0);

        if (badname OR strcmp(pwd->pw_passwd, crypt(password, pwd->pw_passwd)))
        {
          Delay(3 * OneSec);
          write(1, "Login incorrect\n", 16);
          name = NULL;
          continue;
        }
      }
      putmotd();
      initenv();
      setenv("HOME", pwd->pw_dir);
      setenv("SHELL", pwd->pw_shell);
      setenv("USER", pwd->pw_name);
      setenv("PATH", "/helios/bin:.");
      setenv("TERM", "ansi");
      setenv("CONSOLE", fdstream(0)->Name);
      chdir(pwd->pw_dir);
      if (pwd->pw_shell) execl(pwd->pw_shell, "-", "-i", NULL);
      else execl(DEFAULT_SHELL, "-", NULL);
    }
    else 
    {
      putmotd();
      initenv();
      setenv("HOME", "/helios");
      setenv("SHELL", DEFAULT_SHELL);
      setenv("USER", "anon");
      setenv("PATH", "/helios/bin:.");
      setenv("TERM", "ansi");
      setenv("CONSOLE", fdstream(0)->Name);
      execl(DEFAULT_SHELL, "-", NULL);
    }
    write(1, "exec failure\n", 13);
  } /* end of forever */
}

PRIVATE char *getname(void)
{
  static char name[NAME_MAX + 1];
  int length;

  do
  {
    write(1, "login: ", 7);
    if ((length = read(0, name, NAME_MAX)) <= 0) return NULL;
  } while (length == 1);
  name[length - 1] = '\0';
  return name;
}

PRIVATE char *getpass(void)
{
  static char password[PASS_MAX + 1];
  int length;

  noecho();
  write(1, "Password: ", 10);
  if ((length = read(0, password, PASS_MAX)) <= 0) return NULL;
  write(1, "\n", 1);
  echo();
  password[length - 1] = '\0';
  return password;
}

PRIVATE void putmotd(void)
{
  int motd;
  static char motdbuf[128];
  Attributes attr;
  Stream *console;

  /* allow ANSI Character renditions */
  console = fdstream(0);
  GetAttributes(console, &attr);
  AddAttribute(&attr, ConsoleRawOutput);
  SetAttributes(console, &attr);

  unless ((motd = open("etc/motd", O_RDONLY)) == -1)
  {
    int size;

    until ((size = read(motd, motdbuf, 128)) == 0) write(1, motdbuf, size);
    close(motd);
  }

  RemoveAttribute(&attr, ConsoleRawOutput);
  SetAttributes(console, &attr);
}

PRIVATE Attributes attr;
PRIVATE Stream *console;

PRIVATE void initio(void)
{
  console = fdstream(0);
  GetAttributes(console, &attr);
}

PRIVATE void noecho(void)
{
  RemoveAttribute(&attr, ConsoleEcho);
  SetAttributes(console, &attr);
}

PRIVATE void echo(void)
{
  AddAttribute(&attr, ConsoleEcho);
  SetAttributes(console, &attr);
}

PRIVATE char *crypt(char *key, char *salt)
{
  return key;
}

PRIVATE void initenv(void)
{
  environ = (char **)Malloc(sizeof(char *));
  environ[0] = NULL;
}

PRIVATE char *newenv(char *name, char *value)
{
  int length = strlen(name);
  char *env = (char *)Malloc(length + strlen(value) + 2);

  strcpy(env, name);
  env[length] = '=';
  strcpy(env + length + 1, value);
  return env;
}

PRIVATE void setenv(char *name, char *value)
{
  int count = 0;
  int length = strlen(name);
  char *env;
  char **newenviron;

  while ((env = environ[count]) != NULL)
  {
    if ((strncmp(name, env, length) == 0) AND (env[length] == '='))
    {
      free(env);
      environ[count] = newenv(name, value);
      return;
    }
    count++;
  }
  newenviron = (char **)Malloc(sizeof(char *) * (count + 2));
  memcpy(newenviron, environ, sizeof(char *) * count);
  newenviron[count] = newenv(name, value);
  newenviron[count + 1] = NULL;
  free(environ);
  environ = newenviron;
}

