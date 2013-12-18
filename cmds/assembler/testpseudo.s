// pseudo opcode test file
//
// P.A.Beskeen Sept '91

/* block comment */

startofmod:


// GHOF pseudo op tests:

module 1

ref fred, john , harry
export fred, john , harry, mark, delila
codetable fred, john , harry
codetable fred

// standard Helios assembler pseudo ops test:

byte 1, 2, 3 ,4, 5
byte '1', '2', '3' ,'4', '5'
byte "123456"

short 1-1+1, 1+1, 2+1, 2*2, 2*2+1

word 0x1, 0b10, 0b11, 0b0000100, 0x0005
word "abcdefghijk qwertyuiop"
blkb 3, 12, 12, 12
