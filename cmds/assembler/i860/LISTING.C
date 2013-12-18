#include "iasm.h"
#include <string.h>

char   srcline[200];
int    srclinelen;
FILE  *listingfile;
int    listinglevel;
char  *listtext;
int    listing_flags;
int    listing_options;
codeptr listcodep;
uint32 listpcloc;

typedef struct ListMessage {
   struct ListMessage *cdr;
   char   msg[100];
} ListMessage;

ListMessage *freemsgs=NULL;
ListMessage *msgs=NULL;

void addtosrcline(char *s, int l)
{
   if( pass == 1 ) return;
   if( listingfile == NULL ) return;

   memcpy(srcline+srclinelen,s,l);
   srclinelen += l;
   srcline[srclinelen] = '\0';
}

void listmessage(char *ms, char *s, va_list ap)
{  ListMessage *m;
   ListMessage **mp;

   if( pass == 1 || listingfile == NULL ) return;

   if( freemsgs == NULL )
      m = (ListMessage *)aalloc(sizeof(ListMessage));
   else
   {  m = freemsgs;
      freemsgs = m->cdr;
   }
   m->cdr = NULL;

   for( mp = &msgs; *mp != NULL; mp = &((*mp)->cdr));
   *mp = m;

   strcpy(m->msg,ms);
   strcat(m->msg,": ");
   vsprintf(m->msg+strlen(m->msg),s,ap);
   strcat(m->msg,"\n");
}

void outlistline(void)
{
   char *ltext = listtext;
   int lflags = listing_flags;

   listing_flags = 0;
   listtext = NULL;
   srclinelen = 0;

   if( pass == 1 || listingfile == NULL ) return;

   if( (lflags & LF_NOCODE) &&
       (listing_options & LO_CODEONLY) ) return;

   if( (listing_options & LO_NOTMACRO) &&
       (currentfile->flags & ff_macro) )
   {  listcodep.p = codep.p;
      listcodep.seg = codep.seg;
      listpcloc = pcloc;
      return;
   }

   if( listinglevel >= 0 )
   {  int nchars=0;

      fprintf(listingfile, "%4d",currentfile->lineno);
      if( ltext )
         nchars += fprintf(listingfile, "   %s", ltext);

      if( listcodep.p != codep.p || listcodep.seg != codep.seg)
      {
         nchars += fprintf(listingfile, " %06lx", listpcloc);
         do
         {  int thissize = code[listcodep.seg]->codeflags[listcodep.p];
            switch( thissize )
            {
            case 1:
               nchars += fprintf(listingfile," %02x",
                                   code[listcodep.seg]->codebuf[listcodep.p].b1);
               break;
            case 2:
               nchars += fprintf(listingfile," %04x",
                                   code[listcodep.seg]->codebuf[listcodep.p].b2);
               break;
            case 4:
               nchars += fprintf(listingfile," %08lx",
                                   code[listcodep.seg]->codebuf[listcodep.p].b4);
               break;
            }
            listpcloc += thissize;
            listcodep.p = (listcodep.p+1) & CODEBUFSIZEMASK;
            if( listcodep.p == 0 ) listcodep.seg++;
         } while( listcodep.p != codep.p || listcodep.seg != codep.seg);
      }
      else
      	 listpcloc = pcloc;	/* The PC may have changed behind our backs */

      { int i;
         if( nchars < 16 )
            for(i = 16-nchars; i; i--) fputc(' ',listingfile);
      }
   
      fprintf(listingfile, " %s", srcline);

      for(; msgs ; msgs = msgs->cdr)
      {
         fputs(msgs->msg,listingfile);
         freemsgs = msgs;
      }
   }
}

void setlistfile(char *s)
{
   listingfile = fopen(s, "w");
   if( listingfile == NULL )
      parmerror("Can't open list file");
}
