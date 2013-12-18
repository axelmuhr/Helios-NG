#include <root.h>
#include <memory.h>
#include <stdio.h>
#include "/hsrc/nucleus/ARM/ram.h"

bool waiting = FALSE;

void Pause()
{
	if (waiting)
		puts("Hit return to loop...");getchar();
}

int main(int argc, char **argv)
{
	int i;
	RootStruct *root = GetRoot();
	MIInfo *mi = root->MISysMem;
	RRDCommon *rrdc;
	Memory *m;


	if(argc > 1 && !strcmp(argv[1],"-p"))
		waiting = TRUE;

	printf("Memory Indirection Information:\n");

	forever {
		word	tablesize = mi->MITableSize;
		word	*mitable = (word *)mi->MITable;

		if (tablesize == 0) {
			printf("memory indirection table has not been initialised yet\n");
			break;
		}

		printf("\nMITable = %8x\n",(word)mitable);
		printf("MITableSize = %d\n",tablesize);
		printf("freehandles = %d\n",mi->MIFreeSlots);

		for ( i=0; i < tablesize; i++) {
			rrdc = (RRDCommon *)mitable[i];

			if (rrdc == NULL)
				continue; /* unused slot in MI table */

			printf("Block: %d @ %#x size: %d",i,(word)rrdc,rrdc->TotalSize);

			m = ((Memory *)rrdc)-1;
			if(m->Size & Memory_Size_Reloc)
				puts(" is unlocked");
			else
				puts(" is LOCKED");
			
			switch (rrdc->Magic) {
				case RRDFileMagic:
				{
					RRDFileBlock *rrdf = (RRDFileBlock *)rrdc;
	
					printf("Type: File,  file Id: %d, name: /ram/sys/%s\n",rrdf->FileId,rrdf->FullName);
					break;		
				}
				case RRDDirMagic:
				{
					RRDDirBlock *rrdd = (RRDDirBlock *)rrdc;
	
					printf("Type: Directory, name: /ram/sys/%s\n",rrdd->FullName);
					break;		
				}

				case RRDBuffMagic:
				{
					RRDBuffBlock *rrdb = (RRDBuffBlock *)rrdc;
	
					printf("Type: File Buffer, file Id: %d PosInFile: %d\n",rrdb->FileId,rrdb->Pos);
					break;		
				}

				case RRDSymbMagic:
				{
					RRDSymbBlock *rrds = (RRDSymbBlock *)rrdc;
		
					printf("Type: Symbolic Link, /ram/sys/%s, -> %s\n",rrds->FullName,rrds->FullName + strlen(rrds->FullName) + 1);
					break;
				}
	
				default:
					printf("Unknown block type! (%#x)\n",rrdc->Magic);
			}
			putchar('\n');
		}

		if (!waiting)
			break;
		else
			Pause();
	}
}
