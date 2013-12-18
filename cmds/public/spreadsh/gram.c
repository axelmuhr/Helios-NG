
# line 18 "gram.y"
#include <curses.h>
#include "sc.h"

#define ENULL (struct enode *)0

char *strcpy();

# line 26 "gram.y"
typedef union  {
    int ival;
    double fval;
    struct ent_ptr ent;
    struct enode *enode;
    char *sval;
    struct range_s rval;
} YYSTYPE;
# define STRING 257
# define NUMBER 258
# define FNUMBER 259
# define RANGE 260
# define VAR 261
# define WORD 262
# define COL 263
# define S_FORMAT 264
# define S_LABEL 265
# define S_LEFTSTRING 266
# define S_RIGHTSTRING 267
# define S_GET 268
# define S_PUT 269
# define S_MERGE 270
# define S_LET 271
# define S_WRITE 272
# define S_TBL 273
# define S_COPY 274
# define S_SHOW 275
# define S_ERASE 276
# define S_FILL 277
# define S_GOTO 278
# define S_DEFINE 279
# define S_UNDEFINE 280
# define S_VALUE 281
# define S_MDIR 282
# define S_HIDE 283
# define S_SET 284
# define K_FIXED 285
# define K_SUM 286
# define K_PROD 287
# define K_AVG 288
# define K_STDDEV 289
# define K_ACOS 290
# define K_ASIN 291
# define K_ATAN 292
# define K_ATAN2 293
# define K_CEIL 294
# define K_COS 295
# define K_EXP 296
# define K_FABS 297
# define K_FLOOR 298
# define K_HYPOT 299
# define K_LN 300
# define K_LOG 301
# define K_PI 302
# define K_POW 303
# define K_SIN 304
# define K_SQRT 305
# define K_TAN 306
# define K_DTR 307
# define K_RTD 308
# define K_MAX 309
# define K_MIN 310
# define K_RND 311
# define K_PV 312
# define K_FV 313
# define K_PMT 314
# define K_HOUR 315
# define K_MINUTE 316
# define K_SECOND 317
# define K_MONTH 318
# define K_DAY 319
# define K_YEAR 320
# define K_NOW 321
# define K_DATE 322
# define K_FMT 323
# define K_SUBSTR 324
# define K_STON 325
# define K_EQS 326
# define K_EXT 327
# define K_NVAL 328
# define K_SVAL 329
# define K_LOOKUP 330
# define K_INDEX 331
# define K_STINDEX 332
# define K_AUTO 333
# define K_AUTOCALC 334
# define K_BYROWS 335
# define K_BYCOLS 336
# define K_BYGRAPH 337
# define K_ITERATIONS 338
# define K_NUMERIC 339
# define K_PRESCALE 340
# define K_EXTFUN 341
# define K_CELLCUR 342
# define K_TOPROW 343
# define K_TBLSTYLE 344
# define K_TBL 345
# define K_LATEX 346
# define K_TEX 347
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern short yyerrflag;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
YYSTYPE yylval, yyval;
# define YYERRCODE 256
short yyexca[] ={
-1, 1,
	0, -1,
	-2, 0,
-1, 217,
	60, 0,
	61, 0,
	62, 0,
	33, 0,
	-2, 105,
-1, 219,
	60, 0,
	61, 0,
	62, 0,
	33, 0,
	-2, 106,
-1, 220,
	60, 0,
	61, 0,
	62, 0,
	33, 0,
	-2, 107,
-1, 274,
	60, 0,
	61, 0,
	62, 0,
	33, 0,
	-2, 110,
-1, 275,
	60, 0,
	61, 0,
	62, 0,
	33, 0,
	-2, 112,
-1, 276,
	60, 0,
	61, 0,
	62, 0,
	33, 0,
	-2, 111,
-1, 283,
	41, 124,
	-2, 38,
	};
