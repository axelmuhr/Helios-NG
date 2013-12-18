/*	17 janvier 1991
	Modification des itoa dans les retours de parametres 
	Modif dans configur.c et listes.c
*/

/*----------------------------------------------------------------------\
|									|
|	Fichier		:	listes.c 				|
|	Inclus		:	tc_def.h				|
|				tc_listes.h	(inclus tc_types.h)	|
|				tc_erreur.h				|
|									|
|	Modification	:						|
|									|
| .....................................................................	|
|									|
|	Contient les fonctions de manipulation des donnees traitees	|
|	par configure (enregistrement, interrogation, transfert, ...). 	|
|	Ces fonctions ne laisse pas apparaitre, au moment de leur 	|
|	appel, la gestion locale des donnees (chainage, malloc, ...).	|
|									|
|	Cinq listes sont manipulees :					|
|		. Les sommets						|
|		. Les arcs pendant leur description			|
|		. Les arcs selon le cycle Eulerien			|
|		. Les arcs selon les deux pseudo-cycle			|
|									|
| .....................................................................	|
|									|
|	Auteur		:	Christophe Thiblot			|
|	Creation	:	01/06/90				|
|									|
\----------------------------------------------------------------------*/



#include <stdio.h>

#ifdef HELIOS
#include <stdlib.h>
#include <syslib.h>
#endif

#include <string.h>
#include "tc_erreu.h"
#include "tc_liste.h"
#include "tc_def.h"



#ifdef Malloc
#undef Malloc
#endif

#ifdef HELIOS
#pragma -s1
#pragma -f0

#define malloc Malloc
#define free   Free

extern char *itoa ( int i , char *tab );
#endif

/* ---------------  Fonction  creer  ---------------------------------- */
/* 									*/
/* 	Creer en memoire une zone correspondant a la structure		*/
/* 	demandee.							*/
/* 									*/
/* 	Entree	: int 	struct_type		type de la structure 	*/
/* 						demandee		*/
/* 									*/
/* 	Retour	: char * 			adresse de la zone 	*/
/* 						creee			*/
/* 									*/
/* 	Variables globales affectees	:	tc_errno		*/
/* 						tc_msg[]		*/
/* 									*/
/* 	Appels externes	:						*/
/* 									*/
/* -------------------------------------------------------------------- */

#ifdef ANSI
static char *creer( int struct_type, l_ele *Tliste)
#else
static char *creer( struct_type, Tliste)
int struct_type;
l_ele *Tliste;
#endif
{
	type_sommet	*sommet=nul;
	type_arc	*arc=nul;
	liste_sommet	*lsommet=nul;
	liste_arc	*larc=nul;
	char		ito[100];

	switch(struct_type)
	{
		case t_sommet :	if((sommet=(type_sommet *) malloc(sizeof(*sommet))) == nul)
				{
					Tliste->tc_errno=mem_alloc;
#ifdef HELIOS
						strcpy(Tliste->tc_msg,msg6); 
						itoa(sizeof(*sommet),ito);
						strcat(Tliste->tc_msg,ito);
#else
						sprintf(Tliste->tc_msg,msg6,sizeof(*sommet));
#endif
				}
				return((char *) sommet);
				break;
		case t_arc    :	if((arc=(type_arc *) malloc(sizeof(*arc))) == nul)
				{
					Tliste->tc_errno=mem_alloc;
#ifdef HELIOS
					strcpy(Tliste->tc_msg,msg6);
					itoa(sizeof(*arc),ito);
					strcat(Tliste->tc_msg,ito);
#else
					sprintf(Tliste->tc_msg,msg6,sizeof(*arc));
#endif
				}
				return((char *) arc);
				break;
		case l_sommet :	if((lsommet=(liste_sommet *) malloc(sizeof(*lsommet))) == nul)
				{
					Tliste->tc_errno=mem_alloc;
#ifdef HELIOS
					strcpy(Tliste->tc_msg,msg6);
					itoa(sizeof(*lsommet),ito);
					strcat(Tliste->tc_msg,ito);
#else
					sprintf(Tliste->tc_msg,msg6,sizeof(*lsommet));
#endif
				}
				return((char *) lsommet);
				break;
		case l_arc    :	if((larc=(liste_arc *) malloc(sizeof(*larc))) == nul)
				{
					Tliste->tc_errno=mem_alloc;
#ifdef HELIOS
					strcpy(Tliste->tc_msg,msg6);
					itoa(sizeof(*larc),ito);
					strcat(Tliste->tc_msg,ito);
#else
					sprintf(Tliste->tc_msg,msg6,sizeof(*larc));
#endif
				}
				return((char *) larc);
				break;
	}
}









