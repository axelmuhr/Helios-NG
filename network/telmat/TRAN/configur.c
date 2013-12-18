/*	17 janvier 1991
	Modification des itoa dans les retours de parametres 
	Modif dans configur.c et listes.c
*/

/*----------------------------------------------------------------------\
| 									|
|	Fichier		:	configurer.c 				|
|	Inclus		:	tc_def.h				|
|				tc_types.h				|
|				tc_erreur.h				|
|									|
|	Modification	:						|
|									|
| .....................................................................	|
|									|
|	Contient la fonction configure(), elle est la seule visible	|
|	de l'exterieur.							|
|									|
|	Cette fonction se deroule en cinq etapes, l'apparition		|
|	d'une erreur a une de ces etapes provoque l'arret immediat	|
|	de configure :							|
|									|
|	1. Description des arcs sortants	(liens externes)	|
|		arcs_sortant()						|
|	2. Description du graphe		(reseau demande)	|
|		decrire_graphe()					|
|	3. Saturation du graphe			(reseau complet)	|
|		saturer_graphe()					|
|	4. Recherche du cycle Eulerien de ce reseau			|
|		rechercher_cycle()					|
|	5. Restitution du graphe configure a l'utilisateur		|
|		restituer_graphe()					|
|									|
| .....................................................................	|
|									|
|	Auteur		:	Christophe Thiblot			|
|	Creation	:	01/06/90				|
|									|
\----------------------------------------------------------------------*/
#ifdef HELIOS
#include <stdlib.h>
#endif

#include <stdio.h>
#include <string.h>
#include "tc_def.h"			/* definition courante		*/
#include "tc_erreu.h"			/* definition des erreurs	*/
#include "tc_liste.h"			/* definition des types		*/

#ifdef Malloc
#undef Malloc
#endif

#ifdef HELIOS
#pragma -s1
#pragma -f0
#endif

/* 									*/
/* 	Declaration des fonction externes appelees			*/
/* 									*/

#ifdef ANSI
extern type_sommet	*ajouter_sommet( int numero,l_ele *Tliste),
			*lire_sommet( int numero,l_ele *Tliste);


extern type_arc		*ajouter_arc( type_sommet *depart, type_sommet *arrivee, int type,l_ele *Tliste),
			*lire_arc( int type, int numero, int lie,l_ele *Tliste),
			*transferer_arc(int depart, int arrivee, int numero, int lie,l_ele *Tliste);

extern void		liberer_arc( type_arc *arc ),
			liberer_sommets( l_ele *Tliste );

extern int 		count_arc( int type, int numero, int complementaire, l_ele *Tliste);

#else
extern type_sommet	*ajouter_sommet( ),
			*lire_sommet( );

extern type_arc		*ajouter_arc( ),
			*lire_arc( ),
			*transferer_arc( );

extern void		liberer_arc( ),
			liberer_sommets( );

extern int		count_arc( );

#endif



#ifdef HELIOS
char *itoa ( int i , char *tab )
{
	int  j = 0;
	int  k,l;
	int  c;
	
	if ( i == 0 ) {
		tab [ j++ ] = '0';
		tab [ j++ ] = 0;
		}
	while ( i ){
		tab [ j++ ] = ( i % 10 ) + '0' ;
		i = i / 10;
	}
	tab [ j ] = NULL;
	for ( k = 0, l = strlen (tab) -1 ; k < l ; k++ , l-- ){
		c = tab[k];
		tab[k] = tab[l];
		tab[l] = c;
	}
	return ( (char *) tab[0] );
}	
#endif



/* ---------------  Fonction couleur_opposee  ------------------------- */
/* 									*/
/* 	Converti la couleur en entree en sa couleur opposee.		*/
/* 									*/
/* 	Entrees : int 	couleur		couleur a convertir		*/
/* 									*/
/* 	Retour	: int 	couleur convertie				*/
/* 									*/
/* 	Variables globales affectees	:				*/
/* 	Appels externes	:						*/
/* 									*/
/* -------------------------------------------------------------------- */
#ifdef ANSI
static int couleur_opposee(int couleur)
#else
static int couleur_opposee(couleur)
int couleur;
#endif
{
	return(couleur == rouge ? vert : rouge);
}