# define YYNPROD 157
# define YYLAST 2665
short yyact[]={

 374, 152, 153, 154, 155, 158, 159, 160, 161, 162,
 163, 164, 165, 166, 167, 168, 169, 373, 170, 171,
 172, 173, 174, 175, 156, 157, 176, 177, 178, 179,
 180, 181, 182, 183, 184, 185, 186, 189, 190, 197,
 187, 188, 194, 195, 196, 192, 191, 193,  46, 128,
 129, 126, 127,  45,  29, 130, 131, 132, 133, 134,
 123, 121,  44,  67, 100,  29, 206,  43,  34, 118,
 119, 120, 102, 117,  89,  26,  26,  26,  26, 272,
  37,  37,  37,  37,  37,  37,  29, 205,  48,  26,
  26,  26,  26,  37,  26,  66,  71, 204, 203,  56,
 124,  55, 122, 115,  24, 142, 198, 141,  31,  32,
  33,  64, 139,  48,  48,  48,  25, 140, 224, 136,
  26,  49,  50,  52,  57, 135,  61,  70,  69,  68,
  63,  77,  47,  84,  76, 404, 403, 114, 210, 211,
 212, 213, 214, 215, 216, 217, 219, 220, 222, 223,
 402, 225,  78, 207, 208, 209, 332,  73,  74,  75,
 330, 150, 329, 141, 142,  29, 328,  88, 139, 137,
 270, 138,  56, 140,  55, 393, 391,  83, 392, 392,
 113, 327, 326,  29, 269, 268, 267, 105, 266, 265,
 106, 264, 107, 113, 263, 262,  29, 261, 260, 259,
 105, 258, 257, 106, 256, 107,  29, 255, 254, 253,
  85, 104, 252,  56, 251,  55, 101, 250, 249, 274,
 142, 221, 275, 248, 104, 276, 247, 246, 245, 244,
 243, 282, 285, 286, 287, 288, 289, 290, 291, 292,
 293, 294, 295, 296, 297, 298, 299, 300, 301, 302,
 303, 304, 305, 306, 307, 308, 309, 310, 311, 312,
 313, 314, 315, 316, 317, 318, 319, 320, 321, 322,
 323, 324, 242, 112, 325, 113, 241, 240,  29,  30,
 239,  28, 105, 238, 237, 106, 112, 107, 236,  27,
  30, 235,  28, 234, 233, 116,  72, 232, 231,  26,
  26,  26,  26, 283, 283, 230, 104,  36, 229, 113,
 228,  30,  29,  28,  53,  54, 105,  65, 227, 106,
 151, 107, 226, 199, 200,  62,   1,   0,   0, 201,
 202, 277, 278, 279, 280, 281, 284, 218, 376,   0,
 104,   0,   0,   0, 377,   0,   0, 378,  35,   0,
   0, 375,   0,   0, 379, 380, 381,  38,  39,  40,
  41,  42,   0,   0, 382,   0, 383,   0, 112,  60,
 387, 388, 389, 390,  86,  87,  91,  90,  92,  98,
  93,  94,  95,  96,  97,  99,  59,  53,  54,  27,
  30,   0,  28, 409,   0,   0,   0,   0, 410, 411,
 412,   0, 112,   0, 111, 108, 109,  27,  30, 413,
  28,   0,   0,   0,   0,   0,   0, 111, 108, 109,
   0,  30,   0,  28,   0,   0,   0,   0,  53,  54,
  27,  30, 103,  28,   0,   0,   0,   0,   0,  26,
  26,  26,   0,   0,   0, 103,   0,   0,   0, 110,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0, 110,   0,   0,   0,  23,   0,   0,   0,
   0, 384, 385, 386,   6,   3,   4,   5,   7,  10,
   8,   2,  11,  12,  15,  13,  16,  18,  19,  20,
  21,  17,   9,  14,  22,   0,   0,   0,   0, 111,
 108, 109, 149,  30, 150,  28, 141, 147,   0,   0,
 417, 139, 137,   0, 138,   0, 140,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0, 103,   0, 144,
 145, 146, 143, 111, 108, 109,   0,  30, 149,  28,
 150,   0, 141, 147, 110,   0, 416, 139, 137,   0,
 138,   0, 140,   0,   0,   0,   0,   0,   0,   0,
   0, 103,   0, 142,   0, 144, 145, 146, 143,   0,
   0,   0,   0,   0,   0, 149,   0, 150, 110, 141,
 147,   0,   0, 415, 139, 137, 149, 138, 150, 140,
 141, 147,   0, 148, 414, 139, 137,   0, 138, 142,
 140,   0, 144, 145, 146, 143,   0,   0,   0,   0,
   0,   0,   0, 144, 145, 146, 143,   0,   0,   0,
   0, 149,   0, 150,   0, 141, 147,   0,   0, 148,
 139, 137, 408, 138,   0, 140, 142,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0, 142, 144, 145,
 146, 143,   0,   0,   0,   0,   0,   0,   0, 149,
   0, 150,   0, 141, 147,   0, 148, 407, 139, 137,
   0, 138,   0, 140,   0,   0,   0, 148,   0,   0,
   0,   0, 142,   0,   0,   0, 144, 145, 146, 143,
   0,   0,   0,   0, 149,   0, 150,   0, 141, 147,
   0,   0, 406, 139, 137, 149, 138, 150, 140, 141,
 147,   0, 148, 405, 139, 137,   0, 138,   0, 140,
 142, 144, 145, 146, 143,   0,   0,   0,   0,   0,
   0,   0, 144, 145, 146, 143,   0,   0,   0,   0,
   0,   0,   0,   0, 149,   0, 150,   0, 141, 147,
 148,   0, 401, 139, 137, 142, 138,   0, 140,   0,
   0,   0,   0,   0,   0,   0, 142,   0,   0,   0,
   0, 144, 145, 146, 143,   0,   0,   0,   0, 149,
   0, 150,   0, 141, 147, 148,   0, 400, 139, 137,
 149, 138, 150, 140, 141, 147, 148,   0,   0, 139,
 137, 399, 138,   0, 140, 142, 144, 145, 146, 143,
   0,   0,   0,   0,   0,   0,   0, 144, 145, 146,
 143,   0,   0,   0,   0,   0,   0, 149,   0, 150,
   0, 141, 147,   0,   0, 148, 139, 137, 398, 138,
 142, 140,   0,   0,   0,   0,   0,   0,   0,   0,
   0, 142,   0,   0, 144, 145, 146, 143,   0,   0,
   0,   0, 149,   0, 150,   0, 141, 147,   0,   0,
 148, 139, 137, 397, 138,   0, 140,   0,   0,   0,
   0, 148,   0,   0,   0,   0,   0,   0, 142, 144,
 145, 146, 143,   0,   0,   0,   0, 149,   0, 150,
   0, 141, 147,   0,   0, 396, 139, 137,   0, 138,
   0, 140,   0,   0,   0,   0,   0,   0, 148,   0,
   0,   0,   0, 142, 144, 145, 146, 143,   0,   0,
   0,   0, 149,   0, 150,   0, 141, 147,   0,   0,
 395, 139, 137,   0, 138,   0, 140,   0,   0,   0,
   0,   0,   0, 148,   0,   0,   0,   0, 142, 144,
 145, 146, 143,   0,   0,   0,   0, 149,   0, 150,
   0, 141, 147,   0,   0, 394, 139, 137,   0, 138,
   0, 140,   0,   0,   0,   0,   0,   0, 148,   0,
   0,   0,   0, 142, 144, 145, 146, 143,   0,   0,
   0,   0, 149,   0, 150,   0, 141, 147,   0,   0,
   0, 139, 137, 372, 138,   0, 140,   0,   0,   0,
   0,   0,   0, 148,   0,   0,   0,   0, 142, 144,
 145, 146, 143,   0,   0,   0,   0, 149,   0, 150,
   0, 141, 147,   0,   0,   0, 139, 137, 371, 138,
   0, 140,   0,   0,   0,   0,   0,   0, 148,   0,
   0,   0,   0, 142, 144, 145, 146, 143,   0,   0,
   0,   0, 149,   0, 150,   0, 141, 147,   0,   0,
   0, 139, 137, 370, 138,   0, 140,   0,   0,   0,
   0,   0,   0, 148,   0,   0,   0,   0, 142, 144,
 145, 146, 143,   0,   0,   0,   0, 149,   0, 150,
   0, 141, 147,   0,   0,   0, 139, 137, 369, 138,
   0, 140,   0,   0,   0,   0,   0,   0, 148,   0,
   0,   0,   0, 142, 144, 145, 146, 143,   0,   0,
   0,   0, 149,   0, 150,   0, 141, 147,   0,   0,
   0, 139, 137, 368, 138,   0, 140,   0,   0,   0,
   0,   0,   0, 148,   0,   0,   0,   0, 142, 144,
 145, 146, 143,   0,   0,   0,   0, 149,   0, 150,
   0, 141, 147,   0,   0,   0, 139, 137, 367, 138,
   0, 140,   0,   0,   0,   0,   0,   0, 148,   0,
   0,   0,   0, 142, 144, 145, 146, 143,   0,   0,
   0,   0, 149,   0, 150,   0, 141, 147,   0,   0,
   0, 139, 137, 366, 138,   0, 140,   0,   0,   0,
   0,   0,   0, 148,   0,   0,   0,   0, 142, 144,
 145, 146, 143,   0,   0,   0,   0, 149,   0, 150,
   0, 141, 147,   0,   0,   0, 139, 137, 365, 138,
   0, 140,   0,   0,   0,   0,   0,   0, 148,   0,
   0,   0,   0, 142, 144, 145, 146, 143,   0,   0,
   0,   0, 149,   0, 150,   0, 141, 147,   0,   0,
 364, 139, 137,   0, 138,   0, 140,   0,   0,   0,
   0,   0,   0, 148,   0,   0,   0,   0, 142, 144,
 145, 146, 143,   0,   0,   0,   0, 149,   0, 150,
   0, 141, 147,   0,   0,   0, 139, 137, 363, 138,
   0, 140,   0,   0,   0,   0,   0,   0, 148,   0,
   0,   0,   0, 142, 144, 145, 146, 143,   0,   0,
   0,   0, 149,   0, 150,   0, 141, 147,   0,   0,
 362, 139, 137,   0, 138,   0, 140,   0,   0,   0,
   0,   0,   0, 148,   0,   0,   0,   0, 142, 144,
 145, 146, 143,   0,   0,   0,   0, 149,   0, 150,
   0, 141, 147,   0,   0, 361, 139, 137,   0, 138,
   0, 140,   0,   0,   0,   0,   0,   0, 148,   0,
   0,   0,   0, 142, 144, 145, 146, 143,   0,   0,
   0,   0, 149,   0, 150,   0, 141, 147,   0,   0,
 360, 139, 137,   0, 138,   0, 140,   0,   0,   0,
   0,   0,   0, 148,   0,   0,   0,   0, 142, 144,
 145, 146, 143,   0,   0,   0,   0, 149,   0, 150,
   0, 141, 147,   0,   0, 359, 139, 137,   0, 138,
   0, 140,   0,   0,   0,   0,   0,   0, 148,   0,
   0,   0,   0, 142, 144, 145, 146, 143,   0,   0,
   0,   0, 149,   0, 150,   0, 141, 147,   0,   0,
 358, 139, 137,   0, 138,   0, 140,   0,   0,   0,
   0,   0,   0, 148,   0,   0,   0,   0, 142, 144,
 145, 146, 143,   0,   0,   0,   0, 149,   0, 150,
   0, 141, 147,   0,   0, 357, 139, 137,   0, 138,
   0, 140,   0,   0,   0,   0,   0,   0, 148,   0,
   0,   0,   0, 142, 144, 145, 146, 143,   0,   0,
   0,   0, 149,   0, 150,   0, 141, 147,   0,   0,
 356, 139, 137,   0, 138,   0, 140,   0,   0,   0,
   0,   0,   0, 148,   0,   0,   0,   0, 142, 144,
 145, 146, 143,   0,   0,   0,   0, 149,   0, 150,
   0, 141, 147,   0,   0,   0, 139, 137, 355, 138,
   0, 140,   0,   0,   0,   0,   0,   0, 148,   0,
   0,   0,   0, 142, 144, 145, 146, 143,   0,   0,
   0,   0, 149,   0, 150,   0, 141, 147,   0,   0,
   0, 139, 137, 354, 138,   0, 140,   0,   0,   0,
   0,   0,   0, 148,   0,   0,   0,   0, 142, 144,
 145, 146, 143,   0,   0,   0,   0, 149,   0, 150,
   0, 141, 147,   0,   0,   0, 139, 137, 353, 138,
   0, 140,   0,   0,   0,   0,   0,   0, 148,   0,
   0,   0,   0, 142, 144, 145, 146, 143,   0,   0,
   0,   0, 149,   0, 150,   0, 141, 147,   0,   0,
 352, 139, 137,   0, 138,   0, 140,   0,   0,   0,
   0,   0,   0, 148,   0,   0,   0,   0, 142, 144,
 145, 146, 143,   0,   0,   0,   0, 149,   0, 150,
   0, 141, 147,   0,   0, 351, 139, 137,   0, 138,
   0, 140,   0,   0,   0,   0,   0,   0, 148,   0,
   0,   0,   0, 142, 144, 145, 146, 143,   0,   0,
   0,   0, 149,   0, 150,   0, 141, 147,   0,   0,
 350, 139, 137,   0, 138,   0, 140,   0,   0,   0,
   0,   0,   0, 148,   0,   0,   0,   0, 142, 144,
 145, 146, 143,   0,   0,   0,   0, 149,   0, 150,
   0, 141, 147,   0,   0, 349, 139, 137,   0, 138,
   0, 140,   0,   0,   0,   0,   0,   0, 148,   0,
   0,   0,   0, 142, 144, 145, 146, 143,   0,   0,
   0,   0, 149,   0, 150,   0, 141, 147,   0,   0,
 348, 139, 137,   0, 138,   0, 140,   0,   0,   0,
   0,   0,   0, 148,   0,   0,   0,   0, 142, 144,
 145, 146, 143,   0,   0,   0,   0, 149,   0, 150,
   0, 141, 147,   0,   0, 347, 139, 137,   0, 138,
   0, 140,   0,   0,   0,   0,   0,   0, 148,   0,
   0,   0,   0, 142, 144, 145, 146, 143,   0,   0,
   0,   0, 149,   0, 150,   0, 141, 147,   0,   0,
   0, 139, 137, 346, 138,   0, 140,   0,   0,   0,
   0,   0,   0, 148,   0,   0,   0,   0, 142, 144,
 145, 146, 143,   0,   0,   0,   0, 149,   0, 150,
   0, 141, 147,   0,   0, 345, 139, 137,   0, 138,
   0, 140,   0,   0,   0,   0,   0,   0, 148,   0,
   0,   0,   0, 142, 144, 145, 146, 143,   0,   0,
   0,   0, 149,   0, 150,   0, 141, 147,   0,   0,
 344, 139, 137,   0, 138,   0, 140,   0,   0,   0,
   0,   0,   0, 148,   0,   0,   0,   0, 142, 144,
 145, 146, 143,   0,   0,   0,   0, 149,   0, 150,
   0, 141, 147,   0,   0,   0, 139, 137, 343, 138,
   0, 140,   0,   0,   0,   0,   0,   0, 148,   0,
   0,   0,   0, 142, 144, 145, 146, 143,   0,   0,
   0,   0, 149,   0, 150,   0, 141, 147,   0,   0,
 342, 139, 137,   0, 138,   0, 140,   0,   0,   0,
   0,   0,   0, 148,   0,   0,   0,   0, 142, 144,
 145, 146, 143,   0,   0,   0,   0, 149,   0, 150,
   0, 141, 147,   0,   0, 341, 139, 137,   0, 138,
   0, 140,   0,   0,   0,   0,   0,   0, 148,   0,
   0,   0,   0, 142, 144, 145, 146, 143,   0,   0,
   0,   0, 149,   0, 150,   0, 141, 147,   0,   0,
 340, 139, 137,   0, 138,   0, 140,   0,   0,   0,
   0,   0,   0, 148,   0,   0,   0,   0, 142, 144,
 145, 146, 143,   0,   0,   0,   0, 149,   0, 150,
   0, 141, 147,   0,   0, 339, 139, 137,   0, 138,
   0, 140,   0,   0,   0,   0,   0,   0, 148,   0,
   0,   0,   0, 142, 144, 145, 146, 143,   0,   0,
   0,   0, 149,   0, 150,   0, 141, 147,   0,   0,
 338, 139, 137,   0, 138,   0, 140,   0,   0,   0,
   0,   0,   0, 148,   0,   0,   0,   0, 142, 144,
 145, 146, 143,   0,   0,   0,   0, 149,   0, 150,
   0, 141, 147,   0,   0,   0, 139, 137, 337, 138,
   0, 140,   0,   0,   0,   0,   0,   0, 148,   0,
   0,   0,   0, 142, 144, 145, 146, 143,   0,   0,
   0,   0, 149,   0, 150,   0, 141, 147,   0,   0,
 336, 139, 137,   0, 138,   0, 140,   0,   0,   0,
   0,   0,   0, 148,   0,   0,   0,   0, 142, 144,
 145, 146, 143,   0,   0,   0,   0, 149,   0, 150,
   0, 141, 147,   0,   0, 335, 139, 137,   0, 138,
   0, 140,   0,   0,   0,   0,   0,   0, 148,   0,
   0,   0,   0, 142, 144, 145, 146, 143,   0,   0,
   0,   0, 149,   0, 150,   0, 141, 147,   0,   0,
 334, 139, 137,   0, 138,   0, 140,   0,   0,   0,
   0,   0,   0, 148,   0,   0,   0,   0, 142, 144,
 145, 146, 143,   0,   0,   0,   0, 149,   0, 150,
   0, 141, 147,   0,   0,   0, 139, 137, 333, 138,
   0, 140,   0,   0,   0,   0,   0,   0, 148,   0,
   0,   0,   0, 142, 144, 145, 146, 143,   0,   0,
   0,   0, 149,   0, 150,   0, 141, 147,   0,   0,
   0, 139, 137, 331, 138,   0, 140,   0,   0,   0,
   0,   0,   0, 148,   0,   0,   0,   0, 142, 144,
 145, 146, 143,   0,   0,   0,   0, 149,   0, 150,
   0, 141, 147,   0,   0,   0, 139, 137,   0, 138,
   0, 140,   0,   0,   0,   0,   0,   0, 148,   0,
   0,   0, 273, 142, 144, 145, 146, 143,   0,   0,
  51,   0, 149,   0, 150,   0, 141, 147,   0,   0,
 271, 139, 137,   0, 138,   0, 140,   0,   0,   0,
  58,   0,   0, 148,   0,   0,   0,   0, 142, 144,
 145, 146, 143,   0,   0,   0,   0, 149,   0, 150,
   0, 141, 147,   0,   0,   0, 139, 137,   0, 138,
   0, 140,  79,  80,   0,   0,  81,  82, 148,   0,
   0,   0,   0, 142, 144, 145, 146, 143, 149,   0,
 150,   0, 141, 147,   0,   0,   0, 139, 137,   0,
 138, 125, 140,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0, 148,   0, 144, 145, 146, 142,   0,
   0,   0,   0, 149,   0, 150,   0, 141, 147,   0,
   0,   0, 139, 137,   0, 138,   0, 140,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0, 148, 142,
 144, 145, 146, 149,   0, 150,   0, 141,   0,   0,
   0,   0, 139, 137,   0, 138,   0, 140,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0, 148,
 144, 145, 146,   0, 142,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0, 142 };
