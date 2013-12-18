/*
 * dumpa.c PAB 20/3/89
 *
 * Dump the contents of an ARM BSD a.out file in human readable form.
 *
 */

#include <stdio.h>
#include <a.out.h>
#include <stab.h>

/* macros to determine the file position of different segments in a.out */
#define TextREL(x) (N_TXTOFF(x) + (x).a_text+(x).a_data)
#define DataREL(x) (N_TXTOFF(x) + (x).a_text+(x).a_data + (x).a_trsize)
#define SYMTAB(x) (N_TXTOFF(x) + (x).a_text+(x).a_data + (x).a_trsize + \
	(x).a_drsize)
#define STRTAB(x) (N_TXTOFF(x) + (x).a_text+(x).a_data + (x).a_trsize + \
	(x).a_drsize + (x).a_syms)


main (argc,argv)
int	argc;
char	**argv;
{
	FILE	*fp;
	struct exec_header ohead;
	int	i;
	
	if (argc != 2)
	{
		printf("Usage: dumpa a.out.filename\n");
		exit(1);
	}
	if ((fp=fopen(argv[1],"rb")) == NULL)
	{
		printf("dumpa: Cannot open %s\n",argv[1]);
		exit(2);
	}

	if(fread((char *)&i, sizeof(unsigned long), 1, fp) != 1)
	{
		printf("dumpa: Error while reading a.out magic\n");
		exit(3);
	}

	fseek(fp,0,0);
	if (i == OMAGIC)
	{
		if(fread((char *)&ohead, sizeof(ohead.a_exec), 1, fp) != 1)
		{
			printf("dumpa: Error while reading a.out header\n");
			exit(3);
		}
	}
	else
		if((i=fread((char *)&ohead, sizeof(ohead), 1, fp)) != 1)
		{
			printf("dumpa: Error while reading a.out header\n");
			exit(3);
		}

	printf("\na.out header contents:\n");
	printf("----------------------\n\n");
	printf("Type of a.out: ");
	switch (i)
	{
		case OMAGIC:
		printf("ld object - impure\n");
		break;

		case IMAGIC:
		printf("IMAGIC: Impure demand load format\n");
		break;

		case SPOMAGIC:
		printf("SPOMAGIC: Object header with shared library\n");
		break;

		case SLOMAGIC:
		printf("SLOMAGIC: Reference to a shared library with library 'overflow' text\n");
		break;

		case QMAGIC:
		printf("QMAGIC: Squeezed demand paged\n");
		break;

		case SPZMAGIC:
		printf("SPZMAGIC: Program that uses shared lib\n");
		break;

		case SPQMAGIC:
		printf("SPQMAGIC: Sqeezed program that uses shared lib\n");
		break;

		case SLZMAGIC:
		printf("SLZMAGIC: Shared lib part of program\n");
		break;

		case SLPZMAGIC:
		printf("SLPZMAGIC: Shared lib that references another\n");
		break;

		default:
		printf("Warning: a.out Magic number is of an unknown type\n");
		break;
	}
	
/* old header */
	printf("Magic: %ld %#lx %#lo\n",i,i);
	printf("Size of text seg: %ld\n",ohead.a_exec.a_text);
	printf("Size of initialised data: %ld\n",ohead.a_exec.a_data);
	printf("Size of bss (noninit data): %ld\n",ohead.a_exec.a_bss);
	printf("Size of symbol table: %ld\n",ohead.a_exec.a_syms);
	printf("Entry point:%ld\n",ohead.a_exec.a_entry);
	printf("Size of text seg relocation: %ld\n",ohead.a_exec.a_trsize);
	printf("Size of data seg relocation: %ld\n",ohead.a_exec.a_drsize);
/* new header */
	if(i != OMAGIC)
	{
		printf("Version: %s\n",ohead.a_version.v_version);
		printf("Number of squeezed type 4 items: %ld\n",ohead.a_sq4items);
		printf("Number of squeezed type 3 items: %ld\n",ohead.a_sq3items);
		printf("Size of squeezed type 4 items: %ld\n",ohead.a_sq4size);
		printf("Size of squeezed type 3 items: %ld\n",ohead.a_sq3size);
		printf("Last entry in type 4 table (check): %ld\n",ohead.a_sq4last);
		printf("Last entry in type 3 table (check): %ld\n",ohead.a_sq3last);
		printf("Link time: %ld\n",ohead.a_timestamp);
		printf("Shared library timestamp: %ld\n",ohead.a_shlibtime);
		printf("Shared library name: %s\n",ohead.a_shlibname);
	}

	if (ohead.a_exec.a_trsize + ohead.a_exec.a_drsize == 0)
	{
		printf("\nNo relocation required for this file\n");
	}
	else
	{
		struct relocation_info *relinfo;
		int	relitems;

		if (ohead.a_exec.a_trsize != 0)
		{
/* text relocation*/
			printf("\nText relocation information:\n");
			printf("----------------------------\n");
			if (fseek(fp,TextREL(ohead.a_exec),0))
			{
				printf("dumpa: seek failure while reading reloc\n");
			}
			relinfo = (struct relocation_info *)malloc(ohead.a_exec.a_trsize);
			relitems = ohead.a_exec.a_trsize/sizeof(struct relocation_info);

			if (fread((char *)relinfo,sizeof(struct relocation_info),relitems,fp) != relitems)
			{
				printf("dumpa: Error reading relocation info\n");
				exit(6);
			}
			printrel(relinfo,relitems);
			free(relinfo);
		}
		else
			printf("No text relocation\n");

/* data relocation*/
		if (ohead.a_exec.a_drsize != 0)
		{
			printf("\nData relocation information:\n");
			printf("----------------------------\n");
			fseek(fp,DataREL(ohead.a_exec),0);

			relinfo = (struct relocation_info *)malloc(ohead.a_exec.a_drsize);
			relitems = ohead.a_exec.a_drsize/sizeof(struct relocation_info);

			if (fread((char *)relinfo,sizeof(struct relocation_info),relitems,fp) != relitems)
			{
				printf("dumpa: Error reading relocation info\n");
				exit(6);
			}
			printrel(relinfo,relitems);
			free(relinfo);
		}
		else
			printf("No data relocation\n");
	}

/* symbol table */
	if (ohead.a_exec.a_syms == 0)
		printf("No symbol table present\n");
	else
	{
		int symitems, strsize;
		struct nlist *syminfo;
		char *strinfo;

		syminfo = (struct nlist *)malloc(ohead.a_exec.a_syms);
		symitems = ohead.a_exec.a_syms/sizeof(struct nlist);

		readsyms(syminfo,symitems,SYMTAB(ohead.a_exec),fp);

		if(fread(&strsize,sizeof(long),1,fp) != 1)
		{
			printf("dumpa: error reading string table size\n");
			exit(9);
		}
		strinfo = (char *)malloc(strsize);
		if(strsize <= 4) /* size includes itself */
			printf("No string table is present\n");
		else
			readstrs(strinfo,strsize,STRTAB(ohead.a_exec),fp);
		printsyms(syminfo,symitems,strinfo);
		free(syminfo);
		free(strinfo);
	}
}

