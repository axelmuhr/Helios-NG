/*=========================================================================
===		Synergy Interface Definition File			===
===									===
===	Copyright (C) 1989, Perihelion Software Ltd.			===
===	All Rights Reserved						===
===									===
===	Author : BLV, 13.1.89						===
=========================================================================*/

#define Synergy1 1
#define Synergy2 2
extern int syn_Init(int);
extern int syn_Tidy(void);
extern int syn_Restart(void);
extern int syn_Vector(int x_off, int y_off, int width, int operand, int opcode);
extern int syn_Scalar(int x_off, int y_off, int width, int height, int operand,
			int opcode, int value);
extern int syn_Statistics(int x_off, int y_off, int width, int height,
			int operand, int *buffer);
extern int syn_Histogram(int x_off, int y_off, int width, int height,
			int operand, int *buffer);
extern int syn_Convolution(int x_off, int y_off, int width, int height,
			int patch_size, int type, int *buffer);
extern int syn_Zoom(int x_off1, int y_off1, int width1, int height1, int x_off2,
			int y_off2, int width2, int height2);
extern int syn_LoadInputLUT(int lut, int *buffer);
extern int syn_LoadOutputLUT(int luts, int *buffer);
extern int syn_LoadOverlayLUT(int *buffer);
extern int syn_RedefineCharacter(int ch, int *buffer);
extern int syn_String(int x_off, int y_off, int plane, char *text);
extern int syn_CopyBlock(int x_off, int y_off, int plane, int width, 
			int height, int *buffer);
extern int syn_Scroll(int x_off, int y_off, int plane, int width, int height,
			int y_lines, int value);
extern int syn_Wipe(int x_off, int y_off, int plane, int width, int height,
			int value);
extern int syn_Point(int x_off, int y_off, int plane, int value);
extern int syn_Line(int x_off, int y_off, int plane, int x1_off, int y1_off,
			int value);
extern int syn_StartTVGrab(void);
extern int syn_StopTVGrab(void);
extern int syn_TVFilter(int x_off, int y_off, int width, int height,
			int frame_delay, int A_coef, int B_coef);
extern int syn_ReinitTV(void);
extern int syn_SetTVReplication(int display_base, int y_display);
extern int syn_SetSlowScanFilterCoef(int a_coef, int b_coef);
extern int syn_SetFilterMode(int movie, int sequence, int lut, int channel);
extern int syn_SetFilterType(int ignored_pixel, int ignored_line, int type,
			int mode);
extern int syn_FilterStart(int x_off, int y_off, int width, int height,
			int type, int host);
extern int syn_FilterStop(void);
extern int syn_TVFormatSelect(int standard);
extern int syn_CopyVideoMemory(int x_off, int y_off, int width, int height,
			int x1_off, int y1_off, int src_type, int dest_type);
extern int syn_PhotoOutput(int x_off, int y_off, int width, int height,
			 int host);

extern int syn_Execute(int command, ...);
extern int syn_WriteBuffer(int amount, int *data);
extern int syn_ReadBuffer(int amount, int *data);
extern int syn_ReadStatus(int amount, int *data);
extern int syn_WriteVideo(int first_line, int no_lines, int operand,
		 char *data);
extern int syn_ReadVideo(int first_line, int no_lines, int operand, char *data);
extern int syn_BootBoard(void);

#define com_Restart			 0	/* The various commands available */
#define com_Vector			 1
#define com_Scalar			 2
#define com_Statistics			 3
#define com_Histogram			 4
#define com_Convolution			 5
#define com_Zoom			 6
#define com_LoadInputLUT		 7
#define com_LoadOutputLUT		 8
#define com_LoadOverlayLUT		 9
#define com_RedefineCharacter		10
#define com_String			11
#define com_CopyBlock			12
#define com_Scroll			13
#define com_Wipe			14
#define com_Point			15
#define com_Line			16
#define com_StartTVGrab			17
#define com_StopTVGrab			18
#define com_TVFilter			19
#define com_ReinitTV			20
#define com_SetTVReplication		21
#define com_SetSlowScanFilterCoef	22
#define com_SetFilterMode		23
#define com_SetFilterType		24
#define com_FilterStart			25
#define com_FilterStop			26
#define com_TVFormatSelect		29
#define com_CopyVideoMemory		30
#define com_PhotoOutput			31

#define op_plus			 0	/* Additional operands for vector */
#define op_minus		 1	/* and scalar commands */
#define op_and			 2
#define op_xor			 3
#define op_times		 4
#define op_divide		 5
#define op_put			 6
#define op_get			 7
#define op_swap			 8
#define op_remap		 9
#define op_copy			10

#define op_LowerByte		  0	/* which bit of the picture */
#define op_UpperByte		  1
#define op_EntireWord		  2