/* ---------------  Fonction direction_opposee  ----------------------- */
/* 									*/
/* 	Converti la direction en entree en sa direction opposee		*/
/* 		Nord  -> Sud						*/
/* 		Est   -> Ouest						*/
/* 		Ouest -> Est						*/
/* 		Sud   -> Nord						*/
/* 									*/
/* 	Entrees : int 	dir		direction a convertir		*/
/* 									*/
/* 	Retour	: int 	direction convertie				*/
/* 									*/
/* 	Variables globales affectees	:				*/
/* 	Appels externes	:						*/
/* 									*/
/* -------------------------------------------------------------------- */

#ifdef ANSI
static int direction_opposee( int dir)
#else
static int direction_opposee( dir)
int dir;
#endif
{
	return(3-dir);
}






/* ---------------  Fonction direction_complementaire  ---------------- */
/* 									*/
/* 	Converti la direction en entree en sa direction complementaire	*/
/* 		Nord  -> Est						*/
/* 		Est   -> Nord						*/
/* 		Ouest -> Sud						*/
/* 		Sud   -> Ouest						*/
/* 									*/
/* 	Entrees : int 	dir		direction a convertir		*/
/* 									*/
/* 	Retour	: int 	direction convertie				*/
/* 									*/
/* 	Variables globales affectees	:				*/
/* 	Appels externes	:						*/
/* 									*/
/* -------------------------------------------------------------------- */

#ifdef ANSI
static int direction_complementaire( int dir)
#else
static int direction_complementaire( dir)
int dir;
#endif
{
	return(((dir+1)%2)+(dir/2*2));
}






/* ---------------  Fonction arcs_sortants  --------------------------- */
/* 									*/
/* 	Gere un sommet virtuel permettant d'inclure les arcs sortants	*/
/* 	dans le graphe et verifie la possibilite de leur realisation.	*/
/* 									*/
/* 	Entrees : int 	*externes	description des arcs sortants	*/
/* 		  int 	*news		description des directions	*/
/* 					disponibles			*/
/* 									*/
/* 	Retour	: int 	tc_erreur ou tc_valide				*/
/* 									*/
/* 	Variables globales affectees	:	liens_externes		*/
/* 						tc_errno		*/
/* 						tc_msg[]		*/
/* 	Appels externes	:			ajouter_sommet()	*/
/* 						ajouter_arc()		*/
/* 									*/
/* -------------------------------------------------------------------- */

