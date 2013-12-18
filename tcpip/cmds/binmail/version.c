#ifdef __HELIOS
static char *rcsid = "$Header: /hsrc/tcpip/cmds/binmail/RCS/version.c,v 1.9 1992/03/08 14:39:01 craig Exp $";
/*
-- Alpha
-- 1.00 - Initial release
-- 1.01 - 1. added -v command line option (sends verbose flag to sendmail)
--        2. -v also displays version (version.c added to sources)
-- 1.02 - 1. tidied up signal handling + removed CTRL-C trapping
-- 1.03 - 1. pipe output through /helios/bin/more !!!
-- 1.04 - 1. write "dead.letter" to home directory (not 
--           /helios/local/spool/mqueue)
-- 1.05 - 1. explicity strip out CRLF when writing temp file
-- 1.06 - 1. return sensible error codes if unable to process mail
-- 1.07 - 1. print error message if unable to execve sendmail
--      - 2. openlog ("mail", ...) (instead of "binmail")
-- Release version
-- 1.00 - Identical to 1.07 Alpha - only the version number has changed
*/
char	Version[] = "V1.00" ;
#endif
