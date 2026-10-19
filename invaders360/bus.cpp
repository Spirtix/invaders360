#include "stdafx.h"
#include "bus.h"

bus::bus() {
    port1 = 0;
    port2 = 0;
    offset = 0;
    msb = 0;
    lsb = 0;
    memset(memory, 0, 0x4000);
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
	if (port == 3) {
		USHORT shift = (msb << 8) | lsb;
		return (shift >> (8 - offset)) & 0xFF;
	}
	if (port == 1)
		return port1;
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
	}
}

uint8_t* bus::getMemoryPtr(uint16_t address) {
    return memory + address;
}

void bus::setInputMask(uint8_t mask) {
    port1 = mask;
}
