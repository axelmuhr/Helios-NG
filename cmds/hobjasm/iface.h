/* -> iface/h
 * Title:               Interface
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
/*---------------------------------------------------------------------------*/

#ifndef interface_h
#define interface_h

BOOLEAN GetInterface(int argc,char *argv[],char *fileName) ;

BOOLEAN InputFile(char *fileName) ;

BOOLEAN HdrPathname(char *pathname) ;

BOOLEAN Interface_ErrorFile(char *fileName) ;

#endif

/*---------------------------------------------------------------------------*/
/* EOF iface/h */
