#include "memory.hpp"

Memory::Memory() {
    memoryArray.fill(0);
}

uint16_t Memory::read(uint16_t address) const {
    return memoryArray[address];
}

void Memory::write(uint16_t address, uint16_t value) {
    memoryArray[address] = value;
}

uint16_t Memory::getLoadingAddress() const {
    return MEMORY_START_ADDRESS;
}