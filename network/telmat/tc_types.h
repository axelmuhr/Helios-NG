/*----------------------------------------------------------------------\
|									|
|	Fichier		:	tc_types.h 				|
|									|
|	Modification	:						|
|									|
| .....................................................................	|
|									|
|	Definition des types utilises par configure.			|
|									|
| .....................................................................	|
|									|
|	Auteur		:	Christophe Thiblot			|
|	Creation	:	01/06/90				|
|									|
\----------------------------------------------------------------------*/



typedef struct type_sommet
	{
		int		numero,
				poids;
	} type_sommet;
	



typedef struct type_arc
	{
		type_sommet	*depart,
				*arrivee;
		int		type;
	} type_arc;
	






	

	
