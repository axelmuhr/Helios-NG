#define NumTries 000   /* Number of times that the comm ports are checked */
                        /* before a timeout occurs */

#define MAX_BREAKS 8  /* Maximum allowable breakpoints */
#define BREAK_TRAP_NUM   0x000001FF  /* 511 */
#define BREAK_TRAP       0x74000000 | BREAK_TRAP_NUM
#define NO_BREAK2        0x00100100  /* This is an illegal address */

#define CRC16 (unsigned short)0x1021
#define BUFF_SIZE 0xfff


typedef struct{
	unsigned long baud;
	int parity, bits;
	} UART_config;

typedef struct{
	UART_config uartA, uartB;
	unsigned long dram_size, cpu_clock, checksum;
	unsigned long sram1_size, sram2_size, sram3_size, sram4_size;
	unsigned long l_dram_base, l_jtag_base;
	unsigned long daughter;
	} hydra_conf;


typedef struct{
				/*These are the lower 32 bits of r0 - r11 */
	unsigned long DSP_ir0, DSP_ir1, DSP_ir2, DSP_ir3, DSP_ir4, DSP_ir5,
				DSP_ir6, DSP_ir7, DSP_ir8, DSP_ir9, DSP_ir10, DSP_ir11;

				/* These are the upper 32 bits of r0 - r11 */
	unsigned long DSP_fr0, DSP_fr1, DSP_fr2, DSP_fr3, DSP_fr4, DSP_fr5,
				DSP_fr6, DSP_fr7, DSP_fr8, DSP_fr9, DSP_fr10, DSP_fr11;

	unsigned long DSP_ar0, DSP_ar1, DSP_ar2, DSP_ar3,
		      DSP_ar4, DSP_ar5, DSP_ar6, DSP_ar7,

				DSP_DP, DSP_IR0, DSP_IR1, DSP_BK, DSP_SP,
				DSP_ST, DSP_DIE, DSP_IIE, DSP_IIF,
				DSP_RS, DSP_RE, DSP_RC,
				DSP_IVTP, DSP_TVTP;

				/* Resume address for user program */
	unsigned long ret_add;
	} reg_set;


void setup( void (*)(void) );
void init( void );
void bus_error_handler( void );
void c_int01( void );

void monitor( hydra_conf * );
void compare( unsigned long [], char );
void dump( unsigned long [], char  );
void fill( unsigned long [] );
void enter( unsigned long [], char );
void copy( unsigned long [] );
void search( unsigned long [], char );
void help( void );

unsigned long ReadEEPROM( unsigned long );
unsigned long WriteEEPROM( unsigned long, int );

void c40_printf( char *fmt, ... );
void putstr( char * );
char c40_putchar( char );
void ftoa( float, char * );
void send_host( unsigned long );

int  comm_sen(int, unsigned long, int);
int  comm_rec(int, unsigned long *, int);
void com_init(void);
int   o_crdy(int, int );
int   i_crdy(int, int );

void xtoa( unsigned long, char * );
unsigned long atox( char *, int * );
unsigned long atod( char *, int * );

void key_handler( void );
int c40_getchar( void );

int iswhite( char );

void led( int, int );

void configure_uart( hydra_conf *config, char which );
unsigned long get_baud( void );
void get_parity( hydra_conf *config, char which );
unsigned long parity_enable( void );
unsigned long parity_type( void );
void get_bits( hydra_conf *config, char which );
unsigned long get_addr( char *str );
unsigned long get_clock( void );
void get_sram( hydra_conf *config );
unsigned long get_daughter( void );
unsigned int get_dram( void );
int read_config( hydra_conf *config );
void display_conf( hydra_conf config );
void write_config( hydra_conf config );
void update_chksum( hydra_conf *config );


void crcupdate( unsigned long data, unsigned long *accum );
unsigned long x_rcvcrc( unsigned long *data, int data_size );
unsigned long x_sndcrc( unsigned long *buff, int data_size );
unsigned long crchware( unsigned long data, unsigned long genpoly, unsigned long accum );
void mk_crctbl( unsigned long poly, unsigned long (*crcfn)(unsigned long,unsigned long,unsigned long) );

