.extern board
.extern printBoard
.extern printString
.extern printChar
.extern checkWin

turn: .data 1
turns: .data 9
prompt1: .string "Current turn:"
prompt2: .string "Enter insertion spot: "
prompt3: .string "Please enter a valid spot."
prompt4: .string "The winner is: "
prompt5: .string "Game has ended in a draw"
newLine: .data 10
asciiZero: .data 48
freeSpace: .data 0

changeTurns: inc turn
cmp turn, #3
bne changeReturn
mov #1,turn
changeReturn: rts


.entry MAIN
MAIN: jsr printBoard
lea prompt1, r7
jsr printString
lea freeSpace, r7
mov turn, *r7
jsr printChar
prn newLine
lea prompt2, r7
jsr printString
red r1
sub asciiZero,r1
; read \n from the buffer to clear it
red r7
lea board,r2
add r2,r1
cmp *r1, #0
bne requestAgain
mov turn,*r1
jsr checkWin
cmp r0, #0
bne declareWinner
jsr changeTurns
dec turns
cmp turns, #0
bne MAIN
jsr declareDraw

requestAgain: lea prompt3,r7
jsr printString
prn newLine
jmp MAIN

declareWinner: jsr printBoard 
lea prompt4,r7
jsr printString
lea freeSpace,r7
mov turn,*r7
jsr printChar
prn newLine
stop

declareDraw: jsr printBoard 
lea prompt5,r7
jsr printString
prn newLine
stop



