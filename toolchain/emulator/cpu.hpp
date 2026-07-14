#ifndef CPU_HPP
#define CPU_HPP

#include "memory.hpp"
#include "decoder.hpp"

#include <cstdint>
#include <array>
#include <stdlib.h>


#define REGISTER_COUNT 8

class CPU {
public:
    CPU(Memory &mem);
    CPU(Memory &mem,uint16_t pc,uint16_t sp);

    uint16_t fetch(uint16_t address) const {
        return memory.read(address);
    }

    void executeNextInstruction();

    void startProgram(uint16_t pc,uint16_t sp);

    uint isStopped() const {
        return stopped;
    }

    uint16_t getRegister(int registerNumber) const{
        if (registerNumber < 0 || registerNumber >= registers.size()){
            std::cout << "error: invalid get register call\n";
            exit(1);
        }
        return registers[registerNumber];
    }

    void setRegister(int registerNumber,uint16_t value){
        if (registerNumber < 0 || registerNumber >= registers.size()){
            std::cout << "error: invalid set register call\n";
            exit(1);
        }
        registers[registerNumber] = value;
    }

    uint16_t getPc() const{
        return pc;
    }

    uint16_t getSp() const{
        return sp;
    }

    void setPc(uint16_t value){
        pc = value;
    }

    void setSp(uint16_t value){
        sp = value;
    }



private:
    uint16_t pc; // program counter
    uint16_t sp; // stack pointer
    uint zFlag: 1;
    uint stopped: 1; // 0 until the instruction stop is executed
    uint16_t stackBottom; // pointer to the bottom of the stack (last element out)
    std::array<uint16_t, REGISTER_COUNT> registers;
    Memory &memory;

    void executeInstruction(DecodedInstruction di, uint16_t sourceVal, uint16_t targetVal);
    uint16_t getRuntimeValue(int addressingMethod, uint16_t value) const;
    void writeToTarget(int targetAddressingMethod, uint16_t targetValue, uint16_t writeValue);

};

#endif