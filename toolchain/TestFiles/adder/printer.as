zeroAscii: .data 48

.entry printNumber
printNumber: push #0
pushNumber: mov r1,r2
mod #10, r2
add zeroAscii, r2
push r2
div #10, r1
cmp r1, #0
bne pushNumber
popNumber: pop r1
cmp r1, #0
be numberExit
prn r1
jmp popNumber
newLineAscii: .data 10
numberExit: prn newLineAscii
rts

.entry printString
printString: cmp *r7, #0
be stringExit
prn *r7
inc r7
jmp printString
stringExit: prn newLineAscii
rts
