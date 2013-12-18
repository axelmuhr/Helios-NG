/**
*
* Title:  Helios Shell - Substitutions.
*
* Author: Andy England
*
* Date:   May 1988
*
*         (c) Copyright 1988, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
**/
#include "shell.h"

BOOL historyused;
int wordindex = 0;
char wordbuffer[WORD_MAX + 1];
char modifier[WORD_MAX + 1] = "";

ARGV fullsub(ARGV argv)
{
#ifdef	DEBUGGING
  DEBUG ("fullsub (%V)",argv);
#endif
  argv = varsub(argv);
  argv = cmdsub(argv);
  unless (findvar("noglob")) argv = filenamesub(argv);
  argv = quotesub(argv);
  return argv;
}

ARGV smallsub(ARGV argv)
{
#ifdef	DEBUGGING
  DEBUG ("smallsub (%V)",argv);
#endif
  argv = varsub(argv);
/* ACE: I think this should be added here */
  argv = cmdsub(argv);
  argv = quotesub(argv);
  return argv;
}

ARGV modifiersub(
		 ARGV argv,
		 char quickchar )
{
  int c;
  char *arg;
  char *subarg;
  ARGV subargv;
  ARGV oldargv = argv;
  ARGV newargv = nullargv();
  char eventbuffer[NUMSTR_MAX + 1], buffer[WORD_MAX + 1];
  char *start;
  int index = 0;

#ifdef	DEBUGGING
  DEBUG ("modifiersub (%V,%c)",argv,quickchar);
#endif
  ignore sprintf(eventbuffer, "%d", eventnumber - 1);
  if ((subargv = findevent(eventnumber - 1)) == NULL)
  {
    freeargv(oldargv);
    freeargv(newargv);
    error(ERR_EVENT, eventbuffer);
    recover();
  }
  arg = *argv++ + 1;
  until (*arg == quickchar OR *arg == '\0') buffer[index++] = *arg++;
  buffer[index] = '\0';
  if (*arg++ == '\0')
  {
    freeargv(oldargv);
    freeargv(newargv);
    error(ERR_BADSUB, NULL);
    recover();
  }
  if (index == 0)
  {
    if (modifier[0] == '\0')
    {
      freeargv(oldargv);
      freeargv(newargv);
      error(ERR_NOPREVLHS, NULL);
      recover();
    }
  }
  else strcpy(modifier, buffer);
  until ((subarg = *subargv++) == NULL)
  {
    unless (historyused OR (start = strstr(subarg, modifier)) == NULL)
    {
      historyused = TRUE;
      until (subarg == start) addchar(*subarg++);
      until (*arg == quickchar OR *arg == '\0') addchar(*arg++);
      if (*arg == quickchar) arg++;
      else
      {
        until ((arg = *argv++) == NULL)
        {
          if (endword()) newargv = addword(newargv, wordbuffer);
          until (*arg == quickchar OR *arg == '\0') addchar(*arg++);
          if (*arg == quickchar)
          {
            arg++;
            break;
          }
        }
      }
      subarg += strlen(modifier);
      until ((c = *subarg++) == '\0') addchar(c);
      if (endword()) newargv = addword(newargv, wordbuffer);
    }
    else newargv = addword(newargv, subarg);
  }
  unless (arg == NULL)
  {
    until ((c = *arg++) == '\0') addchar(c);
    if (endword()) newargv = addword(newargv, wordbuffer);
    until ((arg = *argv++) == NULL)
    {
      until ((c = *arg++) == '\0') addchar(c);
      if (endword()) newargv = addword(newargv, wordbuffer);
    }
  }
  else if (endword()) newargv = addword(newargv, wordbuffer);
  unless (historyused)
  {
    freeargv(oldargv);
    freeargv(newargv);
    error(ERR_MODIFIER, NULL);
    recover();
  }
  freeargv(oldargv);
  return newargv;
}