/* ---------------  Fonction  ajouter_sommet  ------------------------- */
/* 									*/
/* 	Incremente le compteur d'arcs d'un sommet et verifie que ce	*/
/* 	sommet n'est pas deja sature.					*/
/* 	Ce sommet est cree et ajoute a la liste des sommets s'il	*/
/* 	n'existe pas deja.						*/
/* 									*/
/* 	Entree	: int 	numero			numero du sommet 	*/
/* 									*/
/* 	Retour	: type_sommet * 		adresse du sommet 	*/
/* 									*/
/* 	Variables globales affectees	:	tc_errno		*/
/* 						tc_msg[]		*/
/* 									*/
/* 	Appels externes	:						*/
/* 									*/
/* -------------------------------------------------------------------- */

#ifdef ANSI
type_sommet *ajouter_sommet( int numero, l_ele *Tliste)
#else
type_sommet *ajouter_sommet( numero, Tliste)
int numero;
l_ele *Tliste;
#endif
{
	type_sommet	*sommet=nul;
	liste_sommet	*liste;
	char		ito[100];
	
	if(Tliste->sommet_courant == nul)
	{
		if((Tliste->sommet_courant=(liste_sommet *) creer(l_sommet,Tliste)) == nul)
			return(nul);
		if((sommet=(type_sommet *) creer(t_sommet,Tliste)) == nul)
			return(nul);
		sommet->numero=numero;
		sommet->poids=nul;
 		Tliste->sommet_courant->suivant=Tliste->sommet_courant;
		Tliste->sommet_courant->sommet=sommet;
	}
	
	liste=Tliste->sommet_courant;
	do
	{
		if(liste->sommet->numero == numero)
		{
			sommet=liste->sommet;
			Tliste->sommet_courant=liste;
		}
		else
			liste=liste->suivant;
	} 
	while(liste != Tliste->sommet_courant);

	if(sommet == nul)
	{
		if((sommet=(type_sommet *) creer(t_sommet,Tliste)) == nul)
			return(nul);
		if((liste=(liste_sommet *) creer(l_sommet,Tliste)) == nul)
			return(nul);
		sommet->numero=numero;
		sommet->poids=nul;
		liste->sommet=sommet;
		liste->suivant=Tliste->sommet_courant->suivant;
		Tliste->sommet_courant->suivant=liste;
		Tliste->sommet_courant=liste;
	}
	
	if(sommet->poids == max_poids)
	{
		if(numero == virtuel)
		{
			Tliste->tc_errno=quatre_sorties;
#ifdef HELIOS
			strcpy(Tliste->tc_msg,msg5);
			return(nul);
#else
			sprintf(Tliste->tc_msg,msg5);
#endif
		}
		else
		{
			Tliste->tc_errno=trp_sature;
#ifdef HELIOS
			strcpy(Tliste->tc_msg,msg7);
			itoa(sommet->numero,ito);
			strcat(Tliste->tc_msg,ito);
#else
			sprintf(Tliste->tc_msg,msg7,sommet->numero);
#endif
			return(nul);
		}
	}
	
	sommet->poids++;
	return(sommet);
}








/* ---------------  Fonction  lire_sommet  ---------------------------- */
/* 									*/
/* 	Recherche dans la liste des sommets le sommet de numero		*/
/* 	demande.							*/
/* 	Numero peut etre une constante parmi :				*/
/* 		. courant	(sommet courant dans la liste)		*/
/* 		. apres		(sommet suivant dans la liste)		*/
/* 		. non_sature	(premier sommet non_sature rencontre	*/
/* 				 dans la liste)				*/
/* 									*/
/* 	Entree	: int 	numero			numero du sommet 	*/
/* 									*/
/* 	Retour	: type_sommet * 		adresse du sommet 	*/
/* 									*/
/* 	Variables globales affectees	:				*/
/* 	Appels externes	:						*/
/* 									*/
/* -------------------------------------------------------------------- */