#ifdef ANSI
static int arcs_sortant( int *externes, int *news, l_ele *Tliste)
#else
static int arcs_sortant( externes, news, Tliste)
int *externes, *news;
l_ele *Tliste;
#endif
{
	int	pext=0;
	type_sommet	*depart,
			*arrivee;
	char	ito[100];
			
	while(*(externes+(pext*2)) != fin)
	{
		if((depart=ajouter_sommet(virtuel,Tliste)) == nul)
			return(tc_erreur);
		if((arrivee=ajouter_sommet(*(externes+(pext*2)),Tliste)) == nul)
			return(tc_erreur);
		if(ajouter_arc(depart,arrivee,original,Tliste) == nul)
			return(tc_erreur);
		pext++;
	}
	
	switch(Tliste->liens_externes=pext)
	{
		case 0 :break;
		case 1 :if((news[nord] == occupe) &&
			   (news[est] == occupe) &&
			   (news[ouest] == occupe) &&
			   (news[sud] == occupe))
			{
				Tliste->tc_errno=sorties_occupees;
#ifdef HELIOS
				strcpy(Tliste->tc_msg,msg1);
				itoa(*externes,ito);
				strcat(Tliste->tc_msg,ito);
#else
				sprintf(Tliste->tc_msg,msg1,*externes);
#endif
				return(tc_erreur);
			}
			break;
		case 2 :if(!(((news[nord] == libre) && (news[sud] == libre)) ||
			     ((news[est] == libre) && (news[ouest] == libre))))
			{
				Tliste->tc_errno=sorties_occupees;
#ifdef HELIOS
				strcpy(Tliste->tc_msg,msg2);
				itoa(*externes,ito);
				strcat(Tliste->tc_msg,ito);
				itoa(*(externes+2),ito);
				strcat(Tliste->tc_msg,ito);
#else
				sprintf(Tliste->tc_msg,msg2,*externes,*(externes+2));
#endif
				return(tc_erreur);
			}
			if((depart=ajouter_sommet(virtuel,Tliste)) == nul)
				return(tc_erreur);
			if((arrivee=ajouter_sommet(virtuel,Tliste)) == nul)
				return(tc_erreur);
			if(ajouter_arc(depart,arrivee,ajoute,Tliste) == nul)
				return(tc_erreur);
			break;
		case 3 :if(!(((news[nord] == libre) && (news[sud] == libre) &&
			      ((news[est] == libre) || (news[ouest] == libre)))
			       ||
			     ((news[est] == libre) && (news[ouest] == libre) &&
			      ((news[nord] == libre) || (news[sud] == libre)))))
			{
				Tliste->tc_errno=sorties_occupees;
#ifdef HELIOS
				strcpy(Tliste->tc_msg,msg3);
				itoa(*externes,ito);
				strcat(Tliste->tc_msg,ito);
				itoa(*(externes+2),ito);
				strcat(Tliste->tc_msg,ito);
				itoa(*(externes+4),ito);
				strcat(Tliste->tc_msg,ito);
#else
				sprintf(Tliste->tc_msg,msg3,*externes,*(externes+2),*(externes+4));
#endif
				return(tc_erreur);
			}
			break;
		case 4 :if(!((news[nord] == libre) && (news[sud] == libre) &&
			     (news[est] == libre) && (news[ouest] == libre)))
			{
				Tliste->tc_errno=sorties_occupees;
#ifdef ANSI
				strcpy(Tliste->tc_msg,msg4);
				itoa(*externes,ito);
				strcat(Tliste->tc_msg,ito);
				itoa(*(externes+2),ito);
				strcat(Tliste->tc_msg,ito);
				itoa(*(externes+4),ito);
				strcat(Tliste->tc_msg,ito);
				itoa(*(externes+6),ito);
				strcat(Tliste->tc_msg,ito);
#else
				sprintf(Tliste->tc_msg,msg4,*externes,*(externes+2),*(externes+4),*(externes+6));
#endif
				return(tc_erreur);
			}
			break;
	}

	return(tc_valide);
}






/* ---------------  Fonction decrire_graphe  -------------------------- */
/* 									*/
/* 	Parcours le tableau graphe[][] et l'enregistre au format 	*/
/* 	des types internes.						*/
/* 									*/
/* 	Entrees : int 	*graphe		description du reseau demande	*/
/* 									*/
/* 	Retour	: int 	tc_erreur ou tc_valide				*/
/* 									*/
/* 	Variables globales affectees	:	tc_errno		*/
/* 						tc_msg[]		*/
/* 									*/
/* 	Appels externes	:			ajouter_sommet()	*/
/* 						ajouter_graphe()	*/
/* 									*/
/* -------------------------------------------------------------------- */

#ifdef ANSI
static int decrire_graphe( int *graphe, l_ele *Tliste)
#else
static int decrire_graphe( graphe, Tliste)
int *graphe;
l_ele *Tliste;
#endif
{
	int		pwrk=0;
	type_sommet	*depart,
			*arrivee;

	while(*(graphe+(pwrk*4)) != fin)
	{
		if((depart=ajouter_sommet(*(graphe+(pwrk*4)),Tliste)) == nul)
			return(tc_erreur);
		if((arrivee=ajouter_sommet(*(graphe+(pwrk*4)+1),Tliste)) == nul)
			return(tc_erreur);
		if(ajouter_arc(depart,arrivee,original,Tliste) == nul)
			return(tc_erreur);
		pwrk++;
	}
	return(tc_valide);
}







