#ifndef EMULATOR_H
#define EMULATOR_H

#include <iostream>

#include "memory.hpp"
#include "cpu.hpp"
#include "../common/constants.h"


typedef struct _Program {
    uint16_t programStartAddress;
    uint16_t programEndAddress;
    uint16_t codeStartAddress;
    uint16_t codeEndAddress;
    uint16_t entryAddress;
    uint16_t stackBottomAddress;
} Program;


class Emulator {
public:

    Emulator();

    CPU &getCPU();
    Memory &getMemory();

    Program loadProgram(const std::string &fileName); // returns the entry address of the program
    void startProgram(Program program);

private:
    Memory memory;
    CPU cpu;
};

#endif