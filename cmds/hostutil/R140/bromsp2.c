From charles@oblique Fri Feb 15 18:02:01 1991
Return-Path: <charles@oblique>
Received: from oblique.abcl.co.uk by server.abcl.co.uk (5.51/A-01)
	id AA01572; Fri, 15 Feb 91 18:01:53 GMT
Received: by oblique.abcl.co.uk (5.51/A-01)
	id AA00287; Fri, 15 Feb 91 18:01:47 GMT
Date: Fri, 15 Feb 91 18:01:47 GMT
From: charles@oblique (Charles Reindorf)
Message-Id: <9102151801.AA00287@oblique.abcl.co.uk>
To: jamie@oblique
Status: R

/*------------------------------------------------------------------------*/
/*                                                               bromsp2  */
/*------------------------------------------------------------------------*/

/* This utility converts a ROM image file to make it suaitable to program */
/*    16-bit wide PROMS for use in AB1                                    */

/*------------------------------------------------------------------------*/
/*                                                          Header Files  */
/*------------------------------------------------------------------------*/

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/*------------------------------------------------------------------------*/
/*                                                                main()  */
/*------------------------------------------------------------------------*/

int main(int argc,char **argv)
{  FILE *fi;
   FILE *u0,*l0,*u1,*l1;
   static char nm[1000];
   static int   *ib;
   static short *ob;
   long qty;
   int    i;

   if(argc!=2)
   {  printf("Format : bromsp2 <file-name>\n");
      return 1;
   }
   
   printf("Opening : %s : to read\n",argv[1]);
   fi = fopen(argv[1],"rb");
   if(fi==NULL)
   {  printf("Failed to open \"%s\" for reading\n",argv[1]);
      return 1;
   } else printf("Open was sucessful\n");

   fseek(fi,0L,SEEK_END);
   if(ftell(fi)!=2*1024*1024)
   {  printf("File \"%s\" is not 2 Mega-Bytes in size\n",argv[1]);
      fclose(fi);
      return 1;
   }
   fseek(fi,0L,SEEK_SET);
   
   sprintf(nm,"%s0L",argv[1]);
   l0 = fopen(nm,"wb");
   if(l0==NULL)
   {  printf("Failed to open \"%s\" to write to\n",nm);
      perror(NULL);
      return 1;
   } else printf("Opened : %s : for writing\n",nm);

   sprintf(nm,"%s1L",argv[1]);
   l1 = fopen(nm,"wb");
   if(l1==NULL)
   {  printf("Failed to open \"%s\" to write to\n",nm);
      perror(NULL);
      return 1;
   } else printf("Opened : %s : for writing\n",nm);

   sprintf(nm,"%s0U",argv[1]);
   u0 = fopen(nm,"wb");
   if(u0==NULL)
   {  printf("Failed to open \"%s\" to write to\n",nm);
      perror(NULL);
      return 1;
   } else printf("Opened : %s : for writing\n",nm);

   sprintf(nm,"%s1U",argv[1]);
   u1 = fopen(nm,"wb");
   if(u1==NULL)
   {  printf("Failed to open \"%s\" to write to\n",nm);
      perror(NULL);
      return 1;
   } else printf("Opened : %s : for writing\n",nm);

   ib = calloc(1024*256,sizeof(int));
   if(ib==NULL)
   {  printf("Failed to allocate a mega-byte input buffer\n");
      return 1;
   }
   
   ob = calloc(1024*256,sizeof(short));
   if(ob==NULL)
   {  printf("Failed to allocate 0.5 Mega-Byte output buffer\n");
      return 1;
   }
   
   printf("Reading first mega-byte of input file\n");
   qty = fread(ib,1,1024*1024,fi);
   if(qty<1024*1024)
   {  printf("Failed to read a mega-byte. Read %ld (=0x%lX) bytes\n",
             qty,qty);
      perror(NULL);
      return 1;
   }
   printf("Read was sucessful\n");

   for(i=0;i<1024*256;i++) ob[i] = ib[i]&0xFFFF;
   printf("Writing out file : %s0L\n",argv[1]);
   qty = fwrite(ob,1,1024*512,l0);
   if(qty<1024*512)
   {  printf("Failed to write half a mega-byte. Wrote %ld (=0x%lX) bytes\n",
                  qty,qty);
      perror(NULL);
      return 1;
   }
   printf("Write was sucessful\n");
   
   for(i=0;i<1024*256;i++) ob[i] = (ib[i]>>16)&0xFFFF;
   printf("Writing out file : %s0U\n",argv[1]);
   qty = fwrite(ob,1,1024*512,u0);
   if(qty<1024*512)
   {  printf("Failed to write half a mega-byte. Wrote %ld (=0x%lX) bytes\n",
                  qty,qty);
      perror(NULL);
      return 1;
   }
   printf("Write was sucessful\n");
   
   printf("Reading second mega-byte of input file\n");
   qty = fread(ib,1,1024*1024,fi);
   if(qty<1024*1024)
   {  printf("Failed to read a mega-byte. Read %ld (=0x%lX) bytes\n",
             qty,qty);
      perror(NULL);
      return 1;
   }
   printf("Read was sucessful\n");

   for(i=0;i<1024*256;i++) ob[i] = ib[i]&0xFFFF;
   printf("Writing out file : %s1L\n",argv[1]);
   qty = fwrite(ob,1,1024*512,l1);
   if(qty<1024*512)
   {  printf("Failed to write half a mega-byte. Wrote %ld (=0x%lX) bytes\n",
                  qty,qty);
      perror(NULL);
      return 1;
   }
   printf("Write was sucessful\n");
   
   for(i=0;i<1024*256;i++) ob[i] = (ib[i]>>16)&0xFFFF;
   printf("Writing out file : %s1U\n",argv[1]);
   qty = fwrite(ob,1,1024*512,u1);
   if(qty<1024*512)
   {  printf("Failed to write half a mega-byte. Wrote %ld (=0x%lX) bytes\n",
                  qty,qty);
      perror(NULL);
      return 1;
   }
   printf("Write was sucessful\n");
   
   return 0;
}      

