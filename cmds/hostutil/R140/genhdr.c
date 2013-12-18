/*> genhdr.c <*/
/*----------------------------------------------------------------------*/
/*				genhdr.c				*/
/*				--------				*/
/*									*/
/* Copyright (c) 1990, Active Book Company, Cambridge, United Kingdom.	*/
/*									*/
/* This program is used to generate the "assembler" header files from	*/
/* the C equivalents. The required C header file should be included and	*/
/* a function added to generate the ".s" file from this.		*/
/*----------------------------------------------------------------------*/

#include <stdio.h>

#define whoami		"genhdr"	/* utility name */
#define maxstring	256		/* maximum string length */

/*----------------------------------------------------------------------*/
/* Include files we wish to generate assembler headers for:		*/

#define __HELIOSARM

#include "/hsrc/include/helios.h"		/* Helios manifests */
#include "/hsrc/include/queue.h"		/* Helios manifests */
#include "/hsrc/include/config.h"		/* config structure info */
#include "/hsrc/include/abcARM/ROMitems.h"	/* ITEMs */
#include "/hsrc/include/abcARM/manifest.h"	/* useful manifests */
#include "/hsrc/include/abcARM/ABClib.h"	/* more useful manifests */
#include "/hsrc/include/abcARM/PCcard.h"	/* external CARD info */

/*----------------------------------------------------------------------*/

void build_ROMitems(char *path,int listing)
{
 FILE *outfile = stdout ; /* default is listing */
 char  filename[maxstring] ;

 if (path[0] == '\0')
  sprintf(filename,"ROMitems.s") ;
 else
  sprintf(filename,"%s/ROMitems.s",path) ;

 /* create the file */
 if (!listing)
  {
   if ((outfile = fopen(filename,"wb")) == NULL)
    {
     fprintf(stderr,"%s: cannot open outfile file \"%s\"\n",whoami,filename) ;
     exit(3) ;
    }
  }

 /* write all the necessary information */
 fprintf(outfile,"\tTTL\tCreated by \"%s\"\t> ROMitems/s\n\n",whoami) ;
 fprintf(outfile,"old_opt\tSETA\t{OPT}\n") ;
 fprintf(outfile,"\tOPT\t(opt_off)\n\n") ;

 fprintf(outfile,"\n; ITEM structure manifests\n") ;
 fprintf(outfile,"ITEMMagic             * &%08X\n",ITEMMagic) ;
 fprintf(outfile,"defaultITEMaccess     * &%08X\n",defaultITEMaccess) ;
 fprintf(outfile,"ITEMhdrROM            * &%08X\n",ITEMhdrROM) ;
 fprintf(outfile,"ITEMhdrRAM            * &%08X\n",ITEMhdrRAM) ;
 fprintf(outfile,"ITEMhdrBRANCH         * &%08X\n",ITEMhdrBRANCH) ;
 fprintf(outfile,"ITEMROMend            * &%08X\n",ITEMROMend) ;
 fprintf(outfile,"ITEMRAMend            * &%08X\n",ITEMRAMend) ;

 fprintf(outfile,"\n; ITEM structures\n") ;
 {
  ITEMstructure *b = (ITEMstructure *)0 ;

  fprintf(outfile,"ITEMID                * &%08X\n",(int)&b->ITEMID) ;
  fprintf(outfile,"ITEMLength            * &%08X\n",(int)&b->ITEMLength) ;
  fprintf(outfile,"OBJECTOffset          * &%08X\n",(int)&b->OBJECTOffset) ;
  fprintf(outfile,"OBJECTLength          * &%08X\n",(int)&b->OBJECTLength) ;
  fprintf(outfile,"ITEMAccess            * &%08X\n",(int)&b->ITEMAccess) ;
  fprintf(outfile,"ITEMDate              * &%08X\n",(int)&b->ITEMDate[0]) ;
  fprintf(outfile,"ITEMExtensions        * &%08X\n",(int)&b->ITEMExtensions) ;
  fprintf(outfile,"ITEMNameLength        * &%08X\n",(int)&b->ITEMNameLength) ;
  fprintf(outfile,"ITEMName              * &%08X\n",(int)&b->ITEMName[0]) ;
 }
 {
  ROMITEMstructure *b = (ROMITEMstructure *)0 ;

  fprintf(outfile,"OBJECTInit            * &%08X\n",(int)&b->OBJECTInit) ;
  fprintf(outfile,"ITEMVersion           * &%08X\n",(int)&b->ITEMVersion) ;
  fprintf(outfile,"sizeof_ROMITEMstruct  * &%08X\n",sizeof(ROMITEMstructure)) ;
 }
 {
  RAMITEMstructure *b = (RAMITEMstructure *)0 ;

  fprintf(outfile,"ITEMCheck             * &%08X\n",(int)&b->ITEMCheck[0]) ;
  fprintf(outfile,"ITEMHdrSeq1           * &%08X\n",(int)&b->ITEMHdrSeq1) ;
  fprintf(outfile,"OBJECTUsed            * &%08X\n",(int)&b->OBJECTUsed[0]) ;
  fprintf(outfile,"OBJECTSize            * &%08X\n",(int)&b->OBJECTSize) ;
  fprintf(outfile,"OBJECTRef             * &%08X\n",(int)&b->OBJECTRef[0]) ;
  fprintf(outfile,"ITEMHdrSeq2           * &%08X\n",(int)&b->ITEMHdrSeq2) ;
  fprintf(outfile,"OBJECTRefLast         * &%08X\n",(int)&b->OBJECTRefLast[0]) ;
  fprintf(outfile,"ITEMSpare1            * &%08X\n",(int)&b->ITEMSpare1) ;
  fprintf(outfile,"ITEMNumber            * &%08X\n",(int)&b->ITEMNumber) ;
  fprintf(outfile,"ITEMRAMhdrend         * &%08X\n",(int)&b->ITEMRAMhdrend[0]) ;
 }

 /* terminate the file cleanly */
 fprintf(outfile,"\n\tOPT\t(old_opt)\n") ;
 fprintf(outfile,"\tEND\n") ;

 /* close the file */
 if (!listing)
  fclose(outfile) ;

 return ;
}

