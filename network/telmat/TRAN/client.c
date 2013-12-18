/*
******************************************************************************
*                                                                            *
* Auteur : TGO  Validation : 05-11-90  Modification : XX-XX-XX Version : A.0 *
*                                                                            *
******************************************************************************
*                                                                            *
*.MODULE.........: Client                                                    *
*.FONCTION.......: Contient les routines pour les acces aux serveurs par des *
*................. clients                                                   *
*.NOM.FICHIER....: client.c                                                  *
*.VALIDATION.....: 05-11-90                                                  *
*...FICHIER.TEST.: /kernel/helios/CB/serv -> test_client.c                   *
*...COMPILATEUR..: C helios                                                  *
*...REMARQUE.....: - -                                                       *
*.CONTENU........: OpenServer                       : PUBLIC                 *
*.................   Ouverture d'un Stream vers un serveur                   *
*................. DialServer                       : PUBLIC                 *
*.................   Dialogue avec un serveur (appel+reponse)                *
*................. GetServer                        : PUBLIC                 *
*.................   Emission d'un message vers un serveur                   *
*................. PutServer                        : PUBLIC                 *
*.................   Reception d'un message d'un serveur                     *
*.APPELS.EXTERNES: - -                                                       *
*.REMARQUES......: - -                                                       *
*                                                                            *
******************************************************************************
*/
#include <helios.h>
#include <syslib.h>
#include <string.h>
#include "TRAN/client.h"
#ifdef Malloc
#undef Malloc
#endif

#pragma -s1
#pragma -f0

/*
******************************************************************************
*                                                                            *
* Auteur : TGO  Validation : 05-11-90  Modification : XX-XX-XX Version : A.0 *
*                                                                            *
******************************************************************************
*                                                                            *
*.APPEL............: Stream = OpenServer (Name, Flag)                        *
*.DONNEES..........: char *Name : nom du serveur a rechercher                *
*................... Word Flag  : flag d'ouverture du seveur (O_ReadOnly...) *
*.RESULTAT.........: Stream *Stream : pointeur vers une structure Stream si  *
*...................                  OK, NULL sinon                         *
*.DONNEES.MODIFIEES: - -                                                     *
*.FONCTION.........: Ouverture d'un Stream de communication vers un serveur  *
*................... Helios.                                                 *
*.APPELS.EXTERNES..: Locate : sys Helios-> recherche du serveur              *
*................... Open   : sys Helios-> ouverture du Stream vers le serveu*
*.EFFETS.EXTERNES..: - -                                                     *
*.REMARQUE.........: Pour l'ouverture d'un Stream vers un serveur File->Name *
*................... donne en parametre de Open=Null, sinon pour un serveur  *
*................... de type Dir il faut donner le nom du programme appelant!*
*                                                                            *
******************************************************************************
*/

PUBLIC Stream *OpenServer (char *Name, WORD Flag) 
{
  char fname[100];
  Object *o;
  
  fname[0]=0; 
  strcat (fname,"/");
  strcat (fname, Name);
  if ( (o=Locate (NULL, fname)) != Null(Object) ) 
    return (Open (o, "" /*MyTask->Program->Module.Name*/, Flag));
  return (NULL);
}

/*
******************************************************************************
                                                                            *
* Auteur : TGO  Validation : 05-11-90  Modification : XX-XX-XX Version : A.0 *
*                                                                            *
******************************************************************************
*                                                                            *
*.APPEL............: Stat = DialServer (&Stream, Data, &Size, FnRc, TimeOut) *
*.DONNEES..........: Stream *Stream : voie de communication vers un serveur  *
*................... Word   FnRc    : donnee de retour pour la fonction      *
*...................                  GetMsg sur le serveur                  *
*................... Word TimeOut   : timeout de transmission sur Port       *
*.RESULTAT.........: Word Stat      : resultat de la fonction GetMsg du      *
*...................                  client                                 *
*.DONNEES.MODIFIEES: Byte *Data : en entree, donnee a emettre. En sortie     *
*...................              donnees lues                               *
*................... int  *Size : en entree, taille du message a emettre. En *
*...................              sortie taille du message recu              *
*.FONCTION.........: Dialogue avec un serveur. Emission d'un message et atten*
*................... te de la reponse du serveur.                            *
*.APPELS.EXTERNES..: - -                                                     *
*.EFFETS.EXTERNES..: - -                                                     *
*.REMARQUE.........: - -                                                     *
*                                                                            *
******************************************************************************
*/