short yypact[]={

 210,-1000,  29,  29,  29,  29,-195,  50,  50,  50,
  50,  50,  50,-196,-210,  29,  29,  29, 170, 129,
  50,  29,-1000,-1000,  69,-1000,  53,-1000,  59,-200,
-1000,  68,  67,  66,  38,-1000,-1000,-1000,-1000,-1000,
  29,  29,  29,  76,  73,-1000,-1000,  29,  53,-1000,
-1000,  56,  56,-1000,-1000,  56,  56,-1000,-1000,-1000,
  29,-1000,  41, 242,  18,-1000,-155,  37, 242, 242,
 242,-202,-156,-1000,-1000,-1000,-203,-158,-1000,-1000,
  56,-1000,-1000,-1000,  53,-1000,-1000,-1000,-282,-284,
-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,  64,  58,
2474,-1000,-1000, 242,-285, 242, 242, 242,-1000,-1000,
-1000,-1000, 242, 242,-1000,-1000,-1000,-160,2474,2474,
2474,-161,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,
-1000,-1000,-1000,-1000,-1000,-171,-192, 242, 242, 242,
 242, 242, 242, 242, 276, 242, 160, 242, 242,  57,
 242,-1000, 282, 278, 270, 268, 265, 258, 257, 254,
 253, 251, 248, 244, 243, 240, 237, 236, 232, 190,
 189, 188, 187, 186, 183, 178, 177, 174, 172, 169,
 168, 167, 164, 162, 161, 159,-1000, 158, 157, 155,
 154, 151, 149, 148, 146, 145, 144, 130,2439,-1000,
-1000,-1000,-1000,-1000,-179,-1000,-1000,-1000,-1000,-1000,
  70,  70,  11,  11,  11,-1000,2404, 126, 242, 126,
 126, 242,2570,2540, 242,  70,  29,  29,  29,  29,
 147, 147, 242, 242, 242, 242, 242, 242, 242, 242,
 242, 242, 242, 242, 242, 242, 242, 242, 242, 242,
 242, 242, 242, 242, 242, 242, 242, 242, 242, 242,
 242, 242, 242, 242, 242, 242, 242, 242, 242, 242,
 242,-1000,-1000, 242, 126, 126, 126, 141, 140, 125,
 121, 119,2369,  53, 115,2334,2299,2264,2229,2194,
2159,2124,2089,2054,2019,1984,1949,1914,1879,1844,
1809,1774,1739,1704,1669,1634,1599,1564,1529,1494,
1459,1424,1389,1354,1319,1284,1249,1214,1179,1144,
1109,1074,1039,1004, 969,2505,-1000,-1000,-1000,-1000,
-1000, 242,-1000, 242,-1000,-1000,-1000, 242,-1000,-1000,
-1000,-1000,-1000, 242,-1000,-1000, 242,-1000,-1000,-1000,
-1000,-1000,-1000, 242, 242, 242,-1000,-1000,-1000,-1000,
-1000,-1000,-1000, 242,-1000, 242,  29,  29,  29, 242,
 242, 242, 242, 135,2474, 134, 934, 899, 864, 829,
 794, 757, 746, 711, 109,  95,  94, 672, 661, 626,
 588,-1000, 242,-1000,-1000,-1000,-1000, 242, 242, 242,
-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000, 242,2474,
 553, 542, 505, 469,-1000,-1000,-1000,-1000 };