/* ---------------  Fonction saturer_graphe  -------------------------- */
/* 									*/
/* 	Sature les sommets du graphe demande en ajoutant autant d'arcs	*/
/* 	que possible. 							*/
/* 									*/
/* 	Entrees : void							*/
/* 									*/
/* 	Retour	: int 	tc_erreur ou tc_valide				*/
/* 									*/
/* 	Variables globales affectees	:	tc_errno		*/
/* 						tc_msg[]		*/
/* 									*/
/* 	Appels externes	:			ajouter_sommet()	*/
/* 						ajouter_graphe()	*/
/* 						lire_sommet()		*/
/* 									*/
/* -------------------------------------------------------------------- */

#ifdef ANSI
static int saturer_graphe( l_ele *Tliste)
#else
static int saturer_graphe( Tliste)
l_ele *Tliste;
#endif
{
	type_sommet	*depart,
			*arrivee;
	
	while((depart=lire_sommet(non_sature,Tliste)) != nul)
	{
		depart=ajouter_sommet(depart->numero,Tliste);
		arrivee=lire_sommet(non_sature,Tliste);
		arrivee=ajouter_sommet(arrivee->numero,Tliste);
		if(ajouter_arc(depart,arrivee,ajoute,Tliste) == nul)
			return(tc_erreur);
	}
	return(tc_valide);
}








/* ---------------  Fonction rechercher_cycle_global  ----------------- */
/* 									*/
/* 	Recherche le cycle Eulerien du graphe sature. 			*/
/* 	Ce cycle peut etre compose de plusieurs cycle Euleriens 	*/
/* 	independants dans le cas d'un graphe compose de plusieurs	*/
/* 	graphes satures independants.					*/
/* 									*/
/* 	Entrees : void							*/
/* 									*/
/* 	Retour	: int 	tc_erreur ou tc_valide				*/
/* 									*/
/* 	Variables globales affectees	:	tc_errno		*/
/* 						tc_msg[]		*/
/* 									*/
/* 	Appels externes	:			lire_arc()		*/
/* 						transferer_arc()	*/
/* 									*/
/* -------------------------------------------------------------------- */

#ifdef ANSI
static int rechercher_cycle_global( l_ele *Tliste )
#else
static int rechercher_cycle_global( Tliste )
l_ele *Tliste;
#endif
{
	type_arc	*arc=nul,
			*marque=nulle;
	int		sommet_suivant;

	while(lire_arc(tas,courant,nil,Tliste) != nul)
	{
		if(arc == marque)
		{
			arc=lire_arc(tas,courant,nil,Tliste);
			sommet_suivant=arc->depart->numero;
		}
		while((arc=transferer_arc(tas,cycle,sommet_suivant,nil,Tliste)) != nul)
		{
			sommet_suivant=arc->arrivee->numero;
			marque=arc;
		}
		arc=lire_arc(cycle,apres,nil,Tliste);
		sommet_suivant=arc->arrivee->numero;
	}
	
	return(tc_valide);
}







/* ---------------  Fonction rechercher_cycles  ----------------------- */
/* 									*/
/* 	Recherche le cycle Eulerien du graphe et le decompose en 	*/
/* 	deux pseudo-cycle. La decomposition se fait en parcourant	*/
/* 	le cycle Eulerien trouve et en affectant les arcs lus dans 	*/
/* 	les pseudo-cycle de maniere alternative.			*/
/* 									*/
/* 	Entrees : void							*/
/* 									*/
/* 	Retour	: int 	tc_erreur ou tc_valide				*/
/* 									*/
/* 	Variables globales affectees	:	tc_errno		*/
/* 						tc_msg[]		*/
/* 									*/
/* 	Appels externes	:			lire_arc()		*/
/* 						transferer_arc()	*/
/* 									*/
/* -------------------------------------------------------------------- */

#ifdef ANSI
static int rechercher_cycles( l_ele *Tliste )
#else
static int rechercher_cycles( Tliste )
l_ele *Tliste;
#endif
{
	if(rechercher_cycle_global( Tliste ) == tc_erreur)
		return(tc_erreur);

	while(lire_arc(cycle,courant,nil,Tliste) != nul)
	{
		if(transferer_arc(cycle,rouge,courant,nil,Tliste) == nul)
			return(tc_erreur);
		if(transferer_arc(cycle,vert,courant,nil,Tliste) == nul)
			return(tc_erreur);
	}

	return(tc_valide);
}






