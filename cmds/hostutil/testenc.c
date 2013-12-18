/* test object number encoding
 * PAB 10/3/89
 */

#define ENC_MORE 0x80	/* number encoding - another byte to follow */
#define ENC_NEG  0x40	/* number encoding - number is neg */
#define BOT6BITS 0x3f
#define BOT7BITS 0x7f

main(argc,argv)
int argc;
char **argv;
{
	int i;
	while(--argc)
		writeout(atoi(*(++argv)));
	printf("starting!!!\n");
	for (i = 0x0ff; i >= 255 || i <= 0; i+= 0x0ff )
	{
		if ((i & 0x0ffff) == 0x0ffff)
			printf("ok < %x\n",i);
		writeout(i);
	}
	printf("finished!!!\n");
}

/* Write a number out in Helios encoded format. */
/* This is the same as the encoding of the transputer instruction set. */
/* Only use as many bytes as neccesary to describe a number: */
/* Set top bit of byte if another follows it, set penultimate top bit of */
/* first byte if number is negative, continue setting top bit of byte */
/* ad infinitum until number complete */

unsigned char output[20];
int indx=0;

writeout(n)
int n;
{
	unsigned start = n;
	int i=0;
/*debugprintf("debug +++ number to conv = %x (%d)\n",n,n);*/

	output[indx++] = n < 0 ? enc(-n, ENC_NEG) : enc(n, 0);

	output[indx] = 0;
	indx = 0;

	i = readobjnum(output);
	if (i != start)
	{
		printf("FUCKUP! start = %x (%d) end = %x (%d)\n",start,start,i,i);
		exit(1);
	}

/*
	printf("debug +++ reconv number = %x (%d)\n",i,i);
	for(i=0; output[i] != '\0';i++)
		printf("byte[%d] = %#02.2x, ",(unsigned)i,(unsigned)output[i]);
	putchar('\n');
*/
}


enc(n, nflag)
unsigned n;
int nflag;
{
	if (n>>6 == 0)
		return ((n & BOT6BITS) | nflag);
	output[indx++] = enc(n >> 7, nflag) | ENC_MORE;
	return(n & BOT7BITS);
}

int readobjnum(n)
char	*n;
{
/*debug   int ch = getc(f);*/
   unsigned ch = *n++;
   int nflag  = (ch & ENC_NEG) != 0;
   unsigned r = ch & 0x3f;

   while( (ch & ENC_MORE) != 0 )
   {
      ch = *n++;
      r  = (r << 7) | (unsigned)(ch & 0x7f);
   }
   return nflag? -r: r;
}

