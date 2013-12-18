/* $Header: digitiser.h,v 1.1 90/12/21 20:53:00 charles Locked $ */
/* $Source: /server/usr/users/b/charles/world/microlink/RCS/ext/digitiser.h,v $ */

/*------------------------------------------------------------------------*/
/*                                             hdr/microlink/digitiser.h  */
/*------------------------------------------------------------------------*/

/* The following file contains declarations of the structures and         */
/*   so-forth that would be required by a client to the digitiser server: */
/* To be specific, it contains the structure of the DigitiserInfo         */
/*   packets returned by the digitiser part of the microlink server by    */
/*   client calls to GetInfo() and also accepted by it in response to     */
/*   client calls to SetInfo().                                           */

/*------------------------------------------------------------------------*/
/*                                                             Interlock  */
/*------------------------------------------------------------------------*/

#ifndef MicrolinkDigitiser_h
#define MicrolinkDigitiser_h

/*------------------------------------------------------------------------*/
/*                                                 Digitiser information  */
/*------------------------------------------------------------------------*/

/* This structure is sent and received by the digitiser part of the       */
/*   microlink server in response to GetInfo() and SetInfo(). It contains */
/*   the calibration paramaters to translate 'raw' stylus co-ordinates to */
/*   the co-ordinates to be output in events by the digitiser server. The */
/*   usual approach would be for some sort of calibration program to set  */
/*   this structure after an approriate calibration process.              */

/* The digitiser uses the values below to compute a processed co-ordinate */
/*   (x',y') from a raw co-ordinate (x,y) as follows:                     */
/*                                                                        */
/* x' = (cXX*x+cXY*y+cX1*1)/denom;                                        */
/* y' = (cYX*x+cYY*y+cY1*1)/denom;                                        */
/*                                                                        */
/* Care should be taken that the computations do not arise in integer     */
/*   overflow.                                                            */
/* The four entries minRawX, maxRawX, minRawY, and maxRawY are returned   */
/*   in GetInfo() calls and ignored in SetInfo() calls: They indicate     */
/*   to the clients the ranges of values that the raw data returned by    */
/*   the digitiser can take. Raw (ie. unscaled or translated)             */
/*   co-ordinates are returned if the transformation matrix is set to     */
/*   cXX=1, cXY=0, cX1=0, cYX=0, cYY=1, cY1=0, and denom=1.               */
/* On setting the digitiser translation values, the client should take    */
/*   care that the values that would be obtained prior to dividing        */
/*   through by 'denom' are never expected to exceed +/-(2^31) (although  */
/*   other intermediate values may), otherwise integer overflow will      */
/*   occur and the transformed co-ordinates will be wrong. That is why    */
/*   minRawX, maxRawX, etc. are supplied: To show the user what the       */
/*   expected range of values INTO the tranformation algorithm are.       */

/* xFilter and yFilter represent the filtering factors on the x- and y-   */
/*   data: A simple forward-average is used to filter the x- and y-       */
/*   ordinates reported by the digitiser. This filter simply computes     */
/*   a weighted average of the current co-ordainte obtained from the      */
/*   digitiser with the last ordinate which was returned to the client to */
/*   get the next ordinate to return to the client. The weighting of the  */
/*   average for each ordiante can be specified by xFilter and yFilter.   */
/*   Filtering is performed after the above described transormation. If   */
/*   either value is zero, the ordinate is not filtered at all and comes  */
/*   through un-averaged. At the other end of the scale, if either value  */
/*   is equal to (1<<filterBits) the corresponding ordinate is fully-     */
/*   slugged ie.won't move at all from it's starting value. The default   */
/*   starting value for xFilter and yFilter are both 0.                   */
/*   filterBits is returned in the digitiser information packet in        */
/*   response to a GetInfo() call.                                        */
/* Note that to avoid overflow, it must be born in mind that the values   */
/*   returned by the digitiser tranformation process described above      */
/*   are never expected to exceed +/- (2^(31-DigFIlterBits)). If they do, */
/*   then the filtering algorithm will overflow and destroy the results.  */

/* xGlitch and yGlitch are values which can be set to control some simple */
/*    glitch-removal logic in the digitiser server: If on transforming    */
/*    and filtering the input data the server sees that the change in x   */
/*    or change in y- co-ordiante is greater than xGlitch or yGlitch      */
/*    respectively, the server throws away that report: If the next value */
/*   after that is still far away, however, then it is accepted as the    */
/*   new co-ordinate. The default initial values for xGlitch and yGlitch  */
/*   are very large positive values so effectively no glitch removal      */
/*   occurs at first.                                                     */

typedef struct DigitiserInfo
{  int cXX,cXY,cX1;
   int cYX,cYY,cY1;
   int denom;
   int xFilter, yFilter;
   int filterBits;
   int xGlitch, yGlitch;
   int minRawX,maxRawX;
   int minRawY,maxRawY;
} DigitiserInfo;

/*------------------------------------------------------------------------*/
/*                                                      End-Of-Interlock  */
/*------------------------------------------------------------------------*/

#endif

