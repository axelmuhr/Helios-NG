
#define SPC_MAX_LENGHT			20
#define SPC_MAX_PROCESSOR		10
#define SPC_MAX_PROCESSOR_TYPE		32

typedef struct SPC_Processor {
	char	Type[SPC_MAX_LENGHT];	/* attribut du processor */
	int	Pin;			/* Pin correspond */
	int 	Used[SPC_MAX_PROCESSOR_TYPE];	/* Nombre deja utilise si != 0*/
} SPC_Processor;

#define SPC_TYPE_USED			0	/* code retour de fonction */
#define SPC_NO_EXIST			-1

#define SPC_0		"WRK"		/* liste des attributs et leurs pin */
#define SPC_PIN_0	00000		/* numero de pin correspondant */
#define SPC_1		"MEM"
#define SPC_PIN_1	10000
#define SPC_2		"DSK"
#define SPC_PIN_2	11000
#define SPC_3		"CTL"
#define SPC_PIN_3	20000
#define SPC_4		"SPC"
#define SPC_PIN_4	70000
#define SPC_5		"BUF"
#define SPC_PIN_5	80000
#define SPC_6		""
#define SPC_PIN_6	0
#define SPC_7		""		
#define SPC_PIN_7	0
#define SPC_8		""
#define SPC_PIN_8	0
#define SPC_9		""
#define SPC_PIN_9	0


/*********** definition des fonctions du fichier special.c *********************/

extern void 	Initialise_all_special ( SPC_Processor *special_processor );
extern int 	Is_special_processor   ( SPC_Processor *special_processor, char *type );

