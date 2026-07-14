#include "cpu.hpp"

CPU::CPU(Memory &mem,uint16_t pc,uint16_t sp) : memory(mem), pc(pc), sp(sp), 
stackBottom(sp), zFlag(0), stopped(0){
    registers.fill(0);
}

CPU::CPU(Memory &mem) : memory(mem), pc(0), sp(0),
stackBottom(0), zFlag(0), stopped(1){
    registers.fill(0);
}

void CPU::startProgram(uint16_t pc,uint16_t sp){
    if (stopped == 0){
        std::cout << "error: cpu is already running\n";
        return;
    }

    this->pc = pc;
    this->sp = sp;
    stackBottom = sp;
    zFlag = 0;
    stopped = 0;
    registers.fill(0);
}

uint16_t CPU::getRuntimeValue(int addressingMethod, uint16_t value) const{
    if (addressingMethod == (1 << INSTANT_ADDRESSING)){
      return value;
    } else if (addressingMethod == (1 << DIRECT_ADDRESSING)){
        return memory.read(value);
    } else if (addressingMethod == (1 << INDIRECT_REGISTER_ADDRESSING)){
        return memory.read(registers[value]);
    } else if (addressingMethod == (1 << DIRECT_REGISTER_ADDRESSING)){
        return registers[value];
    }

    return 0;
}

void CPU::writeToTarget(int targetAddressingMethod, uint16_t targetVal, uint16_t writeVal){
    if (targetAddressingMethod == (1 << DIRECT_ADDRESSING)){
        memory.write(targetVal,writeVal);
    } else if (targetAddressingMethod == (1 << INDIRECT_REGISTER_ADDRESSING)){
        memory.write(registers[targetVal],writeVal);
    } else if (targetAddressingMethod == (1 << DIRECT_REGISTER_ADDRESSING)){
        registers[targetVal] = writeVal;
    }
}

void CPU::executeInstruction(DecodedInstruction di, uint16_t sourceVal, uint16_t targetVal){
    uint16_t rs = getRuntimeValue(di.sourceAddressingMethod,sourceVal);
    uint16_t rt = getRuntimeValue(di.targetAddressingMethod,targetVal);
    int16_t result;

    
    //std::cout << di.operation.code << "\n";
    /*std::cout << "source: " << di.sourceAddressingMethod << "\n";
    std::cout << "target: " << di.targetAddressingMethod << "\n";
    std::cout << "source val: " << sourceVal << "\n";
    std::cout << "target val: " << targetVal << "\n"; */

    switch(di.operation.code){
        case 0: // mov
            writeToTarget(di.targetAddressingMethod,targetVal,rs);
            break;
        case 1: // cmp
            zFlag = (rs == rt) ? 1 : 0;
            break;
        case 2: // add
            result = ((int16_t) rt) + ((int16_t) rs);
            writeToTarget(di.targetAddressingMethod,targetVal, (uint16_t) result);
            break;
        case 3: // sub
            result = ((int16_t) rt) - ((int16_t) rs);
            writeToTarget(di.targetAddressingMethod,targetVal, (uint16_t) result);
            break;
        case 4: // lea
            writeToTarget(di.targetAddressingMethod,targetVal,sourceVal);
            break;
        case 5: // clr
            writeToTarget(di.targetAddressingMethod,targetVal,0);
            break;
        case 6: // not
            writeToTarget(di.targetAddressingMethod,targetVal,~rt);
            break;
        case 7: // inc
            result = ((int16_t) rt) + 1;
            writeToTarget(di.targetAddressingMethod,targetVal, (uint16_t) result);
            break;
        case 8: // dec
            result = ((int16_t) rt) - 1;
            writeToTarget(di.targetAddressingMethod,targetVal, (uint16_t) result);
            break;
        case 9: // jmp
            pc = targetVal;
            break;
        case 10: // bne
            if (zFlag == 0){pc = targetVal;}
            break;
        case 11: // red
            writeToTarget(di.targetAddressingMethod,targetVal, getchar());
            break;
        case 12: // prn
            //std::cout << "rt: " << rt << "\n";
            printf("%c",rt);
            break;
        case 13: // jsr
            //std::cout << "address: " << targetVal << "\n";
            memory.write(sp++,pc);
            pc = targetVal;
            break;
        case 14: // rts
            pc = memory.read(--sp);
            break;
        case 15: // stop
            stopped = 1;
            break;
        case 16: // push
            memory.write(sp++,rt);
            break;
        case 17: // pop
            writeToTarget(di.targetAddressingMethod,targetVal,memory.read(--sp));
            break;
        case 18: // be
            if (zFlag != 0){pc = targetVal;}
            break;
        case 19: // mod
            result = ((int16_t) rt) % ((int16_t) rs);
            writeToTarget(di.targetAddressingMethod,targetVal, (uint16_t) result);
            break;
        case 20: // mul
            result = ((int16_t) rt) * ((int16_t) rs);
            writeToTarget(di.targetAddressingMethod,targetVal, (uint16_t) result);
            break;
        case 21: // div
            result = ((int16_t) rt) / ((int16_t) rs);
            writeToTarget(di.targetAddressingMethod,targetVal, (uint16_t) result);
            break;
    }


}

void CPU::executeNextInstruction(){

    if (stopped == 1){return;}

    uint16_t nextInstruction = fetch(pc++);
    DecodedInstruction decodedInstruction = decodeInstruction(nextInstruction);
    uint16_t sourceValue;
    uint16_t targetValue;

    //std::cout << "pc: " << pc << "\n";

    if (isRegisterAddressing(decodedInstruction.sourceAddressingMethod) && isRegisterAddressing(decodedInstruction.targetAddressingMethod)){
        // shared word for registers
        uint16_t registerInstruction = fetch(pc++);
        sourceValue = getSourceValue(decodedInstruction.sourceAddressingMethod,registerInstruction);
        targetValue = getTargetValue(decodedInstruction.targetAddressingMethod,registerInstruction);
    } else {
        if (decodedInstruction.sourceAddressingMethod != 0){
            uint16_t sourceInstruction = fetch(pc++);
            sourceValue = getSourceValue(decodedInstruction.sourceAddressingMethod,sourceInstruction);
        }
        if (decodedInstruction.targetAddressingMethod != 0){
            uint16_t targetInstruction = fetch(pc++);
            targetValue = getTargetValue(decodedInstruction.targetAddressingMethod,targetInstruction);
        }
    }

    executeInstruction(decodedInstruction,sourceValue,targetValue);



}
