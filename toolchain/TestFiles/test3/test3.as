.data 1,2,3
mov r1,r2
.string "hello"

macr testMacro
prn macroString
endmacr

testMacro
macroString: .string "macro"
testMacro

jmp func

test1: .entry macroString
test2: .extern func
