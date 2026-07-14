.extern board

spot1: .data 0
spot2: .data 0
spot3: .data 0

checkSpots: .push r7 
mov spot1,r5
mov spot2,r6
mov spot3,r7
cmp *r5,*r6
bne fail
cmp *r6,*r7
bne fail
mov *r7, r0
pop r7
rts
fail: mov #0, r0
pop r7
rts

macr callCheckSpots
jsr checkSpots
cmp r0, #0
bne returnWinner
endmacr

.entry checkWin
; check first line
checkWin: lea board,r7 
mov r7,spot1
mov spot1,spot2
inc spot2
mov spot2,spot3
inc spot3
callCheckSpots
; check second line
add #3,spot1
add #3,spot2
add #3,spot3
callCheckSpots
; check third line
add #3,spot1
add #3,spot2
add #3,spot3
callCheckSpots
; check first column
mov r7,spot1
mov spot1,spot2
add #3,spot2
mov spot2,spot3
add #3,spot3
callCheckSpots
; check second column
inc spot1
inc spot2
inc spot3
callCheckSpots
; check third column
inc spot1
inc spot2
inc spot3
callCheckSpots
; check negative diagonal
mov r7,spot1
mov spot1,spot2
add #4,spot2
mov spot1,spot3
add #8,spot3
callCheckSpots
; check positive diagonal
add #2,spot1
sub #2,spot3
callCheckSpots
returnWinner: rts

