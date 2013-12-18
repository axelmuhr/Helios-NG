/*----------------------------------------------------------------------\
|									|
|	Fichier		:	tc_def.h 				|
|									|
|	Modification	:						|
|									|
| .....................................................................	|
|									|
|	Definition des constantes utilisees par configure.		|
|									|
| .....................................................................	|
|									|
|	Auteur		:	Christophe Thiblot			|
|	Creation	:	01/06/90				|
|									|
\----------------------------------------------------------------------*/



#define nulle		0
#define nul		nulle

#define vrai		1
#define faux		0

#define nil		-99	/* Pour eviter les confusion entre nul et 	*/
				/* un entier egal a 0 (interpretable)		*/		
#define occupe		faux	/* Marquage des directions disponibles		*/
#define libre		vrai	/* pour les liens externes			*/

#define ajoute		faux	/* Marquage des arcs demandes par l'utilisateur	*/
#define original	vrai	/* ou ajoutes pour saturer le graphe		*/

#define fin		-1

#define courant		-5	/* Constantes pour les requetes vers les	*/
#define apres		-3	/* listes de donnees				*/
#define non_sature	-4	/* (concerne les arcs ou les sommets)		*/

#define tc_erreur	nil	/* Code retour des fonctions internes		*/
#define tc_valide	vrai	/* 						*/

#define max_poids	4	/* Maximum d'arcs arrivant sur un sommet	*/

#define rouge		1	/* Constantes pour les requetes vers les	*/
#define vert		2	/* listes de donnees				*/
#define cycle		3	/* (concerne les listes)			*/
#define tas		4	/* 						*/
#define poubelle	5	/* 						*/

#define virtuel		-1

#define nord		0
#define est		1
#define ouest		2
#define sud		3

#define TC_OK		vrai	/* Code retour de configure			*/
#define TC_PB		faux	/* 						*/