/* ---------------  Fonction restituer_liens  ------------------------- */
/* 									*/
/* 	Affecte les directions obtenues pour un arc dans le tableau	*/ 
/* 	graphe[][] ou externes[][].					*/
/* 									*/
/* 	Entrees : int 	direction	direction de depart		*/
/* 		  type_arc	*arc	arc concerne			*/
/* 		  int 	*graphe		tableau du graphe demande	*/
/* 		  int 	*externes	tableau des liens externes	*/
/* 									*/
/* 	Retour	: void							*/
/* 									*/
/* 	Variables globales affectees	:				*/
/* 									*/
/* 	Appels externes	:			liberer_arc()		*/
/* 									*/
/* -------------------------------------------------------------------- */

#ifdef ANSI
static void restituer_lien( type_arc *arc, int direction, int *graphe, int *externes)
#else
static void restituer_lien( arc, direction, graphe, externes)
type_arc *arc;
int direction, *graphe, *externes;
#endif
{
	int	pwrk=0,
		pext=0;

	if(arc->type == original)
	{
		if((arc->depart->numero == virtuel) || (arc->arrivee->numero == virtuel))
		{
			while(!((*(externes+(pext*2)+1) == -1) &&
				((*(externes+(pext*2)) == arc->depart->numero) ||
				(*(externes+(pext*2)) == arc->arrivee->numero))))
				pext++;
			if(arc->depart->numero == *(externes+(pext*2)))
				*(externes+(pext*2)+1)=direction;
			else
				*(externes+(pext*2)+1)=direction_opposee(direction);
		}
		else
		{
			while(!((*(graphe+(pwrk*4)+2) == -1) &&
			       (  (   (*(graphe+(pwrk*4)) == arc->depart->numero) 
			            && (*(graphe+(pwrk*4)+1) == arc->arrivee->numero)
		        	   )
			         ||(   (*(graphe+(pwrk*4)) == arc->arrivee->numero) 
			            && (*(graphe+(pwrk*4)+1) == arc->depart->numero)
			           )
			        )
			      ))
				pwrk++;
			if(*(graphe+(pwrk*4)) == arc->depart->numero)
			{
				*(graphe+(pwrk*4)+2)=direction;
				*(graphe+(pwrk*4)+3)=direction_opposee(direction);
			}
			else
			{
				*(graphe+(pwrk*4)+3)=direction;
				*(graphe+(pwrk*4)+2)=direction_opposee(direction);
			}
		}
	}

	liberer_arc((type_arc *) arc);
}







/* ---------------  Fonction affecter_cycle  -------------------------- */
/* 									*/
/* 	Parcourt un pseudo-cycle a partir d'un arc donne et affecte	*/
/* 	des directions aux arcs a partir d'une direction de depart 	*/
/* 	donnee.								*/
/* 									*/
/* 	Entrees : int 	couleur		pseudo-cycle concerne		*/
/* 		  type_arc	*arc 	arc de depart			*/
/* 		  int 	direction	direction de depart		*/
/* 		  int 	*graphe		tableau du graphe demande	*/
/* 		  int 	*externes	tableau des liens externes	*/
/* 									*/
/* 	Retour	: void							*/
/* 									*/
/* 	Variables globales affectees	:				*/
/* 									*/
/* 	Appels externes	:			transferer_arc()	*/
/* 						lire_arc()		*/
/* 									*/
/* -------------------------------------------------------------------- */

#ifdef ANSI
static void affecter_cycle( int couleur, type_arc *premier, int direction, int *graphe, int *externes, l_ele *Tliste)
#else
static void affecter_cycle( couleur, premier, direction, graphe, externes, Tliste)
int couleur, direction, *graphe, *externes;
type_arc *premier;
l_ele *Tliste;
#endif

{
	type_arc	*arc;
	int		sommet_suivant,
			arc_present=vrai;

	arc=transferer_arc(couleur,poubelle,premier->depart->numero,premier->arrivee->numero,Tliste);
	restituer_lien(arc,direction,graphe,externes);
	sommet_suivant=arc->arrivee->numero;

	while(arc_present != faux)
	{
		while((arc=transferer_arc(couleur,poubelle,sommet_suivant,nil,Tliste)) != nul)
		{
			restituer_lien(arc,direction,graphe,externes);
			sommet_suivant=arc->arrivee->numero;
		}
		if((arc=lire_arc(couleur,courant,nil,Tliste)) != nul)
			sommet_suivant=arc->depart->numero;
		else
			arc_present=faux;
	}
}








