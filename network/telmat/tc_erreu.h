/*	17 janvier 1991
	Modification des fonctions d'erreurs - on enleve les % 
*/

/*----------------------------------------------------------------------\
|									|
|	Fichier		:	tc_erreur.h 				|
|									|
|	Modification	:						|
|									|
| .....................................................................	|
|									|
|	Contient les codes et les messages d'erreur.			|
|									|
| .....................................................................	|
|									|
|	Auteur		:	Christophe Thiblot			|
|	Creation	:	01/06/90				|
|									|
\----------------------------------------------------------------------*/



#define max_msg			150

#define mem_alloc		1	/* erreur retour de malloc (msg6)			*/
#define trp_sature		2	/* cinq liens sur un transputer	(msg7)			*/
#define sorties_occupees	3	/* directions pour liens externes occupees (msg1..4)	*/	
#define quatre_sorties		4	/* cinq liens externes demandes (msg5)			*/

/*
#define msg1	"Insufficient free external links to connect out transputers %d"
#define msg2	"Insufficient free external links to connect out transputers %d %d"
#define msg3	"Insufficient free external links to connect out transputers %d %d %d"
#define msg4	"Insufficient free external links to connect out transputers %d %d %d %d"
#define msg5	"More than four external links required"
#define msg6	"Insufficient memory to allocate another %d bytes"
#define msg7	"More than four links required for transputer %d"
*/

#define msg1	"Insufficient free external links to connect out transputers "
#define msg2	"Insufficient free external links to connect out transputers "
#define msg3	"Insufficient free external links to connect out transputers "
#define msg4	"Insufficient free external links to connect out transputers "
#define msg5	"More than four external links required"
#define msg6	"Insufficient memory to allocate another bytes"
#define msg7	"More than four links required for transputer "

