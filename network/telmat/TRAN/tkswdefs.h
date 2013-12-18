/************************************************************************/
/*							  		*/
/*			    T.SWITCHER V3.0				*/
/*									*/
/*		Module : Switcher	File : tkdefs.h			*/
/*									*/
/*									*/
/*	Object : definition des commandes et status du T.Kernel		*/
/* 		 correspondant au Switcher.				*/
/* 									*/
/* 									*/
/*	Authors : ype     Creation date : 16/10/90    Version : 1.0	*/
/*									*/
/************************************************************************/


/* T.Kernel commands */

#define TK_c004_reset		0	/* reset du C004		*/
#define TK_c004_conn_bidir	1	/* connexion bidirectionnelle	*/
#define TK_c004_enquire		2	/* lecture de l'etat du C004	*/
#define TK_c004_enquire_dbg	3	/* affichage liste connexions	*/
#define TK_c004_clr_no_sw_res	4	/* clear C004 reset		*/
#define TK_c004_set_no_sw_res	5	/* set C004 reset		*/
#define TK_c004_sync		6	/* synchro			*/
#define TK_c004_conn_in_2_out	7	/* con. d'1 entree sur 1 sortie	*/
#define TK_c004_conn_121	8	/* connexion bidir. simplifiee	*/
#define TK_c004_dis_out		9	/* deconnexion d'1 sortie	*/
#define TK_c004_dis_outs	10	/* deconnexion de 2 sorties	*/

#define TK_swd_reset		0	/* reset partiel du switch	*/
#define TK_swd_conn_bidir	1	/* connexion bidir. de 2 links	*/
#define TK_swd_enquire		2	/* lecture des connexions	*/
#define TK_swd_enquire_dbg	3	/* affichage liste connexions	*/
#define TK_swd_conn_ic_1_2	4	/* connexion du switch IC1,IC2	*/
#define TK_swd_conn_ic_3_4	5	/* connexion du switch IC3,IC4	*/
#define TK_swd_full_reset	6	/* reset complet du switch	*/

#define TK_swm_init		0	/* init. des tables internes	*/
#define TK_swm_connect_link	1	/* connexion par lien de 2 trp	*/
#define TK_swm_connect_dir	2	/* connexion par dir. de 2 trp	*/
#define TK_swm_mode_0		3	/* passage en mode 0 (mono-usr)	*/

#define TK_swm_mode_1		4	/* passage en mode 1 (multi-us)	*/
#define TK_swm_begin_router	5	/* debut session router (Mega)	*/
#define TK_swm_end_router	6	/* fin session router (Mega)	*/
#define TK_swm_ctl_conn		7	/* verif. dir. (cfg par phase)	*/
#define TK_swm_enquire_all	8	/* affichage de toutes les conn	*/

#define TK_swm_connect_helios	9	/* connexion par lien de 2 trp	*/

/* T.Kernel status */

#define TK_ok				0	/* resultat normal	*/
#define TK_illegal_arg			1	/* arguments invalides 	*/
#define TK_illegal_command		2	/* commande inconnue	*/
#define TK_invalid_command		3	/* commande erronee	*/
#define TK_switch_state_not_known	4	/* etat inconnu		*/
#define TK_link_not_switchable		5	/* lien pas switchable	*/
#define TK_c004_state_not_known		6	/* etat inconnu		*/
#define TK_server_error			10	/* pb communic. serveur */

  
/* Constantes generales des serveurs */

#define FG_SW_Driver	0x00020010	/* fonction switch crossbar	*/
#define FG_C004_Driver	0x00020020	/* fonction C004		*/
#define FG_SW_Manager	0x00020030	/* fonction Switch Manager	*/

#define TimeOutCom	10*OneSec	/* time out des communications	*/
#define max_open_attemps 10		/* nb tentatives OpenServer	*/


/* protocole serveurs */

typedef int	STATUS;

#define bufSize	8

typedef struct PARAM {
  int elt[bufSize];	/* cmd en elt[0], parametres en elt[1],[2],...	*/
  int len;		/* longueur hors commande (parameters only)	*/
} PARAM;
