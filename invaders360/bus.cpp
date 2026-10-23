#include "stdafx.h"
#include "bus.h"

bus::bus() {
    offset = 0;
    msb = 0;
    lsb = 0;
    memset(memory, 0, 0x4000);
    memset(ports, 0, numPorts);
}

uint8_t bus::readByte(uint16_t address) {
  if (address >= 0x6000)
    return 0;
  if (address >= 0x4000)
    address -= 0x2000;

  return memory[address];
}

void bus::writeByte(uint16_t address, uint8_t value) {
	if (address >= 0x4000)
		address -= 0x2000;
	if (address >= 0x2000 && address < 0x4000) {
		memory[address] = value;
  }
}

uint8_t bus::portIn(uint8_t port) {
    switch (port) {
    case 0:
    case 1:
    case 2:
        return ports[port];
    case 3:
        uint16_t shift = (msb << 8) | lsb;
		return (shift >> (8 - offset)) & 0xFF;
        break;
    }
	return 0;
}

void bus::portOut(uint8_t port, uint8_t value) {
	switch (port) {
	case 2:
		offset = value & 7;
		break;
	case 4:
		lsb = msb;
		msb = value;
		break;
    case 3:
    case 5:
        ports[port] = value;
        break;
	}
}

uint8_t* bus::getMemoryPtr(uint16_t address) {
    return memory + address;
}

void bus::setInputMask(uint8_t mask) {
    ports[1] = mask;
}

uint8_t bus::getPortValue(uint8_t port) {
    return ports[port];
}
