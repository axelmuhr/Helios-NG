/* linkio.h:	Stand-Alone C Host support header			*/
/* $Id: linkio.h,v 1.1 90/11/21 18:46:53 nick Exp $ */

#include <helios.h>                     /* standard header              */
#include <link.h>                       /* for templates & LinkInfo     */
#include <root.h>
#include <config.h>                     /* for LinkConf                 */
#include <codes.h>                      /* for Err_Null                 */

extern word _LinkTimeout;

extern bool link_open (word linkno);
extern bool link_close(word linkno);
extern word link_boot (word linkno, char *file);

#define link_in_byte(linkno,b) LinkIn(1,linkno,&b,_LinkTimeout)
#define link_in_word(linkno,w) LinkIn(4,linkno,&w,_LinkTimeout)
#define link_in_data(linkno,buf,size) LinkIn(size,linkno,buf,_LinkTimeout)
#define link_in_struct(l,d) link_in_data(l,&(d),sizeof(d))

#define link_out_byte(linkno,b) { char __x = b; LinkOut(1,linkno,&__x,_LinkTimeout); }
#define link_out_word(linkno,w) { int __x = w; LinkOut(4,linkno,&__x,_LinkTimeout); }
#define link_out_data(linkno,buf,size) LinkOut(size,linkno,buf,_LinkTimeout)
#define link_out_struct(l,d) link_out_data(l,&(d),sizeof(d))