ARGV historysub(ARGV argv)
{
  char *histchars;
  char histchar, quickchar;

#ifdef	DEBUGGING
  DEBUG("historysub(%V)",argv);
#endif
  historyused = FALSE;
  if ((histchars = getvar("histchars")) == NULL)
  {
    histchar = '!';
    quickchar = '^';
  }
  else
  {
    if ((histchar = histchars[0]) == '\0') return argv;
    if ((quickchar = histchars[1]) == '\0') quickchar = histchar;
  }
  if (argv[0] && (argv[0][0] == quickchar)) return modifiersub(argv, quickchar);
  else
  {
    int c;
    char *arg;
    ARGV subargv;
    ARGV oldargv = argv;
    ARGV newargv = nullargv();

    until ((arg = *argv++) == NULL)
    {
      until ((c = *arg++) == '\0')
      {
        if (c == '\\')
        {
          addchar('\\');
          addchar(*arg++);
          continue;
        }
        if (c == histchar)
        {
          if (*arg == '\0' OR *arg == ' ' OR *arg == '\t' OR
              *arg == '\n' OR *arg == '=' OR *arg == '('  OR
	      *arg == '~'  OR *arg == '\r' )
            addchar(histchar);
          else
          {
            char eventbuffer[WORD_MAX + 1];
            int number;
            int index = 0;
            int designator[2];
            int inbraces = FALSE;
  
            historyused = TRUE;
            designator[0] = 0;
            designator[1] = -1;
            if (*arg == '{')
            {
              arg++;
              inbraces = TRUE;
            }
            switch (*arg)
            {
              case '-':
              arg = readdecimal(arg + 1, &number);
              ignore sprintf(eventbuffer, "%d", eventnumber - number);
              subargv = findevent(eventnumber - number);
              break;

              case '0': case '1': case '2': case '3': case '4':
              case '5': case '6': case '7': case '8': case '9':
              arg = readdecimal(arg, &number);
              ignore sprintf(eventbuffer, "%d", number);
              subargv = findevent(number);
              break;

              case '^': case '$': case '*': case '%':
              ignore sprintf(eventbuffer, "%d", eventnumber - 1);
              subargv = findevent(eventnumber - 1);
              break;

              default:
              if (*arg == histchar)
              {
                arg++;
                ignore sprintf(eventbuffer, "%d", eventnumber - 1);
                subargv = findevent(eventnumber - 1);
                break;
              }
              if (inbraces)
              {
                until (*arg == '\0' OR *arg == '}')
                  eventbuffer[index++] = *arg++;
              }
              else
              {
                until (*arg == '\0' OR *arg == ' ' OR *arg == '\t' OR
                       *arg == ':' OR *arg == '\n' OR *arg == '\r' )
                  eventbuffer[index++] = *arg++;
              }
              eventbuffer[index] = '\0';
              subargv = findhistory(eventbuffer);
              break;
            }
            if (*arg == ':' OR *arg == '^' OR *arg == '$' OR
                *arg == '*' OR *arg == '-' OR *arg == '%')
            {
              if (*arg == ':') arg++;
              arg = readdesignator(arg, designator);
            }
            if (subargv == NULL)
            {
              freeargv(oldargv);
              freeargv(newargv);
              error(ERR_EVENT, eventbuffer);
              recover();
            }
            else
            {
              char *arg;
              int argc;
              int c;
              int length = lenargv(subargv);
  
              if (designator[0] == -1) designator[0] = length - 1;
              if (designator[1] == -1) designator[1] = length - 1;
              if (designator[0] >= length OR designator[1] >= length)
              {
                freeargv(oldargv);
                freeargv(newargv);
                error(ERR_SUBSCRIPT, NULL);
                recover();
              }

              for (argc = designator[0]; argc <= designator[1]; argc++)
              { 
		if (argc > length)
		    break;
                arg = subargv[argc];
                if (arg == NULL) arg = ""; 
                until ((c = *arg++) == '\0')
                {
                  addchar(c);
                }
                unless (argc == designator[1])
                {
                  if (endword()) newargv = addword(newargv, wordbuffer);
                }
              }
            }
          }
          continue;
        }
        addchar(c);
      }
      if (endword()) newargv = addword(newargv, wordbuffer);
    }
    freeargv(oldargv);
    return newargv;
  }
}

