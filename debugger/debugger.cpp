#include "../emulator/emulator.hpp"
#include "../emulator/decoder.hpp"
#include "../common/constants.h"
#include "../common/operations.h"
#include "input_parser.hpp"

#include <iomanip>
#include <unordered_set>

void printCPU(const CPU &cpu){
    for (int i = 0; i < REGISTER_COUNT; i++){
        uint16_t value = cpu.getRegister(i);
        printf("r%d: %04x %d\n",i,value,value);
    }
    printf("pc: %04x %d\n",cpu.getPc(),cpu.getPc());
    printf("sp: %04x %d\n",cpu.getSp(),cpu.getSp());
}

void printInstructionArgument(int addressingMethod, uint16_t value){
    if (addressingMethod == (1 << INSTANT_ADDRESSING)){
        int16_t signedVal = (int16_t) value;
        std::cout << "#" << signedVal;
    } else if (addressingMethod == (1 << DIRECT_ADDRESSING)){
        std::cout << "0x" << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << value;
    } else if (addressingMethod == (1 << INDIRECT_REGISTER_ADDRESSING)){
        std::cout << "*R" << value;
    } else if (addressingMethod == (1 << DIRECT_REGISTER_ADDRESSING)){
        std::cout << "R" << value;
    }
}

void printInstruction(DecodedInstruction di,uint16_t sourceVal,uint16_t targetVal){
    Operation *operationsTable;
    getOperationsTable(&operationsTable);
    std::cout << operationsTable[di.operation.code].name << " ";

    if (di.sourceAddressingMethod != 0){
        printInstructionArgument(di.sourceAddressingMethod,sourceVal);
        std::cout << " , ";
    }
    
    if (di.targetAddressingMethod != 0){
        printInstructionArgument(di.targetAddressingMethod,targetVal);
    }
    std::cout << "\n";

}

void disassemble(Program &program,CPU &cpu,uint16_t startAddress,uint instructionsCount){
    while (instructionsCount > 0 && startAddress <= program.codeEndAddress){

        std::cout << "0x" << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << startAddress << ": ";

        uint16_t instruction = cpu.fetch(startAddress++);
        DecodedInstruction di = decodeInstruction(instruction);
        uint16_t sourceValue;
        uint16_t targetValue;
        if (isRegisterAddressing(di.sourceAddressingMethod) && isRegisterAddressing(di.targetAddressingMethod)){
            uint16_t registersInstruction = cpu.fetch(startAddress++);
            sourceValue = getSourceValue(di.sourceAddressingMethod,registersInstruction);
            targetValue = getTargetValue(di.targetAddressingMethod,registersInstruction);
        } else {

            if (di.sourceAddressingMethod != 0){
                uint16_t sourceInstruction = cpu.fetch(startAddress++);
                sourceValue = getSourceValue(di.sourceAddressingMethod,sourceInstruction);
            }

            if (di.targetAddressingMethod != 0){
                uint16_t targetInstruction = cpu.fetch(startAddress++);
                targetValue = getTargetValue(di.targetAddressingMethod,targetInstruction);
            }

        }

        printInstruction(di,sourceValue,targetValue);
        instructionsCount--;
    }
}

int main(int argc,char **argv){

    if (argc != 2){
        std::cout << "error: invalid debugger call\n";
        exit(1);
    }

    Emulator emulator;
    Program program = emulator.loadProgram(argv[1]);
    CPU &cpu = emulator.getCPU();

    uint steps = 0;
    bool running = false;
    std::unordered_set<uint16_t> breakpoints;

    Command command;
    while (true){

        if (breakpoints.count(cpu.getPc()) > 0){
            running = false;
            steps = 0;
        }

        if (steps > 0 || running){
            if (steps > 0){steps--;}
            cpu.executeNextInstruction();
            continue;
        }

        command = getNextCommand();
        if (command.action == QUIT){
            break;
        }

        if (command.action == START){
            emulator.startProgram(program);
            continue;
        }

        if (command.action == RUN){
            if (cpu.isStopped()){
                emulator.startProgram(program);
            }
            running = true;
            continue;
        }

        if (command.action == STEP){
            if (cpu.isStopped()){
                std::cout << "program has already stopped\n";
                continue;
            }

            if (command.counter < 1 || command.counter > MAX_STEP){
                std::cout << "invalid step call: illegal steps count\n";
                continue;
            }

            steps = command.counter;
            continue;
        }

        if (command.action == BREAK){
            if (command.offset < program.programStartAddress | command.offset > program.programEndAddress){
                std::cout << "invalid break call: illegal address\n";
                continue;
            }
            breakpoints.insert(command.offset);
            continue;
        }

        if (command.action == DISASSEMBLE){
            if (command.counter < 1 || command.counter > MAX_STEP){
                std::cout << "invalid disassemble call: illegal instructions count\n";
                continue;
            }
            uint16_t address = (command.offset == 0) ? cpu.getPc() : (uint16_t) command.offset;
            disassemble(program,cpu,address,command.counter);
            continue;
        }

        if (command.action == CPU_INFO){
            printCPU(cpu);
            continue;
        }
    }

    std::cout << "debugger stopped\n";

}