#ifdef ANSI
type_sommet *lire_sommet( int numero, l_ele *Tliste)
#else
type_sommet *lire_sommet( numero, Tliste)
int numero;
l_ele *Tliste;
#endif
{
	liste_sommet	*liste;
	
	if(Tliste->sommet_courant == nul)
		return(nul);
		
	switch(numero)
	{
		case courant :	return(Tliste->sommet_courant->sommet);
				break;
		case apres   :	Tliste->sommet_courant=Tliste->sommet_courant->suivant;
				return(Tliste->sommet_courant->sommet);
				break;
		case non_sature:liste=Tliste->sommet_courant;
				do
				{
					if(liste->sommet->poids < max_poids)
					{
						Tliste->sommet_courant=liste;
						return(Tliste->sommet_courant->sommet);
					}
					else
						liste=liste->suivant;
				}
				while(liste != Tliste->sommet_courant);
				return(nul);
				break;
	}
}









/* ---------------  Fonction  ajouter_arc ----------------------------- */
/* 									*/
/* 	Ajoute un arc dans la liste des arcs 				*/
/* 									*/
/* 	Entree	: type_sommet	*depart		sommet 1 de l'arc	*/
/* 		  type_sommet	*arrivee	sommet 2 de l'arc	*/
/* 		  int  		typedef 	type de l'arc		*/
/* 						(demande ou ajoute	*/
/* 						 pour saturation)	*/
/* 									*/
/* 	Retour	: char * 			adresse de l'arc 	*/
/* 									*/
/* 	Variables globales affectees	:	tc_errno		*/
/* 						tc_msg[]		*/
/* 									*/
/* 	Appels externes	:						*/
/* 									*/
/* -------------------------------------------------------------------- */

#ifdef ANSI
type_arc *ajouter_arc( type_sommet *depart, type_sommet *arrivee, int type, l_ele *Tliste)
#else
type_arc *ajouter_arc( depart, arrivee, type, Tliste)
type_sommet *depart, *arrivee;
int type;
l_ele *Tliste;
#endif
{
	type_arc	*arc;
	liste_arc	*liste;
	
	if((arc=(type_arc *) creer(t_arc,Tliste)) == nul)
		return(nul);
	if((liste=(liste_arc *) creer(l_arc,Tliste)) == nul)
		return(nul);
		
	arc->depart=depart;
	arc->arrivee=arrivee;
	arc->type=type;

	liste->arc=arc;

	if(Tliste->tas_courant == nul)
	{
		Tliste->tas_courant=liste;
		Tliste->tas_courant->suivant=liste;
		Tliste->tas_precedent=Tliste->tas_courant;
	}
	else
	{
		liste->suivant=Tliste->tas_courant->suivant;
		Tliste->tas_courant->suivant=liste;
		Tliste->tas_precedent=Tliste->tas_courant;
		Tliste->tas_courant=liste;
	}
	return(arc);
}








/* ---------------  Fonction  inverse_arc  ---------------------------- */
/* 									*/
/* 	Inverse les sommets d'un arc					*/
/* 									*/
/* 	Entree	: type_arc	*arc		arc a inverser	 	*/
/* 									*/
/* 	Retour	: void 							*/
/* 									*/
/* 	Variables globales affectees	:				*/
/* 	Appels externes	:						*/
/* 									*/
/* -------------------------------------------------------------------- */

#ifdef ANSI
static void inverse_arc( type_arc *arc)
#else
static void inverse_arc( arc)
type_arc *arc;
#endif
{
	type_sommet	*sommet;
	
	sommet=arc->depart;
	arc->depart=arc->arrivee;
	arc->arrivee=sommet;
}








/* ---------------  Fonction  liberer_sommets  ------------------------ */
/* 									*/
/* 	Libere les zones memoire correspondants a la liste des sommets	*/
/* 									*/
/* 	Entree	: void 							*/
/* 									*/
/* 	Retour	: void 							*/
/* 									*/
/* 	Variables globales affectees	:				*/
/* 	Appels externes	:						*/
/* 									*/
/* -------------------------------------------------------------------- */