short yypgo[]={

   0,  72,2470, 116, 104, 348,   0, 216,  17, 326,
 325, 210 };
short yyr1[]={

   0,   9,   9,   9,   9,   9,   9,   9,   9,   9,
   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,
   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,
   9,   9,   9,   9,   9,   9,   9,   9,   7,   7,
   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
   7,   7,   7,   7,   7,   7,   7,   6,   6,   6,
   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
   6,   6,   6,   6,   8,   8,   3,   3,   1,   1,
   1,   1,   1,   4,   4,   2,   2,   2,   2,   5,
   5,  10,  10,  11,  11,  11,  11,  11,  11,  11,
  11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
  11,  11,  11,  11,  11,  11,  11 };
short yyr2[]={

   0,   4,   4,   4,   4,   6,   4,   2,   2,   2,
   3,   2,   3,   2,   3,   2,   4,   4,   2,   2,
   3,   1,   2,   1,   2,   3,   4,   2,   2,   2,
   1,   2,   3,   3,   2,   2,   0,   1,   1,   2,
   5,   5,   5,   5,   5,   7,   5,   7,   5,   5,
   5,   7,   5,   5,   5,   5,   5,   7,   5,   5,
   7,   5,   5,   5,   5,   5,   5,   9,   9,   9,
   5,   5,   5,   5,   5,   5,   2,   5,   7,   5,
   7,   7,   7,   7,   7,   7,   7,   9,   3,   2,
   2,   1,   1,   1,   1,   2,   2,   3,   3,   3,
   3,   3,   3,   1,   5,   3,   3,   3,   3,   3,
   4,   4,   4,   3,   1,   3,   3,   1,   2,   3,
   3,   4,   1,   1,   1,   1,   1,   2,   2,   1,
   1,   0,   2,   1,   1,   2,   2,   2,   2,   1,
   1,   1,   1,   2,   1,   2,   1,   2,   1,   2,
   1,   2,   3,   3,   3,   3,   3 };
