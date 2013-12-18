/*
 * Ascii command to print out the ascii table in dec/hex/ascii
 *
 * PAB 19/7/88
 */

static char *rcsid = "$Header: /hsrc/cmds/com/RCS/ascii.c,v 1.4 1990/08/23 09:45:55 james Exp $";

#include <stdio.h>

int main()
{
printf("\
 00 $00 nul ^@   01 $01 soh ^A   02 $02 stx ^B   03 $03 etx ^C   04 $04 eot ^D\n");
printf("\
 05 $05 enq ^E   06 $06 ack ^F   07 $07 bel ^G   08 $08 bs  ^H   09 $09 tab ^I\n");
printf("\
 10 $0a nl  ^J   11 $0b vt  ^K   12 $0c np  ^L   13 $0d cr  ^M   14 $0e so  ^N\n");
printf("\
 15 $0f si  ^O   16 $10 dle ^P   17 $11 dc1 ^Q   18 $12 dc2 ^R   19 $13 dc3 ^S\n");
printf("\
 20 $14 dc4 ^T   21 $15 nak ^U   22 $16 syn ^V   23 $17 etb ^W   24 $18 can ^X\n");
printf("\
 25 $19 em  ^Y   26 $1a sub ^Z   27 $1b esc ^[   28 $1c fs  ^\\   29 $1d gs  ^]\n");
printf("\
 30 $1e rs  ^^   31 $1f us  ^_   32 $20 sp      127 $7f del\n");
printf("\n");
printf("\
 32 $20       33 $21  !    34 $22  \"    35 $23  #    36 $24  $    37 $25  %%\n");
printf("\
 38 $26  &    39 $27  '    40 $28  (    41 $29  )    42 $2a  *    43 $2b  +\n");
printf("\
 44 $2c  ,    45 $2d  -    46 $2e  .    47 $2f  /    48 $30  0    49 $31  1\n");
printf("\
 50 $32  2    51 $33  3    52 $34  4    53 $35  5    54 $36  6    55 $37  7\n");
printf("\
 56 $38  8    57 $39  9    58 $3a  :    59 $3b  ;    60 $3c  <    61 $3d  =\n");
printf("\
 62 $3e  >    63 $3f  ?    64 $40  @    65 $41  A    66 $42  B    67 $43  C\n");
printf("\
 68 $44  D    69 $45  E    70 $46  F    71 $47  G    72 $48  H    73 $49  I\n");
printf("\
 74 $4a  J    75 $4b  K    76 $4c  L    77 $4d  M    78 $4e  N    79 $4f  O\n");
printf("\
 80 $50  P    81 $51  Q    82 $52  R    83 $53  S    84 $54  T    85 $55  U\n");
printf("\
 86 $56  V    87 $57  W    88 $58  X    89 $59  Y    90 $5a  Z    91 $5b  [\n");
printf("\
 92 $5c  \\    93 $5d  ]    94 $5e  ^    95 $5f  _    96 $60  `    97 $61  a\n");
printf("\
 98 $62  b    99 $63  c   100 $64  d   101 $65  e   102 $66  f   103 $67  g\n");
printf("\
104 $68  h   105 $69  i   106 $6a  j   107 $6b  k   108 $6c  l   109 $6d  m\n");
printf("\
110 $6e  n   111 $6f  o   112 $70  p   113 $71  q   114 $72  r   115 $73  s\n");
printf("\
116 $74  t   117 $75  u   118 $76  v   119 $77  w   120 $78  x   121 $79  y\n");
printf("\
122 $7a  z   123 $7b  {   124 $7c  |   125 $7d  }   126 $7e  ~   127 $7f  del\n");

return 0;
}