printrel(relinfo,relitems)
struct relocation_info *relinfo;
int relitems;
{
	int item;

	for (item = 1; item <= relitems; item++)
	{
		printf("\nRelocation item %d\n",item);
		printf("Relocation address %x\n",relinfo[item-1].r_address);
		printf("Local symbol number %d\n",relinfo[item-1].r_symbolnum);
		printf("Size of: ");
		switch(relinfo[item-1].r_length)
		{
		case 0:
			printf("byte\n");
			break;
		case 1:
			printf("word\n");
			break;
		case 2:
			printf("long\n");
			break;
		case 3:
			printf("branch\n");
			break;

		default: /* should never be required!*/
			printf("undefined = %d\n",relinfo[item-1].r_length);
		}
		if (relinfo[item-1].r_pcrel)
			printf("Has already been relocated pc relative\n");
		if (relinfo[item-1].r_extern)
			printf("Doesn't include value of sym refenced\n");
		if (relinfo[item-1].r_neg)
			printf("Negative relocation\n");
	}
}

readsyms(syminfo,symitems,pos,fp)
struct nlist *syminfo;
int symitems;
long pos;
FILE *fp;
{
	if (fseek(fp,pos,0))
	{
		printf("dumpa: seek failure while reading sym tab\n");
	}
	if (fread((char *)syminfo,sizeof(struct nlist),symitems,fp) != symitems)
	{
		printf("dumpa: Error reading symbol table\n");
		exit(8);
	}
}

