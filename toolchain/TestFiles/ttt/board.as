board: .data 0,0,0,   0,0,0,   0,0,0
whitespace: .data 32
line: .data 124
newLine: .data 10
p1Char: .data 88
p2Char: .data 79
asciiZero: .data 48

.entry board
.entry printBoard
.entry printString
.entry printChar

printChar: prn whitespace 
cmp *r7, #1
be print1
cmp *r7, #2
be print2
prn r6
jmp pcr
print1: prn p1Char
jmp pcr
print2: prn p2Char
pcr: inc r6 
prn whitespace
inc r7
rts

macr printBoardLine
jsr printChar
prn line
jsr printChar
prn line
jsr printChar
prn newLine
endmacr

printBoard: lea board, r7
mov asciiZero,r6
printBoardLine
printBoardLine
printBoardLine
rts



printString: cmp *r7,#0
be stringReturn
prn *r7
inc r7
jmp printString
stringReturn: rts




