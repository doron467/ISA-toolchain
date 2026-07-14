#include "emulator.hpp"

Emulator::Emulator() : memory(), cpu(memory){}
CPU &Emulator::getCPU() {return cpu;}
Memory &Emulator::getMemory() {return memory;}


Program Emulator::loadProgram(const std::string &fileName){
    std::string exFileName = fileName + ".out";
    FILE *exFile = fopen(exFileName.c_str(),"r");
    if (exFile == nullptr){
        std::cout << "error: could not open file " << exFileName << "\n";
        exit(1);
    }

    int programEntryAddress;
    if (fscanf(exFile,"%d",&programEntryAddress) == EOF){
        std::cout << "error: no entry address found\n";
        exit(1);
    }

    int instructionsLength,dataLength;
    if (fscanf(exFile,"%d %d",&instructionsLength,&dataLength) == EOF){
        std::cout << "error: no instructions/data length found\n";
        exit(1);
    }

    // load program into memory
    int loadingStartAddress = memory.getLoadingAddress();
    int localAddress, machineCode;
    while (fscanf(exFile,"%d %x",&localAddress,&machineCode) != EOF){
        if (localAddress < instructionsLength){
            // relocate if necessary
            if (machineCode & (1 << RELATIVE_ADDRESSING)){
                int loadedAddress = (machineCode >> (ARE_END+1)) + loadingStartAddress;
                machineCode = (loadedAddress << (ARE_END+1)) | (1 << RELATIVE_ADDRESSING);
            }
            // load instruction into memory
            memory.write(loadingStartAddress + localAddress,machineCode);
            continue;
        }

        // data section, no need to modify anything
        memory.write(loadingStartAddress + localAddress,machineCode);
    }

    //cpu.startProgram(loadingStartAddress + programEntryAddress,loadingStartAddress + 2000);
    Program program;
    program.codeStartAddress = loadingStartAddress;
    program.codeEndAddress = loadingStartAddress + instructionsLength - 1;
    program.programStartAddress = loadingStartAddress;
    program.programEndAddress = program.programStartAddress + 2000;
    program.entryAddress = loadingStartAddress + programEntryAddress;
    program.stackBottomAddress = program.programEndAddress;
    return program;
}

void Emulator::startProgram(Program program){
    cpu.startProgram(program.entryAddress,program.stackBottomAddress);
}