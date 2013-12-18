/* -------------------------------------------------------------------- */
/*									*/
/*									*/
/*				HELIOS					*/
/*			        ------					*/
/*		Copyright ( C ) 1989, Telmat informatique		*/
/*			All rights reserved				*/
/*									*/
/*									*/
/* -------------------------------------------------------------------- */
/*									*/
/*	NOM MODULE	: special					*/
/*	NOM DE FICHIER	: special.c 					*/
/*	AUTEUR		: Philippe MOLLIERE				*/
/*	DATE		: 10:12:90					*/
/*	DATE CREATION	: 10:12:90					*/
/*	DATE MODIF	: 10:12:90					*/
/*									*/
/* -------------------------------------------------------------------- */
/*	Fonction : gestion des processeurs speciaux pour les drivers	*/
/*	telmat ( telmat_r.d telmat_c.d ). Gestion des attributs 	*/
/*									*/
/* -------------------------------------------------------------------- */


#include <string.h>
#include "TRAN/special.h"	/* special processor */

#ifdef Malloc
#undef Malloc
#endif

#pragma -s1
#pragma -f0

/*****************************************************************************
** appel		: Initialise_special 
** fonction		: initialisation de la table des processurs speciaux
** donnees		: name	: attribut du processeur special
**			  pin   : numero de processeur
** donnes modifiees	: processor : table des processeurs
** retour		: /
** appels externes	: /
** effets externes	: /
** date de creation	:	10-12-90
** date de derniere modification	:	10-12-90
*****************************************************************************/

static void Initialise_special ( SPC_Processor *processor , char *name ,
 int pin )
{
  int	i;	
  strcpy ( processor->Type , name ); 
  processor->Pin = pin;
  for ( i = 0 ; i < SPC_MAX_PROCESSOR_TYPE ; i ++ ) processor->Used[i] = 0;
}

/*****************************************************************************
** appel		: Initialise_all_apecial
** fonction		: Initialisation de la table des processeurs
** donnees		: /
** donnes modifiees	: processor : table des processeurs speciaux
** retour		: /
** appels externes	: /
** effets externes	: /
** date de creation	:	10-12-90
** date de derniere modification	:	10-12-90
*****************************************************************************/

extern void Initialise_all_special ( SPC_Processor *special_processor )
{
  Initialise_special ( &special_processor[0] , SPC_0 ,  SPC_PIN_0 );   
  Initialise_special ( &special_processor[1] , SPC_1 ,  SPC_PIN_1 );   
  Initialise_special ( &special_processor[2] , SPC_2 ,  SPC_PIN_2 );   
  Initialise_special ( &special_processor[3] , SPC_3 ,  SPC_PIN_3 );   
  Initialise_special ( &special_processor[4] , SPC_4 ,  SPC_PIN_4 );   
  Initialise_special ( &special_processor[5] , SPC_5 ,  SPC_PIN_5 );   
  Initialise_special ( &special_processor[6] , SPC_6 ,  SPC_PIN_6 );   
  Initialise_special ( &special_processor[7] , SPC_7 ,  SPC_PIN_7 );   
  Initialise_special ( &special_processor[8] , SPC_8 ,  SPC_PIN_8 );   
  Initialise_special ( &special_processor[9] , SPC_9 ,  SPC_PIN_9 );   
 }

/*****************************************************************************
** appel		: Is_special_processor
** fonction		: Teste si le processeur est "special"
** donnees		: processor : table des processeurs speciaux
**			  type : attribut du processeur
** donnes modifiees	: /
** retour		: SPC_NO_EXIST si attribut inexistant
**			  SPC_TYPE_USED si attribut utilise
**			  sinon la valeur pin du processeur
** appels externes	: /
** effets externes	: /
** date de creation	:	10-12-90
** date de derniere modification	:	10-12-90
*****************************************************************************/

extern int Is_special_processor ( SPC_Processor *special_processor, char *type )
{
  int	i;
  int	j = 0;
  int	number = 0;
  char	type1[SPC_MAX_LENGHT];
  char	str[SPC_MAX_LENGHT];
  
  strcpy(type1,type);
		/* on enleve le suffixe nombre de l'attribut special */
  for ( i = 0 ; i < strlen(type) ; i ++ ){
  	if ( (type[i] >= '0') && (type[i] <= '9') ) {
  		str[j++] = type1[i];
  		type1[i] = '\0';
  	}
  }
  str[j] = '\0';

  for ( i = 0 ; (str[i] >= '0') && (str[i] <= '9') ; i++)  /* atoi */
  	number = 10 * number + str[i] - '0';
  	
  if ( number > SPC_MAX_PROCESSOR_TYPE)   return(SPC_NO_EXIST);

  for ( i = 0 ; i < SPC_MAX_PROCESSOR ; i++ ){
	if ( ! strcmp ( special_processor[i].Type , type1 )) {
		if ( special_processor[i].Used[number] )
			return(SPC_TYPE_USED);
		else {
			special_processor[i].Used[number] = special_processor[i].Pin + number;
			return(special_processor[i].Used[number]);
		}
	}
  }
  return(SPC_NO_EXIST);
}