/*----------------------------------------------------------------------*/

void build_manifest(char *path,int listing)
{
 FILE *outfile = stdout ; /* default is listing */
 char  filename[maxstring] ;

 if (path[0] == '\0')
  sprintf(filename,"manifest.s") ;
 else
  sprintf(filename,"%s/manifest.s",path) ;

 /* create the file */
 if (!listing)
  {
   if ((outfile = fopen(filename,"wb")) == NULL)
    {
     fprintf(stderr,"%s: cannot open outfile file \"%s\"\n",whoami,filename) ;
     exit(3) ;
    }
  }

 /* write all the necessary information */
 fprintf(outfile,"\tTTL\tCreated by \"%s\"\t> manifest/s\n\n",whoami) ;
 fprintf(outfile,"old_opt\tSETA\t{OPT}\n") ;
 fprintf(outfile,"\tOPT\t(opt_off)\n\n") ;

 fprintf(outfile,"; -- from \"config.h\" -------------------------\n") ;
 fprintf(outfile,"; IVec indices\n") ;
 fprintf(outfile,"IVecISize             * %d\n",IVecISize) ;
 fprintf(outfile,"IVecKernel            * %d\n",IVecKernel) ;
 fprintf(outfile,"IVecSysLib            * %d\n",IVecSysLib) ;
 fprintf(outfile,"IVecServLib           * %d\n",IVecServLib) ;
 fprintf(outfile,"IVecUtil              * %d\n",IVecUtil) ;
 fprintf(outfile,"IVecABClib            * %d\n",IVecABClib) ;
 fprintf(outfile,"IVecPosix             * %d\n",IVecPosix) ;
 fprintf(outfile,"IVecCLib              * %d\n",IVecCLib) ;
 fprintf(outfile,"IVecFault             * %d\n",IVecFault) ;
 fprintf(outfile,"IVecFPLib             * %d\n",IVecFPLib) ;
 fprintf(outfile,"IVecPatchLib          * %d\n",IVecPatchLib) ;
 fprintf(outfile,"IVecProcMan           * %d\n",IVecProcMan) ;
 fprintf(outfile,"IVecServers           * %d\n",IVecServers) ;
 /* these constants depended DIRECTLY on how the system is built */
 fprintf(outfile,"IVecLoader            * %d\n",IVecServers + 0) ;
 fprintf(outfile,"IVecKeyboard          * %d\n",IVecServers + 1) ;
 fprintf(outfile,"IVecWindow            * %d\n",IVecServers + 2) ;
 fprintf(outfile,"IVecRom               * %d\n",IVecServers + 3) ;
 fprintf(outfile,"IVecRam               * %d\n",IVecServers + 4) ;
 fprintf(outfile,"IVecNull              * %d\n",IVecServers + 5) ;
 fprintf(outfile,"IVecHelios            * %d\n",IVecServers + 6) ;
 fprintf(outfile,"; list terminator\n") ;
 fprintf(outfile,"IVecTotal             * %d\n",IVecServers + 6) ;

 fprintf(outfile,"; -- from \"manifest.h\" -----------------------\n") ;
 fprintf(outfile,"; Timer related manifests\n") ;
 fprintf(outfile,"TickChunk             * &%08X\n",TickChunk) ;
 fprintf(outfile,"TickSize              * &%08X\n",TickSize) ;
 fprintf(outfile,"TicksPerSlice         * &%08X\n",TicksPerSlice) ;

 fprintf(outfile,"; Process priority related manifests\n") ;
 fprintf(outfile,"log2_numpris          * %d\n",log2_numpris) ;
 fprintf(outfile,"NumberPris            * %d\n",NumberPris) ;

 fprintf(outfile,"; ROM and CARD location manifests\n") ;
 fprintf(outfile,"loc_internal          * &%02X\n",loc_internal) ;
 fprintf(outfile,"loc_CARD1             * &%02X\n",loc_CARD1) ;
 fprintf(outfile,"loc_internalFlash     * &%02X\n",loc_internalFlash) ;
 fprintf(outfile,"loc_limit             * &%02X\n",loc_limit) ;


 fprintf(outfile,"; -- from \"ABClib.h\" -------------------------\n") ;
 fprintf(outfile,"\n;RESET information\n") ;
 fprintf(outfile,"ShellBootKey          * %d\n",ShellBootKey) ;

 fprintf(outfile,"EEPROM_ServerID       * %d\n",EEPROM_ServerID) ;
 fprintf(outfile,"EEPROM_ServerIndexDefault  * &%02X\n",EEPROM_ServerIndexDefault) ;
 fprintf(outfile,"EEPROM_ServerIndexIOServer * &%02X\n",EEPROM_ServerIndexIOServer) ;
 fprintf(outfile,"EEPROM_ServerIndexSysROM   * &%02X\n",EEPROM_ServerIndexSysROM) ;
 fprintf(outfile,"EEPROM_ServerIndexROMCard1 * &%02X\n",EEPROM_ServerIndexROMCard1) ;
 fprintf(outfile,"EEPROM_ServerIndexROMCard2 * &%02X\n",EEPROM_ServerIndexROMCard2) ;
 fprintf(outfile,"EEPROM_ServerIndexROMCard3 * &%02X\n",EEPROM_ServerIndexROMCard3) ;
 fprintf(outfile,"EEPROM_ServerIndexROMCard4 * &%02X\n",EEPROM_ServerIndexROMCard4) ;
 fprintf(outfile,"EEPROM_ServerIndexROMCard5 * &%02X\n",EEPROM_ServerIndexROMCard5) ;
 fprintf(outfile,"EEPROM_ServerIndexROMCard6 * &%02X\n",EEPROM_ServerIndexROMCard6) ;
 fprintf(outfile,"EEPROM_ServerIndexROMCard7 * &%02X\n",EEPROM_ServerIndexROMCard7) ;
 fprintf(outfile,"EEPROM_ServerIndexROMCard8 * &%02X\n",EEPROM_ServerIndexROMCard8) ;
 fprintf(outfile,"EEPROM_ServerIndexROMCard9 * &%02X\n",EEPROM_ServerIndexROMCard9) ;
 fprintf(outfile,"EEPROM_ServerIndexSysRAM   * &%02X\n",EEPROM_ServerIndexSysRAM) ;
 fprintf(outfile,"EEPROM_ServerIndexRAMCard1 * &%02X\n",EEPROM_ServerIndexRAMCard1) ;
 fprintf(outfile,"EEPROM_ServerIndexRAMCard2 * &%02X\n",EEPROM_ServerIndexRAMCard2) ;
 fprintf(outfile,"EEPROM_ServerIndexRAMCard3 * &%02X\n",EEPROM_ServerIndexRAMCard3) ;
 fprintf(outfile,"EEPROM_ServerIndexRAMCard4 * &%02X\n",EEPROM_ServerIndexRAMCard4) ;
 fprintf(outfile,"EEPROM_ServerIndexRAMCard5 * &%02X\n",EEPROM_ServerIndexRAMCard5) ;
 fprintf(outfile,"EEPROM_ServerIndexRAMCard6 * &%02X\n",EEPROM_ServerIndexRAMCard6) ;
 fprintf(outfile,"EEPROM_ServerIndexRAMCard7 * &%02X\n",EEPROM_ServerIndexRAMCard7) ;
 fprintf(outfile,"EEPROM_ServerIndexRAMCard8 * &%02X\n",EEPROM_ServerIndexRAMCard8) ;
 fprintf(outfile,"EEPROM_ServerIndexRAMCard9 * &%02X\n",EEPROM_ServerIndexRAMCard9) ;
 fprintf(outfile,"EEPROM_ServerIndexFS       * &%02X\n",EEPROM_ServerIndexFS) ;
 fprintf(outfile,"EEPROM_ServerIndexMSDOSa   * &%02X\n",EEPROM_ServerIndexMSDOSa) ;
 fprintf(outfile,"EEPROM_ServerIndexMSDOSb   * &%02X\n",EEPROM_ServerIndexMSDOSb) ;
 fprintf(outfile,"EEPROM_ServerIndexMSDOSc   * &%02X\n",EEPROM_ServerIndexMSDOSc) ;
 fprintf(outfile,"EEPROM_ServerIndexMSDOSd   * &%02X\n",EEPROM_ServerIndexMSDOSd) ;
 fprintf(outfile,"EEPROM_ServerIndexMSDOSe   * &%02X\n",EEPROM_ServerIndexMSDOSe) ;
 fprintf(outfile,"EEPROM_ServerIndexMSDOSf   * &%02X\n",EEPROM_ServerIndexMSDOSf) ;
 fprintf(outfile,"EEPROM_ServerIndexMSDOSg   * &%02X\n",EEPROM_ServerIndexMSDOSg) ;
 fprintf(outfile,"EEPROM_ServerIndexMSDOSh   * &%02X\n",EEPROM_ServerIndexMSDOSh) ;
 fprintf(outfile,"EEPROM_ServerIndexMSDOSi   * &%02X\n",EEPROM_ServerIndexMSDOSi) ;
 fprintf(outfile,"EEPROM_ServerIndexMSDOSj   * &%02X\n",EEPROM_ServerIndexMSDOSj) ;
 fprintf(outfile,"EEPROM_ServerIndexMSDOSk   * &%02X\n",EEPROM_ServerIndexMSDOSk) ;
 fprintf(outfile,"EEPROM_ServerIndexMSDOSl   * &%02X\n",EEPROM_ServerIndexMSDOSl) ;
 fprintf(outfile,"EEPROM_ServerIndexMSDOSm   * &%02X\n",EEPROM_ServerIndexMSDOSm) ;
 fprintf(outfile,"EEPROM_ServerIndexMSDOSn   * &%02X\n",EEPROM_ServerIndexMSDOSn) ;
 fprintf(outfile,"EEPROM_ServerIndexMSDOSo   * &%02X\n",EEPROM_ServerIndexMSDOSo) ;
 fprintf(outfile,"EEPROM_ServerIndexMSDOSp   * &%02X\n",EEPROM_ServerIndexMSDOSp) ;
 fprintf(outfile,"EEPROM_ServerIndexMSDOSq   * &%02X\n",EEPROM_ServerIndexMSDOSq) ;
 fprintf(outfile,"EEPROM_ServerIndexMSDOSr   * &%02X\n",EEPROM_ServerIndexMSDOSr) ;
 fprintf(outfile,"EEPROM_ServerIndexMSDOSs   * &%02X\n",EEPROM_ServerIndexMSDOSs) ;
 fprintf(outfile,"EEPROM_ServerIndexMSDOSt   * &%02X\n",EEPROM_ServerIndexMSDOSt) ;
 fprintf(outfile,"EEPROM_ServerIndexMSDOSu   * &%02X\n",EEPROM_ServerIndexMSDOSu) ;
 fprintf(outfile,"EEPROM_ServerIndexMSDOSv   * &%02X\n",EEPROM_ServerIndexMSDOSv) ;
 fprintf(outfile,"EEPROM_ServerIndexMSDOSw   * &%02X\n",EEPROM_ServerIndexMSDOSw) ;
 fprintf(outfile,"EEPROM_ServerIndexMSDOSx   * &%02X\n",EEPROM_ServerIndexMSDOSx) ;
 fprintf(outfile,"EEPROM_ServerIndexMSDOSy   * &%02X\n",EEPROM_ServerIndexMSDOSy) ;
 fprintf(outfile,"EEPROM_ServerIndexMSDOSz   * &%02X\n",EEPROM_ServerIndexMSDOSz) ;
 fprintf(outfile,"EEPROM_ServerIndexShell    * &%02X\n",EEPROM_ServerIndexShell) ;

 fprintf(outfile,"\n;CARD information\n") ;
 {
  CardEvent *b = (CardEvent *)0 ;

  fprintf(outfile,"CardEvent_Node_Next   * &%08X\n",(int)&b->Node.Next) ;
  fprintf(outfile,"CardEvent_Node_Prev   * &%08X\n",(int)&b->Node.Prev) ;
  fprintf(outfile,"CardEvent_Type        * &%08X\n",(int)&b->Type) ;
  fprintf(outfile,"CardEvent_Handler     * &%08X\n",(int)&b->Handler) ;
  fprintf(outfile,"CardEvent_data        * &%08X\n",(int)&b->data) ;
  fprintf(outfile,"CardEvent_ModTab      * &%08X\n",(int)&b->ModTab) ;
 }
 fprintf(outfile,"CARDerr_none          * &%08X\n",CARDerr_none) ;
 fprintf(outfile,"CARDerr_badslot       * &%08X\n",CARDerr_badslot) ;
 fprintf(outfile,"CARDerr_nocard        * &%08X\n",CARDerr_nocard) ;
 fprintf(outfile,"CARDerr_badformat     * &%08X\n",CARDerr_badformat) ;
 fprintf(outfile,"CARDerr_badsum        * &%08X\n",CARDerr_badsum) ;
 fprintf(outfile,"CARDerr_badarea       * &%08X\n",CARDerr_badarea) ;

 /* terminate the file cleanly */
 fprintf(outfile,"\n\tOPT\t(old_opt)\n") ;
 fprintf(outfile,"\tEND\n") ;

 /* close the file */
 if (!listing)
  fclose(outfile) ;

 return ;
}