void zero_regs( reg_set * );
void resume_mon( reg_set );
int set_brk( unsigned long [], char );
int del_brk( unsigned long [], char );
void list_brks( char );

void reg_dump( reg_set, char );
void print_reg( unsigned long, unsigned long, char );

void step( reg_set * );
void go( reg_set * );
void run( int );


void writeVIC( unsigned long, unsigned long );
unsigned long readVIC( unsigned long );
void writeVAC( unsigned long, unsigned long );
unsigned long readVAC( unsigned long );
void readVACEPROM( void );


int test( hydra_conf config, char out_where );
int reset_others( void );
int CommTest( hydra_conf config );

int BootOthers( hydra_conf, int * );



#define RED	1
#define GREEN	2
#define ON	1
#define OFF	0

#define SUCCESS 1
#define FAILURE 0

#define TRUE  1
#define FALSE 0

#define BEEP  0x7


/* These are instructions that must be checked for when
	inserting a breakpoint */
#define BcondAF         0x68A00000
#define BcondAF_MASK    0xFDE00000
#define BcondAT         0x68600000
#define BcondAT_MASK    0xFDE00000
#define BcondD          0x68200000
#define BcondD_MASK     0xFDE00000
#define BRD             0x61000000
#define BRD_MASK        0xFF000000
#define DBcondD         0x6C200000
#define DBcondD_MASK    0xFC200000
#define RETIcondD       0x78200000
#define RETIcondD_MASK  0xFFE00000
#define RPTBDreg        0x65000000
#define RPTBDreg_MASK   0xFF000000
#define RPTBDim         0x79800000
#define RPTBDim_MASK    0xFFFFFFE0

/* These are the rest of the control flow instructions */
#define Bcond				0x68000000
#define Bcond_MASK		0xFD000000
#define BR					0x60000000
#define BR_MASK			0xFF000000
#define CALL				0x62000000
#define CALL_MASK			0xFF000000
#define CALLcond			0x70000000
#define CALLcond_MASK	0xFD000000
#define DBcond				0x6C000000
#define DBcond_MASK		0xFC000000
#define LAJ					0x63000000
#define LAJ_MASK			0xFF000000
#define LAJcond			0x70200000
#define LAJcond_MASK		0xFDE00000
#define LATcond			0x74800000
#define LATcond_MASK		0xFFE00000
#define RETIcond			0x78000000
#define RETIcond_MASK	0xFFE0FFFF
#define RETScond			0x78800000
#define RETScond_MASK	0xFFE0FFFF
#define RPTBreg        	0x79000000
#define RPTBreg_MASK   	0xFFFFFFE0
#define RPTBim         	0x64000000
#define RPTBim_MASK    	0xFF000000
#define RPTS				0x139B0000
#define RPTS_MASK			0xFF9F0000
#define SWI					0x66000000
#define SWI_MASK			0xFFFFFFFF
#define TRAPcond			0x74000000
#define TRAPcond_MASK	0xFFE0FE00


/* Regester encodings */
#define C40_R0		0x00
#define C40_R1		0x01
#define C40_R2		0x02
#define C40_R3		0x03
#define C40_R4		0x04
#define C40_R5		0x05
#define C40_R6		0x06
#define C40_R7		0x07
#define C40_R8		0x1C
#define C40_R9		0x1D
#define C40_R10 	0x1E
#define C40_R11 	0x1F
#define C40_AR0 	0x08
#define C40_AR1 	0x09
#define C40_AR2 	0x0A
#define C40_AR3 	0x0B
#define C40_AR4 	0x0C
#define C40_AR5 	0x0D
#define C40_AR6 	0x0E
#define C40_AR7 	0x0F
#define C40_DP	 	0x10
#define C40_IR0 	0x11
#define C40_IR1 	0x12
#define C40_BK	 	0x13
#define C40_SP	 	0x14
#define C40_ST	 	0x15
#define C40_DIE	0x16
#define C40_IIE	0x17
#define C40_IIF	0x18
#define C40_RS	 	0x19
#define C40_RE	 	0x1A
#define C40_RC	 	0x1B
