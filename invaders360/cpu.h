#pragma once
#include "stdint.h"
#include "bus.h"
#include <i8080.h>

class cpu {
public:
    cpu(bus& bus);
    void run(uint32_t cycles);
    void interrupt(uint8_t opcode);
private:
    bus& m_bus;
    i8080 m_cpu;

    static uint8_t readByte(void* data, uint16_t address);
    static void writeByte(void* data, uint16_t address, uint8_t value);
    static uint8_t portIn(void* data, uint8_t port);
    static void portOut(void* data, uint8_t port, uint8_t value);
};