ARGV varsub(ARGV argv)
{
  SUBSTATE state;
  int c;
  char *arg;
  ARGV envargv = NULL;
  ARGV subargv;
  ARGV oldargv = argv;
  ARGV newargv = nullargv();

#ifdef	DEBUGGING
  DEBUG ("varsub(%V)",argv);
#endif
  until ((arg = *argv++) == NULL)
  {
    int argspecial = FALSE;
    state = NEUTRAL;
    until ((c = *arg++) == '\0')
    {
      switch (state)
      {
        case NEUTRAL:
        case INDQUOTE:
        switch (c)
        {
          case '\\':
          addchar('\\');
          if (state == NEUTRAL) addchar(*arg++);
          continue;

          case '\'':
          addchar('\'');
          if (state == NEUTRAL) state = INSQUOTE;
          continue;

          case '`':
          addchar('`');
          if (state == NEUTRAL) state = INBQUOTE;
          continue;

          case '"':
          addchar('"');
          if (state == NEUTRAL) state = INDQUOTE;
          else state = NEUTRAL;
          continue;

          case '$':
          if (*arg == '\0' OR *arg == ' ' OR
              *arg == '\t' OR *arg == '\n' OR *arg == '\r' ) addchar('$');
          else
          {
            char name[VARNAME_MAX + 1], buffer[NUMSTR_MAX + 1];
            int selector[2];
            int index = 0;
            int inbraces = FALSE;
            int wordcount = FALSE;
            int query = FALSE;

            selector[0] = 0;
            selector[1] = -1;
            if (*arg == '{')
            {
              arg++;
              inbraces = TRUE;
            }
            switch (*arg)
            {
              case '<':
              arg++;
              {
              	int saveread;
              	saveread = dup(READ);
                until ((c = getchar()) == '\n' OR c == '\r' OR c == EOF) addchar(c);
                close(READ);
                dup(saveread);
                close(saveread);
              }
              break;

              case '0':
              if (filename == NULL)
              {
                freeargv(oldargv);
                freeargv(newargv);
                error(ERR_NOFILENAME, NULL);
                recover();
              }
              arg++;
              until ((c = filename[index++]) == '\0') addchar(c);
              break;

              case '$':
              arg++;
              ignore sprintf(buffer, "%d", shellpid);
              until ((c = buffer[index++]) == '\0') addchar(c);
              break;

              case '*':
              arg++;
              subargv = findvar("argv");
              goto dosub;

              case '1': case '2': case '3': case '4':
              case '5': case '6': case '7': case '8': case '9': 
              arg = readdecimal(arg, selector);
              selector[1] = selector[0];
              subargv = findvar("argv");
	      argspecial = TRUE;
              goto dosub;

              case '#':
              arg++;
              wordcount = TRUE;
              goto varname;

              case  '?':
              query = TRUE;
              if (*++arg == '0')
              {
                arg++;
                if (filename == NULL) addchar('0');
                else addchar('1');
                break;
              }
varname:
              default:
              unless (isalpha(*arg) OR *arg == '_')
              {
                freeargv(oldargv);
                freeargv(newargv);
                error(ERR_VARSYNTAX, NULL);
                recover();
              }
              arg = readname(arg, name); 
	      if(!query && !wordcount)
	          if(! strcmp(name, "argv"))
		      argspecial = TRUE;
              unless (wordcount OR query OR *arg != '[')
              {
                if (*++arg == ']')
                {
                  freeargv(oldargv);
                  freeargv(newargv);
                  error(ERR_VARSYNTAX, NULL);
                  recover();
                }
                arg = readselector(arg, selector);
                unless (*arg++ == ']')
                {
                  freeargv(oldargv);
                  freeargv(newargv);
                  error(ERR_VARSYNTAX, NULL);
                  recover();
                }
              }
              subargv = findvar(name);

              if (subargv == NULL)
              {
              	envargv = envmakeargv(name);
              	if (**envargv != '\0')
              	  subargv = envargv;
              	else
              	{
              	  freeargv(envargv);
              	  envargv = NULL;
              	}
              }

              if (wordcount)
              {
                if (subargv == NULL)
                {
                  freeargv(oldargv);
                  freeargv(newargv);
                  error(ERR_VARIABLE, name);
                  recover();
                }
                ignore sprintf(buffer, "%d", lenargv(subargv));
                until ((c = buffer[index++]) == '\0') addchar(c);
              }
              else if (query)
              {
                if (subargv == NULL) addchar('0');
                else addchar('1');
              } 
              else
dosub:
              if (subargv == NULL)
              {
                if (envargv != NULL) 
                { 
                  freeargv(envargv); 
                  envargv = NULL; 
                }
                freeargv(oldargv);
                freeargv(newargv);
                error(ERR_VARIABLE, name);
                recover();
              }
              else
              {
                char *arg;
                int argc;
                int c;
                int length = lenargv(subargv);

                if (selector[0] == -1) selector[0] = length;
                if (selector[1] == -1) selector[1] = length;
                if (! argspecial && 
			(selector[0] > length OR selector[1] > length))
                {
                  if (envargv != NULL) 
                  {
                    freeargv(envargv);
                    envargv = NULL;
                  }
                  freeargv(oldargv);
                  freeargv(newargv);
                  error(ERR_SUBSCRIPT, NULL);
                  recover();
                }

                for (argc = selector[0]; argc <= selector[1]; argc++)
                { 
                  if (argc == 0) continue;
		  if (argc > length)
			break;
                  arg = subargv[argc - 1];
                  if (arg == NULL) arg = ""; /*JD fix no 334*/
                  until ((c = *arg++) == '\0')
                  {
                    if (isspace(c))
                    {
                      if (state == NEUTRAL)
                      {
                        if (endword()) newargv = addword(newargv, wordbuffer);
                      }
                      else
                        addchar (c);
                      while (isspace(c)) c = *arg++;
                      if (c == '\0') break;
                    }
                    addchar(c);
                  }
                  unless (argc == selector[1])
                  {
                    if (state == NEUTRAL)
                    {
                      if (endword()) newargv = addword(newargv, wordbuffer);
                    }
                    else
                      addchar (' ');
                  }
                }
              }
              if (envargv != NULL) 
              {
              	freeargv(envargv);
              	envargv = NULL;
              }
              break;
            }
            if (inbraces)
            {
              unless (*arg++ == '}')
              {
                freeargv(oldargv);
                freeargv(newargv);
                error(ERR_VARSYNTAX, NULL);
                recover();
              }
            }
          }
          continue;

          default:
          addchar(c);
          continue;
        }

        case INSQUOTE:
        case INBQUOTE:
        switch (c)
        {
          case '\\':
          addchar('\\');
          addchar(*arg++);
          continue;

          case '\'':
          addchar('\'');
          if (state == INSQUOTE) state = NEUTRAL;
          continue;

          case '`':
          addchar('`');
          if (state == INBQUOTE) state = NEUTRAL;
          continue;

          default:
          addchar(c);
          continue;
        }
      }
    }
    if (endword()) newargv = addword(newargv, wordbuffer);
  } 
  freeargv(oldargv);
  return newargv;
}

