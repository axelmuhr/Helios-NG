
/*
 --   ---------------------------------------------------------------------------
 --
 --      link.c   -   INMOS standard link interface
 --
 --   ---------------------------------------------------------------------------
*/



/*
 *
 *   OpenLink
 *
 *   Ready the link associated with `Name'.
 *   If `Name' is NULL or "" then any free link can be used.
 *   Returns any positive integer as a link id or
 *   a negative value if the open fails.
 *
 */

int OpenLink ( Name )
   char *Name;
{
}



/*
 *
 *   CloseLink
 *
 *   Close the active link `LinkId'.
 *   Returns 1 on success or negative if the close failed.
 *
 */

int CloseLink ( LinkId )
   int LinkId;
{
}



/*
 *
 *   ReadLink
 *
 *   Read `Count' chars into `Buffer' from the specified link.
 *   LinkId is a vaild link identifier, opened with OpenLink.
 *   `Timeout' is a non negative integer representing tenths
 *   of a second.  A `Timeout' of zero is an infinite timeout.
 *   The timeout is for the complete operation.
 *   If `Timeout' is positive then ReadLink may return having
 *   read less that the number of chars asked for.
 *   Returns the number of chars placed in `Buffer' (which may
 *   be zero) or negative to indicate an error.
 *
 */
 
int ReadLink ( LinkId, Buffer, Count, Timeout )
   int LinkId;
   char *Buffer;
   unsigned int Count;
   int Timeout;
{
}



/*
 *
 *   WriteLink
 *
 *   Write `Count' chars from `Buffer' to the specified link.
 *   LinkId is a vaild link identifier, opened with OpenLink.
 *   `Timeout' is a non negative integer representing tenths
 *   of a second.  A `Timeout' of zero is an infinite timeout.
 *   The timeout is for the complete operation.
 *   If `Timeout' is positive then WriteLink may return having
 *   written less that the number of chars asked for.
 *   Returns the number of chars actually written (which may
 *   be zero) or negative to indicate an error.
 *
 */
 
int WriteLink ( LinkId, Buffer, Count, Timeout )
   int LinkId;
   char *Buffer;
   unsigned int Count;
   int Timeout;
{
}



/*
 *
 *   ResetLink
 *
 *   Reset the subsystem associated with the specified link.
 *   Returns 1 if the reset is successful, negative otherwise.
 *
 */
 
int ResetLink ( LinkId )
   int LinkId;
{
}



/*
 *
 *   AnalyseLink
 *
 *   Analyse the subsystem associated with the specified link.
 *   Returns 1 if the analyse is successful, negative otherwise.
 *
 */
 
int AnalyseLink ( LinkId )
   int LinkId;
{
}



/*
 *
 *   TestError
 *
 *   Test the error status associated with the specified link.
 *   Returns 1 if error is set, 0 if it is not and
 *   negative to indicate an error.
 *
 */
 
int TestError ( LinkId )
   int LinkId;
{
}



/*
 *
 *   TestRead
 *
 *   Test input status of the link.
 *   Returns 1 if ReadLink will return one byte without timeout,
 *   0 if it may not and negative to indicate an error.
 *
 */

int TestRead ( LinkId )
   int LinkId;
{
}



/*
 *
 *   TestWrite
 *
 *   Test output status of the link.
 *   Returns 1 if WriteLink can write one byte without timeout,
 *   0 if it may not and negative to indicate an error.
 *
 */

int TestWrite ( LinkId )
   int LinkId;
{
}



/*
 *   Eof
 */
