#include "decoder.hpp"

bool isRegisterAddressing(int addressingMethod){
    return addressingMethod == (1 << DIRECT_REGISTER_ADDRESSING) || addressingMethod == (1 << INDIRECT_REGISTER_ADDRESSING);
}

DecodedInstruction decodeInstruction(uint16_t instructionWord){
    DecodedInstruction decodedInstruction;
    Operation *operationsTable;

    int fourBitMask = 0xF;

    int operationCode = instructionWord >> OP_CODE_START;
    int legalOperationsCount = getOperationsTable(&operationsTable);

    if (operationCode < 0 || operationCode >= legalOperationsCount){
        std::cout << "error: invalid operation code " << operationCode << "\n";
        exit(1);
    }

    decodedInstruction.operation = operationsTable[operationCode];
    decodedInstruction.sourceAddressingMethod = (instructionWord >> SOURCE_METHOD_START) & fourBitMask;
    decodedInstruction.targetAddressingMethod = (instructionWord >> TARGET_METHOD_START) & fourBitMask;

    return decodedInstruction;
}

uint16_t getSourceValue(int sourceAddressingMethod, uint16_t instruction){
    if (sourceAddressingMethod == (1 << INSTANT_ADDRESSING)){
        int16_t signedInstruction = (int16_t) instruction;
        return (uint16_t) (signedInstruction >>= (ARE_END + 1));
    } else if (sourceAddressingMethod == (1 << DIRECT_ADDRESSING)){
        return (instruction >> (ARE_END + 1));
    }
    int bitMask = ((1 << REGISTER_BITS) - 1);
    return (instruction >> (ARE_END + REGISTER_BITS + 1)) & bitMask;
}

uint16_t getTargetValue(int sourceAddressingMethod, uint16_t instruction){
    if (sourceAddressingMethod == (1 << INSTANT_ADDRESSING)){
        int16_t signedInstruction = (int16_t) instruction;
        return (uint16_t) (signedInstruction >>= (ARE_END + 1));
    } else if (sourceAddressingMethod == (1 << DIRECT_ADDRESSING)){
        return (instruction >> (ARE_END + 1));
    }
    int bitMask = ((1 << REGISTER_BITS) - 1);
    return (instruction >> (ARE_END + 1)) & bitMask;
}