short yychk[]={

-1000,  -9, 271, 265, 266, 267, 264, 268, 270, 282,
 269, 272, 273, 275, 283, 274, 276, 281, 277, 278,
 279, 280, 284, 256,  -4,  -3,  -1, 260, 263,  36,
 261,  -4,  -4,  -4, 263,  -5, 257,  -1,  -5,  -5,
  -5,  -5,  -5, 263, 258, 263, 258,  -3,  -1,  -4,
  -4,  -2,  -4, 258, 259,  45,  43,  -4,  -2, 257,
  -5,  -4, -10,  61,  58, 258,  36, 263,  61,  61,
  61,  58, 258,  -3,  -3,  -3,  58,  58,  -4,  -2,
  -2,  -2,  -2,  -3,  -1, -11, 333, 334, 126,  33,
 336, 335, 337, 339, 340, 341, 342, 343, 338, 344,
  -6,  -7,  -1, 285,  64,  40,  43,  45, 258, 259,
 302, 257, 126,  33,  -1, 258, 258,  36,  -6,  -6,
  -6, 263, 258, 263, 258,  -2, 333, 334, 333, 334,
 339, 340, 341, 342, 343,  61,  61,  43,  45,  42,
  47,  37,  94,  63,  60,  61,  62,  38, 124,  33,
  35,  -7, 286, 287, 288, 289, 309, 310, 290, 291,
 292, 293, 294, 295, 296, 297, 298, 299, 300, 301,
 303, 304, 305, 306, 307, 308, 311, 312, 313, 314,
 315, 316, 317, 318, 319, 320, 321, 325, 326, 322,
 323, 331, 330, 332, 327, 328, 329, 324,  -6,  -7,
  -7,  -7,  -7, 258, 258, 258, 258, 345, 346, 347,
  -6,  -6,  -6,  -6,  -6,  -6,  -6,  -6,  61,  -6,
  -6,  61,  -6,  -6,  61,  -6,  40,  40,  40,  40,
  40,  40,  40,  40,  40,  40,  40,  40,  40,  40,
  40,  40,  40,  40,  40,  40,  40,  40,  40,  40,
  40,  40,  40,  40,  40,  40,  40,  40,  40,  40,
  40,  40,  40,  40,  40,  40,  40,  40,  40,  40,
  40,  41, 258,  58,  -6,  -6,  -6,  -4,  -4,  -4,
  -4,  -4,  -6,  -1,  -4,  -6,  -6,  -6,  -6,  -6,
  -6,  -6,  -6,  -6,  -6,  -6,  -6,  -6,  -6,  -6,
  -6,  -6,  -6,  -6,  -6,  -6,  -6,  -6,  -6,  -6,
  -6,  -6,  -6,  -6,  -6,  -6,  -6,  -6,  -6,  -6,
  -6,  -6,  -6,  -6,  -6,  -6,  41,  41,  41,  41,
  41,  44,  41,  44,  41,  41,  41,  44,  41,  41,
  41,  41,  41,  44,  41,  41,  44,  41,  41,  41,
  41,  41,  41,  44,  44,  44,  41,  41,  41,  41,
  41,  41,  41,  44,  41,  44,  44,  44,  44,  44,
  44,  44,  44,  -8,  -6,  -8,  -6,  -6,  -6,  -6,
  -6,  -6,  -6,  -6,  -4,  -4,  -4,  -6,  -6,  -6,
  -6,  41,  44,  41,  41,  41,  41,  44,  44,  44,
  41,  41,  41,  41,  41,  41,  41,  41,  44,  -6,
  -6,  -6,  -6,  -6,  41,  41,  41,  41 };
short yydef[]={

  36,  -2,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,  21,  23,   0,  30,
   0,   0, 131,  37,   0, 123, 124, 117,   0,   0,
 122,   0,   0,   0,   0,   7, 129, 130,   8,   9,
  11,  13,  15,   0,   0,  18,  19,   0,   0,  22,
  24,   0,   0, 125, 126,   0,   0,  27,  28,  29,
  31,  34,  35,   0,   0, 118,   0,   0,   0,   0,
   0,   0,   0,  10,  12,  14,   0,   0,  20,  25,
   0, 127, 128,  32,  33, 132, 133, 134,   0,   0,
 139, 140, 141, 142, 144, 146, 148, 150,   0,   0,
   1, 103,  38,   0,   0,   0,   0,   0,  91,  92,
  93,  94,   0,   0, 116, 120, 119,   0,   2,   3,
   4,   0,   6,  16,  17,  26, 135, 136, 137, 138,
 143, 145, 147, 149, 151,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,  39,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,  76,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,  89,
  90,  95,  96, 121,   0, 152, 153, 154, 155, 156,
  97,  98,  99, 100, 101, 102,   0,  -2,   0,  -2,
  -2,   0, 108, 109,   0, 113,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,  88,   5,   0,  -2,  -2,  -2,   0,   0,   0,
   0,   0,   0,  -2,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0, 104,  40,  41,  42,  43,
  44,   0,  46,   0,  48,  49,  50,   0,  52,  53,
  54,  55,  56,   0,  58,  59,   0,  61,  62,  63,
  64,  65,  66,   0,   0,   0,  70,  71,  72,  73,
  74,  75,  77,   0,  79,   0,   0,   0,   0,   0,
   0,   0,   0,   0, 114,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,  45,   0,  47,  51,  57,  60,   0,   0,   0,
  78,  80,  81,  82,  83,  84,  85,  86,   0, 115,
   0,   0,   0,   0,  67,  68,  69,  87 };
#ifndef lint
static char yaccpar_sccsid[] = "@(#)yaccpar	4.1	(Berkeley)	2/11/83";
#endif not lint

#
# define YYFLAG -1000
# define YYERROR goto yyerrlab
# define YYACCEPT return(0)
# define YYABORT return(1)

/*	parser for yacc output	*/

#ifdef YYDEBUG
int yydebug = 0; /* 1 for debugging */
#endif
YYSTYPE yyv[YYMAXDEPTH]; /* where the values are stored */
int yychar = -1; /* current input token number */
int yynerrs = 0;  /* number of errors */
short yyerrflag = 0;  /* error recovery flag */

