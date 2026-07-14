zeroAscii: .data 48

.entry readNumber
readNumber: mov #0, r0

readDigit: red r1
cmp r1, #10
be readExit
mul #10, r0
add r1, r0
sub zeroAscii, r0
jmp readDigit
readExit: rts