ARGV cmdsub(ARGV argv)
{
  SUBSTATE state;
  int c, cc;
  char *arg;
  ARGV oldargv = argv;
  ARGV newargv = nullargv();
  int addbrack = FALSE;
  BUILTIN *builtin;

#ifdef	DEBUGGING
  DEBUG ("cmdsub(%V)",argv);
#endif

/* horrid kludge to see if this is a set command. If so, treat multiple
   args differently !! */

  unless((builtin = findbuiltin(*argv)) == NULL)
    if(builtin -> func == b_set)
	addbrack = TRUE;

  until ((arg = *argv++) == NULL)
  {
    state = NEUTRAL;
    until ((c = *arg++) == '\0')
    {
      switch (state)
      {
        case NEUTRAL:
        case INDQUOTE:
        switch (c)
        {
          case  '\\':
          addchar('\\');
          if (state == NEUTRAL) addchar(*arg++);
          continue;

          case '\'':
          addchar('\'');
          if (state == NEUTRAL) state = INSQUOTE;
          continue;

          case '"':
          addchar('"');
          if (state == NEUTRAL) state = INDQUOTE;
          else state = NEUTRAL;
          continue;
        
          case '`':
          {
            int index = 0;
            char buffer[WORD_MAX + 1];
            char tempwordbuffer[WORD_MAX + 1];
            int tempwordindex;
            int fds[2];
            FILE *file;

	    if(addbrack && state == NEUTRAL)
		{
		if(endword())
			newargv=addword(newargv, wordbuffer);
		addchar('(');
		if(endword())
			newargv=addword(newargv, wordbuffer);
		}

	    strcpy(tempwordbuffer, wordbuffer);
	    tempwordindex = wordindex;
	    wordindex = 0;
            until ((c = *arg++) == '`') buffer[index++] = c;
            buffer[index] = '\0';
#ifdef HELIOS
            if (fifo(fds) == -1)
#else
            if (pipe(fds) == -1)
#endif
            {
              freeargv(oldargv);
              freeargv(newargv);
              syserr(NULL);
              recover();
            }
            ignore waitforcmd(runsimplecmd(buildargv(makeargv(buffer)), NULL, READ, fds[WRITE], CHECKBUILTIN));
            close(fds[WRITE]);
	    strcpy(wordbuffer, tempwordbuffer);
	    wordindex = tempwordindex;
            file = fdopen(fds[READ], "r");
	    if( GetFileSize(Heliosno(file)))
            until ((c = getc(file)) == EOF)
            {
nextchar:
              if (c == '\n' OR c == '\r')
              {
                if ((cc = getc(file)) == EOF) break;
                if (endword()) newargv = addword(newargv, wordbuffer);
                c = cc;
                goto nextchar;
              }
              if (state == NEUTRAL AND (c == ' ' OR c == '\t'))
              {
                if (endword()) newargv = addword(newargv, wordbuffer);
              } 
              else addchar(c);
            }
            fclose(file);

	    if(addbrack && state == NEUTRAL)
		{
		if(endword())
			newargv=addword(newargv, wordbuffer);
		addchar(')');
		if(endword())
			newargv=addword(newargv, wordbuffer);
		}

          }
          continue;

          default:
          addchar(c);
          continue;
        }

        case INSQUOTE:
        switch (c)
        {
          case '\\':
          addchar('\\');
          addchar(*arg++);
          continue;

          case '\'':
          addchar('\'');
          state = NEUTRAL;
          continue;

          default:
          addchar(c);
          continue;
        }
      }
    }
    if (endword()) newargv = addword(newargv, wordbuffer);
  }
  freeargv(oldargv);
  return newargv;
}

