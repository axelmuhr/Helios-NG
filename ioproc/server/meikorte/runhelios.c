#include <stdio.h>
#include <cstools/build.h>

extern int verbose;
extern int supervisor_mode;

main (int argc, char **argv)
{
  int           errs = 0;
  int           nprocs = 1;
  int           c;
  extern int    optind;
  extern char  *optarg; 
  char         *name;
  extern GROUP *hostGroup;
  GROUP        *group;
  EXE          *exe;
  EXE          *dummy;
  CHANNEL      *channel = cs_channel (NULL, "dummy_chan");


  supervisor_mode = 1 << 2;
  
  while((c = getopt (argc, argv, "n:vis")) != -1)
  {
    switch (c)
      { 
      case 'v':
	verbose++;
	break;
      case 'i':
	cs_nowait ();
	break;
      case 'n':
	nprocs = atoi (optarg);
	break;
      case 's':
	supervisor_mode |= (1 << 0) | (1 << 3);
	break;
      case '?':
        errs++;
        break;
    }
  }

  if (errs || optind != (argc-1))
  {
      fprintf (stderr, "usage: %s [-vin num] file\n", argv[0]);
      exit (1);
  }

  group = cs_group (NULL, "code");
  cs_option (group, "commit", "transputer");
/*
  cs_option (hostGroup, "connect", group);
*/
  exe =  cs_exe (group, "code", name = argv[optind], 
		 "OutChan dummy", channel,
		 0);

  for (c = 0; c < nprocs - 1; c++)
    {
      CHANNEL *next = cs_channel (NULL, "dummy_chan");

      dummy =  cs_exe (NULL, "dummy_exe", "dummy.ex8",
	              "InChan  in", channel,
	              "OutChan out", next,
	              0);
      cs_option (dummy, "mode", "system");

      channel = next;
    }

  dummy = cs_exe (NULL, "dummy_exe", "dummy.ex8",
	          "InChan in", channel,
	          0);
  cs_option (dummy, "mode", "system");
   
  cs_load ();
}
  