PUBLIC WORD DialServer (Stream *Stream, BYTE *Data, int *Size, WORD FnRc, WORD TimeOut)
{
  MCB  mcb;
  WORD result;
  
  Wait (&Stream->Mutex);
  InitMCB (&mcb, MsgHdr_Flags_preserve, Stream->Server, Stream->Reply, FnRc);  
  mcb.MsgHdr.ContSize= 0;
  mcb.MsgHdr.DataSize= *Size;
  mcb.Timeout        = TimeOut;
  mcb.Data           = (BYTE *) Data;
  if ( (result=PutMsg(&mcb))<0 ) {
    *Size = 0;
    Signal (&Stream->Mutex);
    return result;
  }
  InitMCB (&mcb, 0, Stream->Reply, NullPort, 0);  
  mcb.Timeout        = TimeOut;
  mcb.Data           = (BYTE *) Data;
  if ( (result=GetMsg (&mcb))<0 )
    *Size = 0;
  else 
    *Size = mcb.MsgHdr.DataSize;
  Signal (&Stream->Mutex);
  return result;
}

/*
******************************************************************************
*                                                                            *
* Auteur : TGO  Validation : 05-11-90  Modification : XX-XX-XX Version : A.0 *
*                                                                            *
******************************************************************************
*                                                                            *
*.APPEL............: Stat = GetServer (&Stream, Data, &Size, TimeOut)        *
*.DONNEES..........: Stream *Stream: voie de communication vers le serveur   *
*................... Byte *Data    : donnees a transmettre                   *
*................... int  *Size    : longueur du message                     *
*................... Word  TimeOut : time-out sur les messages               *
*.RESULTAT.........: Word Stat     : resultat de la fonction GetMsg          *
*.DONNEES.MODIFIEES: - -                                                     *
*.FONCTION.........: Attente d'un message venant d'un serveur                *
*.APPELS.EXTERNES..: - -                                                     *
*.EFFETS.EXTERNES..: - -                                                     *
*.REMARQUE.........: - -                                                     *
*                                                                            *
******************************************************************************
*/

PUBLIC WORD GetServer (Stream *Stream,BYTE *Data, int *Size, WORD TimeOut)
{
  MCB  mcb;
  WORD result; 
  
  Wait (&Stream->Mutex);
  InitMCB (&mcb, 0 , Stream->Reply, NullPort, 0);  
  mcb.Data        = (BYTE *) Data;
  mcb.Timeout     = TimeOut;
  result          = GetMsg (&mcb);
  Signal (&Stream->Mutex);
  if (result <0) 
    *Size = 0 ;
  else
    *Size = mcb.MsgHdr.DataSize ;
  return result;
}

/*
******************************************************************************
*                                                                            *
* Auteur : TGO  Validation : 05-11-90  Modification : XX-XX-XX Version : A.0 *
*                                                                            *
******************************************************************************
*                                                                            *
*.APPEL............: Stat = PutServer (&Stream, Data, Size, FnRc, TimeOut)   *
*.DONNEES..........: Stream *Stream: voie de communication vers le Serveur   *
*................... Byte   *Data  : donnees a transmettre                   *
*................... int    Size   : longueur du message a transmettre       *
*................... Word   FnRc   : code de retour de la fonction GetMsg sur*
*...................                 le serveur                              *
*................... Word   TimeOut: timeout sur la transmission des messages*
*.RESULTAT.........: Word  Stat   : resultat de l'operation PutMsg sur client*
*.DONNEES.MODIFIEES: - -                                                     *
*.FONCTION.........: Envoie un message vers un serveur                       *
*.APPELS.EXTERNES..: - -                                                     *
*.EFFETS.EXTERNES..: - -                                                     *
*.REMARQUE.........: - -                                                     *
*                                                                            *
******************************************************************************
*/

PUBLIC WORD PutServer (Stream *Stream , BYTE *Data, int Size,  WORD FnRc, WORD TimeOut)
{
  MCB mcb;
  WORD result;
  

  Wait (&Stream->Mutex);
  InitMCB (&mcb, MsgHdr_Flags_preserve, Stream->Server, Stream->Reply, FnRc);  
  mcb.MsgHdr.ContSize= 0 ;
  mcb.MsgHdr.DataSize= Size ;
  mcb.Timeout        = TimeOut;
  mcb.Data           = (BYTE *) Data;
  result = PutMsg (&mcb);
  Signal (&Stream->Mutex);
  return result;
}
 