#ifdef ANSI
void liberer_sommets( l_ele *Tliste )
#else
void liberer_sommets( Tliste )
l_ele *Tliste;
#endif
{
	liste_sommet	*tmp,
			*premier;

	premier=Tliste->sommet_courant;
	do
	{
		tmp=Tliste->sommet_courant;
		Tliste->sommet_courant=Tliste->sommet_courant->suivant;
		free(tmp->sommet);
		free(tmp);
	} while(Tliste->sommet_courant != premier);
}
					








/* ---------------  Fonction lire_arc  ------------------------------- */
/* 									*/
/* 	Recherche un arc dans la liste identifiee par type (tas, cycle, */
/* 	rouge, vert) et repere par le numero d'un des sommet de l'arc. 	*/
/* 	numero peut etre une constante parmi : 				*/
/* 		. courant	(arc courant dans la liste)		*/
/* 		. apres		(arc suivant dans la liste)		*/
/* 	Si numero n'est pas une de ces constante, le deuxieme sommet	*/
/* 	de l'arc peut etre repere par complementaire (facultatif)	*/
/* 									*/
/* 	Entree	: int 	type			liste contenant l'arc 	*/
/* 		  int 	numero			numero d'un des sommets	*/
/* 						de l'arc		*/
/* 		  int 	complementaire		numero de l'autre sommet*/
/* 									*/
/* 	Retour	: type_arc * 			adresse de l'arc  	*/
/* 									*/
/* 	Variables globales affectees	:				*/
/* 	Appels externes	:						*/
/* 									*/
/* -------------------------------------------------------------------- */

#ifdef ANSI
type_arc *lire_arc( int type, int numero, int complementaire, l_ele *Tliste )
#else
type_arc *lire_arc( type, numero, complementaire, Tliste )
int type, numero, complementaire;
l_ele *Tliste;
#endif
{
	liste_arc	*liste_courante,
			*liste_precedente,
			*liste,
			*precedent;
	type_arc	*arc=nul;
	
	switch(type)
	{
		case tas    :	liste_courante=Tliste->tas_courant;
				liste_precedente=Tliste->tas_precedent;
				break;
		case cycle  :	liste_courante=Tliste->cycle_courant;
				liste_precedente=Tliste->cycle_precedent;
				break;
		case rouge  :	liste_courante=Tliste->rouge_courant;
				liste_precedente=Tliste->rouge_precedent;
				break;
		case vert   :	liste_courante=Tliste->vert_courant;
				liste_precedente=Tliste->vert_precedent;
				break;
	}

	if(liste_courante == nul)
		return(nul);
		
	switch(numero)
	{
		case courant :	arc=liste_courante->arc;
				break;
		case apres   :	liste_precedente=liste_courante;
				liste_courante=liste_courante->suivant;
				arc=liste_courante->arc;
				break;
		default      :	liste=liste_courante;
				precedent=liste_precedente;
				do
				{
					if((liste->arc->depart->numero == numero) || (liste->arc->arrivee->numero == numero))
					{
						if(liste->arc->depart->numero != numero)
							inverse_arc((type_arc *) liste->arc);
						if(complementaire != nil)
						{
							if(liste->arc->arrivee->numero == complementaire)
							{
								liste_courante=liste;
								liste_precedente=precedent;
								arc=liste->arc;
							}
							else
							{
								precedent=liste;
								liste=liste->suivant;
							}
								
						}
						else
						{
							liste_courante=liste;
							liste_precedente=precedent;
							arc=liste->arc;
						}
					}
					else
					{
						precedent=liste;
						liste=liste->suivant;
					}
				}
				while(liste != liste_courante);
				break;
	}
	
	if(arc != nul)
		switch(type)
		{
			case tas :	Tliste->tas_courant=liste_courante;
					Tliste->tas_precedent=liste_precedente;
					break;
			case cycle  :	Tliste->cycle_courant=liste_courante;
					Tliste->cycle_precedent=liste_precedente;
					break;
			case rouge  :	Tliste->rouge_courant=liste_courante;
					Tliste->rouge_precedent=liste_precedente;
					break;
			case vert   :	Tliste->vert_courant=liste_courante;
					Tliste->vert_precedent=liste_precedente;
					break;
		}

	return(arc);
}









