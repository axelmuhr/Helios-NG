/* -> constant/h
 * Title:               Definition of certain overall constants required by
 *                      the assembler and type definitions
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
/*---------------------------------------------------------------------------*/

#ifndef constants_h
#define constants_h

#define EOFChar         4       /* End of file, a bad character */
#define TabCh           9
#define LF              0x0A
#define FF              0x0C
#define CR              0x0D
#define Space           ' '
#define Shriek          '!'
#define Quotes          '"'
#define Hash            '#'
#define Dollar          '$'
#define LocLabRefSymbol '%'
#define Quote           '\''
#define PlusSign        '+'
#define Dash            '-'
#define MinusSign       '-'
#define Comma           ','
#define Dot             '.'
#define Colon           ':'
#define CommentSymbol   ';'
#define Equals          '='
#define AtSymbol        '@'
#define SquareBra       '['
#define SquareKet       ']'
#define CurlyBra        '{'
#define CurlyKet        '}'
#define EscapeCh        '\\'
#define Hat             '^'
#define Bar             '|'
#define UnderLine       '_'
#define Del             0x7F

typedef int          BOOLEAN ;
typedef unsigned int CARDINAL ;

#define TRUE 1
#define FALSE 0
#endif

/*---------------------------------------------------------------------------*/
/* EOF constant/h */