/*----------------------------------------------------------------------*/

void build_PCcard(char *path,int listing)
{
 FILE *outfile = stdout ; /* default is listing */
 char  filename[maxstring] ;

 if (path[0] == '\0')
  sprintf(filename,"PCcard.s") ;
 else
  sprintf(filename,"%s/PCcard.s",path) ;

 /* create the file */
 if (!listing)
  {
   if ((outfile = fopen(filename,"wb")) == NULL)
    {
     fprintf(stderr,"%s: cannot open outfile file \"%s\"\n",whoami,filename) ;
     exit(3) ;
    }
  }

 /* write all the necessary information */
 fprintf(outfile,"\tTTL\tCreated by \"%s\"\t> PCcard/s\n\n",whoami) ;
 fprintf(outfile,"old_opt\tSETA\t{OPT}\n") ;
 fprintf(outfile,"\tOPT\t(opt_off)\n\n") ;

 fprintf(outfile,"; -- from \"PCcard.h\" -----------------------\n") ;
 fprintf(outfile,"; CISTPL_CODES\n") ;
 fprintf(outfile,"CISTPL_NULL           * &%08X\n",CISTPL_NULL) ;
 fprintf(outfile,"CISTPL_DEVICE         * &%08X\n",CISTPL_DEVICE) ;
 fprintf(outfile,"CISTPL_CHECKSUM       * &%08X\n",CISTPL_CHECKSUM) ;
 fprintf(outfile,"CISTPL_LONGLINK_A     * &%08X\n",CISTPL_LONGLINK_A) ;
 fprintf(outfile,"CISTPL_LONGLINK_C     * &%08X\n",CISTPL_LONGLINK_C) ;
 fprintf(outfile,"CISTPL_LINKTARGET     * &%08X\n",CISTPL_LINKTARGET) ;
 fprintf(outfile,"CISTPL_NO_LINK        * &%08X\n",CISTPL_NO_LINK) ;
 fprintf(outfile,"CISTPL_VERS_1         * &%08X\n",CISTPL_VERS_1) ;
 fprintf(outfile,"CISTPL_ALTSTR         * &%08X\n",CISTPL_ALTSTR) ;
 fprintf(outfile,"CISTPL_DEVICE_A       * &%08X\n",CISTPL_DEVICE_A) ;
 fprintf(outfile,"CISTPL_JEDEC_C        * &%08X\n",CISTPL_JEDEC_C) ;
 fprintf(outfile,"CISTPL_JEDEC_A        * &%08X\n",CISTPL_JEDEC_A) ;
 fprintf(outfile,"CISTPL_VERS_2         * &%08X\n",CISTPL_VERS_2) ;
 fprintf(outfile,"CISTPL_FORMAT         * &%08X\n",CISTPL_FORMAT) ;
 fprintf(outfile,"CISTPL_GEOMETRY       * &%08X\n",CISTPL_GEOMETRY) ;
 fprintf(outfile,"CISTPL_BYTEORDER      * &%08X\n",CISTPL_BYTEORDER) ;
 fprintf(outfile,"CISTPL_DATE           * &%08X\n",CISTPL_DATE) ;
 fprintf(outfile,"CISTPL_BATTERY        * &%08X\n",CISTPL_BATTERY) ;
 fprintf(outfile,"CISTPL_ORG            * &%08X\n",CISTPL_ORG) ;
 fprintf(outfile,"CISTPL_END            * &%08X\n",CISTPL_END) ;

 fprintf(outfile,"\n; link TAG identifier\n") ;
 fprintf(outfile,"TPLTG_TAG_ID          * &%08X\n",TPLTG_TAG_ID) ;

 fprintf(outfile,"\n; level 1 information\n") ;
 fprintf(outfile,"L1_format_major       * &%08X\n",L1_format_major) ;
 fprintf(outfile,"L1_format_minor       * &%08X\n",L1_format_minor) ;

 fprintf(outfile,"\n; device speed and type information\n") ;
 fprintf(outfile,"DS_Reserved0          * &%08X\n",DS_Reserved0) ;
 fprintf(outfile,"DS_250ns              * &%08X\n",DS_250ns) ;
 fprintf(outfile,"DS_200ns              * &%08X\n",DS_200ns) ;
 fprintf(outfile,"DS_150ns              * &%08X\n",DS_150ns) ;
 fprintf(outfile,"DS_100ns              * &%08X\n",DS_100ns) ;
 fprintf(outfile,"DS_Reserved5          * &%08X\n",DS_Reserved5) ;
 fprintf(outfile,"DS_Reserved6          * &%08X\n",DS_Reserved6) ;
 fprintf(outfile,"DS_Extend             * &%08X\n",DS_Extend) ;
 fprintf(outfile,"DT_Null               * &%08X\n",DT_Null) ;
 fprintf(outfile,"DT_MASK_ROM           * &%08X\n",DT_MASK_ROM) ;
 fprintf(outfile,"DT_OTPROM             * &%08X\n",DT_OTPROM) ;
 fprintf(outfile,"DT_EPROM              * &%08X\n",DT_EPROM) ;
 fprintf(outfile,"DT_EEPROM             * &%08X\n",DT_EEPROM) ;
 fprintf(outfile,"DT_FLASH              * &%08X\n",DT_FLASH) ;
 fprintf(outfile,"DT_SRAM               * &%08X\n",DT_SRAM) ;
 fprintf(outfile,"DT_DRAM               * &%08X\n",DT_DRAM) ;
 fprintf(outfile,"DT_IO                 * &%08X\n",DT_IO) ;
 fprintf(outfile,"DT_EXTEND             * &%08X\n",DT_EXTEND) ;
 fprintf(outfile,"DT_Reserved           * &%08X\n",DT_Reserved) ;
 fprintf(outfile,"DeviceSpeed_mask      * &%08X\n",DeviceSpeed_mask) ;
 fprintf(outfile,"DeviceSpeed_shift     * &%08X\n",DeviceSpeed_shift) ;
 fprintf(outfile,"WPS_mask              * &%08X\n",WPS_mask) ;
 fprintf(outfile,"WPS_shift             * &%08X\n",WPS_shift) ;
 fprintf(outfile,"DeviceType_mask       * &%08X\n",DeviceType_mask) ;
 fprintf(outfile,"DeviceType_shift      * &%08X\n",DeviceType_shift) ;

 fprintf(outfile,"\n; extended device speed and type information\n") ;
 fprintf(outfile,"DS_MT_Reserved        * &%08X\n",DS_MT_Reserved) ;
 fprintf(outfile,"DS_MT_1p0             * &%08X\n",DS_MT_1p0) ;
 fprintf(outfile,"DS_MT_1p2             * &%08X\n",DS_MT_1p2) ;
 fprintf(outfile,"DS_MT_1p3             * &%08X\n",DS_MT_1p3) ;
 fprintf(outfile,"DS_MT_1p5             * &%08X\n",DS_MT_1p5) ;
 fprintf(outfile,"DS_MT_2p0             * &%08X\n",DS_MT_2p0) ;
 fprintf(outfile,"DS_MT_2p5             * &%08X\n",DS_MT_2p5) ;
 fprintf(outfile,"DS_MT_3p0             * &%08X\n",DS_MT_3p0) ;
 fprintf(outfile,"DS_MT_3p5             * &%08X\n",DS_MT_3p5) ;
 fprintf(outfile,"DS_MT_4p0             * &%08X\n",DS_MT_4p0) ;
 fprintf(outfile,"DS_MT_4p5             * &%08X\n",DS_MT_4p5) ;
 fprintf(outfile,"DS_MT_5p0             * &%08X\n",DS_MT_5p0) ;
 fprintf(outfile,"DS_MT_5p5             * &%08X\n",DS_MT_5p5) ;
 fprintf(outfile,"DS_MT_6p0             * &%08X\n",DS_MT_6p0) ;
 fprintf(outfile,"DS_MT_7p0             * &%08X\n",DS_MT_7p0) ;
 fprintf(outfile,"DS_MT_8p0             * &%08X\n",DS_MT_8p0) ;
 fprintf(outfile,"DS_EX_1ns             * &%08X\n",DS_EX_1ns) ;
 fprintf(outfile,"DS_EX_10ns            * &%08X\n",DS_EX_10ns) ;
 fprintf(outfile,"DS_EX_100ns           * &%08X\n",DS_EX_100ns) ;
 fprintf(outfile,"DS_EX_1us             * &%08X\n",DS_EX_1us) ;
 fprintf(outfile,"DS_EX_10us            * &%08X\n",DS_EX_10us) ;
 fprintf(outfile,"DS_EX_100us           * &%08X\n",DS_EX_100us) ;
 fprintf(outfile,"DS_EX_1ms             * &%08X\n",DS_EX_1ms) ;
 fprintf(outfile,"DS_EX_10ms            * &%08X\n",DS_EX_10ms) ;

 fprintf(outfile,"DSExponent_mask       * &%08X\n",DSExponent_mask) ;
 fprintf(outfile,"DSExponent_shift      * &%08X\n",DSExponent_shift) ;
 fprintf(outfile,"DSMantissa_mask       * &%08X\n",DSMantissa_mask) ;
 fprintf(outfile,"DSMantissa_shift      * &%08X\n",DSMantissa_shift) ;
 fprintf(outfile,"SpeedExtend_shift     * &%08X\n",SpeedExtend_shift) ;
 fprintf(outfile,"SpeedExtend_bit       * &%08X\n",SpeedExtend_bit) ;
 fprintf(outfile,"TypeExtend_shift      * &%08X\n",TypeExtend_shift) ;
 fprintf(outfile,"TypeExtend_bit        * &%08X\n",TypeExtend_bit) ;

 fprintf(outfile,"\n; Device size information\n") ;
 fprintf(outfile,"SizeExponent_mask     * &%08X\n",SizeExponent_mask) ;
 fprintf(outfile,"SizeExponent_shift    * &%08X\n",SizeExponent_shift) ;
 fprintf(outfile,"SizeMantissa_mask     * &%08X\n",SizeMantissa_mask) ;
 fprintf(outfile,"SizeMantissa_shift    * &%08X\n",SizeMantissa_shift) ;

 fprintf(outfile,"\n; structures\n") ;
 {
  TPL_hdr *b = (TPL_hdr *)0 ;
  fprintf(outfile,"TPL_CODE              * &%08X\n",(int)&b->TPL_CODE) ;
  fprintf(outfile,"TPL_LINK              * &%08X\n",(int)&b->TPL_LINK) ;
 }
 fprintf(outfile,"; the following are offsets from after TPL_LINK\n") ;
 {
  CISTPL_LONGLINK_struct *b = (CISTPL_LONGLINK_struct *)0 ;
  fprintf(outfile,"; longlink\n") ;
  fprintf(outfile,"TPLL_ADDR             * &%08X\n",(int)&b->TPLL_ADDR[0]) ;
 }
 {
  CISTPL_LINKTARGET_struct *b = (CISTPL_LINKTARGET_struct *)0 ;
  fprintf(outfile,"; link target\n") ;
  fprintf(outfile,"TFLTG_TAG             * &%08X\n",(int)&b->TFLTG_TAG[0]) ;
 }
 {
  CISTPL_CHECKSUM_struct *b = (CISTPL_CHECKSUM_struct *)0 ;
  fprintf(outfile,"; checksum\n") ;
  fprintf(outfile,"TPLCKS_ADDR           * &%08X\n",(int)&b->TPLCKS_ADDR[0]) ;
  fprintf(outfile,"TPLCKS_LEN            * &%08X\n",(int)&b->TPLCKS_LEN[0]) ;
  fprintf(outfile,"TPLCKS_CS             * &%08X\n",(int)&b->TPLCKS_CS) ;
 }
 {
  CISTPL_VERS_1_struct *b = (CISTPL_VERS_1_struct *)0 ;
  fprintf(outfile,"; level 1 information\n") ;
  fprintf(outfile,"TPLLV1_MAJOR          * &%08X\n",(int)&b->TPLLV1_MAJOR) ;
  fprintf(outfile,"TPLLV1_MINOR          * &%08X\n",(int)&b->TPLLV1_MINOR) ;
  fprintf(outfile,"TPLLV1_INFO           * &%08X\n",(int)&b->TPLLV1_INFO[0]) ;
 }
 {
  CISTPL_ALTSTR_struct *b = (CISTPL_ALTSTR_struct *)0 ;
  fprintf(outfile,"; alternative strings\n") ;
  fprintf(outfile,"TPL_ALTSTR_ESC        * &%08X\n",(int)&b->TPL_ALTSTR_ESC[0]) ;
 }
 {
  JEDEC_struct *b = (JEDEC_struct *)0 ;
  fprintf(outfile,"; JEDEC information\n") ;
  fprintf(outfile,"manufacturerID       * &%08X\n",(int)&b->manufacturerID) ;
  fprintf(outfile,"deviceID             * &%08X\n",(int)&b->deviceID) ;
 }
 {
  CISTPL_VERS_2_struct *b = (CISTPL_VERS_2_struct *)0 ;
  fprintf(outfile,"; level 2 information\n") ;
  fprintf(outfile,"TPLLV2_VERS          * &%08X\n",(int)&b->TPLLV2_VERS) ;
  fprintf(outfile,"TPLLV2_COMPLY        * &%08X\n",(int)&b->TPLLV2_COMPLY) ;
  fprintf(outfile,"TPLLV2_DINDEX        * &%08X\n",(int)&b->TPLLV2_DINDEX[0]) ;
  fprintf(outfile,"TPLLV2_RSV           * &%08X\n",(int)&b->TPLLV2_RSV[0]) ;
  fprintf(outfile,"TPLLV2_VSPEC         * &%08X\n",(int)&b->TPLLV2_VSPEC[0]) ;
  fprintf(outfile,"TPLLV2_NHDR          * &%08X\n",(int)&b->TPLLV2_NHDR) ;
  fprintf(outfile,"TPLLV2_OEM           * &%08X\n",(int)&b->TPLLV2_OEM[0]) ;
 }

 /* terminate the file cleanly */
 fprintf(outfile,"\n\tOPT\t(old_opt)\n") ;
 fprintf(outfile,"\tEND\n") ;

 /* close the file */
 if (!listing)
  fclose(outfile) ;

 return ;
}

