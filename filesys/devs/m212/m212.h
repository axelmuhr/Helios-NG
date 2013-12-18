
/* M212 command definitions */

/* m212 command values */
#define m2EndOfSequence      0x00
#define m2Initialise         0x01
#define m2ReadParameter      0x02
#define m2WriteParameter     0x03
#define m2ReadBuffer         0x04
#define m2WriteBuffer        0x05
#define m2ReadSector         0x06
#define m2WriteSector        0x07
#define m2Restore            0x08
#define m2Seek               0x09
#define m2SelectDrive        0x0B
#define m2FormatTrack        0x0D
  
#ifdef MULTI
#define m2MultiReadBuffer    0x14
#define m2MultiWriteBuffer   0x15
#define m2MultiReadSector    0x16
#define m2MultiWriteSector   0x17
#define m2MultiFormatTrack   0x1D
#endif

/* m212 parameter addresses */
#define m2DesiredHead        0x01
#define m2DesiredCylinder    0x02
#define m2DesiredCylinder0   0x02
#define m2DesiredCylinder1   0x03
#define m2LogicalSector      0x04
#define m2LogicalSector0     0x04
#define m2LogicalSector1     0x05
#define m2LogicalSector2     0x06
#define m2Addressing         0x07
#define m2DriveType          0x08
#define m2SectorSizeLg2      0x09
#define m2NumberOfSectors    0x0A
#define m2NumberOfHeads      0x0B
#define m2NumberOfCylinders0 0x0C
#define m2NumberOfCylinders1 0x0D
#define m2Interleave         0x18
#define m2Skew		     0x19
#define m2DesiredSectorBuffer	0x1D
#define m2DesiredDrive       0x1E
#define m2Error              0x20
#define m2Reason             0x21
#define m2BufferSize	     0x22

#ifdef MULTI
#define m2MultiNumSectors    0x24
#define m2MultiNumSecs0      0x24
#define m2MultiNumSecs1      0x25
#define m2MultiNumSecs2      0x26
#define m2MultiMode          0x27
#define m2HeadSkew           0x28
#define m2CylinderSkew       0x29

/* values for MultiMode	*/
#define m2UpdateLogical      0x01
#define m2UpdateBuffer       0x02
#define m2Buffered           0x04
#define m2AbortOnFail	     0x20
#endif

#define m2ControllerAccess   0x7F

/* parameter values */
#define m2ExternalWriteClock 0x07


/* bit values for Addressing */
#define m2LogicalAddressing  0x01
#define m2IncrementLogical   0x02
#define m2IncrementBuffer    0x04
