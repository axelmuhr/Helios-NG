/*----------------------------------------------------------------------\
|									|
|	Fichier		:	tc_listes.h 				|
|									|
|	Modification	:						|
|									|
| .....................................................................	|
|									|
|	Definition des types pour les listes utilisees par configure.	|
|									|
| .....................................................................	|
|									|
|	Auteur		:	Christophe Thiblot			|
|	Creation	:	01/06/90				|
|									|
\----------------------------------------------------------------------*/

#include "tc_types.h"



typedef struct l_s
	{
		type_sommet	*sommet;
		struct l_s 	*suivant;
	} liste_sommet;
	


typedef struct l_a
	{
		type_arc	*arc;
		struct l_a 	*suivant;
	} liste_arc;

typedef struct l_ele
	{
		int 		liens_externes;
		int 		tc_errno;
		char 		tc_msg[max_msg];
		liste_sommet	*sommet_courant;
		liste_arc 	*tas_courant;
		liste_arc 	*tas_precedent;
		liste_arc 	*cycle_courant;
		liste_arc 	*cycle_precedent;
		liste_arc 	*rouge_courant;
		liste_arc 	*rouge_precedent;
		liste_arc 	*vert_courant;
		liste_arc 	*vert_precedent;
} l_ele;


#define t_sommet	1
#define t_arc		2
#define t_cycle		3
#define l_sommet	4
#define l_arc		5
#define l_cycle		6

