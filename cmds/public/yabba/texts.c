/* Placed into the public domain by Daniel J. Bernstein. */

/* If you change the code, you should (1) add your name to sqauthor[] */
/* and/or unsqauthor[]; (2) add a patch label to the version numbers */
/* in sqversion[], sqcopyright[], unsqversion[], and unsqcopyright[]; */
/* (3) change squsage[], sqhelp[], unsqusage[], and unsqhelp[] if you */
/* changed the interface. The patch label should be a slash, your */
/* initials, and the date the patch was released---e.g., version */
/* 0.98 might become 0.98/JS032291 and then 0.98/JS032291/SS040591. */
/* If you let me know about your changes, I'll try to integrate them */
/* into the next release. */

#include "texts.h"

char *sqauthor[] = {
"yabbawhap was written by Daniel J. Bernstein." ,
"Internet address: brnstnd@nyu.edu." ,
0 } ;

char *sqversion[] = {
"yabbawhap version 1.00, March 19, 1991." ,
"Placed into the public domain by Daniel J. Bernstein." ,
0 } ;

char *sqcopyright[] = {
"yabbawhap version 1.00, March 19, 1991." ,
"Placed into the public domain by Daniel J. Bernstein." ,
"" ,
"There is no copyright on this code. You may use it in any way you want." ,
"" ,
"If you have questions about this program or about this notice, or if you" ,
"have a patch that you don't mind sharing, please contact me on the Internet" ,
"at brnstnd@nyu.edu." ,
0 } ;

char *sqwarranty[] = {
"Daniel J. Bernstein disclaims all warranties to the extent permitted" ,
"by applicable law. He is not and shall not be liable for any damages" ,
"arising from the use of this program. This disclaimer shall be governed" ,
"by the laws of the state of New York." ,
"" ,
"In other words, use this program at your own risk." ,
"" ,
"If you have questions about this program or about this disclaimer of" ,
"warranty, please feel free to contact me at brnstnd@nyu.edu on the" ,
"Internet." ,
0 } ;

char *squsage[] = {
#ifdef WHAP
"Usage: whap [ -mmem ] [ -znum ] [ -Zfuzz ] [ -qQv^rRACHUVW ]" ,
"Help:  whap -H" ,
#else
"Usage: yabba [ -mmem ] [ -znum ] [ -Zfuzz ] [ -qQv^rRACHUVW ]" ,
"Help:  yabba -H" ,
#endif
0 } ;

char *sqhelp[] = {
#ifdef WHAP
"whap compresses its input using AP coding. It writes the result to its output." ,
"whap -ACHUVW: print authorship notice, copyright notice, this notice," ,
"              short usage summary, version number, disclaimer of warranty" ,
"whap [ -mmem ] [ -znum ] [ -Zfuzz ] [ -qQv^rR ]: compress" ,
#else
"yabba compresses its input using Y coding. It writes the result to its output." ,
"yabba -ACHUVW: print authorship notice, copyright notice, this notice," ,
"               short usage summary, version number, disclaimer of warranty" ,
"yabba [ -mmem ] [ -znum ] [ -Zfuzz ] [ -qQv^rR ]: compress" ,
#endif
"-mmem: set block size to mem (default: factory 65533 chars, here %d)" ,
"-r: randomize output to prepare for encryption; also omit header" ,
"-R: don't randomize (default); use header" ,
"-q: quiet (nothing on standard error)" ,
"-Q: normal level of verbosity" ,
"-v: verbose: like  In: 312163 chars  Out: 106253 chars  Whapped to: 34%%" ,
"-znum: set blocking test length (factory default 8192)" ,
"-Zfuzz: set blocking fuzz (factory default 30)" ,
"-^: like -v, but print percent saved rather than percent remaining" ,
"" ,
#ifdef WHAP
"unwhap must be given the same settings of -m and -r/-R." ,
#else
"unyabba must be given the same settings of -m and -r/-R." ,
#endif
"" ,
"If you have questions about or suggestions for yabbawhap, please feel free" ,
"to contact the author, Daniel J. Bernstein, at brnstnd@nyu.edu on the" ,
"Internet." ,
0 } ;

char *unsqauthor[] = {
"yabbawhap was written by Daniel J. Bernstein." ,
"Internet address: brnstnd@nyu.edu." ,
0 } ;

char *unsqversion[] = {
"yabbawhap version 1.00, March 19, 1991." ,
"Placed into the public domain by Daniel J. Bernstein." ,
0 } ;

char *unsqcopyright[] = {
"yabbawhap version 1.00, March 19, 1991." ,
"Placed into the public domain by Daniel J. Bernstein." ,
"" ,
"There is no copyright on this code. You may use it in any way you want." ,
"" ,
"If you have questions about this program or about this notice, or if you" ,
"have a patch that you don't mind sharing, please contact me on the Internet" ,
"at brnstnd@nyu.edu." ,
0 } ;

char *unsqwarranty[] = {
"Daniel J. Bernstein disclaims all warranties to the extent permitted" ,
"by applicable law. He is not and shall not be liable for any damages" ,
"arising from the use of this program. This disclaimer shall be governed" ,
"by the laws of the state of New York." ,
"" ,
"In other words, use this program at your own risk." ,
"" ,
"If you have questions about this program or about this disclaimer of" ,
"warranty, please feel free to contact me at brnstnd@nyu.edu on the" ,
"Internet." ,
0 } ;

char *unsqusage[] = {
#ifdef WHAP
"Usage: unwhap [ -mmem ] [ -qQvrRACHUVW ]" ,
"Help:  unwhap -H" ,
#else
"Usage: unyabba [ -mmem ] [ -qQvrRACHUVW ]" ,
"Help:  unyabba -H" ,
#endif
0 } ;

char *unsqhelp[] = {
#ifdef WHAP
"unwhap uncompresses its input, which must have come from whap, to its output." ,
"unwhap -ACHUVW: print authorship notice, copyright notice, this notice," ,
"                short usage summary, version number, disclaimer of warranty" ,
"unwhap [ -mmem ] [ -qQvrR ]: uncompress" ,
#else
"unyabba uncompresses its input, which must have come from yabba, to its output." ,
"unyabba -ACHUVW: print authorship notice, copyright notice, this notice," ,
"                 short usage summary, version number, disclaimer of warranty" ,
"unyabba [ -mmem ] [ -qQvrR ]: uncompress" ,
#endif
"-mmem: set block size to mem (default: factory 65533 chars, here %d)" ,
"-r: accept randomized input without a header" ,
"-R: complain about randomized input; check for header" ,
"-q: quiet (nothing on standard error)" ,
"-Q: normal level of verbosity" ,
"-v: verbose: like  In: 106253 chars  Out: 312163 chars  Whapped from: 34%%" ,
"" ,
#ifdef WHAP
"The whap that wrote the input must have the same settings of -m and -r/-R." ,
#else
"The yabba that wrote the input must have the same settings of -m and -r/-R." ,
#endif
"" ,
"If you have questions about or suggestions for yabbawhap, please feel free" ,
"to contact the author, Daniel J. Bernstein, at brnstnd@nyu.edu on the" ,
"Internet." ,
0 } ;
