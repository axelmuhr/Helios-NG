/*
******************************************************************************
*                                                                            *
* Auteur : TGO  Validation : 05-11-90  Modification : XX-XX-XX Version : A.0 *
*                                                                            *
******************************************************************************
*                                                                            *
*.MODULE.........: client.c                                                  *
*.REMARQUES......: Uniquement les entetes des procedures du client           *
*                                                                            *
******************************************************************************
*/
PUBLIC Stream *OpenServer (char *Name, WORD Flag) ;
PUBLIC WORD DialServer (Stream *Stream, BYTE *Data, int *Size, WORD FnRc, WORD TimeOut);
PUBLIC WORD GetServer  (Stream *Stream, BYTE *Data, int *Size, WORD TimeOut);
PUBLIC WORD PutServer  (Stream *Stream, BYTE *Data, int Size,  WORD FnRc, WORD TimeOut);

 
