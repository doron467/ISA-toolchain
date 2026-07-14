.extern readNumber
.extern printNumber
.extern printString

inputString: .string "enter a number: "

.entry MAIN
MAIN: lea inputString,r7
jsr printString
jsr readNumber
mov r0, r6
lea inputString,r7
jsr printString
jsr readNumber
add r0, r6

outputString: .string "the result is: "
lea outputString, r7
jsr printString
mov r6, r1
jsr printNumber
stop
