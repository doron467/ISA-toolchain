#include "emulator.hpp"

int main(int argc,char **argv){
    if (argc != 2){
        std::cout << "error: invalid emulator call\n";
        exit(1);
    }

    Emulator emulator;
    Program program = emulator.loadProgram(argv[1]);
    emulator.startProgram(program);
    CPU &cpu = emulator.getCPU();
    
    while (cpu.isStopped() == 0){
        cpu.executeNextInstruction();
    }

    std::cout << "emulator stopped\n";

    return 0;
}