/******************************************************************************
*******************************************************************************
	A George Kalwitz Production, 1989
*******************************************************************************
******************************************************************************/

#include	"stdio.h"
#include	"string.h"
#include	"ieee8023.h"
#include	"params.h"
#include	"cvars.h"
#include	"misc.h"
#include	"err_cods.h"
#include	"eth_data.h"
#include	"board_id.h"
#include	"getcnfg.h"

extern	CNFG_Adapter	config_table;

/****  this module is used to build multiple executables...thus the #if's ****/
#if	defined(DIAG)

#include	"diagdata.h"
#include	"diagdefs.h"

#endif
#if	defined(SAMPLDRV)

#include	"sdrvvars.h"

#endif

int batcharg (num_args, arg_ptr)
int num_args;
char *arg_ptr[];
{
    int count,
        index;
    long MACaddr;
    char *data_addr;

    long string_to_hex (char *c);

    count = 1;
    ++arg_ptr;

    batch = FALSE;
    batch_type = 0;

    while (count < num_args)
    {
        if (!(strnicmp ("base", (*arg_ptr) + 1, 4)))
        {
            #if	defined(SAMPLDRV)
            if (micro_chnl)
            {
                printf ("\nThis is a Micro Channel machine...\n");
                printf ("     Use /slot:n to specify adapter.\n");
                return (BASE_IO_ERR);
            }
            #endif
            data_addr = &(*arg_ptr)[6];
            baseio = (short) string_to_hex (data_addr);
	    #if	defined(ENGR)

            #else
              config_table.cnfg_base_io = baseio;
    	      config_table.cnfg_bus = micro_chnl;
              config_table.cnfg_slot = channel_pos + 1;
              if (WDM_GetCnfg (&config_table) == -1)
              {
                  printf ("\nNo board seems to be at baseio = %x.\n", baseio);
                  return (BASE_IO_ERR);
              }
            #endif
            #if	defined(DIAG)
              batch_baseio = baseio;
              chosen = 1;
            #endif
            count++;
            ++arg_ptr;
            batch = TRUE;
            batch_type |= (B_SPEC | B_PARM);
        }
        else if (!(strnicmp ("irq", (*arg_ptr) + 1, 3)))
        {
            if (strlen (*arg_ptr) > 6)
            {
                irq = 0;
                for (index = 5; index < 7; index++)
                    irq = 10 * irq + (*arg_ptr)[index] - '0';
            }
            else
                irq =  (*arg_ptr)[5] - '0';
            if ((irq < MIN_IRQ) || (irq > MAX_IRQ))
            {
                printf ("\nSpecified irq value is out of range.\n");
                return (IRQ_OUT_OF_RANGE_ERR);
            }
            count++;
            ++arg_ptr;
            batch = TRUE;
            batch_type |= B_PARM;
        }
        else if (!(strnicmp ("ram", (*arg_ptr) + 1, 3)))
        {
            data_addr =  &((*arg_ptr)[5]);
            RAMbase = ((string_to_hex (data_addr)) << 12);
            if ((RAMbase > (long)MAX_RAMBASE) || (RAMbase < (long)MIN_RAMBASE))
            {
                printf ("\nRAM base address is out of range.\n");
                return (RAM_OUT_OF_RANGE_ERR);
            }
            #if	defined(DIAG)
            if (is_ram_here (RAMbase, 0x2000))
            {
                printf ("\nRAM base address conflicts with some other\n");
                printf ("     RAM in your system.\n");
                return (RAM_AT_RAM_ERR);
            }
            if (is_rom_here (RAMbase, 0x2000))
            {
                printf ("\nRAM base address conflicts with some ROM\n");
                printf ("     in your system.\n");
                return (ROM_AT_RAM_ERR);
            }
            #endif
            count++;
            ++arg_ptr;
            batch = TRUE;
            batch_type |= B_PARM;
        }
        #if	defined(DIAG)
        else if (!(strnicmp ("rom", (*arg_ptr) + 1, 3)))
        {
            data_addr =  &((*arg_ptr)[5]);
            ROMbase = ((string_to_hex (data_addr)) << 12);
            if ((ROMbase > (long)MAX_ROMBASE) || (ROMbase < (long)MIN_ROMBASE))
            {
                printf ("\nROM base address is out of range.\n");
                return (ROM_OUT_OF_RANGE_ERR);
            }
            switch ((*(char far *)(ROMbase + 2)) & 0xFF)
            {
                case 0x20:
                    ROMsize_index = 1;
                    break;
                case 0x40:
                    ROMsize_index = 2;
                    break;
                case 0x80:
                    ROMsize_index = 3;
                    break;
                default:
                    printf ("\nUnable to read ROM size out of chip.\n");
                    printf ("    Perhaps the ROM base address is invalid.\n");
                    return (ROM_READ_ERR);
                    break;
            }
            ROMsize = ROMsize_vals[ROMsize_index];
            count++;
            ++arg_ptr;
            batch = TRUE;
            batch_type |= B_PARM;
        }
        #endif
        #if	defined(DIAG)
        else if (!(strnicmp ("adapter", (*arg_ptr) + 1, 7)))
        {
            adapter_num =  (*arg_ptr)[9] - '0';
            if (adapter_num == 0)
                adapter_num++;			/* one justified */
            if (!(find_nth_board(adapter_num)))
            {
                printf ("\nUnable to find adapter number %d\n", adapter_num);
                return (NO_ADAPTER_N_ERR);
            }
            batch_baseio = baseio;
            count++;
            ++arg_ptr;
            batch = TRUE;
            batch_type |= B_SPEC;
        }
        #endif
        else if (!(strnicmp ("slot", (*arg_ptr) + 1, 4)))
        {
            micro_chnl = 1;		/* force if user says so */
            channel_pos =  ((*arg_ptr)[6] - '0') - 1;
         #if	defined(ENGR)

         #else
    	    config_table.cnfg_bus = micro_chnl;
            config_table.cnfg_slot = channel_pos + 1;
            if (WDM_GetCnfg (&config_table) == -1)
            {
                printf ("\nNo board in slot number %d.\n", channel_pos+1);
                return (NO_BOARD_IN_SLOT_ERR);
            }
         #endif
         #if	defined(DIAG)
            batch_baseio = config_table.cnfg_base_io;
            chosen = 1;
         #endif
            count++;
            ++arg_ptr;
            batch = TRUE;
            batch_type |= B_SPEC;
        }
        #if	defined(DIAG)
        else if (!(strnicmp ("send", (*arg_ptr) + 1, 4)))
        {
            data_addr = &(*arg_ptr)[5];
            if ((*data_addr == ':') || (*data_addr == '='))
            {
                data_addr++;				/* skip ':' char */
                for (index = 0; index < 6; index++)
                {
                    resp_addr[index] = (char) string_to_hex (data_addr);
                    data_addr += 2;
                }
                responder_found = 1;
            }
            count++;
            ++arg_ptr;
            batch = TRUE;
            batch_type |= B_SEND;
        }
        #endif
        #if	defined(DIAG)
        else if (!(strnicmp ("respond", (*arg_ptr) + 1, 7)))
        {
            count++;
            ++arg_ptr;
            batch = TRUE;
            batch_type |= B_RESP;
        }
        #endif
        #if	defined(DIAG)
        else if (!(strnicmp ("test", (*arg_ptr) + 1, 4)))
        {
            count++;
            ++arg_ptr;
            batch = TRUE;
            batch_type |= B_TEST;
        }
        else if (!(strnicmp ("oem", (*arg_ptr) + 1, 3)))
        {
            printf ("\nOEM Node Address = ");
            for (index = 0; index < 3; index++)
                printf ("%02X ", (*(OEM_node_addr+index) & 0xFF));
            printf ("\n");
            printf ("OEM Card ID = ");
            for (index = 0; index < 2; index++)
                printf ("%02X", (*(OEM_card_id+index) & 0xFF));
            printf ("\n");
            count++;
            ++arg_ptr;
            return (QUICK_EXIT_CODE);
        }
        #endif
        else if (!(strnicmp ("author", (*arg_ptr) + 1, 6)))
        {
            printf ("\n\n%s\n\n", author_string);
            count++;
            ++arg_ptr;
            return (QUICK_EXIT_CODE);
        }
        else if (!(strnicmp ("coauthor", (*arg_ptr) + 1, 8)))
        {
            printf ("\n\n%s\n\n", coauthor_string);
            count++;
            ++arg_ptr;
            return (QUICK_EXIT_CODE);
        }
        else if (!(strnicmp ("v", (*arg_ptr) + 1, 1) ) )
        {
            verscpy();
            count++;
            ++arg_ptr;
            return (QUICK_EXIT_CODE);
        }
        else if (!(strnicmp ("?", (*arg_ptr) + 1, 1)))
        {
            helpscr();
            count++;
            ++arg_ptr;
            return (QUICK_EXIT_CODE);
        }
        else if (!(strnicmp ("h", (*arg_ptr) + 1, 1)))
        {
            helpscr();
            count++;
            ++arg_ptr;
            return (QUICK_EXIT_CODE);
        }
        else
        {
            printf ("\nInvalid command line parameter!\n");
            return (BAD_COMMAND_LINE_ERR);
        }
    }	   /* end of while statement */
    return (NO_ERROR);
}

