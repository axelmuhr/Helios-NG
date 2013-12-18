
/*
******************************************************************************
*                                                                            *
* Auteur : TGO  Validation : 05-11-90  Modification : XX-XX-XX Version : A.0 *
*                                                                            *
******************************************************************************
*                                                                            *
*.MODULE.........: CBcom.h                                                   *
*.FONCTION.......: Code des commandes pour les fonctions Control Bus Man et  *
*................. les codes de retour des fonctions.                        *
*.NOM.FICHIER....: include/CBcom.h                                           *
*.REMARQUES......: - -                                                       *
*                                                                            *
******************************************************************************
*/

/*
******************************************************************************
*       Commandes des fonctions de controle                                  *
******************************************************************************
*/

#define CBunadUOP0		0	/* strobe U0P0 on all slave 	*/
#define CBunadRESETset		1	/* set reset pin		*/
#define CBunadRESETclear	2	/* clear reset pin 		*/
#define CBunadANALset		3	/* set analyse pin		*/
#define CBunadANALclear		4	/* clear analyse pin 		*/
#define CBunadERRRclear		5	/* clear errro latch 		*/
#define CBunadMSDFset		6	/* set MSDF flag		*/
#define CBunadMSDFclear		7	/* clear MSDF flag		*/
#define CBunadMSxfer		8	/* send data to slave 		*/
#define CBunadSMDEset		9	/* set SMDE flag		*/
#define CBunadSMDEclear		10	/* clear SMDE flag		*/
#define CBunadSMxfer		11	/* get data from slave 		*/
#define CBunadMSxferandflag	12	/* send data and set MSDF	*/
#define CBunadSMxferandflag	13	/* get data and set SMDE	*/
#define CBunadPERRclear		14	/* clear parity error latch 	*/    
#define CBunadUOPF		15	/* strobe U0PF on all slave	*/
#define CBunadL0spset		16	/* set link0 speed		*/
#define CBunadL0spclear		17	/* clear link0 speed		*/
#define CBunadL123spset		18	/* set link123 speed		*/
#define CBunadL123spclear	19	/* clear link123 speed		*/ 
#define CBunadLspset		20	/* set link speed		*/
#define CBunadLspclear		21	/* clear link speed		*/
#define TKreadANYstatus		22	/* enable selected slave to use ANY bus */
#define TKreadALLstatus		23
#define TKreadBERRstatus	24
#define TKBootFromROMset	25	/* set boot from rom pin 	*/   
#define TKBootFromROMclear	26	/* clear boot from rom pin	*/
#define TKSPARE1set		27	/* pin SPARE1 set		*/
#define TKSPARE1clear		28	/* pin SPARE1 clear		*/
#define TKSPARE2set		29	/* pin SPARE2 set		*/
#define TKSPARE2clear		30	/* pin SPARE2 clear		*/
#define CBnotused1		31
#define CBunadANALonRESETset	32	/* set ANALYSE pin if analyse on reset enabled */
#define CBunadANALonRESETclear	33	/* clear "	"	"	"	" */

/*
******************************************************************************
*        Commande pour les fonctions de lecture de status                    *
******************************************************************************
*/

#define CBadrdMSDF		0xc	
#define CBadrdSMDE		0xd
#define CBadrdERRR		0xe
#define CBadrdPERR		0xf

/*
******************************************************************************
*       Fonctions de commande Control Bus                                    *
******************************************************************************
*/

#define TK_allocate_processor		60	/* allocation d'un processeur	*/
#define TK_allocate_list		56	/* allocation d'une liste de processeurs, utilisation du mode ANY	*/
#define TK_deallocate_processor		61
#define TK_deallocate_list 		57	
#define TK_enquire_processor		63	/* lecture du masque de programmation	*/
#define TK_control_processor		62	/* fonction de controle avec masque de commande	*/
#define TK_CB_read_status_slave		51	/* lecture d'un registre control bus	*/
#define TK_CB_read_status_slave_list	50	/* lecture du status pour une liste de processeurs	*/
#define TK_CB_control_slave		54	/* fonction de controle pour un slave	*/
#define TK_CB_control_slave_list	55	/* fonction de controle pour une liste de processeurs	*/
#define TK_CB_read_data_from_slave	52	/* lecture d'une chaine de caracteres via le Control Bus	*/
#define TK_CB_write_data_to_slave	53	/* ecriture d'une chaine de caracteres via le Control Bus	*/

/*
******************************************************************************
*         Code de retour des fonctions Control Bus                           *
******************************************************************************
*/

#define TK_OK			1
#define TK_proc_not_available	24	/* processeur inconnu		*/
#define TK_illegal_arg		6	/* mauvais arguments		*/
#define TK_time_out_termination	20	/* pour les timeouts messages 	*/
#define TK_subsystem_selected	36	/* erreur dans le processeurs-> pas de Control Bus	*/
#define TK_illegal_command	4

