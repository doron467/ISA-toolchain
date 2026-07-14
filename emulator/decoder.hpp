#ifndef DECODER_HPP
#define DECODER_HPP

#include "../common/operations.h"
#include "../common/constants.h"

#include <cstdint>
#include <iostream>

typedef struct _DecodedInstruction {
    Operation operation;
    int sourceAddressingMethod;
    int targetAddressingMethod;
} DecodedInstruction;

DecodedInstruction decodeInstruction(uint16_t instructionWord);
uint16_t getSourceValue(int sourceAddressingMethod, uint16_t instruction);
uint16_t getTargetValue(int sourceAddressingMethod, uint16_t instruction);
bool isRegisterAddressing(int addressingMethod);

#endif