/* ---------------  Fonction transferer_arc  -------------------------- */
/* 									*/
/* 	Transfere un arc de la liste identifiee par depart (tas, cycle, */
/* 	rouge, vert) dans la liste identifiee par arrivee (tas , cycle,	*/
/* 	rouge, vert, poubelle). L'arc est repere par le numero d'un des */
/* 	sommet de l'arc qui peut etre une constante parmi : 		*/
/* 		. courant	(arc courant dans la liste)		*/
/* 	Si numero n'est pas une de ces constante, le deuxieme sommet	*/
/* 	de l'arc peut etre repere par complementaire (facultatif)	*/
/* 	Si arrivee est egale a poubelle, l'arc est simplement retire	*/
/* 	de la liste de depart, et son espace memoire libere.		*/
/* 									*/
/* 	Entree	: int 	depart			liste contenant l'arc 	*/
/* 		  int 	arrivee			liste recevant l'arc	*/
/* 		  int 	numero			numero d'un des sommets	*/
/* 						de l'arc		*/
/* 		  int 	complementaire		numero de l'autre sommet*/
/* 									*/
/* 	Retour	: type_arc * 			adresse de l'arc  	*/
/* 									*/
/* 	Variables globales affectees	:				*/
/* 	Appels externes	:						*/
/* 									*/
/* -------------------------------------------------------------------- */

#ifdef ANSI
type_arc *transferer_arc( int depart, int arrivee, int numero , int complementaire, l_ele *Tliste)
#else
type_arc *transferer_arc( depart, arrivee, numero , complementaire, Tliste)
int depart, arrivee, numero, complementaire;
l_ele *Tliste;
#endif
{
	liste_arc	*liste,
			*precedent,
			*liste_courante,
			*liste_precedente;
	type_arc	*arc=nul;

	switch(depart)
	{
		case tas    :	liste_courante=Tliste->tas_courant;
				liste_precedente=Tliste->tas_precedent;
				break;
		case cycle  :	liste_courante=Tliste->cycle_courant;
				liste_precedente=Tliste->cycle_precedent;
				break;
		case rouge  :	liste_courante=Tliste->rouge_courant;
				liste_precedente=Tliste->rouge_precedent;
				break;
		case vert   :	liste_courante=Tliste->vert_courant;
				liste_precedente=Tliste->vert_precedent;
				break;
	}

	if(liste_courante == nulle)
	{
		return(nul);
	}

	switch(numero)
	{
		case courant :	liste=liste_courante;
				arc=liste->arc;
				if(liste_courante == liste_precedente)
				{
					liste_courante=nul;
					liste_precedente=nul;
				}
				else
				{
					liste_courante=liste_courante->suivant;
					liste_precedente->suivant=liste_courante;
				}
				break;
		default      :	liste=liste_courante;
				precedent=liste_precedente;
				do
				{
					if((liste->arc->depart->numero == numero) || (liste->arc->arrivee->numero == numero))
					{
						if(liste->arc->depart->numero != numero)
							inverse_arc((type_arc *) liste->arc);
						if(complementaire != nil)
						{
							if(liste->arc->arrivee->numero == complementaire)
							{
								liste_courante=liste;
								liste_precedente=precedent;
								arc=liste->arc;
							}
							else
							{
								precedent=liste;
								liste=liste->suivant;
							}
								
						}
						else
						{
							liste_courante=liste;
							liste_precedente=precedent;
							arc=liste->arc;
						}
					}
					else
					{
						precedent=liste;
						liste=liste->suivant;
					}
				}
				while(liste != liste_courante);
				if(arc != nul)
				{
					if(liste_courante == liste_precedente)
					{
						liste_courante=nulle;
						liste_precedente=nulle;
					}
					else
					{
						liste_courante=liste_courante->suivant;
						liste_precedente->suivant=liste_courante;
					}
				}
				break;
	}

	if(arc != nul)
	{
		switch(depart)
		{
			case tas :	Tliste->tas_courant=liste_courante;
					Tliste->tas_precedent=liste_precedente;
					break;
			case cycle  :	Tliste->cycle_courant=liste_courante;
					Tliste->cycle_precedent=liste_precedente;
					break;
			case rouge  :	Tliste->rouge_courant=liste_courante;
					Tliste->rouge_precedent=liste_precedente;
					break;
			case vert   :	Tliste->vert_courant=liste_courante;
					Tliste->vert_precedent=liste_precedente;
					break;
		}
		if(arrivee == poubelle)
		{
			free(liste);
		}
		else
		{
			switch(arrivee)
			{
				case tas    :	liste_courante=Tliste->tas_courant;
						liste_precedente=Tliste->tas_precedent;
						break;
				case cycle  :	liste_courante=Tliste->cycle_courant;
						liste_precedente=Tliste->cycle_precedent;
						break;
				case rouge  :	liste_courante=Tliste->rouge_courant;
						liste_precedente=Tliste->rouge_precedent;
						break;
				case vert   :	liste_courante=Tliste->vert_courant;
						liste_precedente=Tliste->vert_precedent;
						break;
			}
			if(liste_precedente == nulle)
			{
				liste_precedente=liste;
				liste_precedente->suivant=liste;
			}
			else
				liste_precedente=liste_courante;
			liste->suivant=liste_precedente->suivant;
			liste_precedente->suivant=liste;
			liste_courante=liste;
			switch(arrivee)
			{
				case tas :	Tliste->tas_courant=liste_courante;
						Tliste->tas_precedent=liste_precedente;
						break;
				case cycle  :	Tliste->cycle_courant=liste_courante;
						Tliste->cycle_precedent=liste_precedente;
						break;
				case rouge  :	Tliste->rouge_courant=liste_courante;
						Tliste->rouge_precedent=liste_precedente;
						break;
				case vert   :	Tliste->vert_courant=liste_courante;
						Tliste->vert_precedent=liste_precedente;
						break;
			}
		}
			
	}

	return(arc);
}



				