yyparse() {

	short yys[YYMAXDEPTH];
	short yyj, yym;
	register YYSTYPE *yypvt;
	register short yystate, *yyps, yyn;
	register YYSTYPE *yypv;
	register short *yyxi;

	yystate = 0;
	yychar = -1;
	yynerrs = 0;
	yyerrflag = 0;
	yyps= &yys[-1];
	yypv= &yyv[-1];

 yystack:    /* put a state and value onto the stack */

#ifdef YYDEBUG
	if( yydebug  ) printf( "state %d, char 0%o\n", yystate, yychar );
#endif
		if( ++yyps> &yys[YYMAXDEPTH] ) { yyerror( "yacc stack overflow" ); return(1); }
		*yyps = yystate;
		++yypv;
		*yypv = yyval;

 yynewstate:

	yyn = yypact[yystate];

	if( yyn<= YYFLAG ) goto yydefault; /* simple state */

	if( yychar<0 ) if( (yychar=yylex())<0 ) yychar=0;
	if( (yyn += yychar)<0 || yyn >= YYLAST ) goto yydefault;

	if( yychk[ yyn=yyact[ yyn ] ] == yychar ){ /* valid shift */
		yychar = -1;
		yyval = yylval;
		yystate = yyn;
		if( yyerrflag > 0 ) --yyerrflag;
		goto yystack;
		}

 yydefault:
	/* default state action */

	if( (yyn=yydef[yystate]) == -2 ) {
		if( yychar<0 ) if( (yychar=yylex())<0 ) yychar = 0;
		/* look through exception table */

		for( yyxi=yyexca; (*yyxi!= (-1)) || (yyxi[1]!=yystate) ; yyxi += 2 ) ; /* VOID */

		while( *(yyxi+=2) >= 0 ){
			if( *yyxi == yychar ) break;
			}
		if( (yyn = yyxi[1]) < 0 ) return(0);   /* accept */
		}

	if( yyn == 0 ){ /* error */
		/* error ... attempt to resume parsing */

		switch( yyerrflag ){

		case 0:   /* brand new error */

			yyerror( "syntax error" );
		yyerrlab:
			++yynerrs;

		case 1:
		case 2: /* incompletely recovered error ... try again */

			yyerrflag = 3;

			/* find a state where "error" is a legal shift action */

			while ( yyps >= yys ) {
			   yyn = yypact[*yyps] + YYERRCODE;
			   if( yyn>= 0 && yyn < YYLAST && yychk[yyact[yyn]] == YYERRCODE ){
			      yystate = yyact[yyn];  /* simulate a shift of "error" */
			      goto yystack;
			      }
			   yyn = yypact[*yyps];

			   /* the current yyps has no shift onn "error", pop stack */

#ifdef YYDEBUG
			   if( yydebug ) printf( "error recovery pops state %d, uncovers %d\n", *yyps, yyps[-1] );
#endif
			   --yyps;
			   --yypv;
			   }

			/* there is no state on the stack with an error shift ... abort */

	yyabort:
			return(1);


		case 3:  /* no shift yet; clobber input char */

#ifdef YYDEBUG
			if( yydebug ) printf( "error recovery discards char %d\n", yychar );
#endif

			if( yychar == 0 ) goto yyabort; /* don't discard EOF, quit */
			yychar = -1;
			goto yynewstate;   /* try again in the same state */

			}

		}

	/* reduction by production yyn */

#ifdef YYDEBUG
		if( yydebug ) printf("reduce %d\n",yyn);
#endif
		yyps -= yyr2[yyn];
		yypvt = yypv;
		yypv -= yyr2[yyn];
		yyval = yypv[1];
		yym=yyn;
			/* consult goto table to find next state */
		yyn = yyr1[yyn];
		yyj = yypgo[yyn] + *yyps + 1;
		if( yyj>=YYLAST || yychk[ yystate = yyact[yyj] ] != -yyn ) yystate = yyact[yypgo[yyn]];
		switch(yym){
			
case 1:
# line 146 "gram.y"
{ let(yypvt[-2].rval.left.vp, yypvt[-0].enode); } break;
case 2:
# line 148 "gram.y"
{ slet(yypvt[-2].rval.left.vp, yypvt[-0].enode, 0); } break;
case 3:
# line 150 "gram.y"
{ slet(yypvt[-2].rval.left.vp, yypvt[-0].enode, -1); } break;
case 4:
# line 152 "gram.y"
{ slet(yypvt[-2].rval.left.vp, yypvt[-0].enode, 1); } break;
case 5:
# line 154 "gram.y"
{ doformat(yypvt[-4].ival,yypvt[-2].ival,yypvt[-1].ival,yypvt[-0].ival); } break;
case 6:
# line 156 "gram.y"
{ doformat(yypvt[-2].ival,yypvt[-2].ival,yypvt[-1].ival,yypvt[-0].ival); } break;
case 7:
# line 157 "gram.y"
{  /* This tmp hack is because readfile
				    * recurses back through yyparse. */
				  char *tmp;
				  tmp = yypvt[-0].sval;
				  readfile (tmp, 1);
				  xfree(tmp);
				} break;
case 8:
# line 164 "gram.y"
{
				  char *tmp;
				  tmp = yypvt[-0].sval;
				  readfile (tmp, 0);
				  xfree(tmp);
				} break;
case 9:
# line 171 "gram.y"
{ if (mdir) xfree(mdir); mdir = yypvt[-0].sval; } break;
case 10:
# line 173 "gram.y"
{ (void) writefile(yypvt[-1].sval, (yypvt[-0].rval.left.vp)->row, 
			 	(yypvt[-0].rval.left.vp)->col, (yypvt[-0].rval.right.vp)->row,
			 	(yypvt[-0].rval.right.vp)->col);
			 	xfree(yypvt[-1].sval); } break;
case 11:
# line 178 "gram.y"
{ (void) writefile (yypvt[-0].sval, 0, 0, maxrow, maxcol);
			 	xfree(yypvt[-0].sval); } break;
case 12:
# line 180 "gram.y"
{ (void) printfile(yypvt[-1].sval, (yypvt[-0].rval.left.vp)->row, 
			 (yypvt[-0].rval.left.vp)->col, (yypvt[-0].rval.right.vp)->row,
			 (yypvt[-0].rval.right.vp)->col);
			 xfree(yypvt[-1].sval); } break;
case 13:
# line 184 "gram.y"
{ (void) printfile (yypvt[-0].sval, 0, 0, maxrow, maxcol);
			 xfree(yypvt[-0].sval); } break;
case 14:
# line 186 "gram.y"
{ (void) tblprintfile(yypvt[-1].sval, (yypvt[-0].rval.left.vp)->row, 
			 (yypvt[-0].rval.left.vp)->col, (yypvt[-0].rval.right.vp)->row,
			 (yypvt[-0].rval.right.vp)->col);
			 xfree(yypvt[-1].sval); } break;
case 15:
# line 190 "gram.y"
{ (void)tblprintfile (yypvt[-0].sval, 0, 0, maxrow, maxcol);
			 xfree(yypvt[-0].sval); } break;
case 16:
# line 193 "gram.y"
{ showcol( yypvt[-2].ival, yypvt[-0].ival); } break;
case 17:
# line 195 "gram.y"
{ showrow( yypvt[-2].ival, yypvt[-0].ival); } break;
case 18:
# line 197 "gram.y"
{ hide_col( yypvt[-0].ival ); } break;
case 19:
# line 199 "gram.y"
{ hide_row( yypvt[-0].ival ); } break;
case 20:
# line 201 "gram.y"
{ copy(yypvt[-1].rval.left.vp,yypvt[-1].rval.right.vp,
					yypvt[-0].rval.left.vp,yypvt[-0].rval.right.vp); } break;
case 21:
# line 204 "gram.y"
{ eraser(lookat(showsr, showsc),
				        lookat(currow, curcol)); } break;
case 22:
# line 207 "gram.y"
{ eraser(yypvt[-0].rval.left.vp, yypvt[-0].rval.right.vp); } break;
case 23:
# line 208 "gram.y"
{ valueize_area(showsr, showsc, currow, curcol);
				 modflg++; } break;
case 24:
# line 210 "gram.y"
{ valueize_area((yypvt[-0].rval.left.vp)->row,
				(yypvt[-0].rval.left.vp)->col,
				(yypvt[-0].rval.right.vp)->row,
				(yypvt[-0].rval.right.vp)->col); modflg++; } break;
case 25:
# line 214 "gram.y"
{ fill(lookat(showsr, showsc),
				      lookat(currow, curcol), yypvt[-1].fval, yypvt[-0].fval); } break;
case 26:
# line 217 "gram.y"
{ fill(yypvt[-2].rval.left.vp, yypvt[-2].rval.right.vp, yypvt[-1].fval, yypvt[-0].fval); } break;
case 27:
# line 218 "gram.y"
{moveto(yypvt[-0].rval.left.vp->row, yypvt[-0].rval.left.vp->col);} break;
case 28:
# line 219 "gram.y"
{num_search(yypvt[-0].fval);} break;
case 29:
# line 220 "gram.y"
{str_search(yypvt[-0].sval);} break;
case 30:
# line 221 "gram.y"
{go_last();} break;
case 31:
# line 222 "gram.y"
{ struct ent_ptr arg1, arg2;
					arg1.vp = lookat(showsr, showsc);
					arg1.vf = 0;
					arg2.vp = lookat(currow, curcol);
					arg2.vf = 0;
					add_range(yypvt[-0].sval, arg1, arg2, 1); } break;
case 32:
# line 229 "gram.y"
{ add_range(yypvt[-1].sval, yypvt[-0].rval.left, yypvt[-0].rval.right, 1); } break;
case 33:
# line 230 "gram.y"
{ add_range(yypvt[-1].sval, yypvt[-0].ent, yypvt[-0].ent, 0); } break;
case 34:
# line 231 "gram.y"
{ del_range(yypvt[-0].rval.left.vp, yypvt[-0].rval.right.vp); } break;
case 38:
# line 236 "gram.y"
{ yyval.enode = new_var('v', yypvt[-0].ent); } break;
case 39:
# line 237 "gram.y"
{ yyval.enode = new_type ('f', ENULL, yypvt[-0].enode); } break;
case 40:
# line 239 "gram.y"
{ yyval.enode = new_range(REDUCE | '+', yypvt[-1].rval); } break;
case 41:
# line 241 "gram.y"
{ yyval.enode = new_range (REDUCE | '*', yypvt[-1].rval); } break;
case 42:
# line 243 "gram.y"
{ yyval.enode = new_range (REDUCE | 'a', yypvt[-1].rval); } break;
case 43:
# line 245 "gram.y"
{ yyval.enode = new_range (REDUCE | 's', yypvt[-1].rval); } break;
case 44:
# line 247 "gram.y"
{ yyval.enode = new_range (REDUCE | MAX, yypvt[-1].rval); } break;
case 45:
# line 249 "gram.y"
{ yyval.enode = new(LMAX, yypvt[-1].enode, yypvt[-3].enode); } break;
case 46:
# line 251 "gram.y"
{ yyval.enode = new_range (REDUCE | MIN, yypvt[-1].rval); } break;
case 47:
# line 253 "gram.y"
{ yyval.enode = new(LMIN, yypvt[-1].enode, yypvt[-3].enode); } break;
case 48:
# line 255 "gram.y"
{ yyval.enode = new(ACOS, ENULL, yypvt[-1].enode); } break;
case 49:
# line 256 "gram.y"
{ yyval.enode = new(ASIN, ENULL, yypvt[-1].enode); } break;
case 50:
# line 257 "gram.y"
{ yyval.enode = new(ATAN, ENULL, yypvt[-1].enode); } break;
case 51:
# line 258 "gram.y"
{ yyval.enode = new(ATAN2, yypvt[-3].enode, yypvt[-1].enode); } break;
case 52:
# line 259 "gram.y"
{ yyval.enode = new(CEIL, ENULL, yypvt[-1].enode); } break;
case 53:
# line 260 "gram.y"
{ yyval.enode = new(COS, ENULL, yypvt[-1].enode); } break;
case 54:
# line 261 "gram.y"
{ yyval.enode = new(EXP, ENULL, yypvt[-1].enode); } break;
case 55:
# line 262 "gram.y"
{ yyval.enode = new(FABS, ENULL, yypvt[-1].enode); } break;
case 56:
# line 263 "gram.y"
{ yyval.enode = new(FLOOR, ENULL, yypvt[-1].enode); } break;
case 57:
# line 264 "gram.y"
{ yyval.enode = new(HYPOT, yypvt[-3].enode, yypvt[-1].enode); } break;
case 58:
# line 265 "gram.y"
{ yyval.enode = new(LOG, ENULL, yypvt[-1].enode); } break;
case 59:
# line 266 "gram.y"
{ yyval.enode = new(LOG10, ENULL, yypvt[-1].enode); } break;
case 60:
# line 267 "gram.y"
{ yyval.enode = new(POW, yypvt[-3].enode, yypvt[-1].enode); } break;
case 61:
# line 268 "gram.y"
{ yyval.enode = new(SIN, ENULL, yypvt[-1].enode); } break;
case 62:
# line 269 "gram.y"
{ yyval.enode = new(SQRT, ENULL, yypvt[-1].enode); } break;
case 63:
# line 270 "gram.y"
{ yyval.enode = new(TAN, ENULL, yypvt[-1].enode); } break;
case 64:
# line 271 "gram.y"
{ yyval.enode = new(DTR, ENULL, yypvt[-1].enode); } break;
case 65:
# line 272 "gram.y"
{ yyval.enode = new(RTD, ENULL, yypvt[-1].enode); } break;
case 66:
# line 273 "gram.y"
{ yyval.enode = new(RND, ENULL, yypvt[-1].enode); } break;
case 67:
# line 275 "gram.y"
{ yyval.enode = new(PV,  yypvt[-5].enode,new(':',yypvt[-3].enode,yypvt[-1].enode)); } break;
case 68:
# line 276 "gram.y"
{ yyval.enode = new(FV,  yypvt[-5].enode,new(':',yypvt[-3].enode,yypvt[-1].enode)); } break;
case 69:
# line 277 "gram.y"
{ yyval.enode = new(PMT, yypvt[-5].enode,new(':',yypvt[-3].enode,yypvt[-1].enode)); } break;
case 70:
# line 279 "gram.y"
{ yyval.enode = new(HOUR,ENULL, yypvt[-1].enode); } break;
case 71:
# line 280 "gram.y"
{ yyval.enode = new(MINUTE,ENULL, yypvt[-1].enode); } break;
case 72:
# line 281 "gram.y"
{ yyval.enode = new(SECOND,ENULL, yypvt[-1].enode); } break;
case 73:
# line 282 "gram.y"
{ yyval.enode = new(MONTH,ENULL,yypvt[-1].enode); } break;
case 74:
# line 283 "gram.y"
{ yyval.enode = new(DAY, ENULL, yypvt[-1].enode); } break;
case 75:
# line 284 "gram.y"
{ yyval.enode = new(YEAR, ENULL, yypvt[-1].enode); } break;
case 76:
# line 285 "gram.y"
{ yyval.enode = new(NOW, ENULL, ENULL);} break;
case 77:
# line 286 "gram.y"
{ yyval.enode = new(STON, ENULL, yypvt[-1].enode); } break;
case 78:
# line 287 "gram.y"
{ yyval.enode = new_type (EQS, yypvt[-3].enode, yypvt[-1].enode); } break;
case 79:
# line 288 "gram.y"
{ yyval.enode = new(DATE, ENULL, yypvt[-1].enode); } break;
case 80:
# line 289 "gram.y"
{ yyval.enode = new(FMT, yypvt[-3].enode, yypvt[-1].enode); } break;
case 81:
# line 291 "gram.y"
{ yyval.enode = new(INDEX, yypvt[-3].enode, new_range(REDUCE | INDEX, yypvt[-1].rval)); } break;
case 82:
# line 293 "gram.y"
{ yyval.enode = new(LOOKUP, yypvt[-3].enode, new_range(REDUCE | LOOKUP, yypvt[-1].rval)); } break;
case 83:
# line 295 "gram.y"
{ yyval.enode = new(STINDEX, yypvt[-3].enode, new_range(REDUCE | STINDEX, yypvt[-1].rval)); } break;
case 84:
# line 296 "gram.y"
{ yyval.enode = new(EXT, yypvt[-3].enode, yypvt[-1].enode); } break;
case 85:
# line 297 "gram.y"
{ yyval.enode = new(NVAL, yypvt[-3].enode, yypvt[-1].enode); } break;
case 86:
# line 298 "gram.y"
{ yyval.enode = new(SVAL, yypvt[-3].enode, yypvt[-1].enode); } break;
case 87:
# line 300 "gram.y"
{ yyval.enode = new(SUBSTR, yypvt[-5].enode, new(',', yypvt[-3].enode, yypvt[-1].enode)); } break;
case 88:
# line 301 "gram.y"
{ yyval.enode = yypvt[-1].enode; } break;
case 89:
# line 302 "gram.y"
{ yyval.enode = yypvt[-0].enode; } break;
case 90:
# line 303 "gram.y"
{ yyval.enode = new_type ('m', ENULL, yypvt[-0].enode); } break;
case 91:
# line 304 "gram.y"
{ yyval.enode = new_const('k', (double) yypvt[-0].ival); } break;
case 92:
# line 305 "gram.y"
{ yyval.enode = new_const('k', yypvt[-0].fval); } break;
case 93:
# line 306 "gram.y"
{ yyval.enode = new_const('k', (double)3.14159265358979323846); } break;
case 94:
# line 307 "gram.y"
{ yyval.enode = new_str(yypvt[-0].sval); } break;
case 95:
# line 308 "gram.y"
{ yyval.enode = new_type ('~', ENULL, yypvt[-0].enode); } break;
case 96:
# line 309 "gram.y"
{ yyval.enode = new_type ('~', ENULL, yypvt[-0].enode); } break;
case 97:
# line 312 "gram.y"
{ yyval.enode = new_type ('+', yypvt[-2].enode, yypvt[-0].enode); } break;
case 98:
# line 313 "gram.y"
{ yyval.enode = new_type ('-', yypvt[-2].enode, yypvt[-0].enode); } break;
case 99:
# line 314 "gram.y"
{ yyval.enode = new_type ('*', yypvt[-2].enode, yypvt[-0].enode); } break;
case 100:
# line 315 "gram.y"
{ yyval.enode = new_type ('/', yypvt[-2].enode, yypvt[-0].enode); } break;
case 101:
# line 316 "gram.y"
{ yyval.enode = new_type ('%', yypvt[-2].enode, yypvt[-0].enode); } break;
case 102:
# line 317 "gram.y"
{ yyval.enode = new_type ('^', yypvt[-2].enode, yypvt[-0].enode); } break;
case 104:
# line 319 "gram.y"
{ yyval.enode = new_type ('?', yypvt[-4].enode, new(':', yypvt[-2].enode, yypvt[-0].enode)); } break;
case 105:
# line 320 "gram.y"
{ yyval.enode = new_type ('<', yypvt[-2].enode, yypvt[-0].enode); } break;
case 106:
# line 321 "gram.y"
{ yyval.enode = new_type ('=', yypvt[-2].enode, yypvt[-0].enode); } break;
case 107:
# line 322 "gram.y"
{ yyval.enode = new_type ('>', yypvt[-2].enode, yypvt[-0].enode); } break;
case 108:
# line 323 "gram.y"
{ yyval.enode = new_type ('&', yypvt[-2].enode, yypvt[-0].enode); } break;
case 109:
# line 324 "gram.y"
{ yyval.enode = new_type ('|', yypvt[-2].enode, yypvt[-0].enode); } break;
case 110:
# line 325 "gram.y"
{ yyval.enode = new_type ('~', ENULL, new_type ('>', yypvt[-3].enode, yypvt[-0].enode)); } break;
case 111:
# line 326 "gram.y"
{ yyval.enode = new_type ('~', ENULL, new_type ('=', yypvt[-3].enode, yypvt[-0].enode)); } break;
case 112:
# line 327 "gram.y"
{ yyval.enode = new_type ('~', ENULL, new_type ('<', yypvt[-3].enode, yypvt[-0].enode)); } break;
case 113:
# line 328 "gram.y"
{ yyval.enode = new_type ('#', yypvt[-2].enode, yypvt[-0].enode); } break;
case 114:
# line 331 "gram.y"
{ yyval.enode = new(ELIST, ENULL, yypvt[-0].enode); } break;
case 115:
# line 332 "gram.y"
{ yyval.enode = new(ELIST, yypvt[-2].enode, yypvt[-0].enode); } break;
case 116:
# line 335 "gram.y"
{ yyval.rval.left = yypvt[-2].ent; yyval.rval.right = yypvt[-0].ent; } break;
case 117:
# line 336 "gram.y"
{ yyval.rval = yypvt[-0].rval; } break;
case 118:
# line 339 "gram.y"
{ yyval.ent.vp = lookat(yypvt[-0].ival , yypvt[-1].ival); yyval.ent.vf = 0;} break;
case 119:
# line 340 "gram.y"
{ yyval.ent.vp = lookat(yypvt[-0].ival , yypvt[-1].ival);
					yyval.ent.vf = FIX_COL;} break;
case 120:
# line 342 "gram.y"
{ yyval.ent.vp = lookat(yypvt[-0].ival , yypvt[-2].ival);
					yyval.ent.vf = FIX_ROW;} break;
case 121:
# line 344 "gram.y"
{ yyval.ent.vp = lookat(yypvt[-0].ival , yypvt[-2].ival);
					yyval.ent.vf = FIX_ROW | FIX_COL;} break;
case 122:
# line 346 "gram.y"
{ yyval.ent = yypvt[-0].rval.left; } break;
case 123:
# line 349 "gram.y"
{ yyval.rval = yypvt[-0].rval; } break;
case 124:
# line 350 "gram.y"
{ yyval.rval.left = yypvt[-0].ent; yyval.rval.right = yypvt[-0].ent; } break;
case 125:
# line 353 "gram.y"
{ yyval.fval = (double) yypvt[-0].ival; } break;
case 126:
# line 354 "gram.y"
{ yyval.fval = yypvt[-0].fval; } break;
case 127:
# line 355 "gram.y"
{ yyval.fval = -yypvt[-0].fval; } break;
case 128:
# line 356 "gram.y"
{ yyval.fval = yypvt[-0].fval; } break;
case 129:
# line 359 "gram.y"
{ yyval.sval = yypvt[-0].sval; } break;
case 130:
# line 360 "gram.y"
{
				    char *s, *s1;
				    s1 = yypvt[-0].ent.vp->label;
				    if (!s1)
					s1 = "NULL_STRING";
				    s = xmalloc((unsigned)strlen(s1)+1);
				    (void) strcpy(s, s1);
				    yyval.sval = s;
				} break;
case 133:
# line 375 "gram.y"
{ setauto(1); } break;
case 134:
# line 376 "gram.y"
{ setauto(1); } break;
case 135:
# line 377 "gram.y"
{ setauto(0); } break;
case 136:
# line 378 "gram.y"
{ setauto(0); } break;
case 137:
# line 379 "gram.y"
{ setauto(0); } break;
case 138:
# line 380 "gram.y"
{ setauto(0); } break;
case 139:
# line 381 "gram.y"
{ setorder(BYCOLS); } break;
case 140:
# line 382 "gram.y"
{ setorder(BYROWS); } break;
case 141:
# line 383 "gram.y"
{ setorder(BYGRAPH); } break;
case 142:
# line 384 "gram.y"
{ numeric = 1; } break;
case 143:
# line 385 "gram.y"
{ numeric = 0; } break;
case 144:
# line 386 "gram.y"
{ prescale = 0.01; } break;
case 145:
# line 387 "gram.y"
{ prescale = 1.0; } break;
case 146:
# line 388 "gram.y"
{ extfunc = 1; } break;
case 147:
# line 389 "gram.y"
{ extfunc = 0; } break;
case 148:
# line 390 "gram.y"
{ showcell = 1; } break;
case 149:
# line 391 "gram.y"
{ showcell = 0; } break;
case 150:
# line 392 "gram.y"
{ showtop = 1; } break;
case 151:
# line 393 "gram.y"
{ showtop = 0; } break;
case 152:
# line 394 "gram.y"
{ setiterations(yypvt[-0].ival); } break;
case 153:
# line 395 "gram.y"
{ tbl_style = yypvt[-0].ival; } break;
case 154:
# line 396 "gram.y"
{ tbl_style = TBL; } break;
case 155:
# line 397 "gram.y"
{ tbl_style = LATEX; } break;
case 156:
# line 398 "gram.y"
{ tbl_style = TEX; } break;
		}
		goto yystack;  /* stack new state and value */

	}