/*
******************************************************************************
*         Format des messages pour le Control Bus Manager                    *
******************************************************************************
*/
/*
******************************************************************************
*	Fonctions d'allocations de processeurs				     *
******************************************************************************
*/
typedef struct {
	byte 	Command;
	byte 	Mask;	/* masque d'initialisation		*/
	int	PIN;	/* identificateur de processeur		*/
	} TK_allocate_processor_COM;

typedef struct {
	byte 	Status;	/* code d'erreur			*/
	} TK_allocate_processor_REPLY;

typedef struct {
	byte 	Command;
	byte 	Mask;	/* masque d'initialisation		*/
	int	PIN[1];	/* debut de la liste des processeurs 	*/
	} TK_allocate_list_COM;

typedef struct {
	byte	Status;
	} TK_allocate_list_REPLY;
	
typedef struct {
	byte 	Command;
	int	PIN;	/* identificateur de processeur		*/
	} TK_deallocate_processor_COM;

typedef struct {
	byte 	Status;	/* code de retour			*/
	} TK_deallocate_processor_REPLY;
	
typedef struct {
	byte 	Command;
	int	PIN[1] ;	/* identificateur de processeur		*/
	} TK_deallocate_list_COM;

typedef struct {
	byte 	Status;		/* code d'erreur			*/
	} TK_deallocate_list_REPLY;
	
/*
******************************************************************************
*	Fonctions de controle des processeurs				     *
******************************************************************************
*/

typedef struct {
	byte	Command;
	byte 	Opcode;		/* code operation		*/
	int	PIN[1];		/* identificateur de processeur	*/
	} TK_control_processor_list_COM;

typedef struct {
	byte 	Status;		/* code de retour		*/
	} TK_control_processor_list_REPLY;
	
typedef struct {
	byte 	Command;
	byte 	Opcode;	/* code d'operation			*/
	byte	Data;	/* donnee a transmettre			*/
	int 	PIN;	/* identificateur de processeur		*/
	} TK_CB_control_slave_COM;

typedef struct {
	byte 	Status;	/* code de retour de la fonction	*/
	byte 	Data;	/* donnee lue en fonction de la commande	*/
	} TK_CB_control_slave_REPLY;
		
typedef struct {
	byte 	Command;
	byte	Opcode;	/* code d'operation			*/
	byte	Data;	/* donnee a transmettre			*/
	int	PIN[1];	/* liste des processeurs		*/
	} TK_CB_control_slave_list_COM;

typedef struct {
	byte 	Status;	/* code d'erreur			*/
	byte 	Data;	/* en fonction de la commande		*/
	} TK_CB_control_slave_list_REPLY;

/*
*****************************************************************************
*	Fonctions de lectures des registres  				    *
*****************************************************************************
*/

typedef struct {
	byte 	Command;
	byte 	Reg;	/* registre a lire			*/
	int	PIN;	/* identificateur de processeur		*/
	} TK_CB_read_status_COM;

typedef struct {
	byte 	Status;	/* code de retour	*/
	byte 	Result;	/* resultat des lecture de registre	*/
	} TK_CB_read_status_REPLY;
	
typedef struct {
	byte 	Command;
	byte 	Reg;	/* registre a lire 			*/
	int 	PIN[1];	/* liste des processeurs 		*/
	} TK_CB_read_status_list_COM;
	
typedef struct {
	byte	Status;		/* code de retour 	*/
	int 	Proc[1];	/* liste des processeurs apres test	*/
	} TK_CB_read_status_list_REPLY ;

typedef struct {
	byte	Status;	/* code d'erreur	*/
	} TK_error_REPLY;	
	
/*
***************************************************************************
*	Structure des commandes d'acces aux parametres processeurs        *
***************************************************************************
*/

typedef struct {
	byte 	Command;
	int	PIN;	
	} TK_enquire_processor_COM;
	
typedef struct {
	byte 	Status;
	byte 	Mask;
	byte  	Node;
	byte 	Ben;
	byte 	Group;
	byte 	Mybit;
	} TK_enquire_processor_REPLY;
	

/* messages pour les fonctions de lecture et d'ecriture de messages via CB
   au format et fonctionnement identique au Kernel 2.2 
*/

typedef struct {
	byte	Command;	/* code fonction		*/   
	byte	Len;		/* taille maximale du message	*/
	int	Timeout;	/* nombre d'essais		*/
	int	PIN;		/* identificateur processeur	*/
    } TK_CB_read_data_COM;
    
typedef struct {
	byte 	Command;	/* code fonction		*/
	int	Timeout;	/* nombre d'essais		*/
	int	PIN;		/* identificateur processeur	*/
	byte 	Msg[1];		/* pour le debut du message	*/
   } TK_CB_write_data_COM;
   
typedef struct {
	byte	Status;		/* code de retour fonction	*/
	byte 	Len;		/* taille message		*/
	byte 	Msg[256];	/* pour le message		*/
  } TK_CB_read_data_REPLY;
  
typedef struct {
	byte 	Status;
  } TK_CB_write_data_REPLY;
  
	