ARGV filenamesub(ARGV argv)
{
  GLOBSTATE state = NOPATTERN;
  BOOL nonomatch = (BOOL)(findvar("nonomatch") != NULL);
  char *arg;
  ARGV oldargv = argv;
  ARGV newargv = nullargv();

#ifdef	DEBUGGING
  DEBUG ("filenamesub(%V)",argv);
#endif
  until ((arg = *argv++) == NULL)
  {
    if (ispattern(arg))
    {
      BOOL iscmd = (BOOL) (arg == oldargv[0]);
      int oldlength = lenargv(newargv);
      int length;
      char path[PATH_MAX + 1], pattern[WORD_MAX + 1];
      char *p = arg;
      char *rest;
      int index;

      if (*p == '~')
      {
        char username[WORD_MAX + 1];
        int i = 0;

        p++;
        until (*p == '\0' OR *p == '/') username[i++] = *p++;
        if (i > 0)
        {
          struct passwd *pwent;

          username[i] = '\0';
          if ((pwent = getpwnam(username)) == NULL)
          {
            endpwent();
            freeargv(oldargv);
            freeargv(newargv);
            error(ERR_UNKNOWNUSER, username);
            recover();
          }
          endpwent();
          strcpy(path, pwent->pw_dir);
        }
        else
        {
          char *home;

          if ((home = getvar("home")) == NULL)
          {
            freeargv(oldargv);
            freeargv(newargv);
            error(ERR_NOHOME, NULL);
            recover();
          }
          strcpy(path, home);
        }
      }
      else path[0] = '\0';
      forever
      {
        if (*p == '/')
        {
          strcat(path, "/");
          p++;
        }
        rest = p;
        index = 0;
        until (*p == '\0' OR *p == '/') pattern[index++] = *p++;
        pattern[index] = '\0';
        if (index == 0 OR ispattern(pattern)) break;
        strcat(path, pattern);
      }
      if (index == 0) newargv = addword(newargv, path);
      else
      {
        newargv = searchdir(newargv, path, rest);
        length = lenargv(newargv);
        if (iscmd)
        {
          unless (length == oldlength + 1)
          {
            freeargv(oldargv);
            freeargv(newargv);
            if (length == oldlength) error(ERR_NOMATCH, arg);
	    else
                error(ERR_AMBIGUOUS, arg);
            recover();
          }
        }
        else
        {
          if (length == oldlength)
          {
            if (nonomatch) newargv = addword(newargv, arg);
            else if (state == NOPATTERN) state = NOMATCH;
          }
          else state = MATCH;
        }
      }
    }
    else newargv = addword(newargv, arg);
  }
  if (state == NOMATCH)
  {
    freeargv(oldargv);
    freeargv(newargv);
    error(ERR_NOMATCH, arg);
    recover();
  }
  freeargv(oldargv);
  return newargv;
}