/* ---------------  Fonction liberer_arc  ----------------------------- */
/* 									*/
/* 	Libere la zone memoire de l'arc designe				*/
/* 									*/
/* 	Entree	: type_arc	*arc		adresse de l'arc	*/
/* 									*/
/* 	Retour	: void 							*/
/* 									*/
/* 	Variables globales affectees	:				*/
/* 	Appels externes	:						*/
/* 									*/
/* -------------------------------------------------------------------- */
#ifdef ANSI
void liberer_arc( type_arc *arc)
#else
void liberer_arc( arc)
type_arc *arc;
#endif
{
	free(arc);
}





/* ---------------  Fonction count_arc  ------------------------------- */
/* 									*/
/*     Compte le nombre d' arc original dans la liste identifiee par 	*/
/*     type (tas, cycle,rouge, vert) et repere par le numero d'un des 	*/
/*     sommet de l'arc. Le deuxieme sommet de l'arc peut etre repere	*/
/*     par complementaire.						*/
/* 									*/
/* 	Entree	: int 	type			liste contenant l'arc 	*/
/* 		  int 	numero			numero d'un des sommets	*/
/* 						de l'arc		*/
/* 		  int 	complementaire		numero de l'autre sommet*/
/* 									*/
/* 	Retour	: int     			nombre d'arc     	*/
/* 									*/
/* 	Variables globales affectees	:				*/
/* 	Appels externes	:						*/
/* 									*/
/* -------------------------------------------------------------------- */

#ifdef ANSI
int count_arc( int type, int numero, int complementaire, l_ele *Tliste)
#else
int count_arc( type, numero, complementaire, Tliste)
int type, numero, complementaire;
l_ele *Tliste;
#endif
{
	liste_arc	*liste_courante,
			*liste;
	int      	count;
	
	switch(type)
	{
		case tas    :	liste_courante=Tliste->tas_courant;
				break;
		case cycle  :	liste_courante=Tliste->cycle_courant;
				break;
		case rouge  :	liste_courante=Tliste->rouge_courant;
				break;
		case vert   :	liste_courante=Tliste->vert_courant;
				break;
	}
	count=0;
	if(liste_courante == nul)
		return(count);
	
	liste=liste_courante;
	do
	{
		if((liste->arc->depart->numero == numero) || 
			(liste->arc->arrivee->numero == numero))
		{
			if(liste->arc->depart->numero != numero)
				inverse_arc((type_arc *) liste->arc);

			if((liste->arc->arrivee->numero == complementaire)
				&&(liste->arc->type == original))
				{
					liste=liste->suivant;
					count=count+1;
				}
				else
				{
					liste=liste->suivant;
				}
			

			
		}
		else
		{
			liste=liste->suivant;
		}
	}
	while(liste != liste_courante);
	return(count);
}