/* ---------------  Fonction marquer_liens  --------------------------- */
/* 									*/
/* 	Marque les lignes des tableaux graphe[][] et externes[][]	*/
/* 	pour reconnaitre les arcs ayant une direction deja affectee.	*/
/* 									*/
/* 	Entrees : int 	*graphe		tableau du graphe demande	*/
/* 		  int 	*externes	tableau des liens externes	*/
/* 									*/
/* 	Retour	: void							*/
/* 									*/
/* 	Variables globales affectees	:				*/
/* 	Appels externes	:						*/
/* 									*/
/* -------------------------------------------------------------------- */

#ifdef ANSI
static void marquer_liens( int *graphe, int *externes)
#else
static void marquer_liens( graphe, externes)
int *graphe, *externes;
#endif
{
	int p;
	
	p=0;
	while(*(externes+(p*2)) != fin)
	{
		*(externes+(p*2)+1)= -1;
		p++;
	}
	p=0;
	while(*(graphe+(p*4)) != fin)
	{
		*(graphe+(p*4)+2)= -1;
		p++;
	}
}








/* ---------------  Fonction restituer_graphe  ------------------------ */
/* 									*/
/* 	Affecte des directions aux arcs des tableaux graphes[][] et	*/
/* 	externes[][] en tenant compte des directions disponibles pour	*/
/* 	les liens externes						*/
/* 									*/
/* 	Entrees : int 	*graphe		tableau du graphe demande	*/
/* 		  int 	*externes	tableau des liens externes	*/
/* 		  int 	*news		tableau des directions 		*/
/* 					disponibles pour les liens 	*/
/* 					externes			*/
/* 									*/
/* 	Retour	: void							*/
/* 									*/
/* 	Variables globales affectees	:				*/
/* 									*/
/* 	Appels externes	:			lire_arc()		*/
/* 						liberer_sommets()	*/
/* 									*/
/* -------------------------------------------------------------------- */

