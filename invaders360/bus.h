#pragma once
#include "stdint.h"

class bus {
public:
    bus();
    uint8_t readByte(uint16_t address);
    void writeByte(uint16_t address, uint8_t value);
    uint8_t portIn(uint8_t port);
    void portOut(uint8_t port, uint8_t value);
    uint8_t* getMemoryPtr(uint16_t address);
    size_t getMemorySize() { return memSize; }
    void setInputMask(uint8_t mask);
private:
    static const size_t memSize = 0x4000;
    uint8_t memory[memSize];
    uint8_t port1, port2, offset, msb, lsb;
};