ARGV searchdir(
	       ARGV argv,
	       char *path,
	       char *rest )
{
  DIR *dir;
  DIRENT *direntry;
  int i = 0;
  char pattern[WORD_MAX + 1];
  int length = strlen(path);

  if ((dir = opendir(path)) == NULL) return argv;
  until (*rest == '\0' OR *rest == '/') pattern[i++] = *rest++;
  pattern[i] = '\0';
  if (*rest)
  {
    rest++;
    until ((direntry = readdir(dir)) == NULL)
    {
      unless (direntry->d_name[0] == '.' AND pattern[0] != '.')
      {
        if (match(direntry->d_name, pattern))
        {
          strcpy(path + length, direntry->d_name);
          strcat(path, "/");
          argv = searchdir(argv, path, rest);
        }
      }
    }
  }
  else
  {
    until ((direntry = readdir(dir)) == NULL)
    {
      unless (direntry->d_name[0] == '.' AND pattern[0] != '.')
      {
        if (match(direntry->d_name, pattern))
        {
          strcpy(path + length, direntry->d_name);
          argv = addword(argv, path);
        }
      }
    }
  }
  path[length] = '\0';
  closedir(dir);
  return argv;
}

BOOL ispattern(char *arg)
{
  SUBSTATE state = NEUTRAL;
  int c;

  if (arg[0] == '~') return TRUE;
  until ((c = *arg++) == '\0')
  {
    switch (state)
    {
      case NEUTRAL:
      switch (c)
      {
        case '*':
        case '?':
        case '[':
        case '{':
        return TRUE;

        case '\'':
        state = INSQUOTE;
        continue;

        case '"':
        state = INDQUOTE;
        continue;

        case '\\':
        arg++;
        continue;
      }
      break;

      case INSQUOTE:
      case INDQUOTE:
      switch (c)
      {
        case '\'':
        if (state == INSQUOTE) state = NEUTRAL;
        continue;

        case '"':
        if (state == INDQUOTE) state = NEUTRAL;
        continue;

        case '\\':
        arg++;
        continue;
      }
      break;
    }
  }
  return FALSE;
}

