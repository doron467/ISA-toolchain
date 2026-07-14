#ifndef MEMORY_HPP
#define MEMORY_HPP

#define MEMORY_SIZE 8192
#define MEMORY_START_ADDRESS 100

#include <array>
#include <cstdint>

class Memory {
public:
    Memory();
    uint16_t read(uint16_t address) const;
    void write(uint16_t address, uint16_t value);

    uint16_t getLoadingAddress() const;


private:
    std::array<uint16_t, MEMORY_SIZE> memoryArray;
};

#endif