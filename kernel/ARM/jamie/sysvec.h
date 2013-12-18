/*> sysvec/h <*/
/*----------------------------------------------------------------------*/
/*									*/
/*				sysvec.h				*/
/*				--------				*/
/*									*/
/* Useful Executive manifests.						*/
/* These values were moved from the system include file "config.h" into	*/
/* this ARM specific file so that an assembler form of the information	*/
/* could be easily generated.						*/
/*									*/
/*----------------------------------------------------------------------*/

/* Special in that it holds all libraries in its system image */
/* and doesn't have a bootstrap */
#define IVecISize	0
#define IVecKernel	1
#define IVecSysLib	2
#define IVecServLib	3
#define IVecUtil	4
#define IVecARMlib	5
#define IVecPosix	6
#define IVecCLib	7
#define IVecFault	8
#define IVecFPLib	9
#define IVecPatchLib	10

/* Add other libraries above here */
#define IVecProcMan	11
#define IVecServers	12

/* These constants depend DIRECTLY on how the system is built */
#define IVecLoader	(IVecServers + 0)
#define IVecKeyboard    (IVecServers + 1)
#define IVecWindow      (IVecServers + 2)
#define IVecRom         (IVecServers + 3)
#define IVecRam         (IVecServers + 4)
#define IVecNull        (IVecServers + 5)
#define IVecHelios      (IVecServers + 6)

#define IVecTotal       (IVecServers + 6)

/*----------------------------------------------------------------------*/
/*> EOF sysvec/h <*/
