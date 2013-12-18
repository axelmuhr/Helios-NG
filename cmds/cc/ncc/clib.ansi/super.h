/* -> H.Super
 *
 *      Brazil OS specific functions
 *      Copyright (C) Acorn Computers Ltd., 1988
 *
 */

/*****************************************************************************/
/* The following #defines allow users to have optional bra_ prefixes to
   function and typedef names, to avoid possible name clashes */

#ifdef BRAZIL_OLD_NAMES

#define bra_reg_set reg_set
#define bra_osfile_block osfile_block
#define bra_osgbpb_block osgbpb_block

#define bra_swi swi
#define bra_swie swie
#define bra_osbyte osbyte
#define bra_osword osword
#define bra_osgbpb osgbpb
#define bra_osfile osfile
#define bra_osargs osargs
#define bra_osfind osfind

#endif

/*****************************************************************************/
/* First structure def's */

typedef struct bra_reg_set
{
        int r[10];               /* only r0 - r9 matter for swi's */
} bra_reg_set;

typedef struct bra_osfile_block
{
        int action;             /* action or object type if output data */
        char * name;            /* pointer to filename or pathname */
        int loadaddr, execaddr; /* load, exec addresses */
        int start, end;         /* start address/length, end add./attributes */
        int reserved[4];        /* space to allow treatment as reg_block */
} bra_osfile_block;

typedef struct bra_osgbpb_block
{
        int action;             /* specifies action of osgbpb */
        int file_handle;        /* file handle, but may be used as a char *
                                 * pointing to wildcarded dir-name */
        void * data_addr;       /* memory address of data */
        int number, seq_point, buf_len;
        char * wild_fld;        /* points to wildcarded filename to match */
        int reserved[3];        /* space to allow treatment as reg_block */
} bra_osgbpb_block;

/*****************************************************************************/
/* Now the actual functions */

bra_reg_set bra_swi(int, bra_reg_set *); /* general access to swi routines */

/* The following functions return FALSE if an error has occurred,
 * otherwise return TRUE (1) */

int bra_swie(int, bra_reg_set *); /* as swi, but 1 reg. set, return FALSE
                                   * if error detected */

int bra_osbyte(bra_reg_set *);

int bra_osword(int,void *);

int bra_osgbpb(bra_osgbpb_block *);

int bra_osfile(bra_osfile_block *);

int bra_osargs(bra_reg_set *);

int bra_osfind(bra_reg_set *);


/******************************************************************************

        Some useful #defines of SWI numbers

******************************************************************************/

/* Mask with SWI number to make error return with V set - Arthur feature */

#define XOS_MASK              0x00020000

/* Brazil and Arthur SWI's */

#define OS_WriteC             0x00000000
#define OS_WriteS             0x00000001
#define OS_Write0             0x00000002
#define OS_NewLine            0x00000003
#define OS_ReadC              0x00000004
#define OS_CLI                0x00000005
#define OS_Byte               0x00000006
#define OS_Word               0x00000007
#define OS_File               0x00000008
#define OS_Args               0x00000009
#define OS_BGet               0x0000000A
#define OS_BPut               0x0000000B
#define OS_GBPB               0x0000000C
#define OS_Find               0x0000000D
#define OS_ReadLine           0x0000000E
#define OS_Control            0x0000000F
#define OS_GetEnv             0x00000010
#define OS_Exit               0x00000011
#define OS_SetEnv             0x00000012
#define OS_IntOn              0x00000013
#define OS_IntOff             0x00000014
#define OS_CallBack           0x00000015
#define OS_EnterOS            0x00000016
#define OS_BreakPt            0x00000017
#define OS_BreakCtrl          0x00000018
#define OS_UnusedSWI          0x00000019
#define OS_UpdateMEMC         0x0000001A
#define OS_SetCallBack        0x0000001B
#define OS_Mouse              0x0000001C

#define OS_WriteI,         0x100

#define OS_UserSWI,        0x200