/*----------------------------------------------------------------------*/
/* main:
 *	command line options
 *		-h	suitable help message
 *		-p	path where ".s" files to be placed
 *		-l	list only (do not create ".s" files)
 */

int main(int argc,char **argv)
{
 char destpath[maxstring] ;	/* destination path */
 int  dolist = 0 ;		/* generate listings (default OFF) */
 int  index ;			/* general counter */

 destpath[0] = NULL ;		/* NULL path by default */

 /* parse the command line options */
 for (index=1; (index < argc); index++)
  {
   if (argv[index][0] == '-')
    {
     switch (argv[index][1])
      {
       case 'h' :
       case 'H' : /* help */
                  printf("%s (%s at %s)\n",whoami,__DATE__,__TIME__) ;
		  printf("-h suitable help message\n") ;
 	          printf("-p path where \".s\" files to be placed\n") ;
		  printf("-l list only (do not create \".s\" files)\n") ;
		  exit(0) ;

       case 'p' :
       case 'P' : /* destination path */
                  {
		   char *cptr ;
	           if (argv[index][2] == '\0')
		    cptr = argv[++index] ;
		   else
		    cptr = &argv[index][2] ;
		   if (strlen(cptr) >= maxstring)
		    {
                     fprintf(stderr,"%s: path too long\n",whoami) ;
                     exit(2) ;
		    }
		   else
		    strcpy(destpath,cptr) ;
		  }
                  break ;

       case 'l' :
       case 'L' : /* listing only */
	          dolist = -1 ;
                  break ;

       default  : /* unrecognised option */
                  fprintf(stderr,"%s: unrecognised option -%c\n",whoami,argv[index][1]) ;
                  exit(1) ;
                  break ;
      }
    }
   else
    {
     fprintf(stderr,"%s: unrecognised parameter \"%s\"\n",whoami,argv[index]) ;
     exit(1) ;
    }
  }

 /* and generate the files */
 build_ROMitems(destpath,dolist) ;	/* construct "ROMitems.s" */
 build_manifest(destpath,dolist) ;	/* construct "manifest.s" */
 build_PCcard(destpath,dolist) ;	/* construct "PCcard.s" */

 return(0) ;
}

/*----------------------------------------------------------------------*/
/*> EOF genhdr.c <*/

