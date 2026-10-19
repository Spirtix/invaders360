#include "stdafx.h"
#include "cpu.h"

cpu::cpu(bus& bus) : m_bus(bus) {
	i8080_init(&m_cpu);
    m_cpu.userdata = this;
	m_cpu.read_byte = &cpu::readByte;
	m_cpu.write_byte = &cpu::writeByte;
	m_cpu.port_in = &cpu::portIn;
	m_cpu.port_out = &cpu::portOut;
}

void cpu::run(uint32_t cycles) {
    m_cpu.cyc = 0;
    while (m_cpu.cyc < cycles) {
	    i8080_step(&m_cpu);
    }
}

void cpu::interrupt(uint8_t opcode) {
    i8080_interrupt(&m_cpu, opcode);
}

uint8_t cpu::readByte(void* data, uint16_t address) {
    return ((cpu*)data)->m_bus.readByte(address);
}

void cpu::writeByte(void* data, uint16_t address, uint8_t value) {
    ((cpu*)data)->m_bus.writeByte(address, value);
}

uint8_t cpu::portIn(void* data, uint8_t port) {
    return ((cpu*)data)->m_bus.portIn(port);
}

void cpu::portOut(void* data, uint8_t port, uint8_t value) {
    ((cpu*)data)->m_bus.portOut(port, value);
}