ARGV quotesub(ARGV argv)
{
  SUBSTATE state;
  char *arg;
  int c;
  ARGV oldargv = argv;
  ARGV newargv = nullargv();

#ifdef	DEBUGGING
  DEBUG("quotesub(%V)",argv);
#endif
  until ((arg = *argv++) == NULL)
  {
    state = NEUTRAL;
    until ((c = *arg++) == '\0')
    {
      switch (state)
      {
        case NEUTRAL:
        switch (c)
        {
          case '\\':
          addchar(*arg++);
          continue;

          case '\'':
          state = INSQUOTE;
          continue;

          case '"':
          state = INDQUOTE;
          continue;

          default:
          addchar(c);
          continue;
        }

        case INSQUOTE:
        case INDQUOTE:
        switch (c)
        {
          case '\\':
          if (*arg == '\n' OR *arg == '\r' OR *arg == '!') addchar(*arg++);
          else addchar('\\');
          continue;

          case '\'':
          if (state == INSQUOTE) state = NEUTRAL;
          else addchar('\'');
          continue;

          case '"':
          if (state == INDQUOTE) state = NEUTRAL;
          else addchar('"');
          continue;

          default:
          addchar(c);
          continue;
        }
      }
    }
/*
 * CFL: empty arg should be preserved !
 *    
 *   if (endword()) newargv = addword(newargv, wordbuffer);
 */
/*
 * CFL: preserving empty args causes system trouble (errors in NewObject...)
 *
 *   endword();
 *   newargv = addword(newargv, wordbuffer);
 */
    if (endword()) newargv = addword(newargv, wordbuffer);
  }
  freeargv(oldargv);
  return newargv;
}

char *readname(
	       char *arg,
	       char name[] )
{
  int i = 0;
  int c;

  while ((isalpha(c = *arg) OR c == '_' OR isdigit(c)) AND i < VARNAME_MAX)
  {
    name[i++] = c;
    arg++;
  }
  name[i] = '\0';
  return arg;
}

char *readnumber(
		 char *arg,
		 int *numberaddr,
		 int base )
{
  int digit;
  int digits = 0;
  int number = 0;

  while ((digit = getdigit(*arg++)) < base) 
  {
    digits++;
    number = (number * base) + digit;
  }
  if (digits)
    *numberaddr = number;

  return arg - 1;
}

int getdigit(int c)
{
  if (isdigit(c)) return c - '0';
  if (isxdigit(c)) return toupper(c) - 'A' + 10;
  return 16;
}

char *readdesignator(
		     char *arg,
		     int designator[] )
{
  if (*arg == '*')
  {
    designator[0] = 1;
    return ++arg;
  }
  if (*arg == '-') arg++;
  else
  {
    arg = readargnumber(arg, &designator[0]);
    if (*arg == '-') arg++;
    else
    {
      designator[1] = designator[0];
      return arg;
    }
  }
  return readargnumber(arg, &designator[1]);
}

char *readargnumber(
		    char *arg,
		    int *numberaddr )
{
  int argnumber = 0;

  if (*arg == '^')
  {
    *numberaddr = 1;
    return ++arg;
  }
  if (*arg == '$')
  {
    *numberaddr = -1;
    return ++arg;
  }
  if (isdigit(*arg))
  {
    do argnumber = argnumber * 10 + (*arg++ - '0'); while (isdigit(*arg));
    *numberaddr = argnumber;
  }
  return arg;
}

char *readselector(
		   char *arg,
		   int selector[] )
{
  if (*arg == '*') return ++arg;
  if (*arg == '-') arg++;
  else
  {
    arg = readdecimal(arg, &selector[0]);
    if (*arg == '-') arg++;
    else
    {
      selector[1] = selector[0];
      return arg;
    }
  }
  return readdecimal(arg, &selector[1]);
}

void addchar(int c)
{
  if (wordindex >= WORD_MAX)
  {
    wordindex = 0;
    error(ERR_WORDTOOLONG, NULL);
    recover();
  }
  wordbuffer[wordindex++] = c;
}

BOOL endword()
{
  wordbuffer[wordindex] = '\0';
  if (wordindex == 0) return FALSE;
  wordindex = 0;
  return TRUE;
}