#ifdef ANSI
static void restituer_graphe( int *graphe, int *externes, int *news, l_ele *Tliste)
#else
static void restituer_graphe( graphe, externes, news, Tliste)
int *graphe, *externes, *news;
l_ele *Tliste;
#endif
{
	int		direction,
			couleur,
			i,
			nbr=0,
			nbv=0,
			ext1, ext2, ext3;
	type_arc	*arc;
	
	marquer_liens(graphe,externes);

	switch(Tliste->liens_externes)
	{
		case 0 :arc=lire_arc(rouge,courant,nil,Tliste);
			affecter_cycle(rouge,arc,nord,graphe,externes,Tliste);
			arc=lire_arc(vert,courant,nil,Tliste);
			affecter_cycle(vert,arc,est,graphe,externes,Tliste);
			break;
		case 1 :if((arc=lire_arc(rouge,*externes,virtuel,Tliste)) != nul)
				couleur=rouge;
			else
 			{
				arc=lire_arc(vert,*externes,virtuel,Tliste);
				couleur=vert;
			}
			if(!(((direction= *(externes+1)) != -1) && 
				(news[direction] == libre)))
			{
				if(news[nord] == libre)
					direction=nord;
				else if(news[est] == libre)
					direction=est;
				else if(news[ouest] == libre)
					direction=ouest;
				else 
					direction=sud;
			}
			affecter_cycle(couleur,arc,direction,graphe,externes,Tliste);
			arc=lire_arc(couleur_opposee(couleur),courant,nil,Tliste);
			affecter_cycle(couleur_opposee(couleur),arc,direction_complementaire(direction),graphe,externes,Tliste);
			break;
		case 2 :if((arc=lire_arc(rouge,virtuel,virtuel,Tliste)) == nul)
			{
				couleur=rouge;
				arc=lire_arc(rouge,virtuel,nil,Tliste);
			}
			else
			{
				couleur=vert;
				arc=lire_arc(vert,virtuel,nil,Tliste);
			}
			if((news[nord] == libre) && (news[sud] == libre))
				direction=nord;
			else
				direction=est;
			affecter_cycle(couleur,arc,direction,graphe,externes,Tliste);
			arc=lire_arc(couleur_opposee(couleur),courant,nil,Tliste);
			affecter_cycle(couleur_opposee(couleur),arc,direction_complementaire(direction),graphe,externes,Tliste);
			break;
		case 3 :i=0;
			ext1 = *(externes+(i*2)); i++;
			nbr += count_arc(rouge, ext1, virtuel, Tliste);
			nbv += count_arc(vert, ext1, virtuel, Tliste);
			ext2 = *(externes+(i*2)); i++;
			if (ext2 != ext1)
				{
				nbr += count_arc(rouge, ext2, virtuel, Tliste);
				nbv += count_arc(vert, ext2, virtuel, Tliste);
				}
			ext3 = *(externes+(i*2)); 
			if ((ext3 != ext1) && (ext3 != ext2))
				{
				nbr += count_arc(rouge, ext3, virtuel, Tliste);
				nbv += count_arc(vert, ext3, virtuel, Tliste);
				}
			if(nbr == 2)
				couleur=rouge;
			else
				couleur=vert;
			if((news[nord] == libre) && (news[sud] == libre))
				direction=nord;
			else
				direction=est;
			arc=lire_arc(couleur,courant,nil,Tliste);
			affecter_cycle(couleur,arc,direction,graphe,externes,Tliste);
			couleur=couleur_opposee(couleur);
			direction=direction_complementaire(direction);
 			if((arc=lire_arc(couleur,*externes,virtuel,Tliste)) == nul)
				if((arc=lire_arc(couleur,*(externes+2),virtuel,Tliste)) == nul)
					arc=lire_arc(couleur,*(externes+4),virtuel,Tliste);
			if(news[direction] == occupe)
				direction=direction_opposee(direction);
			affecter_cycle(couleur,arc,direction,graphe,externes,Tliste);
			break;
		case 4 :arc=lire_arc(rouge,courant,nil,Tliste);
			affecter_cycle(rouge,arc,nord,graphe,externes,Tliste);
			arc=lire_arc(vert,courant,nil,Tliste);
			affecter_cycle(vert,arc,est,graphe,externes,Tliste);
			break;
	}

	liberer_sommets( Tliste );
}



#ifdef ANSI
int configure( int *graphe, int *externes, int *news, int *errno, char *msg)
#else
int configure( graphe, externes, news, errno, msg)
int *graphe, *externes, *news, *errno;
char *msg;
#endif
{
l_ele	Tliste;
	Tliste.tc_errno = nulle;
	Tliste.tc_msg[0] = 0;
	Tliste.sommet_courant = nul;
	Tliste.tas_courant = nul;
	Tliste.tas_precedent = nul;
	Tliste.cycle_courant = nul;
	Tliste.cycle_precedent = nul;
	Tliste.rouge_courant = nul;
	Tliste.rouge_precedent = nul;
	Tliste.vert_courant = nul;
	Tliste.vert_precedent = nul;

	if(arcs_sortant((int *) externes,(int *) news, &Tliste) == tc_erreur){
		*errno = Tliste.tc_errno;
		strcpy( msg , Tliste.tc_msg );
		return(TC_PB);}

	if(decrire_graphe((int *) graphe, &Tliste) == tc_erreur){
		*errno = Tliste.tc_errno;
		strcpy( msg , Tliste.tc_msg );
		return(TC_PB);}

	if(saturer_graphe(&Tliste) == tc_erreur){
		*errno = Tliste.tc_errno;
		strcpy( msg , Tliste.tc_msg );
		return(TC_PB);}

	if(rechercher_cycles( &Tliste ) == tc_erreur){
		*errno = Tliste.tc_errno;
		strcpy( msg , Tliste.tc_msg );
		return(TC_PB);}

	restituer_graphe((int *) graphe,(int *) externes,(int *) news, &Tliste);
	*errno = Tliste.tc_errno;
	strcpy( msg , Tliste.tc_msg );

	return(TC_OK);
}
