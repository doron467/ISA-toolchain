; this is a valid macro
macr mac1
add r1,r2
endmacr

; these macro names are saved keywords of the assembler, so they are illegal
macr mov
inc r1
endmacr

macr data
mov r1,r2
endmacr

macr entry
mac1
stop
endmacr

; the length of this macro's name is above the max, so it is illegal
macr AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
mac1
endmacr

; this line is too long
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA

; I figured comments shouldn't have a length limit, since they get cut off in the preprocessor anyway, so this comment is still legal, despite being this long

; these macros are not declared properly
a macr b
.string "b"
endmacr

macr a b
.string "ab"
endmacr

macr c
.string "abc"
ab endmacr

macr d
.string "abcd"
endmacr ab

; just some nonsense to make sure the preprocessor ignores it
mac1
mac1
illegal operation (preprocessor should ignore anyway)
