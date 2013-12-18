
/*
 --   ---------------------------------------------------------------------------
 --
 --      ISERVER  -  INMOS standard file server
 --
 --      serverc.c
 --
 --      Primary server operations
 --
 --      Copyright (c) INMOS Ltd., 1988.
 --      All Rights Reserved.
 --
 --   ---------------------------------------------------------------------------
*/



#include <stdio.h>
#include <string.h>

#include "inmos.h"
#include "iserver.h"
#include "pack.h"
 

EXTERN BYTE Tbuf[TRANSACTION_BUFFER_SIZE];


EXTERN BOOL CocoPops;							   /*  for DEBUG  */
EXTERN BOOL VerboseSwitch;

EXTERN BYTE RealCommandLine[MAX_COMMAND_LINE_LENGTH+1];			   /*  for SpCommand  */
EXTERN BYTE DoctoredCommandLine[MAX_COMMAND_LINE_LENGTH+1];

EXTERN BOOL AnalyseSwitch;						   /*  for SpCore  */
EXTERN BYTE *CoreDump;
EXTERN INT32 CoreSize;


/*
 *   SpCommand
 */

PUBLIC VOID SpCommand()
{
   BUFFER_DECLARATIONS;
   BOOL All;
   int Size;
   BYTE *Cl;

   DEBUG(( "SP.COMMAND" ));
   INIT_BUFFERS;
   GET_BYTE( All ); DEBUG(( "%d", All ));

   if ( All )
      Cl = RealCommandLine;
   else
      Cl = DoctoredCommandLine;

   PUT_BYTE( SP_SUCCESS );
   Size = strlen( Cl );
   PUT_SLICE( Size, Cl );
   REPLY;
}




/*
 *   SpCore
 */

PUBLIC VOID SpCore()
{
   BUFFER_DECLARATIONS;
   INT32 Offset;
   int Off, Length;

   DEBUG(( "SP.CORE" ));
   INIT_BUFFERS;

   if (AnalyseSwitch == 0)
      {
         DEBUG(("not peeked"));
         PUT_BYTE(SP_ERROR);
         REPLY;
      }
   else
      {
         GET_INT32( Offset ); DEBUG(( "offset %ld", Offset ));
         GET_INT16( Length ); DEBUG(( "length %d", Length ));
         Off = (int)Offset;
         if ( (Off >= CoreSize) || (Off < 0) || (Length < 0) )
            {
               PUT_BYTE( SP_ERROR );
               REPLY;
            }
         if( Length + (int)Offset > CoreSize )
            {
               Length = CoreSize - (int)Offset;
               PUT_BYTE( SP_SUCCESS );
               PUT_SLICE( Length, &CoreDump[Offset] );
               REPLY;
            }
         else
            {
               PUT_BYTE( SP_SUCCESS );
               PUT_SLICE( Length, &CoreDump[Offset] );
               REPLY;
            }
      }
}




/*
 *   SpId
 */

PUBLIC VOID SpId()
{
   BUFFER_DECLARATIONS;
   BYTE Version, Host, OS, Board;

   DEBUG(( "SP.ID" ));
   INIT_BUFFERS;

   Version = VERSION_ID;
   Host = HOST_ID;
   OS = OS_ID;
   Board = BOARD_ID;

   PUT_BYTE( SP_SUCCESS );
   PUT_BYTE( Version );
   PUT_BYTE( Host );
   PUT_BYTE( OS );
   PUT_BYTE( Board );
   REPLY;
}




PUBLIC VOID SpUnknown()
{
   BUFFER_DECLARATIONS;
   INFO(("[Encountered unknown primary tag (%d)]\n", Tbuf[2]));
   INIT_BUFFERS;
   PUT_BYTE( SP_UNIMPLEMENTED );
   PUT_COUNT( OutCount );
}



/*
 *  Eof
 */