readstrs(strinfo,strsize,pos,fp)
char *strinfo;
int strsize;
long pos;
FILE *fp;
{
	if (fseek(fp,pos,0))
	{
		printf("dumpa: seek failure while reading string tab\n");
	}
	if (fread(strinfo,1,strsize,fp) != strsize)
	{
		printf("dumpa: Error reading string table\n");
		exit(10);
	}
}

printsyms(syminfo,symitems,strinfo)
struct nlist *syminfo;
int symitems;
char *strinfo;
{
	int item;

	printf("\nSymbol Table:\n");
	printf("-------------\n");

	for (item = 0; item < symitems; item++)
	{
		printf("\nSymbol No: %d\n",item+1);
		{
			char *name=&strinfo[syminfo[item].n_un.n_strx];

			if (name == NULL)
				printf("Symbol has no name\n");
			else
				printf("Name: %s\n",name);
		}
		printf("n_type: ");
		switch (syminfo[item].n_type & N_TYPE)
		{
			case N_UNDF:
			printf("Undefined");
			break;

			case N_ABS:
			printf("Absolute");
			break;

			case N_TEXT:
			printf("Text");
			break;

			case N_DATA:
			printf("Data");
			break;

			case N_BSS:
			printf("BSS");
			break;

			case N_COMM:
			printf("Common (internal to ld)");
			break;

			case N_FN:
			printf("File name symbol");
			break;

			default:
			printf("UNKNOWN n_type");
		}
		if (syminfo[item].n_type & N_EXT)
			printf(" - Externally defined (global)\n");
		else
			putchar('\n');

		switch(syminfo[item].n_type & N_STAB)
		{
			case N_GSYM:
			printf("Stab = N_GSYM - global symbol: name,,0,type,0\n");
			break;

			case N_FNAME:
			printf("Stab = N_FNAME - procedure name (f77 kludge): name,,0\n");
			break;

			case N_FUN:
			printf("Stab = N_FUN - procedure: name,,0,linenumber,address\n");
			break;

			case N_STSYM:
			printf("Stab = N_STSYM - static symbol: name,,0,type,address\n");
			break;

			case N_LCSYM:
			printf("Stab = N_LCSYM - .lcomm symbol: name,,0,type,address\n");
			break;

			case N_RSYM:
			printf("Stab = N_RSYM - register sym: name,,0,type,register\n");
			break;

			case N_SLINE:
			printf("Stab = N_SLINE - src line: 0,,0,linenumber,address\n");
			break;

			case N_SSYM:
			printf("Stab = N_SSYM - structure elt: name,,0,type,struct_offset\n");
			break;

			case N_SO:
			printf("Stab = N_SO - source file name: name,,0,0,address\n");
			break;

			case N_LSYM:
			printf("Stab = N_LSYM - local sym: name,,0,type,offset\n");
			break;

			case N_SOL:
			printf("Stab = N_SOL - #included file name: name,,0,0,address\n");
			break;

			case N_PSYM:
			printf("Stab = N_PSYM - parameter: name,,0,type,offset\n");
			break;

			case N_ENTRY:
			printf("Stab = N_ENTRY - alternate entry: name,linenumber,address\n");
			break;

			case N_LBRAC:
			printf("Stab = N_LBRAC - left bracket: 0,,0,nesting level,address\n");
			break;

			case N_RBRAC:
			printf("Stab = N_RBRAC - right bracket: 0,,0,nesting level,address\n");
			break;

			case N_BCOMM:
			printf("Stab = N_BCOMM - begin common: name,,\n");
			break;

			case N_ECOMM:
			printf("Stab = N_ECOMM - end common: name,,\n");
			break;

			case N_ECOML:
			printf("Stab = N_ECOML - end common (local name): ,,address\n");
			break;

			case N_LENG:
			printf("Stab = N_LENG - second stab entry with length information\n");
			break;

			case N_PC:
			printf("Stab = N_PC - global pascal symbol: name,,0,subtype,line\n");
			break;

			default:
			printf("No stab bits set\n");
		}

		printf("n_type: %#x, n_other: %#x, n_desc: %#x\n",
			syminfo[item].n_type, syminfo[item].n_other,
			syminfo[item].n_desc);
		printf("Value of symbol (or offset): %#x\n",syminfo[item].n_value);
	}
}
