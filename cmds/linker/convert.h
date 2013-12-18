/*> convert.h <*/
/*---------------------------------------------------------------------------*/

#ifndef __convert_h
#define __convert_h

/*---------------------------------------------------------------------------*/

#include "aof.h"        /* AOF file structures and manifests */
#include "chunkfmt.h"   /* Chunk file structures and manifests */
#include "chunkfls.h"   /* Chunk file manipulating routines */

  
/*---------------------------------------------------------------------------*/
/* s_dataref
 * ---------
 * This structure is used to hold information about a single data
 * reference point within the code.
 */
typedef struct s_dataref
{
 struct s_dataref *next ;
 int32             symindex ; /* symbol table index of reference */
 unsigned32        offset ;   /* required offset from that symbol */
 unsigned32        areanum ;  /* area number of symbol (if required) */
 unsigned32        destreg ;  /* destination register for required address */
 unsigned32        codepos ;  /* return position in the code area */
} s_dataref ;

/*---------------------------------------------------------------------------*/
/* s_aof
 * -----
 * This structure is used to hold all the information required to
 * process an AOF file. Most of the information is cached within the
 * "open_aof" call. It is then used by the "convert_aof" function to
 * process the data.
 */
typedef struct s_aof
{
 int         be ;       /* TRUE if big-endian file */
 char       *fname ;    /* filename of this object */
 ChunkFile  *cf ;       /* cached ChunkFile descriptor */
 cf_header  *cfhdr ;    /* cached ChunkFile header */
 aof_header *aofhdr ;   /* cached AOF header structure */
 aof_symbol *aofsymt ;  /* cached AOF symbol table */
 char       *aofstrt ;  /* cached AOF string table */
 char       *aofareas ; /* cached AOF Code/Data areas */
 unsigned32  codesize ; /* size of the code generated */
 s_dataref  *drefs ;    /* list of data references */
} s_aof ;

/*---------------------------------------------------------------------------*/
/* open_aof
 * --------
 * This function should be called to check and cache the information
 * contained in the specified AOF file. The structure returned can
 * then be passed directly to the "convert_aof" function for
 * processing.
 *
 * in:  fname : a NUL terminated ASCII filename for the AOF file.
 * out: pointer to a malloc'ed AOF description structure.
 */
extern s_aof *open_aof(char *fname) ;

/*---------------------------------------------------------------------------*/
/* convert_aof
 * -----------
 * This function performs the work of converting a cached AOF
 * description (in "adesc") into a Helios object (placed into
 * "outfile").
 *
 * in:  adesc   : AOF description structure.
 *      outfile : a NUL terminated ASCII filename to hold the Helios object.
 *      dolib   : boolean TRUE if a Helios Library object should be generated.
 *      tiny    : boolean TRUE if short data access sequences should be used.
 *      kernel  : boolean TRUE if a Helios Kernel object should be generated.
 * out: size in bytes of the code and data generated.
 */
extern unsigned32 convert_aof(s_aof *adesc,char *outfile,int dolib,int tiny,int kernel) ;

/*---------------------------------------------------------------------------*/
/* check_symbol
 * ------------
 * This function must be provided by the support code. When a piece of
 * code requires the address of an external symbol it is not known
 * whether the symbol is a code or data one. This call allows an external
 * routine to explicitly identify code symbols.
 *
 * in:  symname : NUL terminated ASCII symbol name to be checked.
 * out: boolean TRUE if the given symbol is a code symbol, FALSE if a data
 *      symbol or undefined.
 */
extern int check_symbol(char *symname) ;

/*---------------------------------------------------------------------------*/

#endif /* __convert_h */

/*---------------------------------------------------------------------------*/
/*> EOF convert.h <*/



